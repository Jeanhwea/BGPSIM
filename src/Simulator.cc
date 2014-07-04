#include "Simulator.h"
#include "Interface.h"
#include "Router.h"

#define INIT_MSG_LEN 65535
using namespace std;


Simulator::Simulator()
{
    mQuit = false;
    conf_holdtime = T_HOLD_INITIAL;
    LoadSimConf("./config/peer.conf");
    //LoadListConf("./config/listen.conf");
    LoadListConf();

    // instantiation of listeners
    vector<struct in_addr>::iterator lit;
    Listener * pListener;
    for (lit = vLisAddr.begin(); lit != vLisAddr.end(); ++lit) {
        pListener = new Listener(&(*lit));
        vListeners.push_back(pListener);
    }
    g_log->LogListenerList();

    // instantiation of a peers list
    vector<struct sim_config>::iterator sit;
    Peer * pPeer;
    for (sit = vPeerConf.begin(); sit != vPeerConf.end(); ++sit) {
        pPeer = new Peer();
        pPeer->conf.passive = false;
        pPeer->conf.remote_as = sit->as;
        memcpy(& pPeer->conf.remote_addr, & sit->raddr, sizeof(sit->raddr));
        vPeers.push_back(pPeer);
    }
    g_log->LogPeerList();
}

Simulator::~Simulator()
{
}

void *
Simulator::Run()
{
    SimMain();
    return NULL;
}

void
Simulator::SimMain()
{
    if (isDebug)
        cout << "Start simulator main" << endl;
    vector<Listener *>::iterator lit;
    Listener * pListener;
    for (lit = vListeners.begin(); lit != vListeners.end(); ++lit) {
        pListener = *lit;
        pListener->Start();
    }
    vector<Peer *>::iterator vit;
    Peer * pPeer;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++vit) {
        pPeer = *vit;
        pPeer->Start();
    }
    while (mQuit == false) {
        // simulator main loop
    }
}

