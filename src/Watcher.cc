#include "Watcher.h"
#include "Interface.h"
#include "Logger.h"
#include "Route.h"

using namespace std;


Watcher::Watcher() 
{
    
}

Watcher::~Watcher() 
{
    
}

void * 
Watcher::Run()
{
    if (InitMainSocket())
        StartListen();
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
    vector<Interface *>::iterator iit;
    Interface * pInt;
    
    for (iit = vInt.begin(); iit != vInt.end(); ++iit) {
        pInt = *iit;
        assert(pInt != NULL);
        
        strcpy(ifr.ifr_name, pInt->conf.name);
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
    u_char          buf[BUFSIZE_ETH];
    
    while (true) {
        size_t len = read(sfd, buf, BUFSIZE_ETH);
        if (len < 0)
            g_log->Error("cannot catch packet");
        
        pEthhdr = (struct ethhdr *) buf;
        pIphdr  = (struct iphdr *) (buf + sizeof(struct ethhdr));
        
        if (pEthhdr->h_proto != htons(ETH_P_IP) ) {
            if (pEthhdr->h_proto == htons(ETH_P_ARP) ) {
                if (!CheckInter(pEthhdr->h_dest)) {
                    // send arp request
                }
            } 
            continue;
        }
        
        if (isDebug)
            g_log->LogIPMsg(pIphdr);
        
        if (!CheckInter(pIphdr->daddr)) {
            // try to forward packet
        }
    }
}

bool
Watcher::CheckInter(u_char mac[])
{
    vector<Interface *>::iterator iit;
    Interface * pInt;
    for (iit = vInt.begin(); iit != vInt.end(); ++iit) {
        pInt = *iit;
        assert(pInt != NULL);
        if (memcmp(mac, pInt->conf.mac, ETH_ALEN) == 0)
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
    vector<Interface *>::iterator iit;
    Interface * pInt;
    for (iit = vInt.begin(); iit != vInt.end(); ++iit) {
        pInt = *iit;
        assert(pInt != NULL);
        u_int32_t src, des, mask;
        mask = pInt->conf.netmask.s_addr;
        src  = pInt->conf.ipaddr.s_addr;
        des  = pAd->s_addr;
        src  &= mask;
        des  &= mask;
        if (memcmp(&des, &src, sizeof(src)) == 0)
            return true;
    }
    return false;
}
