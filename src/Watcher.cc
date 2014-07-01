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
        g_log->Fatal("failed to set main socket");
    }
    return NULL;
}

bool
Watcher::InitMainSocket()
{
    sfd = socket(AF_PACKET, SOCK_PACKET, htons(ETH_P_ALL));
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

void
Watcher::StartListen()
{
    struct iphdr  * pIphdr;
    struct ethhdr * pEthhdr;
    struct eth_arphdr * pArphdr;
    struct tcphdr * pTcphdr;
    u_char          buf[ETH_DATA_LEN];
    
    while (true) {
        size_t nread = read(sfd, buf, ETH_DATA_LEN);
        if (nread < 0)
            g_log->Error("cannot catch packet");
        
        
        Message * pMsg;
        pMsg = new Message(ETH_DATA_LEN);
        pMsg->Add(buf, nread);
        
        pEthhdr = (struct ethhdr *) pMsg->ReadPos();
        
        if (isDebug)
           g_log->LogRecvedMsg(pEthhdr);
            
        switch (ntohs(pEthhdr->h_proto)) {
            case ETH_P_ARP:
                pArphdr = (struct eth_arphdr *)
                                (pMsg->ReadPos() + sizeof(struct ethhdr));
                if (CheckInter(pArphdr->ar_tip)) {
                    // send arp request is the mac is unknown
                    if (isDebug) {
                        g_log->Tips("arp unknown");
                    }
                    g_rtr->ARPAdd(pMsg);
                } else {
                    if (isDebug) {
                        g_log->Tips("arp already known");
                    }
                }
                break;
            case ETH_P_IP:
                pIphdr  = (struct iphdr *)
                                (pMsg->ReadPos() + sizeof(struct ethhdr));
                pTcphdr = (struct tcphdr *)
                    (pMsg->ReadPos() + sizeof(struct ethhdr) + sizeof(struct iphdr) );
                if (!CheckInter(pIphdr->daddr)) {
                    // try to forward packet, if ip des is not in my Interface
                    if (pTcphdr->dest != htons(BGP_PORT))
                        g_rtr->PacketForward(pMsg);
                } else {
                    if (isDebug) {
                        g_log->Tips("ip packet reach dest... done");
                    }
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
// return true, if find a mac in arp cache
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
// return true, if find a arp cache for given ip address.
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
        // g_log->TraceIpAddr("target ip", pAd);
        // g_log->TraceIpAddr("interface ip", &pIntCon->ipaddr);
        if (memcmp(pAd, &pIntCon->ipaddr, sizeof(struct in_addr)) == 0) {
            // cout << "+++++++++++ bingo ++++===" << endl;
            // fflush(stdout);
            return true;
        }
    }
    return false;
}
