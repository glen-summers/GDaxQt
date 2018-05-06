#include <QString>
#include <QBitArray>

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

QString diffText(const QString & s1, const QString & s2)
{
    if (s2.isEmpty())
    {
        return QString();
    }

    QString const pattern = R"(<font color="%1">%2</font>)";
    QString const matchColour = "darkred";
    QString const mismatchColour = "red";

    QBitArray bits(s2.length());
    for(auto it1 = s1.begin(), it2 = s2.begin(); it2!=s2.end(); ++it1, ++it2)
    {
        bool value = it1==s1.end() || *it1!=*it2;
        bits.setBit(it2-s2.begin(), value);
    }

//    for (int i = 0; i< bits.size(); ++i)
//    {
//        qWarning() << bits[i];
//    }

    QString result;
    bool comp = bits[0];
    int len = 1;
    for (int i = 1; i< bits.size(); ++i)
    {
        if (bits[i]==comp)
        {
            ++len;
        }
        else
        {
            if (len!=0) // redunent
            {
                QStringRef sub = s2.midRef(i-len, len);
                if (comp)
                {
                    result.append(pattern.arg(mismatchColour).arg(sub));
                }
                else
                {
                    result.append(sub);
                }
                len = 1;
            }
            comp = bits[i];
        }
    }
    if (len!=0)
    {
        QStringRef sub = s2.midRef(s2.length()-len);
        if (comp)
        {
            result.append(pattern.arg(mismatchColour).arg(sub));
        }
        else
        {
            result.append(sub);
        }
    }

    return result;

//    if (s1.empty())
//    {
//        if (s2.empty)
//        {
//            return QString();
//        }
//        return QString(pattern).arg(mismatchColour, s2);
//    }
    // s1 empty, return s2 as diff

//    auto it1 = s1.begin();
//    auto it2 = s2.begin();
//    int matchCount = 0, mismatchCount = 0;
//    int current = 0;
//    for(;it1!=s1.end() && it2!=s2.end();++it1, ++it2)
//    {
//        int compare = *it1==*it2 ? 1:-1;;
//        if (compare!=current)
//        {
//            ++match;
//        }
//        else
//        {
//            ++mismatch;
//        }
//    }
}

