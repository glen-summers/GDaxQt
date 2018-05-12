
#include "filelogger.h"
#include "flogging.h"

#include <QCoreApplication>
#include <QProcess>

#include <iomanip>
#include <sstream>
#include <thread>

namespace fs = std::experimental::filesystem;

namespace
{
    static constexpr const char * HeaderFooterSeparator = "------------------------------------------------";
    static constexpr const char * Delimiter = " : ";
    static constexpr int THREAD_ID_WIDTH = 5;
    static constexpr size_t maxFileSize = 5*1024*1024;
    static constexpr size_t ReserveDiskSpace = 10*1024*1024;
    static Flog::Level logLevel = Flog::Level::Info;

    void TranslateLevel(std::ostream & stm, Flog::Level level)
    {
        switch (level)
        {
            case Flog::Level::Fatal:
                stm << "FATAL   ";
                break;
            case Flog::Level::Critical:
                stm << "CRITICAL";
                break;
            case Flog::Level::Error:
                stm << "ERROR   ";
                break;
            case Flog::Level::Warning:
                stm << "WARNING ";
                break;
            case Flog::Level::Info:
                stm << "INFO    ";
                break;
            case Flog::Level::Debug:
                stm << "DEBUG   ";
                break;
            case Flog::Level::Spam:
                stm << "SPAM    ";
                break;
        }
    }

    void LocalTime(tm & tm, const time_t & t)
    {
        #ifdef __linux__
        localtime_r(&t, &tm);
        #elif _WIN32
        localtime_s(&tm, &t);
        #else
        //?
        #endif
    }

    void GmTime(tm & tm, const time_t & t)
    {
        #ifdef __linux__
        gmtime_r(&t, &tm);
        #elif _WIN32
        gmtime_s(&tm, &t);
        #else
        //?
        #endif
    }

    unsigned int GetDate()
    {
        std::time_t t = std::time(nullptr);
        std::tm tm;
        LocalTime(tm, t);
        return ((1900 + tm.tm_year) * 100 + tm.tm_mon + 1) * 100 + tm.tm_mday;
    }

    uintmax_t GetFreeDiskSpace(const fs::path & path)
    {
        //return 100 * 1024 * 1024;
        return space(path).available;
    }

    static std::string GetBaseName()
    {
        return fs::path(QCoreApplication::applicationFilePath().toStdString()).filename().u8string();
    }

    static std::string GetFullName()
    {
        return QCoreApplication::applicationFilePath().toStdString();
    }

    static unsigned long long GetProcessId()
    {
        return QCoreApplication::applicationPid();
    }

    static std::thread::id GetThreadId()
    {
        return std::this_thread::get_id();
    }

    template<class Arg> struct Manip
    {
        Manip(void(*left)(std::ostream&, Arg), Arg val) : p(left), arg(val) {}
        void(*p)(std::ostream&, Arg);
        Arg arg;
    };

    template <typename Arg>
    std::ostream & operator<<(std::ostream& str, const Manip<Arg>& manip)
    {
        (*manip.p)(str, manip.arg);
        return str;
    }

    class FloggerBuf : public std::basic_streambuf<char>
    {
        static constexpr int bufsize = 256;
        typedef std::vector<char> buffer;
        typedef basic_streambuf<char> base;
        mutable buffer buf;

        int_type overflow(int_type c) override
        {
            if (traits_type::eq_int_type(c, traits_type::eof()))
            {
                return traits_type::not_eof(c);
            }
            buf.push_back(traits_type::to_char_type(c));
            return c;
        }
    public:
        FloggerBuf()
        {
            buf.reserve(bufsize);
        }

        const char * Get() const
        {
            buf.push_back(0);
            return buf.data();
        }

        void Reset() const
        {
            buf.clear();
        }
    };

    template <typename T, class BufferType> // BufferType : basic_streambuf<_Elem, _Traits>
    class GenericOutStream : public std::basic_ostream<T>
    {
        typedef std::basic_ostream<T> base;

        BufferType m_buf;

    public:
        GenericOutStream(std::ios_base::fmtflags f = std::ios_base::fmtflags()) : base(&m_buf)
        {
            base::setf(f);
        }

