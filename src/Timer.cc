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
Timer::DoSchedule()
{
    vector<Peer *>::iterator vit;
    Peer * pPeer;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++vit) {
        pPeer = *vit;
        if (IsExpire(pPeer->KeepaliveTimer)) {
            pPeer->Lock();
            if (pPeer->GetPeerState() == OPENCONFIRM ||
                    pPeer->GetPeerState() == ESTABLISHED ) {
                pPeer->Start(KEEPALIVE_TIMER_EXPIRED);
            }
            pPeer->Unlock();
        }
        if (IsExpire(pPeer->HoldTimer)) {
            pPeer->Lock();
            if (pPeer->GetPeerState() == OPENSENT ||
                    pPeer->GetPeerState() == ESTABLISHED )
                pPeer->Start(HOLD_TIMER_EXPIRED);
            pPeer->Unlock();
        }
        if (IsExpire(pPeer->ConnetRetryTimer)) {
            pPeer->Lock();
            if (pPeer->GetPeerState() == ACTIVE)
                pPeer->Start(CONN_RETRY_TIMER_EXPIRED);
            pPeer->Unlock();
        }
        if (IsExpire(pPeer->IdleHoldTimer)) {
            pPeer->Lock();
            pPeer->Start();
            pPeer->Unlock();
        }
    }
    g_sim->DoDispatch();
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
    GetInst()->DoSchedule();
}

bool
Timer::IsExpire(time_t timer)
{
    if (timer > 0 && timer <= time(NULL))
        return true;
    return false;
}

