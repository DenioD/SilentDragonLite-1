#ifndef LOGGER_H
#define LOGGER_H

#include "LogContext.h"
#include "LogStrategy.h"
#include "LogWriter.h"

class Logger : LogContext
{
    private:
        LogStrategy * strategy = nullptr;

    public:
        Logger(LogStrategy * strategy)
        {
            this->strategy = strategy;
        }

        void log(std::string message)
        {
            this->strategy->log(message);
        }
};
LogWriter* LogWriter::instance = nullptr;
#endif