        const BufferType & rdbuf() { return m_buf; }
    };

    using FlogStream = GenericOutStream<char, FloggerBuf>;
}

namespace Flog
{
// initer is too early as QApplication not created yet;
//    struct Initialiser
//    {
//        Initialiser()
//        {
//            FileLogger::Write(Flog::Level::Spam, "Initialising");
//        }
//    };
//    static Initialiser initialiser;

//    std::ostream & Log::Stream(const Log * log)
//    {
//        static thread_local FlogStream stream(std::ios_base::boolalpha);
//        if (log)
//        {
//            stream << log->name << " : ";
//        }
//        return stream;
//    }

//    void Log::Write(Level level)
//    {
//        FlogStream & ss(static_cast<FlogStream &>(Stream(nullptr)));
//        FileTraceListener::Write(level, ss.rdbuf().Get());
//        ss.rdbuf().Reset(); // AV here
//    }

    void Log::Write(Level level, const char * message) const
    {
        FileLogger::Write(level, name.c_str(), message);
    }
}

FileLogger::FileLogger()
    : baseFileName(GetBaseName() + "_" + std::to_string(GetProcessId()))
    , path(fs::temp_directory_path() / "glogfiles")
{
    create_directories(path);
}

// avoid
FileLogger::~FileLogger()
{
    CloseStream(); //
}

void FileLogger::Write(Flog::Level level, const char * prefix, const char *message)
{
    static FileLogger ftl;
    ftl.InternalWrite(level, message);
}

StreamInfo FileLogger::GetStream() const
{
    // the reason to add yyyy-MM-dd at the onset is so that the file collision rate is lower in that it wont hit a random old file
    // and we're not renaming old files here atm (which has the file time tunneling problem http://support2.microsoft.com/kb/172190)
    // with date added at start theres no need to add at file rollover time, and no need to rename the file at all
    // however flogTail currently doesnt work with this format
    // without adding the date at start, it currently wont get added at rollover
    // rollover will just increase the fileCount app_pid_n, and again will confuse flogtails fileChain logic
    // change this to be full flog compliant, with some params, then upgrade flogtail to handle any scenario
    static constexpr bool alwaysAddDate = false;

    std::ostringstream s;
    s << baseFileName;
    if (alwaysAddDate)
    {
        time_t t = std::time(nullptr);
        tm tm;
        LocalTime(tm, t);
        s << "_" << std::put_time(&tm, "%Y-%m-%d");
    }
    fs::path logFileName = path / (s.str() + ".log"); // combine, check trailing etc.
    unsigned int date = GetDate();

    const int MaxTries = 1000;

    // consolidate with renameOldFile??
    for (int num = 0; num < MaxTries; ++num)
    {
        if (num != 0)
        {
            logFileName.replace_filename(s.str() + "_" + std::to_string(num) + ".log");
        }

        //RenameOldFile(logFileName.u8string());

        if (exists(logFileName))
        {
            continue;
        }

        std::ofstream newStreamWriter;//ctor?
        newStreamWriter.open(logFileName); // FileShare.ReadWrite | FileShare.Delete? HANDLE_FLAG_INHERIT
        if (!newStreamWriter)
        {
            continue;
        }

        //Debug::Write("Flogging to file : {0}", logFileName.u8string());
        try
        {
            //					Stream stream = File.Open(str, FileMode.CreateNew, FileAccess.Write, FileShare.ReadWrite | FileShare.Delete);
            //					newStreamWriter = new StreamWriter(stream, encoding){ AutoFlush = true };
            WriteHeader(newStreamWriter);

            // rolled over from file...
            // flush?
            // ensure we are not tunnelled here...
            // http://blogs.msdn.com/oldnewthing/archive/2005/07/15/439261.aspx
            // http://support.microsoft.com/?kbid=172190
            //FILETIME ft;
            //::GetSystemTimeAsFileTime(&ft);
            //::SetFileTime(GetImpl(m_stream), &ft, NULL, NULL); // filesystem ver? nope

            return StreamInfo{ move(newStreamWriter), move(logFileName.u8string()), std::move(date) };
        }
        catch (...) // specific
        {
        }
    }
    throw std::runtime_error("Exhausted possible stream names " + logFileName.u8string());
}

