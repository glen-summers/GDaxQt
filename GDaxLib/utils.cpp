#include "utils.h"
#include "flogging.h"

#include <QObject>
#include <QString>
#include <QBitArray>
#include <QMutex>
#include <QCoreApplication>

#include <qglobal.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

QString Utils::DiffText(const QString & s1, const QString & s2)
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

#ifdef QT_DEBUG
namespace
{
    Flog::Log flog = Flog::LogManager::GetLog("Utils");

    QMutex mutex;
    std::unordered_map<const QObject *, std::string> objectMap;
}

void Utils::Detail::Constructed(const char * name, QObject * object)
{
    QMutexLocker lock(&mutex);
    flog.Debug("{0}:{1} constructed", object, name);
    objectMap.insert({object, name});

    QObject::connect(object, &QObject::destroyed, [object, name]()
    {
        QMutexLocker lock(&mutex);

        flog.Debug("{0}:{1} destroyed", object, name);
        objectMap.erase(object);
    });
}

void Utils::DumpObjects()
{
    // try catch latent deleteLaters
    QCoreApplication::processEvents(QEventLoop::AllEvents, 0);

    QMutexLocker lock(&mutex);
    if (objectMap.empty())
    {
        return;
    }

    flog.Info("ObjectDump...");
    for( const auto & x : objectMap)
    {
        flog.Info("{0}:{1}", x.first, x.second);
    }
}

#else
void Utils::DumpObjects()
{}
#endif
