#include "Interface.h"
#include "Logger.h"

using namespace std;

vector<Interface *> vInt;

Interface::Interface() 
{
    
}


Interface::~Interface() 
{
    
}

#define BUFSIZE_MAXIF 8096
void
Interface::LoadInfo()
{
    sockfd          sfd;
    struct ifconf * ifc;
    char          buf[BUFSIZE_MAXIF];


    ifc = (struct ifconf *) malloc(sizeof (struct ifconf));
    memset(ifc, 0, sizeof(struct ifconf));
    memset(buf, 0, sizeof(buf));

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    ifc->ifc_buf = buf;
    ifc->ifc_len = sizeof(buf);
    
    if ( ioctl(sfd, SIOCGIFCONF, (char *) ifc) == -1) {
        g_log->Fatal("fail to get interface list");
    }
    
    int n, i;
    Interface     * pInt;
    struct ifreq * pIfreq;
    struct sockaddr_in * pSad;
    
    n = ifc->ifc_len / sizeof(struct ifreq);
    pIfreq = (struct ifreq *) buf;
    
    for (i = 0; i < n; ++i) {
        pInt = new Interface;
        assert(pInt != NULL);
        
        // get inter id
        if (ioctl(sfd, SIOCGIFINDEX, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load interface id");
        }
        pInt->conf.id = pIfreq->ifr_ifindex;
        
        // set mac address
        if (ioctl(sfd, SIOCGIFHWADDR, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load mac address");
        }
        memcpy(pInt->conf.mac, pIfreq->ifr_hwaddr.sa_data, sizeof(pIfreq->ifr_hwaddr.sa_data));
        
        // get ip address
        if (ioctl(sfd, SIOCGIFADDR, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load ip address");
        }
        pSad = (struct sockaddr_in *) &(pIfreq->ifr_addr);
        memcpy(&(pInt->conf.ipaddr), &(pSad->sin_addr), sizeof(pSad->sin_addr));
        
        // get broadcast address
        if (ioctl(sfd, SIOCGIFBRDADDR, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load broadcast");
        } 
        pSad = (struct sockaddr_in *) &(pIfreq->ifr_broadaddr);
        memcpy(&(pInt->conf.broadcast), &(pSad->sin_addr), sizeof(pSad->sin_addr));
        
        // get netmask
        if (ioctl(sfd, SIOCGIFNETMASK, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load net mask");
        }
        pSad = (struct sockaddr_in *) &(pIfreq->ifr_netmask);
        memcpy(&(pInt->conf.netmask), &(pSad->sin_addr), sizeof(pSad->sin_addr));
        
        // interface name
        strncpy(pInt->conf.name, pIfreq->ifr_name, IFNAMSIZ-1);
        
        // move to next interface
        pIfreq ++;
        
        vInt.push_back(pInt);
    }
        
}
