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

    if (Logger::file.is_open()) {
        std::cout << "sccs opened" << std::endl;
    }
    else {
        std::cout << "fail closed" << std::endl;
    }
}


/******************************************************************************
 *
 * 	Destructor closes log file.
 *
 */
Logger::~Logger() {
    Logger::file.close();

    if (Logger::file.is_open()) {
        std::cout << "fail opened" << std::endl;
    }
    else {
        std::cout << "sccs closed" << std::endl;
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

void Logger::fatal(const std::string &msg) {
    if (this->file.is_open() && this->level >= Fatal) {
        this->file << Logger::LOG_FATAL << getDateTime() << msg << std::endl;
    }
}

void Logger::error(const std::string &msg) {
    if (this->file.is_open() && this->level >= Error) {
        this->file << Logger::LOG_ERROR << getDateTime() << msg << std::endl;
    }
}

void Logger::warning(const std::string &msg) {
    if (this->file.is_open() && this->level >= Warning) {
        this->file << Logger::LOG_WARNING << getDateTime() << msg << std::endl;
    }
}

void Logger::info(const std::string &msg) {
    if (this->file.is_open() && this->level >= Info) {
        this->file << Logger::LOG_INFO << getDateTime() << msg << std::endl;
    }
}

void Logger::debug(const std::string &msg) {
    if (this->file.is_open() && this->level >= Debug) {
        this->file << Logger::LOG_DEBUG << getDateTime() << msg << std::endl;
    }
}

void Logger::trace(const std::string &msg) {
    if (this->file.is_open() && this->level == Trace) {
        this->file << Logger::LOG_TRACE << getDateTime() << msg << std::endl;
    }
}
