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
    return inet_ntoa(*pAd);
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
            fprintf(out, "ARP.fr[%s][%s] to[%s][%s]\n",
                    MacToStr(pArphdr->ar_sha), AddrToStr(pArphdr->ar_sip),
                        MacToStr(pArphdr->ar_tha), AddrToStr(pArphdr->ar_tip) );
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
    fprintf(out, "Destination\tNextHop\t\tNetMast\t\tInterface\n");
    for (rit = vRtConf.begin(); rit != vRtConf.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        fprintf(out, "%-16s", AddrToStr(&pRtCon->dest));
        fprintf(out, "%-16s", AddrToStr(&pRtCon->nhop));
        fprintf(out, "%-16s", AddrToStr(&pRtCon->mask));
        fprintf(out, "%-16s", Interface::GetIfNameById(pRtCon->ifid));
        fprintf(out, "\n");
        fflush(out);
    }
    
}

