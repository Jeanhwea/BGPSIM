#ifndef TIMER_1889413Y

#define TIMER_1889413Y

#include "global.h"
#include "Thread.h"
#include "Peer.h"


class Timer : public Thread {
    private:
        static Simulator * sim;
        static time_t interval;

        static Timer * instance;
        static int ref_times;

        static Timer * GetInst();
        static void TimerHandler(int sig);

    public:
        Timer();
        virtual ~Timer();

        void * Run();
        void Schedule();
};

#endif /* end of include guard: TIMER_1889413Y */
