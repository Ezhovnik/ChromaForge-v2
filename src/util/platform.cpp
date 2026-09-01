#include <util/platform.h>

#include <sstream>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <thread>

#include <typedefs.h>
#include <debug/Logger.h>
#include <util/stringutil.h>
#include <frontend/locale.h>

static debug::Logger logger("platform");

namespace platform {
    const std::string DEFAULT_LOCALE = "en_US"; // Локаль по умолчанию, используемая, если системную определить не удалось.
}

#ifdef _WIN32
    #include <Windows.h>
    #include <conio.h>
    #pragma comment(lib, "winmm.lib")
#else
    #include <sys/poll.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <stdio.h>
#endif
#ifdef __linux__
    #include <sys/prctl.h>
    #include <sys/wait.h>
#endif

namespace platform::internal {
    std::filesystem::path get_executable_path();
}

#ifdef _WIN32
#include <cstdio>

void platform::configure_encoding() {
	// Устанавливаем кодовую страницу вывода консоли на UTF-8,
    // чтобы корректно отображать символы Unicode.
	SetConsoleOutputCP(CP_UTF8);
    // Устанавливаем полную буферизацию для stdout, чтобы избежать проблем с производительностью.
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}
#else
void platform::configure_encoding() {
}
#endif

#ifdef _WIN32

std::string platform::detect_locale() {
	wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
    // Получаем имя локали пользователя по умолчанию.
    if (GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH) > 0) {
        char buffer[256];
        // Конвертируем из UTF-16 в UTF-8.
        WideCharToMultiByte(CP_UTF8, 0, localeName, -1, buffer, sizeof(buffer), nullptr, nullptr);
        std::string result(buffer);

        // Заменяем дефис на подчёркивание, чтобы получить формат "язык_СТРАНА".
		std::replace(result.begin(), result.end(), '-', '_');

        logger.debug() << "Detected environment language local: " << result;
		return result;
    } else {
        // Если не удалось определить, используем локаль по умолчанию.
        logger.debug() << "Detected environment language local: " << platform::DEFAULT_LOCALE;
        return platform::DEFAULT_LOCALE;
    }
}
#else
#include <cstdlib>

std::string platform::detect_locale() {
    // Сначала проверяем переменную LC_ALL, затем LANG.
	const char* lang = getenv("LC_ALL");
    if (!lang || *lang == '\0') lang = getenv("LANG");
    if (!lang || *lang == '\0') lang = platform::DEFAULT_LOCALE.c_str();
    std::string result(lang);

    // Отбрасываем часть с кодировкой после точки
    size_t dotPos = result.find('.');
    if (dotPos != std::string::npos) result = result.substr(0, dotPos);

    logger.debug() << "Detected environment language local: " << result;
    return result;
}
#endif

#ifdef _WIN32

void platform::sleep(size_t millis) {
    static const UINT periodMin = []{
        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        return tc.wPeriodMin;
    }();

    timeBeginPeriod(periodMin);

    Sleep(static_cast<DWORD>(millis));

    timeEndPeriod(periodMin);
}

#else
void platform::sleep(size_t millis) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

#endif

