#include "Peer.h"
#include "Simulator.h"

vector<Peer *>  vPeers;

map<state_t, string> mapStateName = {
    { IDLE, "idle" },
    { CONNECT, "connect" },
    { ACTIVE, "active" },
    { OPEN_SENT, "open_sent" },
    { OPEN_CONFIRM, "open_confirm" },
    { ESTABLISHED, "established" }
};

Peer::Peer()
{
    HoldTimer = -1;
    KeepaliveTimer = -1;
    ConnetRetryTimer = -1;
    sfd = -1;
    rbuf = NULL;
    wbuf = NULL;
    mState = IDLE;
}

Peer::~Peer()
{
    if (rbuf != NULL)
        delete rbuf;
    if (wbuf != NULL)
        delete wbuf;
}

void *
Peer::Run()
{
    g_log->Tips("Peer runs ...");
    if (isDebug)
        cout << "Peer_" << Self() << " Runs ..." << endl;
    if (mState == IDLE)
        g_sim->FSM(this, BGP_START);
    else
        g_log->Warning("Peer try start with non-IDLE state");
    return NULL;
}

void *
Peer::Run(event_t eve)
{
    g_log->LogPeerEve(eve);
    if (isDebug)
        cout << "Peer_" << Self() << " Runs ..." << endl;
    g_sim->FSM(this, eve);
    return NULL;
}

void
Peer::InitTimer()
{
    HoldTimer = 0;
    KeepaliveTimer = 0;
    IdleHoldTimer = time(NULL);
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
