#include "simulator.h"

using namespace std;

map<State_t, string> Simulator::mapStateName = {
    { IDLE, "idle" },
    { CONNECT, "connect" },
    { ACTIVE, "active" },
    { OPEN_SENT, "open_sent" },
    { OPEN_CONFIRM, "open_confirm" },
    { ESTABLISHED, "established" }
};

Simulator::Simulator()
: curState(IDLE)
{

}

Simulator::~Simulator()
{
}

void * Simulator::Run()
{
    return NULL;
}

void Simulator::FSM(Event * pEve, Simulator * sim)
{
    switch ( sim->GetCurState() ) {
        case IDLE:
            IdleStateHandler(pEve);
            break;
        case CONNECT:
            ConnectStateHandler(pEve);
            break;
        case ACTIVE:
            ActiveStateHandler(pEve);
            break;
        case OPEN_SENT:
            OpenSendStateHandler(pEve);
            break;
        case OPEN_CONFIRM:
            OpenConfirmStateHandler(pEve);
            break;
        case ESTABLISHED:
            EstablishedStateHandler(pEve);
            break;
    }
}

void Simulator::FSM(Event * pEve)
{
    FSM(pEve, this);
}

void Simulator::IdleStateHandler(Event * event)
{
    switch (event->GetEventType()) {
        case BGP_START:
            break;
        default:
            break;
    }

}

void Simulator::ConnectStateHandler(Event * event)
{
    // switch (event->GetEventType()) { 
    //     case BGP_START : 
    //         break;
    //     default:
    //         break;
    // }
}

void Simulator::ActiveStateHandler(Event * event)
{
}

void Simulator::OpenSendStateHandler(Event * event)
{
}

void Simulator::OpenConfirmStateHandler(Event * event)
{
}

void Simulator::EstablishedStateHandler(Event * event)
{
}

bool Simulator::InitConn()
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    int success;
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_port = htons(BGP_PORT);
    success = inet_pton(AF_INET, "192.168.1.102", &ad.sin_addr);
    assert(success >= 0);
    success = connect(sockfd, (struct sockaddr *) &ad,sizeof(ad));
    assert(success >= 0);

    vSockFd.push_back(sockfd);

    return true;
}


void Simulator::SendOpenMsg()
{
    int sockfd;

    if (vSockFd.empty()) return;
    sockfd = vSockFd[0];

    pMsg = new Message;
    pMsg->InitOpenMsg(100, 5, 8);
    pMsg->SendMsg(sockfd);
    pMsg->DumpSelf();
    
    free(pMsg);
}
