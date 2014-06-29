#include "Dispatcher.h"
#include "Logger.h"
#include "Buffer.h"
#include "Message.h"
#include "Peer.h"
#include "Simulator.h"

#define BUFSIZE_MAX 65536

Dispatcher::Dispatcher()
{
    sfd = -1;
    isReading = false;
    pthread_mutex_init(&mutex, NULL);
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
    if (!isReading) {
        Lock();
        isReading = true;
        Unlock();
        
        ReadMsg();
        
        Lock();
        isReading = false;
        Unlock();
    }

    DispatchMsg();
    //g_log->Tips("end of dispatcher run");
    return NULL;
}

void 
Dispatcher::Lock()
{
    pthread_mutex_lock(&mutex);
}


void 
Dispatcher::Unlock()
{
    pthread_mutex_unlock(&mutex);
}


bool
Dispatcher::ReadMsg()
{
    Peer * pPeer;
    pPeer = g_sim->GetPeerBySockfd(sfd);

    if (pPeer == NULL) 
        return false;

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
        return false;
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
        return false;
    }

    if (pBuf->Length() < MSGSIZE_HEADER) {
        g_log->Warning("dispatch a message with len less than message header");
        return false;
    }

    g_log->Tips("try to dispatch msg");
    g_log->LogDumpMsg(pBuf->ReadPos(), pBuf->Length());

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
    pPeer->Unlock();
    
    return true;
}

bool
Dispatcher::ReadMsg(Peer* pPeer)
{
    u_char      buf[MSGSIZE_MAX];
    int         nread;

    if (pPeer->sfd == -1)
        return false;
    
    nread = read(pPeer->sfd, buf, MSGSIZE_MAX);
    if (nread <= 0 || nread > MSGSIZE_MAX)
        return false;
    assert(nread <= preBuf->LeftSize());
    preBuf->Add(buf, nread);
    
    u_int16_t len;
    Buffer    * pBuf;
    while (preBuf->Length() >= MSGSIZE_HEADER) {
        
        if (! GetMsgLen(preBuf->ReadPos(), len) ) {
            g_log->Error("Dispatcher read error message length");
            return false;
        }
        
        pBuf = new Buffer(len);
        assert(pBuf != NULL);
        pBuf->Add(preBuf->ReadPos(), len);
        preBuf->Skip(len);

        g_log->Tips("dispatcher recv msg");
        g_log->LogDumpMsg(pBuf->ReadPos(), pBuf->Length());
        pPeer->qBuf.push(pBuf);
    }
    
    if (preBuf->Length() == 0)
        preBuf->Clear();
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
