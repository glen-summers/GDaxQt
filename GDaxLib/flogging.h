#ifndef FLOGGING_H
#define FLOGGING_H

#include <QString>
#include <string>

namespace Flog
{
    enum class Level : unsigned { Spam, Debug, Info, Warning, Error, Critical, Fatal } ;

    class LogManager;

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

    private:
        Log(const std::string & name) : name(name) {}

        static void Write(Level level, const char * message);
    };

    class LogManager
    {
    public:
        static Log GetLog(const std::string & name)
        {
            return Log(name);
        }

        template <typename T>
        static Log GetLog()
        {
            return Log(typeid(T).name());
        }
    };
}



#endif // FLOGGING_H
