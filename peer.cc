#include "peer.h"

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
    TimerBeZero();
}

Peer::~Peer()
{
}

bool
Peer::ParseHeader()
{
    return true;
}

bool
Peer::ParseOpen()
{
    return true;
}

bool
Peer::ParseNotification() 
{
    return true;
}

bool
Peer::ParseUpdate() 
{
    return true;
}

bool
Peer::ParseKeepalive() 
{
    return true;
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