#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <string>
#include <fstream>
#include <sstream>
#include <ctime> 
#include "LogType.h"

class LogWriter
{
    public:
        static LogWriter* getInstance();
        std::string logfile = "";
        void setLogFile(std::string file);
        void write(LogType::TYPE t, std::string message);

    private:
        static LogWriter* instance;
};

#endif