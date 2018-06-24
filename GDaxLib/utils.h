#ifndef UTILS_H
#define UTILS_H

#include <QString>
class QObject;

namespace Utils
{
    QString DiffText(const QString & s1, const QString & s2);

#ifdef QT_DEBUG
    namespace Detail
    {
        void Constructed(const char * name, QObject * object);
    }
#endif

    template <typename T, class... Types>
    inline T * QMake(const char * name, Types&&... types)
    {
        T* t= new T(types...);
#ifdef QT_DEBUG
        Detail::Constructed(name, t);
#else
        (void)name;
#endif
        return t;
    }

    void DumpObjects();
}

#endif // UTILS_H
