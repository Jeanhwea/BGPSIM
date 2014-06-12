#include "Logger.h"
#include "Peer.h"
#include "Event.h"
#include "Listener.h"

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
        fprintf(out, "Tips\t: %s\n", emsg);
    fflush(out);
}

void
Logger::Warning(const char * emsg)
{
    if (emsg != NULL)
        fprintf(war, "Warning\t: %s\n", emsg);
    fflush(war);
}

void
Logger::Error(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Error\t: %s\n", emsg);
    fflush(err);
}

void
Logger::Fatal(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Fatal\t: %s\n", emsg);
    exit(1);
}

void
Logger::ShowErrno()
{
    fprintf(err, "Errno%d\t: %s\n", errno, strerror(errno));
    fflush(err);
}

char *
Logger::AddrToStr(struct in_addr * pAd)
{
    assert(pAd != NULL);
    return inet_ntoa(*pAd);
}

char *
Logger::AddrToStr(struct sockaddr_in * pSad)
{
    assert(pSad != NULL);
    return AddrToStr(&pSad->sin_addr);
}


void
Logger::ShowIPAddr(struct in_addr * pAd)
{
    fprintf(out, "ip:%s\n", AddrToStr(pAd));
}

void
Logger::ShowIPAddr(struct sockaddr_in * pSad)
{
    assert(pSad != NULL);
    ShowIPAddr(&pSad->sin_addr);
}

void
Logger::LogStateChage(state_t from, state_t to, event_t eve)
{
    fprintf(out, "Peer\t: { %s }->{ %s } : [%s]\n",
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
    fprintf(out, "Load Simu Config : AS%d, %s\n", as, ra);
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

void
Logger::LogListenerList()
{
    vector<Listener *>::iterator lit;
    Listener * pListener;
    struct in_addr * pAd;
    fprintf(out, "Listener list addr = [ ");
    for (lit = vListeners.begin(); lit != vListeners.end(); ++lit) {
        pListener = *lit;
        assert(pListener != NULL);
        pAd = pListener->GetLisAddr();
        assert(pAd != NULL);
        fprintf(out, "%s ", AddrToStr(pListener->GetLisAddr()));
    }
    fprintf(out, "]\n");
}

void
Logger::LogPeerList()
{
    vector<Peer *>::iterator    pit;
    Peer                      * pPeer;
    u_int16_t                   as;
    fprintf(out, "Peers list config  = [ ");
    for (pit = vPeers.begin(); pit != vPeers.end(); ++pit) {
        pPeer = *pit;
        assert(pPeer != NULL);
        as = ntohs(pPeer->conf.remote_as);
        fprintf(out,"{ AS%d, %s } ", as, AddrToStr(&pPeer->conf.remote_addr));
    }
    fprintf(out, "]\n");
}
