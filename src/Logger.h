#ifndef LOGGER_6S0KXYMJ

#define LOGGER_6S0KXYMJ

#include "global.h"

using namespace std;

class Peer;

class Logger {
    private:
        FILE * out;
        FILE * war;
        FILE * err;

        char * AddrToStr(u_int32_t addr);
        char * AddrToStr(struct in_addr * ad);
        char * AddrToStr(struct sockaddr_in * sad);
        char * MacToStr(u_char * mac_addr);

    public:
        Logger();
        virtual ~Logger();
        void Tips(const char *);
        void Warning(const char *);
        void Error(const char *);
        void Fatal(const char *);
        void TraceSize(const char * msg, ssize_t siz);
        void ShowErrno();
        void ShowIPAddr(struct in_addr * ad);
        void ShowIPAddr(struct sockaddr_in * sad);

        void LogStateChage(Peer *, state_t to, event_t eve);
        void LogDumpMsg(u_char * data, size_t len);
        void LogPeerEve(Peer * pPeer, event_t eve);
        void LogSimConf(int as, const char * ra);
        void LogDispatchMsg(u_int16_t len, u_int8_t type);
        void LogRecvedMsg(struct ethhdr * pEthhdr);

        void LogIntList();
        void LogPeerList();
        void LogListenerList();
        void LogRouteList();
};

#endif /* end of include guard: LOGGER_6S0KXYMJ */
