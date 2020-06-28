#ifndef LOGSTRATEGY_H
#define LOGSTRATEGY_H

#include <string>
class LogStrategy
{
    public:
    virtual void log(std::string message) {};
};

#endif