void
Simulator::FSM(Peer * pPeer, event_t eve)
{
    switch ( pPeer->GetPeerState() ) {
        case IDLE:
            switch ( eve ) {
                case BGP_START :
                    pPeer->IdleHoldTimer = 0;
                    pPeer->HoldTimer = 0;
                    pPeer->KeepaliveTimer = 0;

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
                    ChangeState(pPeer, OPENSENT, eve);
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
                    ChangeState(pPeer, OPENSENT, eve);
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
        case OPENSENT:
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
                    ChangeState(pPeer, OPENCONFIRM, eve);
                    break;
                case RECV_NOTIFICATION_MSG :
                    if ( ! ParseNotification(pPeer) ) {
                        ChangeState(pPeer, IDLE, eve);
                        pPeer->IdleHoldTimer = time(NULL);
                        pPeer->IdleHoldTime /= 2;
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
        case OPENCONFIRM:
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
                    if (!ParseKeepalive(pPeer)) {
                        ChangeState(pPeer, IDLE, eve);
                    } else {
                        ChangeState(pPeer, ESTABLISHED, eve);
                    }
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
                    if (! ParseKeepalive(pPeer))
                        ChangeState(pPeer, IDLE, eve);
                    else
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
    // no
    FSM(pPeer, pEve->GetEventType());
}

void
Simulator::ChangeState(Peer * pPeer, state_t state, event_t eve)
{
    g_log->LogStateChage(pPeer, state, eve);
    switch (state) {
        case IDLE:
            if (pPeer->GetPeerState() != IDLE) {
                pPeer->ConnetRetryTimer = 0;
            }
            if (pPeer->IdleHoldTime == 0)
                pPeer->IdleHoldTime = T_IDLE_INITIAL;
            pPeer->holdtime = T_HOLD_INITIAL;
            pPeer->KeepaliveTimer = 0;
            pPeer->HoldTimer = 0;
            pPeer->ConnetRetryTimer = 0;
            SimColseConnect(pPeer);
            if (eve != BGP_STOP) {
                pPeer->IdleHoldTimer = time(NULL) + pPeer->IdleHoldTime;
                pPeer->IdleHoldTime *= 2;
            }
            break;
        case CONNECT:
            break;
        case ACTIVE:
            break;
        case OPENSENT:
            break;
        case OPENCONFIRM:
            break;
        case ESTABLISHED:
            // once ESTABLISHED state reached, advertise update message first.
            AdvertUpdate(pPeer);
            break;
    }
    pPeer->SetPeerState(state);
}

bool
Simulator::SimSetupSocket(Peer * pPeer)
{
    if (!Listener::SetTTL(pPeer->sfd, 64))
        return false;
    if (!Listener::SetNonBlock(pPeer->sfd))
        return false;
    return true;
}

void
Simulator::SimTCPEstablished(Peer * pPeer)
{
    socklen_t len;

    assert(pPeer->sfd != -1);
    len = sizeof(pPeer->sa_local);
    if (getsockname(pPeer->sfd, (struct sockaddr *) & pPeer->sa_local,
            &len) == -1)
        g_log->Warning("getsockname failed");
    len = sizeof(pPeer->sa_remote);
    if (getpeername(pPeer->sfd, (struct sockaddr *) & pPeer->sa_remote,
            &len) == -1)
        g_log->Warning("getpeername failed");
}

bool
Simulator::SimConnect(Peer * pPeer)
{
    struct sockaddr_in sad;
    if (pPeer->sfd != -1)
        return false;

    pPeer->sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (pPeer->sfd == -1) {
        g_log->Warning("Simulator socket connect");
        FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
        return false;
    }

    if (!SimSetupSocket(pPeer)) {
        g_log->Warning("SimConnect sim setup failed");
        FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
        return false;
    }

    memcpy(&sad.sin_addr, &pPeer->conf.remote_addr, sizeof(sad.sin_addr));
    sad.sin_family = AF_INET;
    sad.sin_port = htons(BGP_PORT);

    cout << "try connect ";
    g_log->ShowIPAddr(&sad);
    if (connect(pPeer->sfd, (struct sockaddr *) &sad, sizeof(sad)) < 0) {
        FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
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
    if (pPeer->pDis != NULL) {
        pPeer->pDis->Detach();
    }
}

void
Simulator::SimOpen(Peer * pPeer)
{
    struct openmsg  msg;
    u_int16_t       len;

    len = MSGSIZE_OPEN_MIN;
    memset(msg.msghdr.marker, 0xff, sizeof(msg.msghdr.marker));
    msg.msghdr.length = htons(len);
    msg.msghdr.type = OPEN;
    msg.version = BGP_VERSION;
    msg.myas = conf_as; // already in network order
    if (pPeer->holdtime > 0)
        msg.holdtime = htons(pPeer->holdtime);
    else
        msg.holdtime = htons(conf_holdtime);
    msg.bgpid = conf_bgpid; // already in network order
    msg.optparamlen = 0;

    Message * pMsg;
    pMsg = new Message(MSGSIZE_MAX);
    if (pMsg == NULL) {
        g_log->Warning("open msg malloc failed");
        g_sim->FSM(pPeer, BGP_TRANS_CONN_OPEN_FAILED);
        return ;
    }
    pMsg->sfd = pPeer->sfd;
    pMsg->Add(&msg, sizeof(msg));
    pPeer->qMsg.push(pMsg);
    if( !pPeer->Send() ) {
        g_log->Error("simopen, failed to send.");
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
    }
    g_log->Tips("Sim open msg send successful");
}

void
Simulator::SimKeepalive(Peer * pPeer)
{
    struct bgphdr   hdr;
    u_int16_t       len;

    len = MSGSIZE_KEEPALIVE;
    memset(hdr.marker, 0xff, sizeof(hdr.marker));
    hdr.length = htons(len);
    hdr.type = KEEPALIVE;

    Message * pMsg;
    pMsg = new Message(MSGSIZE_KEEPALIVE);
    if (pMsg == NULL) {
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
        return;
    }
    pMsg->sfd = pPeer->sfd;
    pMsg->Add(&hdr, sizeof(hdr));
    pPeer->qMsg.push(pMsg);
    if ( !pPeer->Send() ) {
        g_log->Error("simkeepalive, failed to send");
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
    }
    g_log->Tips("Sim keepalive msg send successful");
    
    pPeer->StartTimerKeepalive();
}


void
Simulator::AdvertUpdate(Peer * pPeer)
{
    // 初始通告可达路由
    g_log->Tips("send init update advertise in AdvertUpdate");
    vector<struct rtcon *>::iterator rit;
    struct rtcon * pRtCon;
    for (rit = loc_RIB.begin(); rit != loc_RIB.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        
        // find route's interface first
        struct ifcon * pIntCon;
        pIntCon = Interface::GetIfByAddr(&pPeer->conf.remote_addr);
        if (pIntCon == NULL)
            continue;
        
        // check if network reacheable
        if (!Router::InAddrCmp(&pPeer->conf.remote_addr,
                &pRtCon->dest, &pRtCon->mask)) {
            // if can not reach the network, then advertise the <nlri> 
            struct _bgp_update_info * pUpInfo;
            pUpInfo = (struct _bgp_update_info *) malloc(sizeof(struct _bgp_update_info));
            assert(pUpInfo != NULL);
            memset(pUpInfo, 0, sizeof(struct _bgp_update_info));
            
            struct _bgp_path_attr   * pAttr;
            pAttr = (struct _bgp_path_attr *) malloc(sizeof(struct _bgp_path_attr));
            assert(pAttr != NULL);
            memset(pAttr, 0, sizeof(struct _bgp_path_attr));
            pAttr->origin = ORIGIN_IGP;
            
            struct _as_path_segment * pSeg;
            pSeg = (struct _as_path_segment *) malloc(sizeof(struct _as_path_segment));
            assert(pSeg != NULL);
            memset(pSeg, 0, sizeof(struct _as_path_segment));
            pSeg->type = AS_SEQUENCE;
            pSeg->length = 1;
            pSeg->value = new Buffer(2);
            pSeg->value->Add(&conf_as, 2);
            pAttr->aspath.push_back(pSeg);
            
            pAttr->nhop = pIntCon->ipaddr;
            pUpInfo->pathattr = pAttr;
            
            struct _prefix * pPre;
            pPre = (struct _prefix *) malloc(sizeof(struct _prefix));
            memset(pPre, 0, sizeof(struct _prefix));
            assert(pPre != NULL);
            pPre->maskln = (u_int8_t)Router::AddrToMask(&pRtCon->mask);
            pPre->ipaddr = pRtCon->dest;
            pUpInfo->nlri.push_back(pPre);
            
            SimUpdate(pPeer, pUpInfo);
            adj_RIB_out.push_back(pRtCon);
        }
    }

    g_log->Tips("end of advertise routing table");
}

void
Simulator::TransUpdate(struct _bgp_update_info * pUpInfo)
{
    u_int16_t asno;
    u_int8_t  aslen;
    struct _as_path_segment * pSeg;
    pSeg = pUpInfo->pathattr->aspath.front();
    assert(pSeg != NULL);
    aslen = pSeg->length;
    memcpy(&asno, pSeg->value->ReadPos(), 2);
    if (isDebug) {
        g_log->TraceVar("aslen", aslen);
        g_log->TraceVar("asno", ntohs(asno));
    }

    // reset as path attributions
    Buffer * tmpbuf;
    tmpbuf = pSeg->value;
    pSeg->length ++;
    pSeg->value = new Buffer(2 * (aslen+1));
    pSeg->value->Add(&conf_as, 2);
    pSeg->value->Add(tmpbuf->ReadPos(), tmpbuf->Length());
    delete tmpbuf;
    
    vector<Peer *>::iterator pit;
    Peer * pPeer;
    for (pit = vPeers.begin(); pit != vPeers.end(); ++pit) {
        pPeer = *pit;
        
        if (pPeer->GetPeerState() != ESTABLISHED) {
            pPeer->cachedUpinfo.push(pUpInfo);
            continue;
        }
        
        rtcon * pRtCon;
        pRtCon = g_rtr->LookupRoutingTable(pPeer->conf.remote_bgpid);
        if (pRtCon == NULL) {
            g_log->Error("Peer unreacheable in trans update");
            continue;
        }
        struct ifcon * pIntCon;
        if (Router::isDefaultAddr(&pRtCon->nhop)) {
            pIntCon = Interface::GetIfByAddr(& pRtCon->dest);
        } else {
            pIntCon = Interface::GetIfByAddr(& pRtCon->nhop);
        }
        if (pIntCon == NULL) {
            g_log->Error("interface : no found in trans update");
            continue;
        }
        if (pPeer->conf.remote_as == asno) {
            continue;
        }

        pUpInfo->pathattr->nhop = pIntCon->ipaddr;

        SimUpdate(pPeer, pUpInfo);
    }


}


void
Simulator::SimUpdate(Peer * pPeer, void * data, size_t datalen)
{
    if (pPeer == NULL) {
        g_log->Warning("Cannot find peer in sim update");
        return ;
    }
    
    struct bgphdr   hdr;
    u_int16_t       len;
    
    len = MSGSIZE_HEADER + datalen;
    memset(hdr.marker, 0xff, sizeof(hdr.marker));
    hdr.length = htons(len);
    hdr.type = UPDATE;
    
    Message * pMsg;
    pMsg = new Message(MSGSIZE_MAX);
    if (pMsg == NULL) {
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
        return;
    }
    pMsg->sfd = pPeer->sfd;
    pMsg->Add(&hdr, sizeof(hdr));
    pMsg->Add(data, datalen);
    pPeer->qMsg.push(pMsg);
    if ( !pPeer->Send() ) {
        g_log->Error("simupdate failed to send");
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
    }
    g_log->Tips("Sim update msg send successful");
    pPeer->StartTimerKeepalive();
}

void
Simulator::SimUpdate(Peer * pPeer, struct _bgp_update_info * pUpInfo)
{
    Buffer * pBuf;
    pBuf = new Buffer(MSGSIZE_MAX);

    vector<struct _prefix *>::iterator pit;
    struct _prefix * pPre;

    // add withdraw list
    u_int16_t wdLen = 0;
    u_char * resv = pBuf->Reserve(sizeof(wdLen));
    for (pit = pUpInfo->withdraw.begin();
            pit != pUpInfo->withdraw.end(); ++pit) {
        pPre = *pit;
        int octLen;
        if (pPre->maskln > 0)
            octLen = (pPre->maskln - 1) / 8 + 1;
        else
            octLen = 0;
        pBuf->Add(&pPre->maskln, sizeof(pPre->maskln));
        pBuf->Add(&pPre->ipaddr, octLen);
        wdLen += octLen + sizeof(pPre->maskln);
    }
    wdLen = htons(wdLen);
    memcpy(resv, &wdLen, sizeof(wdLen));

    // add path attributions
    u_int16_t paLen = 0;
    resv = pBuf->Reserve(sizeof(paLen));
    
    // fill origin attribution
    struct _path_attr_type pat;
    pat.flag = FLAG_TRANSITIVE;
    pat.typecode = PATHATTR_ORIGIN;
    u_int8_t alen = 1;
    pBuf->Add(&pat.flag, 1);
    pBuf->Add(&pat.typecode, 1);
    pBuf->Add(&alen, 1);
    pBuf->Add(&pUpInfo->pathattr->origin, 1);
    paLen += 3 + alen;
    
    // fill as path attribution
    vector<struct _as_path_segment *>::iterator sit;
    struct _as_path_segment * pSeg;
    for (sit = pUpInfo->pathattr->aspath.begin(); sit != pUpInfo->pathattr->aspath.end(); ++sit) {
        pSeg = *sit;
        pat.flag = FLAG_TRANSITIVE;
        pat.typecode = PATHATTR_ASPATH;
        pBuf->Add(&pat.flag, 1);
        pBuf->Add(&pat.typecode, 1);
        alen = pSeg->length * 2 + 2;
        pBuf->Add(&alen, 1);
        pBuf->Add(&pSeg->type, 1);
        pBuf->Add(&pSeg->length, 1);
        assert(pSeg->length * 2 == pSeg->value->Length());
        pBuf->Add(pSeg->value->ReadPos(), pSeg->value->Length());
        paLen += 3 + alen;
    }

    // fill nexthop
    pat.flag = FLAG_TRANSITIVE;
    pat.typecode = PATHATTR_NEXTHOP;
    pBuf->Add(&pat.flag, 1);
    pBuf->Add(&pat.typecode, 1);
    alen = 4;
    pBuf->Add(&alen, 1);
    pBuf->Add(&pUpInfo->pathattr->nhop, sizeof(struct in_addr));
    paLen += 3 + alen;

    paLen = htons(paLen);
    memcpy(resv, &paLen, sizeof(paLen));


    // fill network layer reachability infomation
    for (pit = pUpInfo->nlri.begin(); pit != pUpInfo->nlri.end(); ++pit) {
        pPre = *pit;
        int octLen;
        if (pPre->maskln > 0)
            octLen = (pPre->maskln - 1) / 8 + 1;
        else
            octLen = 0;
        g_log->TraceSize("nlri length", pPre->maskln);
        pBuf->Add(&pPre->maskln, sizeof(pPre->maskln));
        u_int32_t tipaddr;
        tipaddr = 0;
        memcpy(&tipaddr, &pPre->ipaddr, 4);
        //tipaddr = htonl(tipaddr);
        pBuf->Add(&tipaddr, octLen);
        g_log->TraceIpAddr("nlri ipaddr", &pPre->ipaddr);
        wdLen += octLen + sizeof(pPre->maskln);
    }

    SimUpdate(pPeer, pBuf->ReadPos(), pBuf->Length());
    delete pBuf;
}

void
Simulator::SimNotification(Peer * pPeer, u_int8_t e, u_int8_t es, void * data, ssize_t datalen)
{
    struct bgphdr   hdr;
    ssize_t         len;

    len = MSGSIZE_NOTIFICATION_MIN + datalen;
    memset(&hdr.marker, 0xff, sizeof(hdr.marker));
    hdr.length = htons(len);
    hdr.type = NOTIFICATION;

    Message * pMsg;
    pMsg = new Message(MSGSIZE_MAX);
    if (pMsg == NULL) {
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
        return;
    }
    pMsg->sfd = pPeer->sfd;
    pMsg->Add(&hdr, sizeof(hdr));
    pMsg->Add(&e, sizeof(e));
    pMsg->Add(&es, sizeof(es));
    if (datalen > 0)
        pMsg->Add(data, datalen);

    pPeer->qMsg.push(pMsg);
    if ( !pPeer->Send() ) {
        g_log->Error("sim notification, failed to send");
        g_sim->FSM(pPeer, BGP_TRANS_FATAL_ERROR);
    }
    g_log->Tips("sim notification msg send successful");
}


Peer *
Simulator::GetPeerByAddr(struct sockaddr_in * pSad)
{
    if (pSad == NULL)
        return NULL;
    return GetPeerByAddr(&pSad->sin_addr);
}


Peer *
Simulator::GetPeerByAddr(struct in_addr * pAd)
{
    if (pAd == NULL)
        return NULL;

    vector<Peer *>::iterator vit;
    Peer * pPeer, * ret;
    ret = NULL;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++vit) {
        pPeer = *vit;
        if (memcmp(&pPeer->conf.remote_addr.s_addr,
                &pAd->s_addr, sizeof(pAd->s_addr)) == 0) {
            ret = pPeer;
            break;
        }
    }

    return ret;
}

Peer * 
Simulator::GetPeerByAddr(u_int32_t bgpid)
{
    vector<Peer *>::iterator vit;
    Peer * pPeer, * ret;
    ret = NULL;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++ vit) {
        pPeer = *vit;
        if (memcpy(&pPeer->conf.remote_bgpid,
                &bgpid, sizeof(bgpid)) == 0) {
            ret = pPeer;
            break;
        }
    }
    
    return ret;
}


Peer *
Simulator::GetPeerBySockfd(sockfd fd)
{
    if (fd == -1)
        return NULL;

    vector<Peer *>::iterator vit;
    Peer * pPeer, * ret;
    ret = NULL;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++vit) {
        pPeer = *vit;
        if (pPeer->sfd == fd) {
            ret = pPeer;
            break;
        }
    }

    return ret;
}

void
Simulator::DoDispatch()
{
    //g_log->Tips("do dispatch schedule in sim");
    vector<Peer *>::iterator vit;
    Peer * pPeer;
    for (vit = vPeers.begin(); vit != vPeers.end(); ++vit) {
        pPeer = * vit;
        //g_log->TraceSize("peer buffer size", pPeer->qBuf.size());
        /*
        queue<Buffer *> tmpq(pPeer->qBuf);
        while (!tmpq.empty()) {
            g_log->LogDumpMsg(tmpq.front()->ReadPos(), tmpq.front()->Length());
            tmpq.pop();
        }
        */
        // if peer in refuse message state
        if (pPeer->RefuseMsg()) {
            //g_log->Tips("peer in a refuse state");
            continue;
        }
        if (pPeer->pDis == NULL)
            pPeer->pDis = new Dispatcher;
        assert(pPeer->pDis != NULL);
        pPeer->pDis->SetReadFd(pPeer->sfd);
        pPeer->pDis->Start();
    }
}



bool
Simulator::ParseHeader(Peer * pPeer, u_char * data, u_int16_t & len, u_int8_t & type)
{
    u_char    * pos;
    u_char      one = 0xff;
    u_int16_t   olen;

    // parse at least 19 bytes, we do not check it right now
    pos = data;
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
    u_int8_t    version, rversion;
    u_int16_t   as, msglen;
    u_int16_t   holdtime, oholdtime, myholdtime;
    u_int32_t   bgpid;
    u_int8_t    optparamlen, plen;


    Buffer    * pBuf;
    pBuf = pPeer->qBuf.front();
    assert(pBuf != NULL);

    if (isDebug) {
        g_log->Tips("parse open msg");
        //g_log->LogDumpMsg(pBuf->data, pBuf->Length());
    }

    pos = pBuf->ReadPos();
    pos += MSGSIZE_HEADER_MARKER;
    memcpy(&msglen, pos, sizeof(msglen));
    msglen = ntohs(msglen);

    pos = pBuf->ReadPos();
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
    if (pPeer->conf.remote_as != as) {
        fprintf(errfd, "peer sent wrong AS %u\n", ntohs(as));
        SimNotification(pPeer, ERR_OPEN, ERR_OPEN_AS, NULL, 0);
        ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
        return false;
    }

    memcpy(&oholdtime, pos, sizeof(oholdtime));
    pos += sizeof(oholdtime);
    holdtime = ntohs(oholdtime);
    if (holdtime > T_HOLD_INITIAL) {
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
        // TODO: add ParseCapabilities ....
        if (plen < 2) {
            fprintf(errfd, "corrupt OPEN message received: length mismatch\n");
            SimNotification(pPeer, ERR_OPEN, 0, NULL, 0);
            ChangeState(pPeer, IDLE, RECV_OPEN_MSG);
            return false;
        }
    }

    delete pBuf;
    pPeer->qBuf.pop();

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

    Buffer * pBuf;
    pBuf = pPeer->qBuf.front();
    assert(pBuf != NULL);

    pos = pBuf->ReadPos();
    pos += MSGSIZE_HEADER_MARKER;
    memcpy(&datalen, pos, sizeof(datalen));
    datalen = ntohs(datalen);

    //pos = pPeer->rbuf->rptr;
    pos += MSGSIZE_HEADER;
    datalen -= MSGSIZE_HEADER;


    if (isDebug)
        g_log->Tips("parse notification msg");
    memcpy(&errcode, pos, sizeof(errcode));
    pos += sizeof(errcode);
    datalen -= sizeof(errcode);

    memcpy(&subcode, pos, sizeof(subcode));
    pos += sizeof(subcode);
    datalen -= sizeof(subcode);
    
    // TODO: do things after recv errcode and subcode

    delete pBuf;
    pPeer->qBuf.pop();
    return true;
}

bool
Simulator::ParseUpdate(Peer * pPeer)
{
    Buffer * pBuf;
    pBuf = pPeer->qBuf.front();
    assert(pBuf != NULL);
    
    u_char    * pos;
    u_int16_t   datalen;
    pos = pBuf->ReadPos();
    pos += MSGSIZE_HEADER_MARKER;
    memcpy(&datalen, pos, sizeof(datalen));
    datalen = ntohs(datalen);
    
    pos = pBuf->ReadPos();
    pos += MSGSIZE_HEADER;
    datalen -= MSGSIZE_HEADER;

    if (isDebug)
        g_log->Tips("parse update msg");

    struct _bgp_update_info * pUpInfo;
    pUpInfo = (struct _bgp_update_info *) malloc(sizeof(struct _bgp_update_info));
    assert(pUpInfo != NULL);
    memset(pUpInfo, 0, sizeof(struct _bgp_update_info));

    // to parse Withdrawn Routes
    u_int16_t   wdLen; // Unfeasible Routes Length
    memcpy(&wdLen, pos, sizeof(wdLen));
    wdLen = ntohs(wdLen);
    pos += sizeof(wdLen);
    datalen -= sizeof(wdLen);
    if (wdLen > datalen)
        g_log->Error("length of update message miss match");
    else
        datalen -= wdLen;
    if (isDebug)
        g_log->TraceSize("withdraw length", wdLen);
    while ( wdLen > 0 ) {
        struct _prefix          * pPre;
        pPre = (struct _prefix *) malloc(sizeof(struct _prefix));
        assert(pPre != NULL);
        u_int16_t rLen = 0;
        
        memcpy(&pPre->maskln, pos, 1);
        pos ++;
        u_int8_t octLen = (pPre->maskln - 1) / 8 + 1;
        memset(&pPre->ipaddr, 0, sizeof(pPre->ipaddr));
        if ( octLen > 0 && octLen <= 4) {
            // u_int32_t tipaddr;
            // memcpy(& tipaddr, pos, octLen);
            // tipaddr = ntohl(tipaddr);
            memcpy(&pPre->ipaddr, pos, octLen);
        } else {
            g_log->Error("miss match size of prefix");
        }
        pos += octLen;
        rLen += octLen + 1;
        pUpInfo->withdraw.push_back(pPre);
        
        wdLen -= rLen;
        // TODO: you may delete route in routing table for profix in vProfix
    }
    
    
    // to parse Path Attributes
    u_int16_t   paLen; // Total Path Attribute Length
    memcpy(&paLen, pos, sizeof(paLen));
    paLen = ntohs(paLen);
    pos += sizeof(paLen);
    datalen -= sizeof(paLen);
    if (paLen > datalen)
        g_log->Error("mismatch update message length");
    else
        datalen -= paLen;
    if (isDebug)
        g_log->TraceSize("path attribute length", paLen);
    
    struct _bgp_path_attr   * pAttr = NULL;
    pAttr = (struct _bgp_path_attr *) malloc(sizeof(struct _bgp_path_attr));
    assert(pAttr != NULL);
    memset(pAttr, 0, sizeof(struct _bgp_path_attr));
    while (paLen > 0) {
        u_int16_t           rLen = 0;
        struct _path_attr_type pat;
        memcpy(&pat.flag, pos++, 1);
        memcpy(&pat.typecode, pos++, 1);
        rLen += 2;
        
        /*     attr_flag        attr_typecode     attr_value
         *   0 1 2 3 4 5 6 7
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  |0 0 0 0|E P T O|
         *  +-+-+-+-+-+-+-+-+
         * 
         */
        
        u_int16_t   para_len = 0; // length of a single path attribution
        
        // attr len, depends on EXTENDED flag 
        //      E = 0, 1 octet
        //      E = 1, 2 octet
        if (pat.flag & FLAG_EXTENDED ) {
            u_int16_t len_tmp;
            memcpy(&len_tmp, pos, 2);
            para_len = ntohs(len_tmp);
            pos += 2;
            rLen += 2;
        } else {
            u_char len_tmp;
            memcpy(&len_tmp, pos, 1);
            para_len = len_tmp;
            pos ++;
            rLen ++;
        }
        
         printf("typecode = %d ////\n", pat.typecode);
         fflush(stdout);
        
        switch (pat.typecode) {
            case PATHATTR_ORIGIN:
                if (para_len != 1)
                    g_log->Error("pathattr origin length miss match");
                memcpy(&pAttr->origin, pos, para_len);
                pos ++;
                if (pAttr->origin > 2) {
                    SimNotification(pPeer, ERR_UPDATE, ERR_UPDATE_INVALIDORIGIN, NULL, 0);
                    ChangeState(pPeer, IDLE, RECV_UPDATE_MSG);
                }
                break;
            case PATHATTR_ASPATH:
                struct _as_path_segment * pSeg;
                pSeg = (struct _as_path_segment *) malloc(sizeof(struct _as_path_segment));
                assert(pSeg != NULL);
                memcpy(&pSeg->type, pos++, 1);
                memcpy(&pSeg->length, pos++, 1);
                // remember the as_length sugguest length of as_list in TWO-OCTETS
                u_int16_t   asTotalLen;
                asTotalLen = pSeg->length * 2;
                pSeg->value = new Buffer((int)asTotalLen);
                assert(pSeg != NULL);
                // if (isDebug)
                //   g_log->TraceSize("asTotalLen", asTotalLen);
                pSeg->value->Add(pos, asTotalLen);
                pos += asTotalLen;
                pAttr->aspath.push_back(pSeg);
                break;
            case PATHATTR_NEXTHOP:
                memset(&pAttr->nhop, 0, sizeof(pAttr->nhop));
                if (para_len > 0 && para_len <= 4) {
                    //u_int32_t tipaddr;
                    //tipaddr = 0;
                    //memcpy(&tipaddr, pos, para_len);
                    //tipaddr = ntohl(tipaddr);
                    memcpy(&pAttr->nhop, pos, para_len);
                } else {
                    g_log->Error("miss match path attribution length of nexthop");
                }
                pos += para_len;
                break;
            default:
                g_log->Error("unknown path attribution in parse update");
                printf("typecode = %d ////\n", pat.typecode);
                fflush(stdout);
                assert(false);
        }
        rLen += para_len;
        paLen -= rLen;
    }
    pUpInfo->pathattr = pAttr;
    
    // to parse Network Layer Reachability Information
    u_int16_t nlriLen; // length of Network Layer Reachability Information
    nlriLen = datalen;
    if (isDebug)
        g_log->TraceSize("nlriLen", nlriLen);
    while (nlriLen > 0) {
        
        struct _prefix          * pPre;
        pPre = (struct _prefix *) malloc(sizeof(struct _prefix));
        assert(pPre != NULL);
        u_int16_t rLen = 0;
        
        memcpy(&pPre->maskln, pos, 1);
        pos ++;
        u_int8_t octLen = (pPre->maskln - 1) / 8 + 1;
        memset(&pPre->ipaddr, 0, sizeof(pPre->ipaddr));
        if ( octLen > 0 && octLen <= 4) {
            // u_int32_t tipaddr;
            // memcpy(& tipaddr, pos, octLen);
            // tipaddr = ntohl(tipaddr);
            memcpy(&pPre->ipaddr, pos, octLen);
        } else {
            g_log->Error("miss match size of prefix");
        }
        pos += octLen;
        rLen += octLen + 1;
        pUpInfo->nlri.push_back(pPre);

        nlriLen -= rLen;
        
        // add the nlri to routing table
    }

    if (isDebug)
        g_log->LogUpdateInfo(pUpInfo);

    // update self infomation
    g_rtr->UpdateRt(pUpInfo);

    // trans update infomation to other peers
    TransUpdate(pUpInfo);
    
    delete pBuf;
    pPeer->qBuf.pop();
    return true;
}

bool
Simulator::ParseKeepalive(Peer * pPeer)
{
    if (isDebug)
        g_log->Tips("parse keepalive msg");


    Buffer * pBuf;
    pBuf = pPeer->qBuf.front();
    assert(pBuf != NULL);
    delete pBuf;
    pPeer->qBuf.pop();
    return true;
}

bool
Simulator::LoadSimConf(const char * filename)
{
    FILE          * ffd;
    int             as;
    char            lad[32];
    char            rad[32];
    struct in_addr  linad;
    struct in_addr  rinad;
    sim_config    * sconf;

    ffd = fopen(filename, "r");
    if (ffd == NULL) {
        g_log->Error("Cannot open sim config file");
        return false;
    }

    fscanf(ffd, "%d%s", &as, lad);

    if (!inet_aton(lad, &linad))
        g_log->Warning("not a valid ip");
    conf_as     = htons(as);
    conf_bgpid  = linad.s_addr;
    memcpy(&lisaddr, &linad, sizeof(linad));

    while (fscanf(ffd, "%d%s", &as, rad) != EOF) {
        if(!inet_aton(rad, &rinad))
            g_log->Warning("not a valid ip");
        sconf = (sim_config *) malloc(sizeof(sim_config));
        sconf->as = htons(as);
        memcpy(&sconf->raddr, &rinad, sizeof(rinad));
        vPeerConf.push_back(*sconf);
        //g_log->LogSimConf(as, rad);
    }
    fclose(ffd);
    return true;
}

bool
Simulator::LoadListConf(const char * filename)
{
    FILE           * ffd;
    char             ad[32];
    struct in_addr * pAd;

    ffd = fopen(filename, "r");
    if (ffd == NULL) {
        g_log->Error("Cannot open listen config file");
        return false;
    }

    while (fscanf(ffd, "%s", ad) != EOF) {
        pAd = (struct in_addr *) malloc(sizeof(struct in_addr));
        if(!inet_aton(ad, pAd))
            g_log->Warning("not a lister valid ip");
        vLisAddr.push_back(*pAd);
    }
    fclose(ffd);
    return true;
}

bool
Simulator::LoadListConf()
{
    vector<struct ifcon *>::iterator iit;
    struct ifcon * pIntCon;
    struct in_addr * pAd;
    for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
        pIntCon = * iit;
        if ( strcmp(pIntCon->name, "lo") == 0)
            continue;
        pAd = (struct in_addr *) malloc(sizeof(struct in_addr));
        assert(pIntCon != NULL);
        assert(pAd != NULL);
        memcpy(pAd, & pIntCon->ipaddr, sizeof(pIntCon->ipaddr));
        vLisAddr.push_back(*pAd);
    }
    
    return true;
}
