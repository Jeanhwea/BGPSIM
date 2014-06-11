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
    fflush(out);
}

void
Logger::Warning(const char * emsg)
{
    if (emsg != NULL)
        fprintf(war, "Warning: %s\n", emsg);
    fflush(war);
}

void
Logger::Error(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Error: %s\n", emsg);
    fflush(err);
}

void
Logger::Fatal(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Fatal Error: %s\n", emsg);
    exit(1);
}

void
Logger::ShowErrno()
{
    fprintf(err, "!!!!!!!!!!!!!!!! Errno : %s\n", strerror(errno));
    fflush(err);
}


void
Logger::LogStateChage(state_t from, state_t to, event_t eve)
{
    fprintf(out, "Peer : { %s }->{ %s } : [%s]\n",
        mapStateName[from].c_str(),
            mapStateName[to].c_str(),
                mapEventName[eve].c_str());
    fflush(out);
}

void
Logger::LogPeerEve(event_t eve)
{
    fprintf(out, "Peer runs, handling event : %s\n",
            mapEventName[eve].c_str());
    fflush(out);
}

void
Logger::LogSimConf(int as, const char * ra)
{
    fprintf(out, "Load Simu Config : AS%d, ra(%s)\n", as, ra);
}

#define PRINT_ALIGN 16
void
Logger::LogDumpMsg(u_char * data, size_t len)
{
    fprintf(out, "MSG with %uBytes, Dump\n", (unsigned int)len);
    if (len > 4096) return;
    for (size_t i = 0; i < len; ++i) {
        if (!((i)%PRINT_ALIGN))
            fprintf(out, "0x%04x : ", (unsigned int)i);

        fprintf(out, "%02x%c", *(data+i), (i+1)%PRINT_ALIGN ? ' ' : '\n');
        fflush(out);
    }
    if (len%PRINT_ALIGN)
        fprintf(out, "\n");
    fflush(out);
}
