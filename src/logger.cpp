#include "logger.hpp"

Logger::Logger() {
    auto tier0 = GetModuleHandleA("tier0.dll");
    if(tier0 != nullptr) {
        this->ColorMsg = reinterpret_cast<ColorMsg_t>(GetProcAddress(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ"));
    }

#ifndef NDEBUG
    AllocConsole();
    FILE* dummy = nullptr;
    freopen_s(&dummy, "CONOUT$", "w", stdout);

    CONSOLE_SCREEN_BUFFER_INFOEX consoleInfo = { };
    consoleInfo.cbSize = sizeof(consoleInfo);

    const auto consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfoEx(consoleHandle, &consoleInfo);
    consoleInfo.ColorTable[7] = LOG_COLOR.ToBGR();
    SetConsoleScreenBufferInfoEx(consoleHandle, &consoleInfo);
#endif // NDEBUG
}

Logger::~Logger() {
    // don't log to source console on destroy, shit would go south quick
    this->ColorMsg = nullptr;
}
