#include "Watcher.h"
#include "Message.h"
#include "Interface.h"
#include "Logger.h"
#include "Router.h"

using namespace std;


Watcher::Watcher() 
{
}

Watcher::~Watcher() 
{
}

sockfd
Watcher::GetMainSFD()
{
    return sfd;
}


void * 
Watcher::Run()
{
    if (InitMainSocket())
        StartListen();
    else {
        g_log->Error("failed to set main socket");
        g_log->ShowErrno();
        exit(-1);
    }
    return NULL;
}

bool
Watcher::InitMainSocket()
{
    sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sfd == -1)
        g_log->Fatal("failed to set main sfd");
    return true;
}

bool
Watcher::SetPromisc()
{
    struct ifreq ifr;
    vector<struct ifcon *>::iterator iit;
    struct ifcon * pIntCon;
    
    for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
        pIntCon = *iit;
        assert(pIntCon != NULL);
        
        strcpy(ifr.ifr_name, pIntCon->name);
        if (ioctl(sfd, SIOCGIFFLAGS, &ifr) != 0) {
            g_log->Fatal("cannot SetPromisc");
        }
        
        ifr.ifr_flags |= IFF_PROMISC;
        if (ioctl(sfd, SIOCSIFFLAGS, &ifr) == 0) {
            g_log->Fatal("cannot SetPromisc *");
        }
    }
    
    return true;
}

#define BUFSIZE_ETH 8096
void
Watcher::StartListen()
{
    struct iphdr  * pIphdr;
    struct ethhdr * pEthhdr;
    struct eth_arphdr * pArphdr;
    u_char          buf[BUFSIZE_ETH];
    
    while (true) {
        size_t nread = read(sfd, buf, BUFSIZE_ETH);
        if (nread < 0)
            g_log->Error("cannot catch packet");
        
        pEthhdr = (struct ethhdr *) buf;
        
        if (isDebug)
           g_log->LogRecvedMsg(pEthhdr);
            
        switch (ntohs(pEthhdr->h_proto)) {
            case ETH_P_ARP:
                pArphdr = (struct eth_arphdr *) (buf + sizeof(struct ethhdr));
                if (!CheckInter(pArphdr->ar_tip)) {
                    // send arp request is the mac is unknown
                    Message * pMsg;
                    pMsg = new Message(BUFSIZE_ETH);
                    pMsg->Add(buf, nread);
                    g_rtr->ARPRos(pMsg);
                }
                break;
            case ETH_P_IP:
                pIphdr  = (struct iphdr *) (buf + sizeof(struct ethhdr));
                if (!CheckInter(pIphdr->daddr)) {
                    // try to forward packet, if ip des is not in my Interface
                    Message * pMsg;
                    pMsg = new Message(BUFSIZE_ETH); 
                    pMsg->Add(buf, nread);
                    g_rtr->PacketForward(pMsg);
                }
                break;
            default:
                continue;
        }
        
    }
}

bool
Watcher::CheckInter(u_char mac[])
{
    vector<struct ifcon *>::iterator iit;
    struct ifcon * pIntCon;
    for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
        pIntCon = * iit;
        assert(pIntCon != NULL);
        if (memcmp(mac, pIntCon->mac, ETH_ALEN) == 0)
            return true;
    }
    
    return false;
}

bool
Watcher::CheckInter(u_int32_t ipaddr)
{
    return CheckInter((struct in_addr *)&ipaddr);
}

bool
Watcher::CheckInter(struct in_addr * pAd)
{
    vector<struct ifcon *>::iterator iit;
    struct ifcon * pIntCon;
    for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
        pIntCon = * iit;
        assert(pIntCon != NULL);
        u_int32_t src, des, mask;
        mask = pIntCon->netmask.s_addr;
        src  = pIntCon->ipaddr.s_addr;
        des  = pAd->s_addr;
        src  &= mask;
        des  &= mask;
        if (memcmp(&des, &src, sizeof(src)) == 0)
            return true;
    }
    return false;
}
