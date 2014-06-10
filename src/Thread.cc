#include "Thread.h"
#include "Logger.h"

using namespace std;

pthread_t Thread::thread_cnt = 0;

Thread::Thread()
: mTid(0), isRunning(false), isDetached(false)
{
    ++ thread_cnt;
//     if (isDebug)
//         fprintf(outfd, "Add Thread%d\n", thread_cnt);
}

Thread::~Thread()
{
    if (isRunning && (!isDetached))
        pthread_detach(mTid);
    if (isRunning)
        pthread_cancel(mTid);
}

void *
Thread::RunThread_one(void * arg)
{
    return ((Thread *)arg)->Run();
}

void *
Thread::RunThread_two(void * arg)
{
    struct arg_eve * arge = (struct arg_eve *) arg;
    return ((Thread *)arge->thd)->Run(arge->eve);
}


bool
Thread::Start() {
    int res = pthread_create(&mTid, NULL, RunThread_one, this);
    if (res == 0)
        isRunning = true;
    return (res == 0);
}

bool
Thread::Start(event_t eve)
{
    struct arg_eve * arg;
    arg = (struct arg_eve *) malloc(sizeof(struct arg_eve));
    arg->thd = this;
    arg->eve = eve;
    int res = pthread_create(&mTid, NULL, RunThread_two, arg);
    if (res == 0)
        isRunning = true;
    return (res == 0);
}


bool
Thread::Join() {
    int res = -1;
    if (isRunning) {
        res = pthread_join(mTid, NULL);
        if (res == 0)
            isDetached = true;
    }
    return (res == 0);
}

bool
Thread::Detach() {
    int res = -1;
    if (isRunning && (!isDetached) ) {
        res = pthread_detach(mTid);
        if (res == 0) isDetached = true;
    }
    return (res == 0);
}

pthread_t
Thread::Self()
{
    return mTid;
}
