#include <qglobal.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

void EnableAnsiConsole()
{
#ifdef Q_OS_WIN32
    HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        throw ::GetLastError(); // ex
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        throw ::GetLastError();
    }
#endif
}
