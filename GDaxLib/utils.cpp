
#include "filelogger.h"

#include <QString>
#include <QBitArray>

#include <qglobal.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

namespace Utils
{
    void EnableAnsiConsole()
    {
    #ifdef _MSC_VER
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

    QString DiffText(const QString & s1, const QString & s2)
    {
        if (s2.isEmpty())
        {
            return s2;
        }

        static const QString pattern = R"(<span>%1</span>)";

        int matchLength = 0;
        for(auto it1 = s1.begin(), it2 = s2.begin(); it2!=s2.end(); ++it1, ++it2)
        {
            if (it1==s1.end() || *it1!=*it2) break;
            ++matchLength;
        }

        if (matchLength==s2.length())
        {
            return pattern.arg(s2);
        }
        if (matchLength==0)
        {
            return s2;
        }

        QString result;
        result.append(pattern.arg(s2.leftRef(matchLength)));
        result.append(s2.midRef(matchLength));
        return result;
    }

    namespace Detail
    {
        void OnDestroyed(const char * objectName)
        {
            FileLogger::Write(static_cast<Flog::Level>(2), objectName, "destroyed");
        }
    }
}
