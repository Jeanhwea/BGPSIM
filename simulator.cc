#include "simulator.h"

using namespace std;

Simulator::Simulator()
{
}

Simulator::~Simulator()
{
}

void * 
Simulator::Run()
{
    return NULL;
}

void
Simulator::FSM(Peer * pPeer, event_t eve)
{
   switch ( pPeer->GetPeerState() ) {
        case IDLE:
            switch ( eve ) { 
                case BGP_START : 
                    pPeer->TimerBeZero();
                    break;
                default:
                    // ignore
                    break;
            }
            break;
        case CONNECT:
            switch ( eve ) { 
                case BGP_START : 
                    break;
                case BGP_TRANS_CONN_OPEN:
                    SimTCPEstablished(pPeer);
                    SimOpen(pPeer);
                    pPeer->ConnetRetryTimer = 0;
                    ChangeState(pPeer, OPEN_SENT, eve);
                    break;
                case BGP_TRANS_CONN_OPEN_FAILED : 
                    pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    SimColseConnect(pPeer);
                    ChangeState(pPeer, ACTIVE, eve);
                    break;
                case CONN_RETRY_TIMER_EXPIRED : 
                    pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    SimConnect(pPeer);
                    break;
                default:
                    ChangeState(pPeer, IDLE, eve);
                    break;
            }
            break;
        case ACTIVE:
            switch ( eve ) { 
                case BGP_START : 
                    // ignore
                    break;
                case BGP_TRANS_CONN_OPEN : 
                    SimTCPEstablished(pPeer);
                    SimOpen(pPeer);
                    pPeer->ConnetRetryTimer = 0;
                    pPeer->SetHoldtime(T_HOLD_INITIAL);
                    pPeer->StartTimerHoldtime();
                    ChangeState(pPeer, OPEN_SENT, eve);
                    break;
                case BGP_TRANS_CONN_OPEN_FAILED : 
                    pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    SimColseConnect(pPeer);
                    ChangeState(pPeer, CONNECT, eve);
                    break;
                case CONN_RETRY_TIMER_EXPIRED : 
                    pPeer->ConnetRetryTimer = time(NULL) + pPeer->GetHoldtime();
                    ChangeState(pPeer, CONNECT, eve);
                    SimConnect(pPeer);
                    break;
                default:
                    ChangeState(pPeer, IDLE, eve);
                    break;
            }
            break;
        case OPEN_SENT:
            switch ( eve ) { 
                case BGP_START : 
                    // ignore
                    break;
                case BGP_STOP : 
                    SimNotification(pPeer, ERR_CEASE, 0, NULL, 0);
                    pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    ChangeState(pPeer, ACTIVE, eve);
                    break;
                case BGP_TRANS_CONN_CLOSED : 
                    SimColseConnect(pPeer);
                    pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    ChangeState(pPeer, ACTIVE, eve);
                    break;
                case BGP_TRANS_FATAL_ERROR : 
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case HOLD_TIMER_EXPIRED : 
                    SimNotification(pPeer, ERR_HOLDTIMEREXPIRED, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case RECV_OPEN_MSG : 
                    if ( ! pPeer->ParseOpen() ) break;
                    SimKeepalive(pPeer);
                    ChangeState(pPeer, OPEN_CONFIRM, eve);
                    break;
                case RECV_NOTIFICATION_MSG : 
                    if ( ! pPeer->ParseNotification() ) {
                        ChangeState(pPeer, IDLE, eve);
                        pPeer->IdleHoldTimer = time(NULL);
                        pPeer->HoldTimer /= 2;
                    } else {
                        ChangeState(pPeer, IDLE, eve);
                    }
                    break;
                default:
                    SimNotification(pPeer, ERR_FSM, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
            }
            break;
        case OPEN_CONFIRM:
            switch ( eve ) { 
                case BGP_START : 
                    // ignore
                    break;
                case BGP_STOP : 
                    SimNotification(pPeer, ERR_CEASE, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case BGP_TRANS_CONN_CLOSED : 
                case BGP_TRANS_FATAL_ERROR : 
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case KEEPALIVE_TIMER_EXPIRED : 
                    SimKeepalive(pPeer);
                    break;
                case RECV_KEEPALIVE_MSG : 
                    pPeer->StartTimerHoldtime();
                    ChangeState(pPeer, ESTABLISHED, eve);
                    break;
                case RECV_NOTIFICATION_MSG : 
                    pPeer->ParseNotification();
                    ChangeState(pPeer, IDLE, eve);
                    break;
                default:
                    SimNotification(pPeer, ERR_FSM, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
            }
            break;
        case ESTABLISHED:
            switch ( eve ) { 
                case BGP_START : 
                    // ignore
                    break;
                case BGP_STOP : 
                    SimNotification(pPeer, ERR_CEASE, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case BGP_TRANS_CONN_CLOSED : 
                case BGP_TRANS_FATAL_ERROR :
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case HOLD_TIMER_EXPIRED : 
                    SimNotification(pPeer, ERR_HOLDTIMEREXPIRED, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
                case KEEPALIVE_TIMER_EXPIRED : 
                    SimKeepalive(pPeer);
                    break;
                case RECV_KEEPALIVE_MSG :  
                    pPeer->StartTimerHoldtime();
                    break;
                case RECV_UPDATE_MSG : 
                    pPeer->StartTimerHoldtime();
                    if ( ! pPeer->ParseUpdate() )
                        ChangeState(pPeer, IDLE, eve);
                    else 
                        pPeer->StartTimerHoldtime();
                    break;
                case RECV_NOTIFICATION_MSG : 
                    pPeer->ParseNotification();
                    ChangeState(pPeer, IDLE, eve);
                    break;
                default:
                    SimNotification(pPeer, ERR_FSM, 0, NULL, 0);
                    ChangeState(pPeer, IDLE, eve);
                    break;
            }
            break;
    }
}

void 
Simulator::FSM(Peer * pPeer, Event * pEve)
{
    FSM(pPeer, pEve->GetEventType());
}

void 
Simulator::ChangeState(Peer * pPeer, state_t state, event_t eve)
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
    }
    pPeer->SetPeerState(state);
}

void
Simulator::SimTCPEstablished(Peer * pp) 
{
    
}

void
Simulator::SimConnect(Peer * pp) 
{
    
}

void
Simulator::SimColseConnect(Peer * pp) 
{
    
}

void
Simulator::SimOpen(Peer * pp) 
{
    
}

void
Simulator::SimKeepalive(Peer * pp) 
{
    
}

void
Simulator::SimUpdate(Peer * pp, void * data, ssize_t len) 
{
    
}

void
Simulator::SimNotification(Peer * pp, u_int8_t e, u_int8_t es, void * data, ssize_t len) 
{
    
}

bool 
Simulator::InitConn()
{
    sockfd sfd;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sfd >= 0);

    int success;
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_port = htons(BGP_PORT);
    success = inet_pton(AF_INET, "192.168.1.108", &ad.sin_addr);
    assert(success >= 0);
    success = connect(sfd, (struct sockaddr *) &ad,sizeof(ad));
    assert(success >= 0);

    fd = sfd;

    return true;
}


void 
Simulator::SendOpenMsg()
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
