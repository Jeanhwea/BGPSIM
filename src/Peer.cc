#include "Peer.h"
#include "Simulator.h"

vector<Peer *>  vPeers;
int Peer::peer_cnt = 0;

map<state_t, string> mapStateName = {
    { IDLE, "idle" },
    { CONNECT, "connect" },
    { ACTIVE, "active" },
    { OPENSENT, "opensent" },
    { OPENCONFIRM, "openconfirm" },
    { ESTABLISHED, "established" }
};

Peer::Peer()
{
    peerid = ++peer_cnt;
    mutex = PTHREAD_MUTEX_INITIALIZER;
    sfd = -1;
    holdtime = T_HOLD_INITIAL;
    IdleHoldTime = T_IDLE_INITIAL;
    HoldTimer = 0;
    KeepaliveTimer = 0;
    IdleHoldTimer = 0;
    ConnetRetryTimer = 0;
    //rbuf = NULL;
    //wbuf = NULL;
    mState = IDLE;
}

Peer::~Peer()
{
//     if (rbuf != NULL)
//         delete rbuf;
//     if (wbuf != NULL)
//         delete wbuf;
}

void *
Peer::Run()
{
    if (isDebug)
        cout << "Peer_" << peerid << " Runs ..." << endl;
    if (mState == IDLE)
        g_sim->FSM(this, BGP_START);
    else
        g_log->Warning("Peer try to start with non-IDLE state");
    return NULL;
}

void *
Peer::Run(event_t eve)
{
    g_log->LogPeerEve(eve);
    if (isDebug)
        cout << "Peer_" << peerid << " Runs ..." << endl;
    g_sim->FSM(this, eve);
    return NULL;
}

void
Peer::StartTimerHoldtime() {
    if ( holdtime > 0)
        HoldTimer = time(NULL) + holdtime;
    else
        HoldTimer = 0;
}

void
Peer::StartTimerKeepalive() {
    if ( holdtime > 0)
        KeepaliveTimer = time(NULL) + holdtime / 3;
    else
        KeepaliveTimer = 0;
}

void
Peer::Lock()
{
    pthread_mutex_lock(&mutex);
}

void
Peer::UnLock()
{
    pthread_mutex_unlock(&mutex);
}
