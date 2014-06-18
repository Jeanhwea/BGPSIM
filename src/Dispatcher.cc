#include "Dispatcher.h"
#include "Logger.h"
#include "Buffer.h"
#include "Message.h"
#include "Peer.h"
#include "Simulator.h"

#define BUFSIZE_MAX 65536

Dispatcher::Dispatcher()
: sfd(-1), isReading(false)
{
    preBuf = new Buffer(BUFSIZE_MAX);
}

Dispatcher::~Dispatcher()
{
    if (preBuf != NULL)
        delete preBuf;
}

void *
Dispatcher::Run()
{
    if (sfd == -1) {
        g_log->Error("dispatcher's sfd is -1, no valid, STOP");
        return NULL;
    }
    
    isReading = true;
    if (ReadMsg()) {
        DispatchMsg();
    }
    isReading = false;

    return NULL;
}

bool
Dispatcher::isRead()
{
    return isReading;
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

    return ReadMsg(pPeer);
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

    if ( !g_sim->ParseHeader(pPeer, pBuf->ReadPos(), len, type) )
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

bool
Dispatcher::ReadMsg(Peer* pPeer)
{
    u_char      buf[MSGSIZE_MAX];
    int         nread;

    if (pPeer->sfd == -1)
        return false;
    
    while ( preBuf->Length() < MSGSIZE_HEADER ) {
        nread = read(pPeer->sfd, buf, MSGSIZE_MAX);
        if (nread <= 0 || nread > MSGSIZE_MAX)
            return false;
        g_log->Tips("dispatcher recv msg");
        g_log->LogDumpMsg(buf, nread);
        preBuf->Add(buf, nread);
    }
    
    u_int16_t len;
    
    if (! GetMsgLen(preBuf->ReadPos(), len) ) {
        g_log->Tips("Dispatcher read error message length");
        return false;
    }


    Buffer    * pBuf;
    pBuf = new Buffer(len);
    assert(pBuf != NULL);
    pBuf->Add(preBuf->ReadPos(), len);
    preBuf->Skip(len);

    pPeer->qBuf.push(pBuf);
    return true;
}

bool
Dispatcher::GetMsgLen(u_char * data, u_int16_t & len)
{
    u_int16_t olen;
    memcpy(&olen, data + MSGSIZE_HEADER_MARKER, 2);
    len = ntohs(olen);
    
    if (len < 0 || len > MSGSIZE_MAX)  
        return false;
    
    return true;
}
