#include "Logger.h"
#include "Peer.h"
#include "Event.h"
#include "Listener.h"
#include "Interface.h"
#include "Router.h"

using namespace std;

Logger::Logger()
{
    out = outfd;
    war = errfd;
    err = errfd;
}

Logger::~Logger()
{
}

void
Logger::TraceSize(const char* msg, ssize_t siz)
{
    if (msg != NULL)
        fprintf(out, "TraceSize: %s = %d\n", msg, (int)siz);
    fflush(out);
}

void
Logger::TraceIpAddr(const char * msg, struct in_addr * pAd)
{
    if (msg != NULL)
        fprintf(out, "TraceIP : %s = %s\n", msg, AddrToStr(pAd));
    fflush(out);
}

void
Logger::TraceIpAddr(const char* msg, u_int32_t ipaddr)
{
    TraceIpAddr(msg, (struct in_addr *)&ipaddr);
}


void
Logger::Tips(const char * emsg)
{
    if (emsg != NULL)
        fprintf(out, "Tips\t: %s\n", emsg);
    fflush(out);
}

void
Logger::Warning(const char * emsg)
{
    if (emsg != NULL)
        fprintf(war, "Warning\t: %s\n", emsg);
    fflush(war);
}

void
Logger::Error(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Error\t: %s\n", emsg);
    fflush(err);
}

void
Logger::Fatal(const char * emsg)
{
    if (emsg != NULL)
        fprintf(err, "Fatal\t: %s\n", emsg);
    ShowErrno();
    exit(1);
}

void
Logger::ShowErrno()
{
    fprintf(err, "Errno = (%d)\t: %s\n", errno, strerror(errno));
    fflush(err);
}

char *
Logger::AddrToStr(u_int32_t addr)
{
    return AddrToStr((struct in_addr *) &addr);
}


char *
Logger::AddrToStr(struct in_addr * pAd)
{
    assert(pAd != NULL);
    char * ret = (char *) malloc(16);
    sprintf(ret, "%s", inet_ntoa(*pAd));
    return ret;
}

char *
Logger::AddrToStr(struct sockaddr_in * pSad)
{
    assert(pSad != NULL);
    return AddrToStr(&pSad->sin_addr);
}

char * 
Logger::MacToStr(u_char * mac_addr)
{
    char * ret;
    ret = (char *) malloc(ETH_ALEN * 3 * sizeof(u_char));
    
    sprintf(ret, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac_addr[0], mac_addr[1], mac_addr[2], 
            mac_addr[3], mac_addr[4], mac_addr[5]);
    
    return ret;
}


void
Logger::ShowIPAddr(struct in_addr * pAd)
{
    fprintf(out, "ip:%s\n", AddrToStr(pAd));
}

void
Logger::ShowIPAddr(struct sockaddr_in * pSad)
{
    assert(pSad != NULL);
    ShowIPAddr(&pSad->sin_addr);
}

void
Logger::LogStateChage(Peer * pPeer, state_t to, event_t eve)
{
    fprintf(out, "Peer_%d\t: { %s }->{ %s } : [%s]\n",
        pPeer->GetId(),
            mapStateName[pPeer->GetPeerState()].c_str(),
                mapStateName[to].c_str(),
                    mapEventName[eve].c_str());
    fflush(out);
}

void
Logger::LogPeerEve(Peer * pPeer, event_t eve)
{
    fprintf(out, "Peer_%d runs, handling event : %s\n",
                pPeer->GetId(), mapEventName[eve].c_str());
    fflush(out);
}


void
Logger::LogSimConf(int as, const char * ra)
{
    fprintf(out, "Load Simu Config : AS%d, %s\n", as, ra);
}

void
Logger::LogDispatchMsg(u_int16_t len, u_int8_t type)
{
    fprintf(out, "Dipatch Message with type=%s, len=%d\n",
            mapMsgName[(message_t)type].c_str(), len);
}

