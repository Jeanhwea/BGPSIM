#ifndef THREAD_Z0X3KQ1B

#define THREAD_Z0X3KQ1B

#include "global.h"

class Thread {
    private:
        static pthread_t thread_cnt;
        pthread_t tid;
        bool isRunning;
        bool isDetached;

        static void * RunThread_one(void * arg);
        static void * RunThread_two(void * arg);

    public:
        Thread();
        virtual ~Thread();
        virtual void * Run() = 0;
        // use for just Peer when handle event
        virtual void * Run(event_t eve) { return NULL;}

        bool Start();
        bool Start(event_t eve);
        bool Join();
        bool Detach();
        pthread_t Self();
};

struct arg_eve {
    event_t     eve;
    Thread    * thd;
};

#endif /* end of include guard: THREAD_Z0X3KQ1B */