void FileLogger::InternalWrite(Flog::Level level, const char * prefix, const char *message)
{
    // ShouldTrace ...
    if (level < logLevel)
    {
        return;
    }

    std::lock_guard<std::mutex> guard(streamMonitor);
    {
        try
        {
            size_t newEntrySize = strlen(message);
            HandleFileRollover(newEntrySize);
            EnsureStreamIsOpen();
            if (!ResourcesAvailable(newEntrySize))
            {
                return;
            }

            // flags etc.
            // move some formatting out of lock

            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm;
            LocalTime(tm, t);

            int ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

            streamInfo.Stream()
                << std::put_time(&tm, "%d %b %Y, %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms << std::setfill(' ')
                << " : [ " << std::setw(THREAD_ID_WIDTH) << GetThreadId() << " ] : "
                << std::setw(8) << Manip<Flog::Level>(TranslateLevel, level) << " : "
                << prefix << " : "
                << message << std::endl << std::flush;
        }
        catch (...)
        {
            CloseStream();
            throw;
        }
    }
}

void FileLogger::EnsureStreamIsOpen()
{
    if (!streamInfo)
    {
        streamInfo = GetStream();
    }
}

void FileLogger::HandleFileRollover(size_t newEntrySize)
{
    if (streamInfo)
    {
        std::string oldFile = streamInfo.FileName();
        auto size = streamInfo.Stream().tellp();
        if (newEntrySize + size >= maxFileSize || streamInfo.Date() != GetDate())
        {
            CloseStream();
            //RenameOldFile(oldFile);
        }
    }
}

void FileLogger::CloseStream()
{
    if (streamInfo)
    {
        try
        {
            WriteFooter(streamInfo.Stream());
        }
        catch (...) // specific?
        {
        }
        try
        {
            streamInfo.Stream().flush();
        }
        catch (...) // specific?
        {
        }
        //streamWriter.close();
        streamInfo = StreamInfo();
    }
}

void FileLogger::WriteHeader(std::ostream &writer) const
{
    writer << HeaderFooterSeparator << std::endl;

    std::time_t t = std::time(nullptr);
    std::tm tm;
    LocalTime(tm, t);
    std::tm gtm;
    GmTime(gtm, t);

    static constexpr bool is64BitProcess = sizeof(void*) == 8;
    static constexpr int bits = is64BitProcess ? 64 : 32; // more?

    writer
            << "Opened      : " << std::put_time(&tm, "%d %b %Y, %H:%M:%S (%z)") << std::endl
            << "OpenedUtc   : " << std::put_time(&gtm, "%F %TZ") << std::endl
            << "ProcessName : (" << bits << " bit) " << GetBaseName() << std::endl
            << "FullPath    : " << GetFullName() << std::endl
            << "ProcessId   : " <<  GetProcessId() << std::endl
            << "ThreadId    : " << GetThreadId() << std::endl;
    //Formatter::Format(writer, "UserName    : {0}\\{1}", Environment.UserDomainName, Environment.UserName);

    writer << HeaderFooterSeparator << std::endl;
    writer.flush();
}

void FileLogger::WriteFooter(std::ostream &writer) const
{
    std::time_t t = std::time(nullptr);
    std::tm tm;
    LocalTime(tm, t);

    writer
            << HeaderFooterSeparator << std::endl
            << "Closed       " << std::put_time(&tm, "%d %b %Y, %H:%M:%S (%z)") << std::endl
            << HeaderFooterSeparator << std::endl;
    writer.flush();
}

bool FileLogger::ResourcesAvailable(size_t newEntrySize) const
{
    // called after rollover check
    // if newEntrySize large and just caused a rollover, and new+header > MaxFileSize then it will not get logged!
    return streamInfo && (newEntrySize + streamInfo.Stream().tellp() <= maxFileSize)
            && GetFreeDiskSpace(path) - newEntrySize >= ReserveDiskSpace;
}