void 
Logger::LogRecvedMsg(struct ethhdr * pEthhdr)
{    
    switch (ntohs(pEthhdr->h_proto)) {
        case ETH_P_ARP:
            struct eth_arphdr * pArphdr;
            pArphdr = (struct eth_arphdr *) ( ((u_char *)pEthhdr) + sizeof(struct ethhdr) );
            fprintf(out, "ARP.fr[%s][%s] to[%s][%s] ",
                    MacToStr(pArphdr->ar_sha), AddrToStr(pArphdr->ar_sip),
                        MacToStr(pArphdr->ar_tha), AddrToStr(pArphdr->ar_tip) );
            switch (ntohs(pArphdr->hdr.ar_op)) {
                case ARPOP_REPLY:
                    fprintf(out, "ARPOP_REPLY");
                    break;
                case ARPOP_REQUEST:
                    fprintf(out, "ARPOP_REQUEST");
                    break;
                default:
                    fprintf(out, "unknow op=%d", ntohs(pArphdr->hdr.ar_op));
                    break;
            }
            fprintf(out, "\n");
            fflush(out);
            break;
        case ETH_P_IP:
            struct iphdr * pIphdr;
            pIphdr = (struct iphdr *) ( ((u_char *)pEthhdr) + sizeof(struct ethhdr) );
            fprintf(out, "IP..fr[%s] to[%s] frag_off<%d>\n",
                    AddrToStr(pIphdr->saddr), AddrToStr(pIphdr->daddr), pIphdr->frag_off );
            break;
        default:
            break;
    }
    fflush(out);
}


#define PRINT_ALIGN 16
void
Logger::LogDumpMsg(u_char * data, size_t len)
{
    fprintf(out, "MSG with %uBytes, Dump\n", (unsigned int)len);
    if (len > 4096) return;
    for (size_t i = 0; i < len; ++i) {
        if (!((i)%PRINT_ALIGN))
            fprintf(out, "0x%04x : ", (unsigned int)i);

        fprintf(out, "%02x%c", *(data+i), (i+1)%PRINT_ALIGN ? ' ' : '\n');
        fflush(out);
    }
    if (len%PRINT_ALIGN)
        fprintf(out, "\n");
    fflush(out);
}

void
Logger::LogListenerList()
{
    vector<Listener *>::iterator lit;
    Listener * pListener;
    struct in_addr * pAd;
    fprintf(out, "Listener list addr = [ ");
    for (lit = vListeners.begin(); lit != vListeners.end(); ++lit) {
        pListener = *lit;
        assert(pListener != NULL);
        pAd = pListener->GetLisAddr();
        assert(pAd != NULL);
        fprintf(out, "%s ", AddrToStr(pListener->GetLisAddr()));
    }
    fprintf(out, "]\n");
}

void
Logger::LogPeerList()
{
    vector<Peer *>::iterator    pit;
    Peer                      * pPeer;
    u_int16_t                   as;
    fprintf(out, "Peers list config  = [ ");
    for (pit = vPeers.begin(); pit != vPeers.end(); ++pit) {
        pPeer = *pit;
        assert(pPeer != NULL);
        as = ntohs(pPeer->conf.remote_as);
        fprintf(out,"{ AS%d, %s, sfd=%d } ", as, 
                AddrToStr(&pPeer->conf.remote_addr), pPeer->sfd);
    }
    fprintf(out, "]\n");
}

void 
Logger::LogIntList()
{
    vector<struct ifcon *>::iterator iit;
    struct ifcon * pIntCon;
    for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
        pIntCon = * iit;
        assert(pIntCon != NULL);
        fprintf(out, "%s", pIntCon->name);
        //fprintf(out, "\tid = %d\n",pInt->conf.id);
        fprintf(out, "\tmac = %s\n", MacToStr(pIntCon->mac));
        fprintf(out, "\tip = %s\n", AddrToStr(&pIntCon->ipaddr));
        fprintf(out, "\tmask = %s\n", AddrToStr(&pIntCon->netmask));
        fprintf(out, "\tbroadcast = %s", AddrToStr(&pIntCon->broadcast));
        fprintf(out, "\n");
        fflush(out);
    }
}

