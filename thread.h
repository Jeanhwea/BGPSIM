#ifndef THREAD_Z0X3KQ1B

#define THREAD_Z0X3KQ1B

#include "global.h"

class Thread {
    public:
        Thread ();
        virtual ~Thread ();
        virtual void * Run() = 0; 

        bool Start();
        bool Join();
        bool Detach();
        pthread_t Self();

    private:
        pthread_t tid;
        bool isRunning;
        bool isDetached;

        static void * RunThread(void * arg) {
            return ((Thread *)arg)->Run();
        }
};

#endif /* end of include guard: THREAD_Z0X3KQ1B */
