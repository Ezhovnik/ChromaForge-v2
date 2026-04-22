#pragma once

#include <queue>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <functional>
#include <condition_variable>
#include <utility>

#include <delegates.h>
#include <debug/Logger.h>
#include <interfaces/Task.h>

namespace util {

    /**
     * @brief Вспомогательная структура для передачи результата выполнения задания из рабочего потока в основной.
     *
     * Содержит информацию о задании, условную переменную для синхронизации,
     * индекс рабочего потока, флаг блокировки и собственно результат.
     *
     * @tparam J Тип задания.
     * @tparam T Тип результата.
     */
    template<class J, class T>
    struct ThreadPoolResult {
        std::shared_ptr<J> job; ///< Указатель на выполненное задание.
        std::condition_variable& variable; ///< Условная переменная для ожидания обработки результата.
        int workerIndex; ///< Индекс рабочего потока, выполнившего задание.
        bool& locked; ///< Флаг, указывающий, что результат ещё не обработан.
        T entry; ///< Полученный результат.
    };

    /**
     * @brief Абстрактный интерфейс рабочего объекта, выполняющего конкретную обработку задания.
     *
     * Пользователь пула потоков должен реализовать этот класс, определив метод operator(),
     * который принимает задание и возвращает результат.
     *
     * @tparam T Тип задания.
     * @tparam R Тип результата.
     */
    template<class T, class R>
    class Worker {
    public:
        Worker() = default;
        virtual ~Worker() = default;

        /**
         * @brief Выполняет обработку задания.
         * @param job Задание для обработки.
         * @return Результат обработки.
         */
        virtual R operator()(const std::shared_ptr<T>&) = 0;
    };

    /**
     * @brief Пул потоков, реализующий интерфейс Task и предназначенный для параллельной обработки заданий
     *
     * Рабочие потоки создаются с использованием переданной фабрики Worker
     * Задания помещаются в очередь и распределяются по потокам
     * Результаты накапливаются и могут быть обработаны потребителем
     * Поддерживаются режимы остановки при ошибке и автономной обработки результатов
     *
     * @tparam T Тип задания
     * @tparam R Тип результата
     */
    template<class T, class R>
    class ThreadPool : public Task {
        std::string name; ///< Имя пула (для логирования)

        std::queue<std::shared_ptr<T>> jobs; ///< Очередь заданий, ожидающих выполнения
        std::queue<ThreadPoolResult<T, R>> results; ///< Очередь готовых результатов
        std::mutex resultsMutex; ///< Мьютекс для синхронизации доступа к очереди результатов

        std::vector<std::thread> threads; ///< Контейнер рабочих потоков
        std::condition_variable jobsMutexCondition; ///< Условная переменная для уведомления потоков о новых заданиях
        std::mutex jobsMutex; ///< Мьютекс для синхронизации доступа к очереди заданий

        std::vector<std::unique_lock<std::mutex>> workersBlocked; ///< Вектор блокировок для управления паузой рабочих потоков

        consumer<R&> resultConsumer; ///< Функция-потребитель для обработки готовых результатов
        consumer<std::shared_ptr<T>&> onJobFailed = nullptr; ///< Колбэк при ошибке выполнения задания
        runnable onComplete = nullptr; ///< Колбэк при завершении всех заданий

        std::atomic<int> busyWorkers = 0; ///< Количество потоков, занятых обработкой заданий
        std::atomic<uint> jobsDone = 0; ///< Счётчик выполненных заданий
        std::atomic<bool> working = true; ///< Флаг активности пула
        bool failed = false; ///< Флаг наличия ошибки
        bool standaloneResults = true; ///< Режим обработки результатов: true — без ожидания, false — с блокировкой
        bool stopOnFail = true; ///< Останавливать пул при первой ошибке

        /**
         * @brief Основной цикл рабочего потока.
         * @param index Индекс потока.
         * @param worker Умный указатель на экземпляр Worker, выполняющий обработку.
         */
        void threadLoop(int index, std::shared_ptr<Worker<T, R>> worker) {
            std::condition_variable variable;
            std::mutex mutex;
            bool locked = false;
            while (working) {
                std::shared_ptr<T> job;
                {
                    std::unique_lock<std::mutex> lock(jobsMutex);
                    jobsMutexCondition.wait(lock, [this] {
                        return !jobs.empty() || !working;
                    });
                    if (!working || failed) break;
                    job = jobs.front();
                    jobs.pop();

                    busyWorkers++;
                }
                try {
                    R result = (*worker)(job);
                    {
                        std::lock_guard<std::mutex> lock(resultsMutex);
                        results.push(ThreadPoolResult<T, R> {job, variable, index, locked, result});
                        if (!standaloneResults) locked = true;
                        busyWorkers--;
                    }
                    if (!standaloneResults){
                        std::unique_lock<std::mutex> lock(mutex);
                        variable.wait(lock, [&] {
                            return !working || !locked;
                        });
                    }
                } catch (std::exception& err) {
                    busyWorkers--;
                    if (onJobFailed) {
                        onJobFailed(job);
                    }
                    if (stopOnFail) {
                        std::lock_guard<std::mutex> lock(jobsMutex);
                        failed = true;
                    }
                    LOG_ERROR("['{}' Thread Pool] Uncaught exception: {}", name, err.what());
                }
                jobsDone++;
            }
        }
    public:
        /**
         * @brief Конструктор пула потоков.
         *
         * Создаёт количество потоков, равное аппаратной конкурентности.
         *
         * @param name Имя пула (для логов).
         * @param workersSupplier Функция-поставщик экземпляров Worker (вызывается для каждого потока).
         * @param resultConsumer Функция-потребитель результатов.
         */
        ThreadPool(
            std::string name,
            supplier<std::shared_ptr<Worker<T, R>>> workersSupplier, 
            consumer<R&> resultConsumer
        ) : name(std::move(name)), resultConsumer(resultConsumer) {
            const uint num_threads = std::thread::hardware_concurrency();
            for (uint i = 0; i < num_threads; ++i) {
                threads.emplace_back(&ThreadPool<T,R>::threadLoop, this, i, workersSupplier());
                workersBlocked.emplace_back();
            }
        }

