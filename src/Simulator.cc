#include "Simulator.h"

#define INIT_MSG_LEN 65535
using namespace std;

Simulator::Simulator()
{
    mQuit = false;
    LoadSimConf("/home/fuzl/.peer.conf");

    Peer * pPeer;
    vector<struct sim_config>::iterator sit;
    for (sit = vPeerConf.begin(); sit != vPeerConf.end(); ++sit) {
        pPeer = new Peer();
        pPeer->conf.passive = false;
        pPeer->conf.remote_as = sit->as;
        memcpy(& pPeer->conf.remote_addr, & sit->ipaddr, sizeof(sit->ipaddr));
        mvPeers.push_back(pPeer);
    }
    cout << "initail peer size = " << mvPeers.size() << endl;
}

Simulator::~Simulator()
{
}

void *
Simulator::Run()
{
    tim.Start();
    tim.Join();
    lis.Start();
    lis.Join();
    SimMain();
    return NULL;
}


void
Simulator::SimMain()
{
    vector<Peer *>::iterator vit;
    Peer * pPeer;
    cout << "in sim main" << endl;
    for (vit = mvPeers.begin(); vit != mvPeers.end(); ++vit) {
        pPeer = *vit;
        pPeer->Start();
    }
    while (mQuit == false) {
    }
}

