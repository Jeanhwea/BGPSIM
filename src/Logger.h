#ifndef LOGGER_6S0KXYMJ

#define LOGGER_6S0KXYMJ

#include "global.h"

using namespace std;

class Logger {
    private:
        FILE * out;
        FILE * err;

    public:
        Logger();
        virtual ~Logger();
        void Warning(const char *);
        void Error(const char *);
        void Fatal(const char *);
};

#endif /* end of include guard: LOGGER_6S0KXYMJ */
