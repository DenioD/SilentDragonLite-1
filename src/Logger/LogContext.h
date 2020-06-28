#ifndef LOGCONTEXT_H
#define LOGCONTEXT_H

#include <string>
class LogContext
{
    public:
    virtual void log(std::string message) {};
};

#endif