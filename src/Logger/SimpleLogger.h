#ifndef SIMPLELOGGER_H
#define SIMPLELOGGER_H

#include "Logger.h"
#include "LogInfo.h"
#include "LogDebug.h"
#include "LogSuccess.h"
#include "LogWarning.h"
#include "LogError.h"
#include "LogFatal.h"
#include "LogCrtitical.h"
#include "LogWriter.h"

class SimpleLogger
{
    public:
        SimpleLogger()
        {
            LogWriter::getInstance()->setLogFile("log.txt");
        }

        SimpleLogger(std::string logFile)
        {
            LogWriter::getInstance()->setLogFile(logFile);
        }

        void logInfo(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogInfo();
            logger = new Logger(li);
            logger->log(message);
        }

        void logDebug(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogDebug();
            logger = new Logger(li);
            logger->log(message);
        }

        void logSuccess(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogSuccess();
            logger = new Logger(li);
            logger->log(message);
        }

        void logWarning(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogWarning();
            logger = new Logger(li);
            logger->log(message);
        }

        void logError(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogError();
            logger = new Logger(li);
            logger->log(message);
        }

        void logFatal(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogFatal();
            logger = new Logger(li);
            logger->log(message);
        }

        void logCritical(std::string message)
        {
            Logger* logger = nullptr;
            LogStrategy* li = new LogCritical();
            logger = new Logger(li);
            logger->log(message);
        }
};

#endif