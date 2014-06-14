#include "Dispatcher.h"
#include "Logger.h"
#include "Message.h"
#include "Peer.h"
#include "Simulator.h"

Dispatcher::Dispatcher()
{
    sfd = -1;
}

Dispatcher::~Dispatcher()
{

}

void *
Dispatcher::Run()
{
    if (sfd == -1) {
        g_log->Error("dispatcher's sfd is -1, no valid, STOP");
        return NULL;
    }

    if (ReadMsg()) {
        DispatchMsg();
    }

    return NULL;
}

bool
Dispatcher::ReadMsg()
{
    Peer * pPeer;
    pPeer = g_sim->GetPeerBySockfd(sfd);

    if (pPeer == NULL) {
        //g_log->Warning("dispatch cannot get a peer with given sockfd");
        return false;
    }

    if (pPeer->sfd == -1)
        return false;

    return g_sim->SimRecvMsg(pPeer);
}

bool
Dispatcher::DispatchMsg()
{
    Peer * pPeer;
    pPeer = g_sim->GetPeerBySockfd(sfd);

    if (pPeer == NULL) {
        g_log->Warning("dispatch cannot get a peer with given sockfd");
    }

    return DispatchMsg(pPeer);
}


bool
Dispatcher::DispatchMsg(Peer * pPeer)
{
    assert(pPeer != NULL);
    Buffer * pBuf;

    pBuf = pPeer->qBuf.front();

    if (pBuf == NULL) {
        g_log->Warning("no buf to dispatch");
        return false;
    }

    if (pBuf->Length() < MSGSIZE_HEADER) {
        g_log->Warning("dispatch a message with len less than message header");
        return false;
    }

    u_int16_t len;
    u_int8_t type;

    if ( !g_sim->ParseHeader(pPeer, pBuf->data, len, type) )
        return false;

    g_log->LogDispatchMsg(len, type);

    pPeer->Lock();
    switch (type) {
        case OPEN:
            pPeer->Start(RECV_OPEN_MSG);
            break;
        case UPDATE:
            pPeer->Start(RECV_UPDATE_MSG);
            break;
        case NOTIFICATION:
            pPeer->Start(RECV_NOTIFICATION_MSG);
            break;
        case KEEPALIVE:
            pPeer->Start(RECV_KEEPALIVE_MSG);
            break;
        default:
            g_log->Error("dispatch non-valid message type");
    }
    pPeer->UnLock();

    return true;
}
