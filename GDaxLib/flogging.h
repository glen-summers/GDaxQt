#ifndef FLOGGING_H
#define FLOGGING_H

#include "accuratetimer.h"

#include <QString>
#include <string>

namespace Flog
{
    enum class Level : unsigned { Spam, Debug, Info, Warning, Error, Critical, Fatal } ;

    class LogManager;
    class ScopeLog;

    class Log
    {
        std::string const name;

    public:
        void Spam(const char * message) const { Write(Level::Spam, message); }
        void Spam(const std::string & message) const { Write(Level::Spam, message.c_str()); }
        void Spam(const QString & message) const { Write(Level::Spam, message.toStdString().c_str()); }

        void Info(const char * message) const { Write(Level::Info, message); }
        void Info(const std::string & message) const { Write(Level::Info, message.c_str()); }
        void Info(const QString & message) const { Write(Level::Info, message.toStdString().c_str()); }

        void Warning(const char * message) const { Write(Level::Warning, message); }
        void Warning(const std::string & message) const { Write(Level::Warning, message.c_str()); }
        void Warning(const QString & message) const { Write(Level::Warning, message.toStdString().c_str()); }

        void Error(const char * message) const { Write(Level::Error, message); }
        void Error(const std::string & message) const { Write(Level::Error, message.c_str()); }
        void Error(const QString & message) const { Write(Level::Error, message.toStdString().c_str()); }

        friend class LogManager;
        friend class ScopeLog;

    private:
        Log(const std::string & name) : name(name) {}

        void Write(Level level, const char * message) const;
        void ScopeStart(Level level, const char * message) const;
        void ScopeEnd(Level level, const char * message, std::chrono::nanoseconds ns) const;
    };

    class ScopeLog
    {
        const Log & log;
        Level level;
        std::string scope;
        AccurateTimer timer;

        ScopeLog(const ScopeLog&) = delete;
        ScopeLog& operator=(const ScopeLog&) = delete;
        ScopeLog(ScopeLog&&) = default;
        ScopeLog& operator=(ScopeLog&&) = default;

    public:
        ScopeLog(const Log & log, Level level, const char * scope)
            : log(log), level(level), scope(scope)
        {
            log.ScopeStart(level, scope);
        }

        ~ScopeLog()
        {
            log.ScopeEnd(level, scope.c_str(), timer.Elapsed<long long, std::nano>());
        }
    };

    class LogManager
    {
        static std::string Unmangle(const std::string & name);
    public:
        static void SetLevel(Level level);

        static Log GetLog(const std::string & name)
        {
            return Log(name);
        }

        template <typename T>
        static Log GetLog()
        {
            return Log(Unmangle(typeid(T).name()));
        }
    };
}



#endif // FLOGGING_H
