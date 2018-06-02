#ifndef UTILS_H
#define UTILS_H

#include <QString>

#ifdef QT_DEBUG
#include <QObject>
#endif

namespace Utils
{
    void EnableAnsiConsole();

    QString DiffText(const QString & s1, const QString & s2);

    namespace Detail
    {
        void OnDestroyed(const char * objectName);
    }

    template <typename T, class... Types>
    inline T * QMake(const char * name, Types&&... types)
    {
        T* t= new T(types...);
#ifdef QT_DEBUG
        QObject::connect(t, &QObject::destroyed, [=]() { Detail::OnDestroyed(name); });
#endif
        return t;
    }
}

#endif // UTILS_H
