#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <fstream>
#include <filesystem>
#include <mutex>

namespace Flog { enum class Level : unsigned; }

class StreamInfo
{
    std::ofstream mutable stream;
    std::string fileName;
    unsigned int date;

public:
    StreamInfo() : date()
    {}

    StreamInfo(std::ofstream && stream, std::string && fileName, unsigned int && date)
        : stream(std::move(stream))
        , fileName(std::move(fileName))
        , date(std::move(date))
    {}

    std::ofstream & Stream() const { return stream; }
    std::string FileName() const { return fileName; }
    unsigned int Date() const { return date; }
    operator bool() const { return stream.is_open() && stream.good(); }
};

class FileLogger
{
    std::string const baseFileName;
    std::experimental::filesystem::path const path;
    std::mutex streamMonitor;
    StreamInfo streamInfo;

public:
    FileLogger();

    static void Write(Flog::Level level, const char * message);

private:
    ~FileLogger();

    StreamInfo GetStream() const;
    void InternalWrite(Flog::Level level, const char * message);
    void EnsureStreamIsOpen();
    void HandleFileRollover(size_t newEntrySize);
    void CloseStream();
    void WriteHeader(std::ostream & writer) const;
    void WriteFooter(std::ostream & writer) const;
    bool ResourcesAvailable(size_t newEntrySize) const;
    std::string RenameOldFile(const std::string & oldFileName) const;
};

#endif // FILELOGGER_H
