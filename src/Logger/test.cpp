#include "SimpleLogger.h"

int main(int argc, char** argv)
{
    SimpleLogger sl = SimpleLogger("/tmp/simplelog.log");
    sl.logInfo("test info");
    sl.logDebug("test debug");
    sl.logSuccess("test success");
    sl.logWarning("test warning");
    sl.logError("test error");
    sl.logFatal("test fatal");
    sl.logCritical("test critical");
    return 0;
}