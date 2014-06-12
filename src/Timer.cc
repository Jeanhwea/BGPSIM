#include "Timer.h"
#include "Simulator.h"

using namespace std;

time_t walltime = 0;
time_t Timer::interval = 1;

// all timer shares this one instance
Timer * Timer::instance = NULL;
int Timer::ref_times = 0;

Timer::Timer()
{
    ++ref_times;
}

Timer::~Timer()
{
    --ref_times;
    if ( (NULL!=instance) && (0==ref_times) )
        delete instance;
}

void *
Timer::Run()
{
    if (NULL == instance) {
        // set timer at the very beginning
        alarm(interval);
        signal(SIGALRM, TimerHandler);
    }
    return NULL;
}

void
Timer::Schedule()
{
    vector<Peer *>::iterator vit;
    Peer * pPeer;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++vit) {
        pPeer = *vit;
        if (pPeer->KeepaliveTimer >= 0)
            pPeer->KeepaliveTimer ++;
        if (pPeer->IdleHoldTimer >= 0)
            pPeer->IdleHoldTimer ++;
        if (pPeer->ConnetRetryTimer > 0 &&
                pPeer->ConnetRetryTimer <= time(NULL)) {
            pPeer->Lock();
            pPeer->Start(CONN_RETRY_TIMER_EXPIRED);
            pPeer->UnLock();
        }
    }
}

Timer *
Timer::GetInst()
{
    if (NULL == instance)
        instance = new Timer;
    return instance;
}

void
Timer::TimerHandler(int sig)
{
    if (isDebug)
        cout << "Tick\t: " << ++ walltime << endl;
    alarm(Timer::interval);
    GetInst()->Schedule();
}

