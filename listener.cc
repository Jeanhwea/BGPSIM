#include "listener.h"

using namespace std;

#define BUF_SIZE 8096
#define IP_ADDR_SIZE 64

Listener::Listener()
{
    bool sucess(false);
    if (vSockFd.empty()) sucess = SetMainSocket();
    printf("%s to add a main socket\n", sucess ? "sucess" : "failed");
}

Listener::~Listener()
{

}

bool Listener::SetMainSocket()
{
    int sockfd;
    
    // htons h:host n:network s:short
    // AF_foo := address family
    // PF_foo := protocol family
    sockfd = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
    if (sockfd < 0) return false;
    vSockFd.push_back(sockfd);

    int recLen = 1024 * 100;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recLen, sizeof(int));
    return true;
}

bool Listener::ListenAll()
{
    struct ethhdr * pEthhdr;
    struct iphdr * pIphdr;
    char buf[BUF_SIZE];

    if (vSockFd.empty()) return false;

    int mainSocket = vSockFd[0];

    while (true) {
        ssize_t len = recv(mainSocket, buf, BUF_SIZE, 0);
        assert(len >= 0);

        char * pBuf = buf;
        pEthhdr = (struct ethhdr *) pBuf;
        pBuf += sizeof(struct ethhdr);
        pIphdr = (struct iphdr *) pBuf;

        if (pEthhdr->h_proto != htons(ETH_P_IP)) {
            if (pEthhdr->h_proto != htons(ETH_P_ARP))
                cout << "arp here ... " << endl;
            continue;
        }

        if (isDebug) OutputPacket(pIphdr);
    }
    
    return true;
}

bool Listener::SetPromisc(string ifname)
{
    if (vSockFd.empty()) return false;
    return SetPromisc(ifname, vSockFd[0]);
}

bool Listener::UnsetPromisc(string ifname)
{
    if (vSockFd.empty()) return false;
    return UnsetPromisc(ifname, vSockFd[0]);
}

bool Listener::SetPromisc(string ifname, int sockfd)
{
    struct ifreq ifr;
    int res;
    strncpy(ifr.ifr_name, ifname.c_str(), ifname.length()+1);
    res = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    assert(res >= 0);
    ifr.ifr_flags |= IFF_PROMISC;
    res = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    assert(res >= 0);
    return true;
}

bool Listener::UnsetPromisc(string ifname, int sockfd)
{
    struct ifreq ifr;
    int res;
    strncpy(ifr.ifr_name, ifname.c_str(), ifname.length()+1);
    res = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
    assert(res >= 0);
    ifr.ifr_flags &= ~IFF_PROMISC;
    res = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
    assert(res >= 0);
    return true;
}

void Listener::OutputPacket(struct iphdr * pIphdr) 
{
    char src[IP_ADDR_SIZE+1], des[IP_ADDR_SIZE+1];

    strncpy(src, (char *) inet_ntoa( *(struct in_addr *)&(pIphdr->saddr) ), IP_ADDR_SIZE);
    strncpy(des, (char *) inet_ntoa( *(struct in_addr *)&(pIphdr->saddr) ), IP_ADDR_SIZE);

    printf("%6d %15s > %15s : flag_off=%d\n",pIphdr->id, src, des, pIphdr->frag_off);

}


