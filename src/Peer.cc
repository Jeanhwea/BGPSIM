#include "Peer.h"

map<state_t, string> Peer::mapStateName = {
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
    IdleHoldTimer = time(NULL);
}

Peer::~Peer()
{
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
