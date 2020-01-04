#include <cstdarg>
#include <iostream>

#include "Helper.hpp"
#include "Logger.hpp"

/** Initialize logger instance to null. */
Logger* Logger::instance = nullptr;


/******************************************************************************
 *
 * 	Constructor opens log file and sets file's severity level (Level.Info).
 * 	Singleton pattern.
 *
 */
Logger::Logger() {
    Logger::file.open(Logger::LOG_FNAME);
    Logger::setLevel(Info);

    if (!Logger::file.is_open()) {
        std::cout << "[WARNING] Log file could not be opened. Log messages will not be writen." << std::endl;
    }
}


/******************************************************************************
 *
 * 	Destructor closes log file.
 *
 */
Logger::~Logger() {
    if (Logger::file.is_open()) {
        logger->info("Closing log file.");
        Logger::file.close();
    }
}


/******************************************************************************
 *
 * 	Get instance of logger -- only access to it.
 *
 */
Logger* Logger::getInstance() {
    if (Logger::instance == nullptr) {
        Logger::instance = new Logger;
    }

    return Logger::instance;
}


/******************************************************************************
 *
 * 	Clear instance, in order to clean all used memory.
 *
 */
void Logger::clearInstance() {
    delete Logger::instance;
}


/******************************************************************************
 *
 * 	Set severity level of logger.
 *
 */
void Logger::setLevel(const Level lvl) {
    this->level = lvl;
}


// ****************     LOG MESSAGES     **************************************


void Logger::log(const char* severity, const char* buff) {
    std::stringstream out;
    out << severity << getDateTime() << buff << std::endl;
    this->file << out.str();
    std::cout << out.str();

}

void Logger::fatal(const char* msg, ...) {
    if (this->file.is_open() && this->level >= Fatal) {
        char buff[Logger::BUFF_SIZE];

        // format msg in case of arguments
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, Logger::BUFF_SIZE, msg, args);
        va_end(args);

        // log message
        this->log(Logger::LOG_FATAL, buff);
    }
}

void Logger::error(const char* msg, ...) {
    if (this->file.is_open() && this->level >= Error) {
        char buff[Logger::BUFF_SIZE];

        // format msg in case of arguments
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, Logger::BUFF_SIZE, msg, args);
        va_end(args);

        // log message
        this->log(Logger::LOG_ERROR, buff);
    }
}

void Logger::warning(const char* msg, ...) {
    if (this->file.is_open() && this->level >= Warning) {
        char buff[Logger::BUFF_SIZE];

        // format msg in case of arguments
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, Logger::BUFF_SIZE, msg, args);
        va_end(args);

        // log message
        this->log(Logger::LOG_WARNING, buff);
    }
}

void Logger::info(const char* msg, ...) {
    if (this->file.is_open() && this->level >= Info) {
        char buff[Logger::BUFF_SIZE];

        // format msg in case of arguments
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, Logger::BUFF_SIZE, msg, args);
        va_end(args);

        // log message
        this->log(Logger::LOG_INFO, buff);
    }
}

void Logger::debug(const char* msg, ...) {
    if (this->file.is_open() && this->level >= Debug) {
        char buff[Logger::BUFF_SIZE];

        // format msg in case of arguments
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, Logger::BUFF_SIZE, msg, args);
        va_end(args);

        // log message
        this->log(Logger::LOG_DEBUG, buff);
    }
}

void Logger::trace(const char* msg, ...) {
    if (this->file.is_open() && this->level == Trace) {
        char buff[Logger::BUFF_SIZE];

        // format msg in case of arguments
        va_list args;
        va_start(args, msg);
        vsnprintf(buff, Logger::BUFF_SIZE, msg, args);
        va_end(args);

        // log message
        this->log(Logger::LOG_TRACE, buff);
    }
}
