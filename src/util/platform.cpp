#include <util/platform.h>

#include <sstream>
#include <iomanip>
#include <time.h>
#include <algorithm>
#include <thread>

#include <typedefs.h>
#include <debug/Logger.h>
#include <util/stringutil.h>

namespace platform {
    const std::string DEFAULT_LOCALE = "en_US"; // Локаль по умолчанию, используемая, если системную определить не удалось.
}

#ifdef _WIN32
#include <Windows.h>
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
#include <Windows.h>

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

        LOG_DEBUG("Detected environment language local: {}", result);
		return result;
    } else {
        // Если не удалось определить, используем локаль по умолчанию.
        LOG_DEBUG("Detected environment language local: {}", platform::DEFAULT_LOCALE);
        return platform::DEFAULT_LOCALE;
    }
}
#else
#include <cstdlib>
#include <locale.h>

std::string platform::detect_locale() {
    // Сначала проверяем переменную LC_ALL, затем LANG.
	const char* lang = getenv("LC_ALL");
    if (!lang || *lang == '\0') lang = getenv("LANG");
    if (!lang || *lang == '\0') lang = platform::DEFAULT_LOCALE;
    std::string result(lang);

    // Отбрасываем часть с кодировкой после точки
    size_t dotPos = result.find('.');
    if (dotPos != std::string::npos) result = result.substr(0, dotPos);

    LOG_DEBUG("Detected environment language local: {}", result);
    return result;
}
#endif

#ifdef _WIN32
#pragma comment(lib, "winmm.lib")

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
    if (!std::filesystem::is_directory(folder)) return;
#ifdef __APPLE__
    auto cmd = "open " + util::quote(folder.u8string());
    system(cmd.c_str());
#elif defined(_WIN32)
    auto cmd = "start explorer " + util::quote(folder.u8string());
    ShellExecuteW(NULL, L"open", folder.wstring().c_str(), NULL, NULL, SW_SHOWDEFAULT);
#else
    auto cmd = "xdg-open " + util::quote(folder.u8string());
    system(cmd.c_str());
#endif
}
