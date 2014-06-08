#ifndef THREAD_Z0X3KQ1B

#define THREAD_Z0X3KQ1B

#include "global.h"

class Thread {
    private:
        static pthread_t thread_cnt;
        pthread_t mTid;
        bool isRunning;
        bool isDetached;

        static void * RunThread(void * arg) {
            return ((Thread *)arg)->Run();
        }

    public:
        Thread ();
        virtual ~Thread ();
        virtual void * Run() = 0;

        bool Start();
        bool Join();
        bool Detach();
        pthread_t Self();
};

#endif /* end of include guard: THREAD_Z0X3KQ1B */