void
Simulator::FSM(Peer * pPeer, event_t eve)
{
    switch ( pPeer->GetPeerState() ) {
        case IDLE:
            switch ( eve ) {
                case BGP_START :
                    pPeer->InitTimer();
                    pPeer->rbuf = new Buffer();
                    pPeer->wbuf = new Message(MSGBUFSIZE);

                    if (pPeer->conf.passive) {
                        ChangeState(pPeer, ACTIVE, eve);
                        pPeer->ConnetRetryTimer = 0;
                    } else {
                        ChangeState(pPeer, CONNECT, eve);
                        SimConnect(pPeer);
                        pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    }
                    break;
                default:
                    // ignore other event type
                    break;
            }
            break;
        case CONNECT:
            switch ( eve ) {
                case BGP_START :
                    // ignore
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
                    pPeer->holdtime = T_HOLD_INITIAL;
                    pPeer->StartTimerHoldtime();
                    ChangeState(pPeer, OPEN_SENT, eve);
                    break;
                case BGP_TRANS_CONN_OPEN_FAILED :
                    pPeer->ConnetRetryTimer = time(NULL) + T_CONNRETRY;
                    SimColseConnect(pPeer);
                    ChangeState(pPeer, CONNECT, eve);
                    break;
                case CONN_RETRY_TIMER_EXPIRED :
                    pPeer->ConnetRetryTimer = time(NULL) + pPeer->holdtime;
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
                    if ( ! ParseOpen(pPeer) ) break;
                    SimKeepalive(pPeer);
                    ChangeState(pPeer, OPEN_CONFIRM, eve);
                    break;
                case RECV_NOTIFICATION_MSG :
                    if ( ! ParseNotification(pPeer) ) {
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
                    ParseNotification(pPeer);
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
                    if ( ! ParseUpdate(pPeer) )
                        ChangeState(pPeer, IDLE, eve);
                    else
                        pPeer->StartTimerHoldtime();
                    break;
                case RECV_NOTIFICATION_MSG :
                    ParseNotification(pPeer);
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
    g_log->LogStateChage(pPeer->GetPeerState(), state, eve);
    pPeer->SetPeerState(state);
}

bool
Simulator::SimSetupSocket(Peer * pPeer)
{
    int ttl = 2;
    if (setsockopt(pPeer->sfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) == -1) {
        g_log->Warning("failed to set TTL");
        return false;
    }
    int nodelay = 1;
    if (setsockopt(pPeer->sfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) == -1) {
        g_log->Warning("failed to set TCP nodelay");
        return false;
    }
    return true;
}

void
Simulator::SimTCPEstablished(Peer * pPeer)
{
    socklen_t len;

    len = sizeof(pPeer->sa_local);
    if (getsockname(pPeer->sfd, (struct sockaddr *) & pPeer->sa_local, &len) == -1)
        g_log->Warning("getsockname failed");
    len = sizeof(pPeer->sa_remote);
    if (getpeername(pPeer->sfd, (struct sockaddr *) & pPeer->sa_remote, &len) == -1)
        g_log->Warning("getpeername failed");
}

bool
Simulator::SimConnect(Peer * pPeer)
{
    struct sockaddr_in sad;
    if (pPeer->sfd != -1)
        return false;

    pPeer->sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (pPeer->sfd == -1) {
        g_log->Warning("Simulator socket connect");
        FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
        return false;
    }

    pPeer->wbuf->sfd = pPeer->sfd;

    if (!SimSetupSocket(pPeer)) {
        FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
        g_log->Warning("SimConnect sim setup failed");
        return false;
    }

    UnsetBlock(pPeer->sfd);

    sad.sin_addr = pPeer->conf.remote_addr;
    sad.sin_family = AF_INET;
    sad.sin_port = htons(BGP_PORT);
    if (connect(pPeer->sfd, (sockaddr *) &sad, sizeof(sad)) == -1) {
        if (errno != EINPROGRESS) {
            g_log->Warning("peer connect failed");
            FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
            return false;
        } else {
            g_log->Tips("peer connect in progress...");
        }
    } else {
        FSM(pPeer, BGP_TRANS_CONN_OPEN);
    }

    return true;
}

void
Simulator::SimColseConnect(Peer * pPeer)
{
    if (pPeer->sfd != -1) {
        shutdown(pPeer->sfd, SHUT_RDWR);
        close(pPeer->sfd);
    }
    pPeer->sfd = -1;
    pPeer->wbuf->sfd = -1;
}

void
Simulator::SimOpen(Peer * pPeer)
{
    struct openmsg     msg;
    u_int16_t   len;

    len = MSGSIZE_OPEN_MIN;
    memset(msg.msghdr.marker, 0xff, sizeof(msg.msghdr.marker));
    msg.msghdr.length = htons(len);
    msg.msghdr.type = OPEN;
    msg.version = BGP_VERSION;
    msg.myas = htons(conf_as);
    if (pPeer->holdtime > 0)
        msg.holdtime = htons(pPeer->holdtime);
    else
        msg.holdtime = htons(conf_holdtime);
    msg.bgpid = conf_bgpid;
    msg.optparamlen = 0;

    pPeer->wbuf->Add(&msg, sizeof(msg));
}

void
Simulator::SimKeepalive(Peer * pPeer)
{

}

void
Simulator::SimUpdate(Peer * pPeer, void * data, ssize_t len)
{

}

void
Simulator::SimNotification(Peer * pPeer, u_int8_t e, u_int8_t es, void * data, ssize_t len)
{

}

Peer *
Simulator::GetPeerByAddr(struct in_addr * addr)
{
    vector<Peer *>::iterator vit;
    Peer * pPeer, * ret;
    ret = NULL;
    for (vit = mvPeers.begin(); vit != mvPeers.end(); ++vit) {
        pPeer = *vit;
        if (memcmp(& pPeer->conf.remote_addr.s_addr, &addr->s_addr, sizeof(addr->s_addr)) == 0) {
            ret = pPeer;
            break;
        }
    }

    return ret;
}

bool
Simulator::ParseHeader(Peer * pPeer, u_char & data, u_int16_t & len, u_int8_t & type)
{
    u_char    * pos;
    u_char      one = 0xff;
    u_int16_t   olen;

    // parse at least 19 bytes, we do not check it right now
    pos = &data;
    for (int i = 0; i < MSGSIZE_HEADER_MARKER; ++i) { // marker
        if (memcmp(pos, &one, 1)) {
            g_log->Warning("sync error in parse header");
            SimNotification(pPeer, ERR_HEADER, ERR_HDR_SYNC, NULL, 0);
            FSM(pPeer, CONN_RETRY_TIMER_EXPIRED);
            return false;
        }
        ++pos;
    }
    memcpy(&olen, pos, 2);
    len = ntohs(olen);
    pos += 2;
    memcpy(&type, pos, 1);

    if (len < MSGSIZE_HEADER || len > MAX_PKTSIZE) {
        fprintf(errfd, "received MESSAGE: illegal length: %u bytes\n", len);
        SimNotification(pPeer, ERR_HEADER, ERR_HDR_LEN, &olen, sizeof(olen));
        return false;
    }

    switch (type) {
        case OPEN :
            if (len < MSGSIZE_OPEN_MIN) {
                fprintf(errfd, "received OPEN: illegal length: %u byte\n", len);
                SimNotification(pPeer, ERR_HEADER, ERR_HDR_LEN, &olen, sizeof(olen));
                return false;
            }
            break;
        case NOTIFICATION:
            if (len < MSGSIZE_NOTIFICATION_MIN) {
                fprintf(errfd, "received NOTIFICATION: illegal length: %u byte\n", len);
                SimNotification(pPeer, ERR_HEADER, ERR_HDR_LEN, &olen, sizeof(olen));
                return false;
            }
            break;
        case UPDATE:
            if (len < MSGSIZE_UPDATE_MIN) {
                fprintf(errfd, "received UPDATE: illegal length: %u byte\n", len);
                SimNotification(pPeer, ERR_HEADER, ERR_HDR_LEN, &olen, sizeof(olen));
                return false;
            }
            break;
        case KEEPALIVE:
            if (len != MSGSIZE_KEEPALIVE) {
                fprintf(errfd, "received KEEPALIVE: illegal length: %u byte\n", len);
                SimNotification(pPeer, ERR_HEADER, ERR_HDR_LEN, &olen, sizeof(olen));
                return false;
            }
            break;
        default:
            fprintf(errfd, "received msg with unknown type %u\n", type);
            SimNotification(pPeer, ERR_HEADER, ERR_HDR_TYPE, &type, 1);
            return false;
    }
    return true;
}

bool
Simulator::ParseOpen(Peer * pPeer)
{
    u_char    * pos;
    // u_char    * op_val;
    u_int8_t    version, rversion;
    u_int16_t   as, msglen;
    u_int16_t   holdtime, oholdtime, myholdtime;
    u_int32_t   bgpid;
    u_int8_t    optparamlen, plen;
    // u_int8_t    op_type, op_len;

    pos = pPeer->rbuf->rptr;
    pos += MSGSIZE_HEADER_MARKER;
    memcpy(&msglen, pos, sizeof(msglen));
    msglen = ntohs(msglen);

    pos = pPeer->rbuf->rptr;
    pos += MSGSIZE_HEADER;

    memcpy(&version, pos, sizeof(version));
    pos += sizeof(version);
    if (version != BGP_VERSION) {
        fprintf(errfd, "Peer wants unrecognized version %u\n", version);
        if (version > BGP_VERSION)
            rversion = version - BGP_VERSION;
        else
            rversion = BGP_VERSION;
        SimNotification(pPeer, ERR_OPEN, ERR_OPEN_VERSION, &rversion, sizeof(rversion));
        ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
        return false;
    }

    memcpy(&as, pos, sizeof(as));
    pos += sizeof(as);
    if (pPeer->conf.remote_as != ntohs(as)) {
        fprintf(errfd, "peer sent wrong AS %u\n", ntohs(as));
        SimNotification(pPeer, ERR_OPEN, ERR_OPEN_AS, NULL, 0);
        ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
        return false;
    }

    memcpy(&oholdtime, pos, sizeof(oholdtime));
    pos += sizeof(oholdtime);
    holdtime = ntohs(oholdtime);
    if (holdtime && holdtime < pPeer->conf.min_holdtime) {
        fprintf(errfd, "peer requests unacceptable holdtime %u\n", holdtime);
        SimNotification(pPeer, ERR_OPEN, ERR_OPEN_HOLDTIME, NULL, 0);
        ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
        return false;
    }
    myholdtime = pPeer->conf.holdtime;
    if (myholdtime <= 0)
        myholdtime = conf_holdtime;
    if (holdtime < myholdtime)
        pPeer->holdtime = holdtime;
    else
        pPeer->holdtime = myholdtime;

    memcpy(&bgpid, pos, sizeof(bgpid));
    pos += sizeof(bgpid);
    pPeer->conf.remote_bgpid = bgpid;

    memcpy(&optparamlen, pos, sizeof(optparamlen));
    pos += sizeof(optparamlen);
    if (optparamlen > msglen - MSGSIZE_OPEN_MIN) {
        fprintf(errfd, "corrupt OPEN message received: length mismatch\n");
        SimNotification(pPeer, ERR_OPEN, 0, NULL, 0);
        ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
        return false;
    }

    plen = optparamlen;
    while (plen > 0) {
        if (plen < 2) {
            fprintf(errfd, "corrupt OPEN message received: length mismatch\n");
            SimNotification(pPeer, ERR_OPEN, 0, NULL, 0);
            ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
            return false;
        }
        // TODO: add ParseCapabilities ....
    }

    return true;
}

bool
Simulator::ParseNotification(Peer * pPeer)
{
    u_char    * pos;
    u_int8_t    errcode;
    u_int8_t    subcode;
    u_int16_t   datalen;
    // u_int8_t    capa_code, capa_len;

    pos = pPeer->rbuf->rptr;
    pos += MSGSIZE_HEADER_MARKER;
    memcpy(&datalen, pos, sizeof(datalen));
    datalen = ntohs(datalen);

    pos = pPeer->rbuf->rptr;
    pos += MSGSIZE_HEADER;
    datalen -= MSGSIZE_HEADER;


    memcpy(&errcode, pos, sizeof(errcode));
    pos += sizeof(errcode);
    datalen -= sizeof(errcode);

    memcpy(&subcode, pos, sizeof(subcode));
    pos += sizeof(subcode);
    datalen -= sizeof(subcode);

    return true;
}

bool
Simulator::ParseUpdate(Peer * pPeer)
{
    return true;
}

bool
Simulator::ParseKeepalive(Peer * pPeer)
{
    return true;
}

bool
Simulator::SetBlock(sockfd sfd)
{
    return lis.SetBlock(sfd);
}

bool
Simulator::UnsetBlock(sockfd sfd)
{
    return lis.UnsetBlock(sfd);
}

bool
Simulator::LoadSimConf(const char * filename)
{
    FILE          * ffd;
    int             as;
    char            addr[32];
    struct in_addr  inad;
    sim_config    * sconf;

    ffd = fopen(filename, "r");
    if (ffd == NULL) {
        g_log->Error("cannot open sim config file");
        return false;
    }

    fscanf(ffd, "%d", &as);
    conf_as = htons(as);
    if (isDebug)
        fprintf(outfd, "Local Simulator in AS%d\n", as);
    while (fscanf(ffd, "%d%s", &as, addr) != EOF) {
        if( !inet_aton(addr, &inad) ) {
            g_log->Warning("no a valid ip");
        }
        sconf = (sim_config *) malloc(sizeof(sim_config));
        sconf->as = htons(as);
        sconf->bgpid = htonl((u_int32_t) inad.s_addr);
        memcpy(&sconf->ipaddr, &inad, sizeof(inad));
        vPeerConf.push_back(*sconf);
        if (isDebug)
            fprintf(outfd, "Add peer config AS%d, %s\n", as, addr);
    }
    fclose(ffd);
    return true;
}