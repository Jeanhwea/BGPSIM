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
    peer_id = ++peer_cnt;
    mutex = PTHREAD_MUTEX_INITIALIZER;
    sfd = -1;
    holdtime = T_HOLD_INITIAL;
    IdleHoldTime = T_IDLE_INITIAL;
    HoldTimer = 0;
    KeepaliveTimer = 0;
    IdleHoldTimer = 0;
    ConnetRetryTimer = 0;
    pDis = NULL;
    mState = IDLE;
}

Peer::~Peer()
{
    if (pDis != NULL)
        delete pDis;
}

void *
Peer::Run()
{
    if (isDebug)
        cout << "Peer_" << peer_id << " Runs ..." << endl;
    if (mState == IDLE)
        g_sim->FSM(this, BGP_START);
    else
        g_log->Warning("Peer try to start with non-IDLE state");
    return NULL;
}

void *
Peer::Run(event_t eve)
{
    g_log->LogPeerEve(this, eve);
    if (isDebug)
        cout << "Peer_" << peer_id << " Runs ..." << endl;
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

bool
Peer::Send()
{
    Message * pMsg = qMsg.front();

    if (pMsg == NULL) {
        g_log->Error("Peer :: send a empty msg");
        return false;
    }

    if (! pMsg->Write(this->sfd, pMsg)) {
        g_log->Error("Peer :: write msg err");
        return false;
    }

    qMsg.pop();

    delete pMsg;
    return true;
}