void platform::open_folder(const std::filesystem::path& folder) {
    if (!std::filesystem::is_directory(folder)) {
        logger.warning() << "'" << folder.u8string() << "' is not a directory or does not exist";
        return;
    }
#ifdef __APPLE__
    auto cmd = "open " + util::quote(folder.u8string());
    if (int res = system(cmd.c_str())) {
        logger.warning() << "'" << cmd << "' returned code " << res;
    }
#elif defined(_WIN32)
    ShellExecuteW(nullptr, L"open", folder.wstring().c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
#else
    auto cmd = "xdg-open " + util::quote(folder.u8string());
    if (int res = system(cmd.c_str())) {
        logger.warning() << "'" << cmd << "' returned code " << res;
    }
#endif
}

int platform::get_process_id() {
#ifndef _WIN32
    return getpid();
#else
    return GetCurrentProcessId();
#endif
}

bool platform::open_url(const std::string& url) {
    if (url.empty()) return false;
#ifdef __APPLE__
    auto cmd = "open " + util::quote(url);
    if (int res = system(cmd.c_str())) {
        logger.warning() << "'" << cmd << "' returned code " << res;
    } else {
        return false;
    }
#elif defined(_WIN32)
    int wlen = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return false;

    std::wstring wurl(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, &wurl[0], wlen);

    HINSTANCE result = ShellExecuteW(
        nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL
    );

    return reinterpret_cast<intptr_t>(result) > 32;
#else
    auto cmd = "xdg-open " + util::quote(url);
    if (int res = system(cmd.c_str())) {
        logger.warning() << "'" << cmd << "' returned code " << res;
    } else {
        return false;
    }
#endif
    return true;
}

std::filesystem::path platform::get_executable_path() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD result = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (result == 0) {
        DWORD error = GetLastError();
        throw std::runtime_error("GetModuleFileName failed with code: " + std::to_string(error));
    }

    int size = WideCharToMultiByte(
        CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr
    );
    if (size == 0) {
        throw std::runtime_error("Could not get executable path");
    }
    std::string str(size, 0);
    size = WideCharToMultiByte(
        CP_UTF8, 0, buffer, -1, str.data(), size, nullptr, nullptr
    );
    if (size == 0) {
        DWORD error = GetLastError();
        throw std::runtime_error("WideCharToMultiByte failed with code: " + std::to_string(error));
    }
    str.resize(size - 1);
    return std::filesystem::path(str);

#elif defined(__APPLE__)
    auto path = platform::internal::get_executable_path();
    if (path.empty()) {
        throw std::runtime_error("Could not get executable path");
    }
    return path;
#else
    char buffer[1024];
    ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (count != -1) {
        return std::filesystem::canonical(std::filesystem::path(
            std::string(buffer, static_cast<size_t>(count))
        ));
    }
    throw std::runtime_error("Could not get executable path");
#endif
}

class SystemProcess final : public Process {
public:
#ifdef __linux__
    SystemProcess(pid_t pid) : pid(pid) {
    }

    ~SystemProcess() {
        terminate();
    }
#elif defined(_WIN32)
    SystemProcess(const PROCESS_INFORMATION& pi)
        : processHandle(pi.hProcess), pid(pi.dwProcessId) {
        if (pi.hThread != nullptr) {
            CloseHandle(pi.hThread);
        }
    }

    ~SystemProcess() {
        terminate();
    }
#endif
    SystemProcess(const SystemProcess&) = delete;
    SystemProcess(SystemProcess&&) noexcept = default;
    SystemProcess& operator=(const SystemProcess&) = delete;

    SystemProcess& operator=(SystemProcess&& other) noexcept {
        if (this != &other) {
#ifdef __linux__
            pid = other.pid;
#elif defined(_WIN32)
            if (processHandle != nullptr) {
                CloseHandle(processHandle);
            }
            processHandle = other.processHandle;
            pid = other.pid;
            other.processHandle = nullptr;
            other.pid = 0;
#endif
        }
        return *this;
    }

    bool isActive() const override {
#ifdef __linux__
        if (kill(pid, 0) == 0) {
            return true;
        }
        return errno != ESRCH;
#elif defined(_WIN32)
        if (processHandle == nullptr) {
            return false;
        }
        DWORD exitCode;
        if (GetExitCodeProcess(processHandle, &exitCode)) {
            return exitCode == STILL_ACTIVE;
        }
        return false;
#endif
    }

    void update() override {}

    void waitForEnd() override {
#ifdef __linux__
        waitpid(pid, nullptr, 0);
#elif defined(_WIN32)
        if (processHandle != nullptr) {
            WaitForSingleObject(processHandle, INFINITE);
        }
#endif
    };

    void terminate() final override {
#ifdef __linux__
        kill(pid, SIGKILL);
#elif defined(_WIN32)
        if (processHandle != nullptr) {
            TerminateProcess(processHandle, 1);
            WaitForSingleObject(processHandle, INFINITE);
        }
#endif
    }
private:
#ifdef __linux__
    pid_t pid;
#elif defined(_WIN32)
    HANDLE processHandle = nullptr;
    DWORD pid = 0;
#endif
};

