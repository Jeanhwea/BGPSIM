#include "Logger.h"

using namespace std;

Logger::Logger() 
{
    out = outfd;
    err = errfd;
}

Logger::~Logger() 
{
}

void
Logger::Warning(const char * emsg) 
{
    if (emsg != NULL) 
        fprintf(out, "warning: %s\n", emsg);
}

void
Logger::Error(const char * emsg) 
{
    if (emsg != NULL) 
        fprintf(err, "error: %s\n", emsg);
}

void
Logger::Fatal(const char * emsg) 
{
    if (emsg != NULL) 
        fprintf(err, "fatal: %s\n", emsg);
    exit(1);
}

// const char *
// Logger::LogAddr(const struct in_addr * addr) 
// {
//     static char buf[48];
//
// }
