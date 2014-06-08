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
};

#endif /* end of include guard: LOGGER_6S0KXYMJ */