std::unique_ptr<Process> platform::new_engine_instance(
    const std::vector<std::string>& args,
    std::filesystem::path outputFile,
    bool subProcess
) {
    auto executable = get_executable_path();

#ifdef _WIN32
    std::stringstream ss;
    ss << util::quote(executable.u8string());
    for (int i = 0; i < args.size(); ++i) {
        ss << " " << util::quote(args[i]);
    }

    auto toWString = [](const std::string& src) -> std::wstring {
        if (src.empty()) return L"";
        int size = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, nullptr, 0);
        if (size == 0) {
            throw std::runtime_error(
                "MultiByteToWideChar failed with code: " +
                std::to_string(GetLastError())
            );
        }
        std::vector<wchar_t> buffer(size + 1);
        buffer[size] = 0;
        size = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, buffer.data(), size);
        if (size == 0) {
            throw std::runtime_error(
                "MultiByteToWideChar failed with code: " +
                std::to_string(GetLastError())
            );
        }
        return std::wstring(buffer.data(), size + 1);
    };

    auto commandString = toWString(ss.str());

    STARTUPINFOW si = { sizeof(si) };
    if (!outputFile.empty()) {
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = CreateFileW(
            toWString(outputFile.u8string()).data(),
            GENERIC_WRITE,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        if (si.hStdOutput == INVALID_HANDLE_VALUE) {
            throw std::runtime_error(
                "CreateFileW failed with code: " +
                std::to_string(GetLastError())
            );
        }
    }
    PROCESS_INFORMATION pi = { 0 };
    DWORD flags = CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS;
    // | CREATE_NO_WINDOW;
    BOOL success = CreateProcessW(
        nullptr,
        commandString.data(),
        nullptr,
        nullptr,
        FALSE,
        flags,
        nullptr,
        nullptr,
        &si,
        &pi
    );
    if (subProcess) {
        HANDLE job = CreateJobObjectW(nullptr, nullptr);

        JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
        info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

        try {
            if (!SetInformationJobObject(
                job,
                JobObjectExtendedLimitInformation,
                &info,
                sizeof(info))
            ) {
                throw std::runtime_error(
                    "SetInformationJobObject failed with code: " +
                    std::to_string(GetLastError())
                );
            }
            if (!AssignProcessToJobObject(job, pi.hProcess)) {
                throw std::runtime_error(
                    "SetInformationJobObject failed with code: " +
                    std::to_string(GetLastError())
                );
            }
            return std::make_unique<SystemProcess>(pi);
        } catch (const std::exception& err) {
            CloseHandle(job);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return nullptr;
        }
    }
    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        throw std::runtime_error(
            "starting an engine instance failed with code: " +
            std::to_string(GetLastError())
        );
    }
#else
// TODO: implement
#ifndef __APPLE__
    if (subProcess) {
        pid_t pid = fork();

        int fd = open(
            outputFile.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644
        );

        if (pid == 0) {
            prctl(PR_SET_PDEATHSIG, SIGTERM);

            if (getppid() == 1) {
                _exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);

            std::vector<const char*> argv(args.size() + 2);
            argv[0] = const_cast<char*>(executable.c_str());
            for (int i = 0; i < args.size(); ++i) {
                argv[i + 1] = args[i].c_str();
            }
            argv[args.size() + 1] = nullptr;

            prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
            execvp(executable.c_str(), const_cast<char**>(argv.data()));

            _exit(127);
        } else {
            close(fd);
            return std::make_unique<SystemProcess>(pid);
        }
    }
#endif // __APPLE_

    std::stringstream ss;
    ss << executable;
    for (int i = 0; i < args.size(); ++i) {
        ss << " " << util::quote(args[i]);
    }
    if (outputFile.empty()) {
        ss << " >/dev/null &";
    } else {
        ss << " >" << util::quote(outputFile.u8string()) << " &";
    }

    auto command = ss.str();
    if (int res = system(command.c_str())) {
        throw std::runtime_error(
            "starting an engine instance failed with code: " +
            std::to_string(res)
        );
    }
#endif // _WIN32
    return nullptr;
}

bool platform::stdin_has_data() {
#ifdef _WIN32
    return _kbhit();
#else
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;
    return poll(&fds, 1, 0) == 1;
#endif
}
