#ifndef LOGSUCCESS_H
#define LOGSUCCESS_H

#include "LogType.h"
#include "LogStrategy.h"
#include "LogWriter.h"

class LogSuccess : public LogStrategy
{
    public:
        void log(std::string message)
        {
            LogWriter* lw = LogWriter::getInstance();
            lw->write(LogType::SUCCESS, message);
        }
};

#endif