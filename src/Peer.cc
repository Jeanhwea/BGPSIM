#include "Peer.h"

vector<Peer *>  mvPeers;

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
    sfd = -1;
    rbuf = NULL;
    wbuf = NULL;
    mState = IDLE;
    Init();
}

Peer::~Peer()
{
}

void *
Peer::Run()
{
    return NULL;
}

void 
Peer::Init()
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

#define INIT_MSG_LEN 65535

void
Peer::InitWbuf() {
    wbuf = new Message(INIT_MSG_LEN);
}