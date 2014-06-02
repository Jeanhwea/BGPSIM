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
<<<<<<< HEAD
    
=======
>>>>>>> 2536ea4f0de498bc464fe2ebd412b942354efef5
    TimerBeZero();
}

Peer::~Peer()
{
}
