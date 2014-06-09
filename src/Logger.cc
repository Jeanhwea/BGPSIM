#include "Logger.h"
#include "Peer.h"
#include "Event.h"

using namespace std;

Logger::Logger()
{
    out = outfd;
    war = errfd;
    err = errfd;
}

Logger::~Logger()
{
}

void
Logger::Tips(const char * emsg)
{
    if (emsg != NULL)
        fprintf(out, "Tips: %s\n", emsg);
}

void
Logger::Warning(const char * emsg)
{
    if (emsg != NULL)
        fprintf(war, "Warning: %s\n", emsg);
}

void
Logger::Error(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Error: %s\n", emsg);
}

void
Logger::Fatal(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Fatal Error: %s\n", emsg);
    exit(1);
}

void
Logger::LogStateChage(state_t from, state_t to, event_t eve)
{
    fprintf(out, "Peer : { %s }->{ %s } : [%s]\n",
        mapStateName[from].c_str(),
            mapStateName[to].c_str(),
                mapEventName[eve].c_str());
}

#define PRINT_ALIGN 16
void
Logger::LogDumpMsg(u_char* data, ssize_t len)
{
    fprintf(out, "MSG with %luBytes, Dump\n", len);
    if (len > 4096) return;
    for (int i = 0; i < len; ++i) {
        if (!((i)%PRINT_ALIGN)) fprintf(out, "0x%04x : ", i);
        fprintf(out, "%02x%c", *(data+i), (i+1)%PRINT_ALIGN ? ' ' : '\n');
    }
    if (len%PRINT_ALIGN) fprintf(out, "\n");
}
