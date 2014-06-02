#ifndef TIMER_1889413Y

#define TIMER_1889413Y

#include "global.h"
#include "thread.h"

class Timer : public Thread {
    public:
        Timer ();
        virtual ~Timer ();

        void * Run();
        void Schedule();

    private:
        static time_t interval;
        static time_t walltime;

        static Timer * instance;
        static int ref_times;

        static Timer * GetInst();
        static void TimerHandler(int sig);
};

#endif /* end of include guard: TIMER_1889413Y */
