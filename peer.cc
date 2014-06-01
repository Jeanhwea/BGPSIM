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
