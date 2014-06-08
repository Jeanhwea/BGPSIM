#include "Timer.h"
#include "Simulator.h"

using namespace std;

time_t Timer::interval = 1;
time_t Timer::walltime = 0;

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
    for (vit = mvPeers.begin(); vit != mvPeers.end(); ++vit) {
        pPeer = *vit;
        if (pPeer->KeepaliveTimer >= 0)
            pPeer->KeepaliveTimer ++;
        if (pPeer->IdleHoldTimer >= 0)
            pPeer->IdleHoldTimer ++;
        if (pPeer->ConnetRetryTimer <= time(NULL)) {
            g_sim->FSM(pPeer, CONN_RETRY_TIMER_EXPIRED);
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
        cout << "Tick : " << ++ Timer::walltime << endl;
    alarm(Timer::interval);
    GetInst()->Schedule();
}

