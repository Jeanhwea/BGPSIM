#include "simulator.h"

using namespace std;


Simulator::Simulator()
{
}

Simulator::~Simulator()
{
}

void * Simulator::Run()
{
    return NULL;
}

void Simulator::FSM(Peer * pPeer, Event * pEve)
{
    switch ( pPeer->GetPeerState() ) {
        case IDLE:
            switch ( pEve->GetEventType() ) { 
                case BGP_START : 
                    break;
                default:
                    break;
            }
            break;
        case CONNECT:
            break;
        case ACTIVE:
            break;
        case OPEN_SENT:
            break;
        case OPEN_CONFIRM:
            break;
        case ESTABLISHED:
            break;
        default:
            // do nothing
            break;
    }
}

void Simulator::ChangeState(Peer * pPeer, Event * pEve, state_t state)
{
    switch ( pPeer->GetPeerState() ) {
        case IDLE:
            break;
        case CONNECT:
            break;
        case ACTIVE:
            break;
        case OPEN_SENT:
            break;
        case OPEN_CONFIRM:
            break;
        case ESTABLISHED:
            break;
        default:
            // do nothing
            break;
    }
    pPeer->SetPeerState(state);
}

bool Simulator::InitConn()
{
    sockfd sfd;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sfd >= 0);

    int success;
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_port = htons(BGP_PORT);
    success = inet_pton(AF_INET, "192.168.1.102", &ad.sin_addr);
    assert(success >= 0);
    success = connect(sfd, (struct sockaddr *) &ad,sizeof(ad));
    assert(success >= 0);

    fd = sfd;

    return true;
}


void Simulator::SendOpenMsg()
{
    sockfd sfd;

    if (fd < 0) return;
    sfd = fd;

    mpMsg = new Message;
    mpMsg->InitOpenMsg(100, 5, 0xc0a80164);
    mpMsg->SendMsg(sfd);
    mpMsg->DumpSelf();
    
    free(mpMsg);
}