void
Logger::LogRouteList()
{
    vector<struct rtcon *>::iterator rit;
    struct rtcon * pRtCon;
    fprintf(out, "Routing Table ...\n");
    fprintf(out, "Destination\t\tNextHop\t\tNetMask\t\tInterface\t\tloc_RIB\n");
    for (rit = loc_RIB.begin(); rit != loc_RIB.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        fprintf(out, "%-16s", AddrToStr(&pRtCon->dest));
        fprintf(out, "\t%-16s", AddrToStr(&pRtCon->nhop));
        fprintf(out, "\t%-16s", AddrToStr(&pRtCon->mask));
        fprintf(out, "\t%-16s", Interface::GetIfNameById(pRtCon->ifid));
        fprintf(out, "\n");
        fflush(out);
    }
    
    fprintf(out, "Destination\t\tNextHop\t\tNetMask\t\tInterface\t\tadj_RIB_in\n");
    for (rit = adj_RIB_in.begin(); rit != adj_RIB_in.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        fprintf(out, "%-16s", AddrToStr(&pRtCon->dest));
        fprintf(out, "\t%-16s", AddrToStr(&pRtCon->nhop));
        fprintf(out, "\t%-16s", AddrToStr(&pRtCon->mask));
        fprintf(out, "\t%-16s", Interface::GetIfNameById(pRtCon->ifid));
        fprintf(out, "\n");
        fflush(out);
    }
    
    fprintf(out, "Destination\t\tNextHop\t\tNetMask\t\tInterface\t\tadj_RIB_out\n");
    for (rit = adj_RIB_out.begin(); rit != adj_RIB_out.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        fprintf(out, "%-16s", AddrToStr(&pRtCon->dest));
        fprintf(out, "\t%-16s", AddrToStr(&pRtCon->nhop));
        fprintf(out, "\t%-16s", AddrToStr(&pRtCon->mask));
        fprintf(out, "\t%-16s", Interface::GetIfNameById(pRtCon->ifid));
        fprintf(out, "\n");
        fflush(out);
    }
}

void
Logger::LogARPCache()
{
    vector<struct arpcon *>::iterator ait;
    struct arpcon * pArpCon;
    fprintf(out, "ARP Cache ...\n");
    fprintf(out, "MAC\t\tIP\n");
    fflush(out);
    for (ait = vARPConf.begin(); ait != vARPConf.end(); ++ait) {
        pArpCon = *ait;
        fprintf(out, "%s\t", MacToStr(pArpCon->mac));
        fprintf(out, "%-16s", AddrToStr(&pArpCon->ipadd));
        fprintf(out, "\n");
        fflush(out);
    }
}


void
Logger::LogUpdateInfo(struct _bgp_update_info * pUpInfo)
{
    fprintf(out, "logged update msg info\n");
    fflush(out);
    if (pUpInfo == NULL)
        return ;
    vector<struct _prefix *>::iterator pit;
    for (pit = pUpInfo->withdraw.begin();
        pit != pUpInfo->withdraw.end(); ++pit) {
        fprintf(out, "withdraw route: length=%d, ip=%s\n",
                (*pit)->maskln, AddrToStr(& ((*pit)->ipaddr)) );
        fflush(out);
    }
    fprintf(out, "update");
    fprintf(out, "\torigin=%d\n", pUpInfo->pathattr->origin);
    u_int8_t len = pUpInfo->pathattr->as_path.length;
    fprintf(out, "\tas_path_type=%d, as_path_length=%d, as_num_list=",
            pUpInfo->pathattr->as_path.type, len);
    for (u_int8_t i = 0; i < len; ++i) {
        u_int16_t as_num;
        memcpy(&as_num, pUpInfo->pathattr->as_path.value->ReadPos()+2*i, 2);
        fprintf(out, "%d ", ntohs(as_num));
    }
    fprintf(out, "\n");
    fflush(out);
    fprintf(out, "\tnexthop=%s\n", AddrToStr(&pUpInfo->pathattr->nhop));
    for (pit = pUpInfo->nlri.begin();
        pit != pUpInfo->nlri.end(); ++pit) {
        fprintf(out, "nlri route: length=%d, ip=%s\n",
                (*pit)->maskln, AddrToStr(& ((*pit)->ipaddr)) );
        fflush(out);
    }

    fflush(out);
}


