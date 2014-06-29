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
        queue<Message *> qMessage;  // waiting message queue
        pthread_mutex_t  msgMutex;  // waiting message queue mutex
        
        static int rtseq;
        bool isDefaultAddr(u_int32_t ipaddr);
        bool isDefaultAddr(struct in_addr * pAd);
        void CalIpChecksum(struct iphdr * pIphdr);
        unsigned short CalChechsum(unsigned short * addr, unsigned int count);

    public:
        Router();
        virtual ~Router();
        
        static struct in_addr MaskToAddr(int mask);
        static u_int32_t AddrToMask(struct in_addr * pAd);
        static u_int32_t AddrToMask(u_int32_t ipaddr);
        
        // message queue tools
        void MsgQueueLock();
        void MsgQueueUnlock();
        void MsgQueueSend();
        
        void LoadKernelRouter();
        bool LoadRouterConf(const char *);
        
        // ip packet toolkits
        bool PacketForward(Message * pMsg);
        struct rtcon * LookupRoutingTable(u_int32_t ipaddr);
        struct rtcon * LookupRoutingTable(struct in_addr * pAd);
        
        // arp utils
        void ARPRos(Message * pMsg);
        void ARPReq(struct in_addr * pAd);
        void ARPReq(u_int32_t ipaddr);
        struct arpcon * LookupARPCache(struct in_addr * pAd);
        
};


extern vector<struct arpcon *> vARPConf;    // arp cache
extern vector<struct rtcon *>   loc_RIB;      // routing table
extern vector<struct rtcon *>   adj_RIB_in;      // routing table
extern vector<struct rtcon *>   adj_RIB_out;      // routing table

#endif /* end of include guard: ROUTER_J2V5X69X */