        /**
         * @brief Деструктор. Автоматически завершает работу пула.
         */
        ~ThreadPool(){
            terminate();
        }

        /**
         * @brief Проверяет, активен ли пул.
         * @return true, если пул работает; иначе false.
         */
        bool isActive() const override {
            return working;
        }

        /**
         * @brief Принудительно завершает работу пула.
         *
         * Освобождает все ожидающие потоки, очищает очереди результатов,
         * устанавливает флаг working = false и присоединяет потоки.
         */
        void terminate() override {
            if (!working) return;
            {
                std::lock_guard<std::mutex> lock(jobsMutex);
                working = false;
            }
            {
                std::lock_guard<std::mutex> lock(resultsMutex);
                while (!results.empty()) {
                    ThreadPoolResult<T, R> entry = results.front();
                    results.pop();
                    if (!standaloneResults) {
                        entry.locked = false;
                        entry.variable.notify_all();
                    }
                }
            }

            jobsMutexCondition.notify_all();
            for (auto& thread : threads) {
                thread.join();
            }
        }

        /**
         * @brief Обновляет состояние пула: обрабатывает накопленные результаты, проверяет завершение.
         *
         * Должен вызываться периодически из основного потока.
         * Может выбросить исключение, если произошла ошибка и stopOnFail == true.
         */
        void update() override {
            if (!working) return;

            if (failed) {
                LOG_ERROR("['{}' Thread Pool] Some job failed", name);
                throw std::runtime_error("Some job failed");
            }

            bool complete = false;
            {
                std::lock_guard<std::mutex> lock(resultsMutex);
                while (!results.empty()) {
                    ThreadPoolResult<T,R> entry = results.front();
                    results.pop();

                    try {
                        resultConsumer(entry.entry);
                    } catch (std::exception& err) {
                        LOG_ERROR("['{}' Thread Pool] {}", name, err.what());
                        if (onJobFailed) onJobFailed(entry.job);
                        if (stopOnFail) {
                            std::lock_guard<std::mutex> jobsLock(jobsMutex);
                            failed = true;
                            complete = false;
                        }
                        break;
                    }

                    if (!standaloneResults) {
                        entry.locked = false;
                        entry.variable.notify_all();
                    }
                }

                if (onComplete && busyWorkers == 0) {
                    std::lock_guard<std::mutex> jobsLock(jobsMutex);
                    if (jobs.empty()) {
                        onComplete();
                        complete = true;
                    }
                }
            }
            if (failed) {
                LOG_ERROR("['{}' Thread Pool] Some job failed", name);
                throw std::runtime_error("Some job failed");
            }
            if (complete) terminate();
        }

        /**
         * @brief Добавляет задание в очередь.
         * @param job Умный указатель на задание.
         */
        void enqueueJob(const std::shared_ptr<T>& job) {
            {
                std::lock_guard<std::mutex> lock(jobsMutex);
                jobs.push(job);
            }
            jobsMutexCondition.notify_one();
        }

        /**
         * @brief Устанавливает режим автономной обработки результатов.
         *
         * Если true (по умолчанию), рабочий поток не ждёт обработки результата потребителем.
         * Если false, поток блокируется до тех пор, пока результат не будет обработан.
         *
         * @param flag Новое значение флага.
         */
        void setStandaloneResults(bool flag) {
            standaloneResults = flag;
        }

        /**
         * @brief Устанавливает поведение при ошибке выполнения задания.
         *
         * Если true (по умолчанию), пул останавливается при первом же исключении.
         *
         * @param flag Новое значение флага.
         */
        void setStopOnFail(bool flag) {
            stopOnFail = flag;
        }

        /**
         * @brief Устанавливает колбэк, вызываемый при ошибке выполнения задания.
         * @param callback Функция-потребитель, принимающая указатель на задание.
         */
        void setOnJobFailed(consumer<T&> callback) {
            this->onJobFailed = callback;
        }

        /**
         * @brief Устанавливает колбэк, вызываемый при завершении всех заданий.
         * @param callback Функция без аргументов.
         */
        void setOnComplete(runnable callback) {
            this->onComplete = callback;
        }

        /**
         * @brief Возвращает общее количество заданий (в очереди, в обработке и выполненных).
         * @return Общее количество заданий.
         */
        uint getWorkTotal() const override {
            return jobs.size() + jobsDone + busyWorkers;
        }

        /**
         * @brief Возвращает количество выполненных заданий.
         * @return Количество выполненных заданий.
         */
        uint getWorkDone() const override {
            return jobsDone;
        }

        /**
         * @brief Ожидает завершения работы пула.
         *
         * Блокирует поток, периодически вызывая update().
         */
        virtual void waitForEnd() override {
            using namespace std::chrono_literals;
            while (working) {
                std::this_thread::sleep_for(2ms);
                update();
            }
        }

        /**
         * @brief Возвращает количество рабочих потоков.
         * @return Число потоков.
         */
        uint getWorkersCount() const {
            return threads.size();
        }
    };

} // namespace util
