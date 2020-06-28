#include "LogWriter.h"

LogWriter* LogWriter::getInstance()
{
    if(instance == nullptr)
        instance = new LogWriter();

    return instance;
}

void LogWriter::setLogFile(std::string file)
{
    this->logfile = file;
}

void LogWriter::write(LogType::TYPE type, std::string message)
{
    std::ofstream writer(this->logfile, std::ios::out | std::ios::app);
    if(writer.good())
    {
        time_t now = time(0);
	    tm *ltm = localtime(&now);
        std::stringstream ss;
        ss << "[" << LogType::enum2String(type) << "] " << 
        ltm->tm_mon << "-" << 
        ltm->tm_mday << "-" << 
        (1900 + ltm->tm_year) << " " <<
        ltm->tm_hour << ":" << 
        ltm->tm_min << ":" <<
        ltm->tm_sec << " > " << message;
        writer << ss.str() << "\n";
    }

    writer.close();
}