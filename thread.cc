#include "thread.h"

Thread::Thread()
: mTid(0), isRunning(false), isDetached(false)
{
}

Thread::~Thread()
{
    if (isRunning && (!isDetached) ) 
        pthread_detach(mTid);
    if (isRunning)
        pthread_cancel(mTid);
}

bool Thread::Start() {
    int res = pthread_create(&mTid, NULL, RunThread, this);
    if (res == 0) isRunning = true;
    return (res == 0);
}

bool Thread::Join() {
    int res = -1;
    if (isRunning) {
        res = pthread_join(mTid, NULL);
        if (res == 0) isDetached = true;
    }
    return (res == 0);
}

bool Thread::Detach() {
    int res = -1;
    if (isRunning && (!isDetached) ) {
        res = pthread_detach(mTid);
        if (res == 0) isDetached = true;
    }
    return (res == 0);
}

pthread_t Thread::Self() 
{
    return mTid;
}
