#include "Interface.h"
#include "Logger.h"
#include "Router.h"

using namespace std;

vector<struct ifcon *> vIntConf;

Interface::Interface() 
{
    
}


Interface::~Interface() 
{
    
}

char *
Interface::GetIfNameById(int ifid)
{
   struct ifcon * pIntCon;
   vector<struct ifcon *>::iterator iit;
   
   for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
       pIntCon = *iit;
       if (pIntCon->ifid == ifid) {
           return pIntCon->name;
       }
   }
   
   return NULL;
}

struct ifcon *
Interface::GetIfconById(int ifid)
{
   struct ifcon * pIntCon;
   vector<struct ifcon *>::iterator iit;
   
   for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
       pIntCon = *iit;
       if (pIntCon->ifid == ifid) {
           return pIntCon;
       }
   }
   
   return NULL;
}

int
Interface::GetIfidByName(char * ifname)
{
   struct ifcon * pIntCon;
   vector<struct ifcon *>::iterator iit;
   
   for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
       pIntCon = *iit;
       if (strncmp(pIntCon->name, ifname, IFNAMSIZ-1) == 0) {
           return pIntCon->ifid;
       }
   }

   return -1;
}

struct ifcon *
Interface::GetIfByDest(struct in_addr * pAd)
{
   struct ifcon * pIntCon;
   vector<struct ifcon *>::iterator iit;
   
   for (iit = vIntConf.begin(); iit != vIntConf.end(); ++iit) {
       pIntCon = *iit;
       // g_log->ShowIPAddr(pAd);
       // g_log->ShowIPAddr(&pIntCon->ipaddr);
       // g_log->ShowIPAddr(&pIntCon->netmask);
       // cout << "--------in the testing----------" << endl;
       if (Router::InAddrCmp(pAd, &pIntCon->ipaddr, &pIntCon->netmask)) {
       // cout << "-------bingo-----------" << endl;
           return pIntCon;
       }
   }
   
   return NULL;
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
    struct ifcon       * pIntCon;
    struct ifreq       * pIfreq;
    struct sockaddr_in * pSad;
    
    n = ifc->ifc_len / sizeof(struct ifreq);
    pIfreq = (struct ifreq *) buf;
    
    for (i = 0; i < n; ++i) {
        pIntCon = (struct ifcon *) malloc(sizeof(struct ifcon));
        assert(pIntCon != NULL);
        
        // get inter id
        if (ioctl(sfd, SIOCGIFINDEX, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load interface id");
        }
        pIntCon->ifid = pIfreq->ifr_ifindex;
        
        // set mac address
        if (ioctl(sfd, SIOCGIFHWADDR, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load mac address");
        }
        memcpy(pIntCon->mac, pIfreq->ifr_hwaddr.sa_data, sizeof(pIfreq->ifr_hwaddr.sa_data));
        
        // get ip address
        if (ioctl(sfd, SIOCGIFADDR, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load ip address");
        }
        pSad = (struct sockaddr_in *) &(pIfreq->ifr_addr);
        memcpy(&(pIntCon->ipaddr), &(pSad->sin_addr), sizeof(pSad->sin_addr));
        
        // get broadcast address
        if (ioctl(sfd, SIOCGIFBRDADDR, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load broadcast");
        } 
        pSad = (struct sockaddr_in *) &(pIfreq->ifr_broadaddr);
        memcpy(&(pIntCon->broadcast), &(pSad->sin_addr), sizeof(pSad->sin_addr));
        
        // get netmask
        if (ioctl(sfd, SIOCGIFNETMASK, (char *)pIfreq) == -1) {
            g_log->Fatal("cannot load net mask");
        }
        pSad = (struct sockaddr_in *) &(pIfreq->ifr_netmask);
        memcpy(&(pIntCon->netmask), &(pSad->sin_addr), sizeof(pSad->sin_addr));
        
        // interface name
        strncpy(pIntCon->name, pIfreq->ifr_name, IFNAMSIZ-1);
        
        // move to next interface
        pIfreq ++;
        
        vIntConf.push_back(pIntCon);
    }
        
}
