#ifndef LOGGER_6S0KXYMJ

#define LOGGER_6S0KXYMJ

#include "global.h"

using namespace std;

class Logger {
    private:
        FILE * out;
        FILE * war;
        FILE * err;

    public:
        Logger();
        virtual ~Logger();
        void Tips(const char *);
        void Warning(const char *);
        void Error(const char *);
        void Fatal(const char *);
        void LogStateChage(state_t from, state_t to, event_t eve);
        void LogDumpMsg(u_char * data, size_t len);
        void LogPeerEve(event_t eve);
        void LogSimConf(int as, const char * la, const char * ra);
};

#endif /* end of include guard: LOGGER_6S0KXYMJ */
