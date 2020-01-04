#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>

enum Level {
    Off     = 0,
    Fatal   = 1,
    Error   = 2,
    Warning = 3,
    Info    = 4,
    Debug   = 5,
    Trace   = 6
};

class Logger {
private:
    /** Size of buffer for logging. */
    constexpr static const int BUFF_SIZE = 1024;
    /** Log file name. */
    constexpr static const char* LOG_FNAME   = "../log/server.log";
    // severity message constants
    constexpr static const char* LOG_FATAL   = "[FATAL]   ";
    constexpr static const char* LOG_ERROR   = "[ERROR]   ";
    constexpr static const char* LOG_WARNING = "[WARNING] ";
    constexpr static const char* LOG_INFO    = "[INFO]    ";
    constexpr static const char* LOG_DEBUG   = "[DEBUG]   ";
    constexpr static const char* LOG_TRACE   = "[TRACE]   ";

    /** Pointer to itself -- singleton. */
    static Logger* instance;

    /** File for logging to. */
    std::ofstream file;

    /** Level of logger severity. */
    Level level;

    /** Prevent construction. */
    Logger();
    /** Prevent unwanted destruction. */
    ~Logger();
    /** Prevent construction by copying. */
    Logger(const Logger&);
    /** Prevent assignment. */
    Logger& operator=(const Logger&);

public:
    /** Get pointer to itself. */
    static Logger* getInstance();
    /** Clear instance from memory -- used before end. */
    static void clearInstance();

    /** Set level of logger severity. */
    void setLevel(Level);

    /** Types of log messages. */
    void fatal(const char*, ...);
    void error(const char*, ...);
    void warning(const char*, ...);
    void info(const char*, ...);
    void debug(const char*, ...);
    void trace(const char*, ...);
};

#define logger Logger::getInstance()

#endif
