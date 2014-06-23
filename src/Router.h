#ifndef ROUTER_J2V5X69X

#define ROUTER_J2V5X69X

#include "global.h"

using namespace std;

#pragma pack(push)
#pragma pack(1)

struct rtcon {
    struct in_addr  dest;
    struct in_addr  nhop;
    struct in_addr  mask;
    int             ifid;
};

struct arpcon {
    u_char          mac[ETH_ALEN];
    struct in_addr  ipadd;
};

struct eth_arphdr {
    struct arphdr   hdr;                // origin arphdr see <linux/if_arp.h>
    u_char          ar_sha[ETH_ALEN];   /* sender hardware address  */
    u_int32_t       ar_sip;             /* sender IP address        */
    u_char          ar_tha[ETH_ALEN];   /* target hardware address  */
    u_int32_t       ar_tip;             /* target IP address        */
};

#pragma pack(pop)

class Message;

class Router {
    private:
        static int rtseq;
        struct in_addr MaskToAddr(int mask);

    public:
        Router();
        virtual ~Router();
        
        void LoadKernelRoute();
        
        void PacketForward(Message * pMsg);
        
        // arp utils
        void ARPRos(Message * pMsg);
        void ARPReq(struct in_addr * pAd, struct ifcon * pInt);
        struct arpcon * LookupARPCache(struct in_addr * pAd);
};


extern vector<struct arpcon *> vARPConf;
extern vector<struct rtcon *> vRtConf;
#endif /* end of include guard: ROUTE_J2V5X69X */
