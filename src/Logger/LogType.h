#ifndef LOGTYPE_H
#define LOGTYPE_H

#include <string>

class LogType
{
    public:
    enum TYPE {
        INFO = 0,
        DEBUG = 1,
        SUCCESS = 2,
        WARNING = 3,
        ERROR = 4,
        FATAL = 5,
        CRITICAL = 6
    };

    static std::string enum2String(int type)
    {
        switch (type)
        {
            default:
            case 0:
                return "INFO";

            case 1:
                return "DEBUG";

            case 2:
                return "SUCCESS";

            case 3:
                return "WARNING";

            case 4:
                return "ERROR";

            case 5:
                return "FATAL";

            case 6:
                return "CRITICAL";
        }
    }
};
#endif