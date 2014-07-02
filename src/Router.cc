#include "Router.h"
#include "Logger.h"
#include "Interface.h"
#include "Message.h"
#include "Watcher.h"

using namespace std;

vector<struct arpcon *> vARPConf;
vector<struct rtcon *>   loc_RIB;      // routing table
vector<struct rtcon *>   adj_RIB_in;      // routing table
vector<struct rtcon *>   adj_RIB_out;      // routing table

int Router::rtseq = 0;

Router::Router()
{
    msgMutex = PTHREAD_MUTEX_INITIALIZER;
}


Router::~Router()
{

}


void 
Router::MsgQueueLock()
{
    pthread_mutex_lock(&msgMutex);
    return ;
}


void 
Router::MsgQueueUnlock()
{
    pthread_mutex_unlock(&msgMutex);
}



struct in_addr
Router::MaskToAddr(int mask)
{
    u_int32_t u_addr;
    
    if (mask > 32 || mask <= 0) {
        memset(&u_addr, 0, sizeof(u_addr));
    } else {
        memset(&u_addr, 0xff, sizeof(u_addr));
        u_addr = u_addr << (32 - mask);
    }
    
    u_addr = htonl(u_addr);
    
    return *(struct in_addr *) &u_addr;
}

u_int32_t
Router::AddrToMask(struct in_addr * pAd)
{
    u_int32_t ret;
    u_int32_t ipaddr;
    ret = 0;
    ipaddr = htonl(pAd->s_addr);
    //g_log->TraceIpAddr("********mask=", pAd);
    while (ipaddr & 0x80000000) {
    //fprintf(stdout, "%08x ======\n", ipaddr);
    //fflush(stdout);
        ipaddr = ipaddr << 1;
        ret ++;
    }
    return ret;
}

u_int32_t 
Router::AddrToMask(u_int32_t ipaddr)
{
    return AddrToMask((struct in_addr *) &ipaddr);
}

bool
Router::InAddrCmp(struct in_addr * pSrc, struct in_addr * pDes, struct in_addr * pMask)
{
    u_int32_t src, des, mask;
    src = pSrc->s_addr;
    des = pDes->s_addr;
    mask = pMask->s_addr;
    src &= mask;
    des &= mask;

    if (memcmp(&src, &des, sizeof(u_int32_t)) == 0)
        return true;
    else
        return false;
}

bool
Router::InAddrCmp(struct in_addr * pSrc, struct _prefix * pPre)
{
    u_int32_t src, des, mask;
    src = pSrc->s_addr;
    des = pPre->ipaddr.s_addr;
    mask = 0xffffffff;
    mask = mask << (32 - pPre->maskln);
    
    if (memcmp(&src, &des, sizeof(u_int32_t)) == 0)
        return true;
    else
        return false;
}



bool
Router::LoadRouterConf(const char * filename)
{
    struct rtcon * pRtCon;
    char    des[32];
    char    nexthop[32];
    char    mask[32];
    char    ifname[IFNAMSIZ+1];
    struct in_addr addr_tmp;
    FILE * ffd;

    ffd = fopen(filename, "r");
    if (ffd == NULL) {
        g_log->Error("Cannot open routing table config file");
        return false;
    }

    while (fscanf(ffd, "%s %s %s %s", des, nexthop, mask, ifname) != EOF) {
        pRtCon = (struct rtcon *) malloc(sizeof(struct rtcon));

        if (!inet_aton(des, &addr_tmp))
            g_log->Warning("not a valid ip addr_tmp in load route conf");
        memcpy(&pRtCon->dest, &addr_tmp, sizeof(pRtCon->dest));
        if (!inet_aton(mask, &addr_tmp))
            g_log->Warning("not a valid ip addr_tmp in load route conf");
        memcpy(&pRtCon->mask, &addr_tmp, sizeof(pRtCon->mask));
        if (!inet_aton(nexthop, &addr_tmp))
            g_log->Warning("not a valid ip addr_tmp in load route conf");
        memcpy(&pRtCon->nhop, &addr_tmp, sizeof(pRtCon->nhop));
        pRtCon->ifid = Interface::GetIfidByName(ifname);
        loc_RIB.push_back(pRtCon);
        //adj_RIB_in.push_back(pRtCon);

    }

    fclose(ffd);
    return true;
}


#define BUFSIZE_MAXRT 8096
void 
Router::LoadKernelRoute()
{
    // create kernel socket
    sockfd sfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    
    if (sfd == -1) 
        g_log->Error("cannot create kernel route socket");
    
    u_char  buf[BUFSIZE_MAXRT];
    struct nlmsghdr Nlhdr;
    
    // fill socket data
    memset(buf, 0, sizeof(buf));
    Nlhdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    Nlhdr.nlmsg_type = RTM_GETROUTE;
    Nlhdr.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    Nlhdr.nlmsg_seq = ++ rtseq;
    Nlhdr.nlmsg_pid = getpid();
    
    // write kernel socket
    int ret = write(sfd, &Nlhdr, Nlhdr.nlmsg_len);
    
    if (ret < 0) {
        g_log->Error("cannot to read kernel socket");
        g_log->ShowErrno();
    }
    
    size_t nread;
    while (true) {
        
        // read message from kernel
        nread = read(sfd, buf, sizeof(buf));
        
        if (nread < 0)
            break;

        struct nlmsghdr * pNlhdr;
        struct rtmsg    * pRtmsg;
        struct rtattr   * pRtattr;
        
        pNlhdr = (struct nlmsghdr *) buf;
        while (NLMSG_OK(pNlhdr, nread)) {
            
            pRtmsg = (struct rtmsg *) NLMSG_DATA(pNlhdr);
            pRtattr = (struct rtattr *) RTM_RTA(pRtmsg);
            
            if (pRtmsg->rtm_family != AF_INET || pRtmsg->rtm_table != RT_TABLE_MAIN) {
                pNlhdr = NLMSG_NEXT(pNlhdr, nread);
                continue;
            }
            
            int rt_len = IFA_PAYLOAD(pNlhdr);
            struct rtcon * pRtCon = (struct rtcon *) malloc( sizeof( struct rtcon));
            assert(pRtCon != NULL);
            pRtCon->mask = MaskToAddr(pRtmsg->rtm_dst_len);
           
            // read route config
            while ( RTA_OK(pRtattr, rt_len) ) {
                switch (pRtattr->rta_type) {
                    case RTA_OIF:
                        // set interface id
                        pRtCon->ifid = *(int *) RTA_DATA(pRtattr);
                        break;
                    case RTA_DST:
                        // set destination address
                        *(u_int32_t *)&(pRtCon->dest) = *(u_int32_t *)RTA_DATA(pRtattr);
                        break;
                    case RTA_GATEWAY:
                        // set next hop
                        *(u_int32_t *)&(pRtCon->nhop) = *(u_int32_t *)RTA_DATA(pRtattr);
                        break;
                    default:
                        break;
                }
                pRtattr = RTA_NEXT(pRtattr, rt_len);
            }
            
            // add to route table
            loc_RIB.push_back(pRtCon);
            
            pNlhdr = NLMSG_NEXT(pNlhdr, nread);
        }
        
        if (pNlhdr->nlmsg_type == NLMSG_DONE || (pNlhdr->nlmsg_flags & NLM_F_MULTI) == 0) 
            break;
    }
    
    close(sfd);
}

bool
Router::AddKernelRoute(struct in_addr * pDest, struct in_addr * pNhop, struct in_addr* pMask, int ifid)
{
    // in_addr_t s, d, m;
    // s = inet_addr("192.168.4.4"),
    // d = inet_addr("192.168.4.1"),
    // m = inet_addr("255.255.255.255"),
    // g_rtr->AddKernelRoute((struct in_addr *)&s, (struct in_addr *)&d, (struct in_addr *)&m, 3);

    
    // g_log->TraceIpAddr("dest", pDest);
    // g_log->TraceIpAddr("nhop", pNhop);
    // g_log->TraceIpAddr("mask", pMask);
    // g_log->TraceSize("mask size", AddrToMask(pMask));
    
    struct {
        struct nlmsghdr Nlmsghdr;
        struct rtmsg Rtmsg;
        char buf[MSGSIZE_MAX];
    } req;
    
    req.Rtmsg.rtm_family = AF_INET;
    req.Rtmsg.rtm_table = RT_TABLE_MAIN;
    req.Rtmsg.rtm_protocol = RTPROT_STATIC;
    req.Rtmsg.rtm_scope = RT_SCOPE_UNIVERSE;
    req.Rtmsg.rtm_type = RTN_UNICAST;
    req.Rtmsg.rtm_dst_len = (u_char)AddrToMask(pMask);
    req.Rtmsg.rtm_src_len = 0;
    req.Rtmsg.rtm_tos = 0;
    req.Rtmsg.rtm_flags = RT_TABLE_MAIN;
    
    req.Nlmsghdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.Nlmsghdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;
    req.Nlmsghdr.nlmsg_type = RTM_NEWROUTE;
    req.Nlmsghdr.nlmsg_pid = getpid();
    req.Nlmsghdr.nlmsg_seq = ++ rtseq;

    struct rtattr * pRtattr;

    // set destination
    pRtattr = (struct rtattr *)
        (((char *)&req.Nlmsghdr) + NLMSG_ALIGN(req.Nlmsghdr.nlmsg_len));
    pRtattr->rta_type = RTA_DST;
    pRtattr->rta_len = RTA_LENGTH(sizeof(struct in_addr));
    memcpy(RTA_DATA(pRtattr), pDest, 4);
    req.Nlmsghdr.nlmsg_len = NLMSG_ALIGN(req.Nlmsghdr.nlmsg_len) + pRtattr->rta_len;

    // set the nexthop (or gateway in linux)
    pRtattr = (struct rtattr *)
        (((char *)&req.Nlmsghdr) + NLMSG_ALIGN(req.Nlmsghdr.nlmsg_len));
    pRtattr->rta_type = RTA_GATEWAY;
    pRtattr->rta_len = RTA_LENGTH(sizeof(struct in_addr));
    memcpy(RTA_DATA(pRtattr), pNhop, 4);
    req.Nlmsghdr.nlmsg_len = NLMSG_ALIGN(req.Nlmsghdr.nlmsg_len) + pRtattr->rta_len;

    // set the interface id, usaually 0 for lo; 1 for eth0, 2 for eth1
    pRtattr = (struct rtattr *)
        (((char *)&req.Nlmsghdr) + NLMSG_ALIGN(req.Nlmsghdr.nlmsg_len));
    pRtattr->rta_type = RTA_OIF;
    pRtattr->rta_len = RTA_LENGTH(sizeof(struct in_addr));
    memcpy(RTA_DATA(pRtattr), &ifid, 4);
    req.Nlmsghdr.nlmsg_len = NLMSG_ALIGN(req.Nlmsghdr.nlmsg_len) + pRtattr->rta_len;
    
    
    sockfd sfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    
    if (sfd == -1) {
        g_log->Error("failed create kernel route socket");
        return false;
    }

    struct sockaddr_nl nlSad;
    nlSad.nl_family = AF_NETLINK;
    nlSad.nl_pad = 0;
    nlSad.nl_pid = getpid();
    nlSad.nl_groups = getgid();

    if (bind(sfd, (struct sockaddr *)&nlSad, sizeof(nlSad)) < 0)
        g_log->Error("cannot bind a socket in adding route");

    ssize_t nwrite;
    nwrite = write(sfd, &req, req.Nlmsghdr.nlmsg_len);

    // g_log->TraceSize("nwrite size", nwrite);

    if (nwrite < 0) {
        g_log->Error("cannot add a kernel route");
        return false;
    }

    close(sfd);

    return true;
}



bool
Router::PacketForward(Message * pMsg)
{
    // return false, if the *pMsg do not send ...
    
    if (pMsg == NULL)
        return false;
    
    struct ethhdr * pEthhdr;
    struct iphdr  * pIphdr;
    
    pEthhdr = (struct ethhdr *) pMsg->ReadPos();
    pIphdr = (struct iphdr *) (pMsg->ReadPos() + sizeof(struct ethhdr));
    
    struct rtcon * pRtCon;
    pRtCon = LookupRoutingTable(pIphdr->daddr);
    
    if (pRtCon == NULL) {
        g_log->TraceIpAddr("network unreachable with ip", pIphdr->daddr);
        return false;
    }
    
    // try to forward the packet
    // or just push the packet in the waiting message queue
    //      to let it be forwarded in next schedule
    struct arpcon * pArpCon;
    
    if (isDefaultAddr(&pRtCon->dest)) {
        pArpCon = LookupARPCache(&pRtCon->nhop);
    } else {
        pArpCon = LookupARPCache(&pRtCon->dest);
    }

    // if cannot find mac addr in arp cache, then send a arp request 
    if (pArpCon == NULL) {
        if (isDefaultAddr(pIphdr->daddr)) {
            ARPReq(&pRtCon->nhop);
        } else {
            ARPReq(&pRtCon->dest);
        }
        // move message into a waiting queue
        MsgQueueLock();
        qMessage.push(pMsg);
        MsgQueueUnlock();
        return false;
    }
    
    // just send the message
    
    struct ifcon * pIntCon;
    if (isDefaultAddr(&pRtCon->nhop)) {
        pIntCon = Interface::GetIfByAddr(&pRtCon->dest);
    } else {
        pIntCon = Interface::GetIfByAddr(&pRtCon->nhop);
    }
    if (pIntCon == NULL)
        return false;
    
    
    pIphdr->ttl --;
    if (pIphdr->ttl <= 0) {
        // ignore ... 
        return false;
    } else {
        if (isDebug)
            g_log->Tips("get a packet to forward");
    }
    CalIpChecksum(pIphdr);
    
    // do send the message
    if (g_wtc->GetMainSFD() < 0) {
        g_log->Error("router detect watcher didnot set main socket fd");
        return false;
    }
    
    struct sockaddr saddr;
    saddr.sa_family = AF_INET;
    strcpy(saddr.sa_data, pIntCon->name);
    
    ssize_t nsend;
    nsend = sendto(g_wtc->GetMainSFD(), pMsg->ReadPos(), pMsg->Length(), 0, &saddr, sizeof(saddr));

    g_log->TraceSize("forward size", nsend);
    if (nsend < 0) {
        g_log->Warning("Arp send failed");
        return false;
    }

    
    delete pMsg;
    
    return true;
}

bool 
Router::isDefaultAddr(u_int32_t ipaddr)
{
    return isDefaultAddr((struct in_addr *) & ipaddr);
}


bool 
Router::isDefaultAddr(struct in_addr * pAd)
{
    struct in_addr default_addr;
    memset(&default_addr, 0, sizeof(default_addr));
    return (memcmp(&default_addr, pAd, sizeof(struct in_addr)) == 0);
}


struct rtcon *
Router::LookupRoutingTable(u_int32_t ipaddr)
{
    return LookupRoutingTable((struct in_addr *) &ipaddr);
}

struct rtcon * 
Router::LookupRoutingTable(struct in_addr * pAd)
{
    vector<struct rtcon *>::iterator rit;
    struct rtcon * pRtCon;
    struct rtcon * ret = NULL;
    u_int32_t longest_mask= 0;
    for (rit = loc_RIB.begin(); rit != loc_RIB.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        u_int32_t mask;
        mask = pRtCon->mask.s_addr;
        if (InAddrCmp(pAd, &pRtCon->dest, &pRtCon->mask)) {
            if (mask > longest_mask) {
                ret = pRtCon;
                longest_mask= mask;
            } else if (mask == longest_mask) {
                ret = pRtCon;
            }
        }
    }
    
    return ret;
}

struct rtcon *
Router::LookupRoutingTable(struct _prefix * pPre)
{
    if(pPre == NULL)
        return NULL;
    
    vector<struct rtcon *>::iterator rit;
    struct rtcon * pRtCon;
    struct rtcon * ret = NULL;
    u_int32_t longest_mask= 0;
    for (rit = loc_RIB.begin(); rit != loc_RIB.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        u_int32_t mask;
        mask = 0xffffffff;
        mask = mask << (32 - pPre->maskln);
        if (InAddrCmp(&pRtCon->dest, pPre)) {
            if (mask > longest_mask) {
                ret = pRtCon;
                longest_mask= mask;
            } else if (mask == longest_mask) {
                ret = pRtCon;
            }
        }
    }

    return ret;
}


void
Router::CalIpChecksum(struct iphdr * pIphdr)
{
    pIphdr->check = 0;
    pIphdr->check = CalChechsum((unsigned short *)pIphdr, pIphdr->ihl<<2);
}


unsigned short
Router::CalChechsum(short unsigned int* addr, unsigned int count)
{
    unsigned long sum = 0;
    while (count > 1) {
        sum += * addr++;
        count -= 2;
    }
    
    //if any bytes left, pad the bytes and add
    if(count > 0) {
        sum += ((*addr)&htons(0xff00));
    }
    
    //Fold sum to 16 bits: add carrier to result
    while (sum>>16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    
    //one's complement
    sum = ~sum;
    return ((unsigned short)sum);
}


void
Router::ARPRosHandle(struct eth_arphdr * pArphdr)
{
    assert(pArphdr != NULL);
    
    if (ntohs(pArphdr->hdr.ar_op) != ARPOP_REPLY) {
        fprintf(stdout, "unknow op=%d\n", ntohs(pArphdr->hdr.ar_op));
        return ;
    }
    
    
    ARPAdd((struct in_addr *)&pArphdr->ar_sip, pArphdr->ar_sha);
    g_log->LogARPCache();
}

void
Router::ARPReq(u_int32_t ipaddr)
{
    return ARPReq((struct in_addr *) &ipaddr);
}


void 
Router::ARPReq(struct in_addr * pAd)
{
    struct arpcon * pArpCon;
    pArpCon = LookupARPCache(pAd);
    if (pArpCon != NULL) 
        return;
    
    struct ifcon * pIntCon;
    pIntCon = Interface::GetIfByAddr(pAd);
    if (pIntCon == NULL) {
        g_log->Error("failed to find interface in arp request");
        return;
    }

    g_log->TraceIpAddr("arp request destip", pAd);
    Buffer buf(ETH_DATA_LEN);
    
    sockfd sfd;
    sfd = socket(AF_PACKET, SOCK_PACKET, htons(ETH_P_ALL));
    if (sfd < 0) {
        g_log->Error("failed to initial sockfd in arp");
        return;
    }
    
    struct ethhdr Ethhdr;
    
    // fill ether header
    memcpy(Ethhdr.h_source, pIntCon->mac, ETH_ALEN);
    memset(Ethhdr.h_dest, 0xff, ETH_ALEN);
    Ethhdr.h_proto = htons(ETH_P_ARP);
    buf.Add(&Ethhdr, sizeof(Ethhdr));
    
    struct eth_arphdr Arphdr;
    // fill arp header
    Arphdr.hdr.ar_hrd = htons((__be16)ARPHRD_ETHER);
    Arphdr.hdr.ar_pro = htons(ETH_P_IP);
    Arphdr.hdr.ar_hln = ETH_ALEN;
    Arphdr.hdr.ar_pln = 4;
    Arphdr.hdr.ar_op  = htons(ARPOP_REQUEST);
    memcpy(Arphdr.ar_sha, pIntCon->mac, ETH_ALEN);
    memset(Arphdr.ar_tha, 0, ETH_ALEN);
    memcpy(&Arphdr.ar_sip, &pIntCon->ipaddr, sizeof(Arphdr.ar_sip));
    memcpy(&Arphdr.ar_tip, pAd, sizeof(Arphdr.ar_tip));
    buf.Add(&Arphdr, sizeof(Arphdr));

    struct sockaddr saddr;
    saddr.sa_family = AF_INET;
    strcpy(saddr.sa_data, pIntCon->name);
    
    ssize_t nsend;
    nsend = sendto(sfd, buf.ReadPos(), buf.Length(), 0, &saddr, sizeof(saddr));

    //g_log->Tips("send arp msg");
    //g_log->LogDumpMsg(buf.ReadPos(), buf.Length());
    
    if (nsend < 0) {
        g_log->Warning("Arp send failed");
    }

    close(sfd);
}


void
Router::ICMPRos(Message* pMsg)
{
    struct ethhdr * pEthhdr;
    pEthhdr = (struct ethhdr *) pMsg->ReadPos();
    struct iphdr * pIphdr;
    pIphdr = (struct iphdr *) (pMsg->ReadPos() + sizeof(struct iphdr));
    struct icmphdr Icmphdr;
    pIphdr->protocol = IP_PROTO_ICMP;
    g_log->Tips("TODO send a icmp");
}

void
Router::ARPAdd(struct in_addr * pAd, u_char * mac)
{
    struct arpcon * pArpCon;
    pArpCon = (struct arpcon *) malloc(sizeof(struct arpcon));
    assert(pArpCon);
    
    // set target ip address
    memcpy(&pArpCon->ipadd, pAd, sizeof(struct in_addr));
    // set hardware address
    memcpy(&pArpCon->mac, mac, ETH_ALEN);
    vARPConf.push_back(pArpCon);
}

struct arpcon *
Router::LookupARPCache(u_int32_t ipaddr)
{
    return LookupARPCache((struct in_addr *)&ipaddr);
}


struct arpcon * 
Router::LookupARPCache(struct in_addr * pAd)
{
    vector<arpcon *>::iterator ait;
    struct arpcon * pArpCon;
    for (ait = vARPConf.begin(); ait != vARPConf.end(); ++ait) {
        pArpCon = * ait;
        assert(pArpCon != NULL);
        if (memcmp(&pArpCon->ipadd, pAd, sizeof(pArpCon->ipadd)) == 0)
            return pArpCon;
    }
    return NULL;
}

void
Router::MsgQueueSend()
{
    MsgQueueLock();
    queue<Message *> qMsgCopy(qMessage);
    while (!qMessage.empty()) {
        qMessage.pop();
    }
    
    MsgQueueUnlock();
    
    Message * pMsg;
    while (!qMsgCopy.empty()) {
        pMsg = qMsgCopy.front();
        PacketForward(pMsg);
        qMsgCopy.pop();
    }
}

void
Router::UpdateRt(struct _bgp_update_info * pUpInfo)
{
    g_log->Tips("updating routing table");
    struct rtcon * pRtCon;
    for (vector<struct _prefix *>::iterator pIt = pUpInfo->nlri.begin();
        pIt != pUpInfo->nlri.end();
            ++ pIt) {
        struct _prefix  * pPre = *pIt;
        pRtCon = LookupRoutingTable(pPre);

        // if no routing item find, we may try to add routing item
        if (pRtCon == NULL) {
            pRtCon = AddRoutingItem(pPre, & pUpInfo->pathattr->nhop);
        } else {
            g_log->Tips("route already in routing table");
        }
    }
    g_log->LogRouteList();
}

struct rtcon *
Router::AddRoutingItem(struct _prefix * pPre, struct in_addr * pNhop)
{
    struct rtcon * pRtCon;
    struct ifcon * pIntCon;

    pIntCon = Interface::GetIfByAddr(pNhop);

    if (pIntCon == NULL) {
        g_log->Warning("network unreachable in adding routing item");
        pRtCon = NULL;
    } else {
        pRtCon = (struct rtcon *) malloc(sizeof(struct rtcon));
        assert(pRtCon != NULL);
        memset(pRtCon, 0, sizeof(struct rtcon));
        pRtCon->mask = MaskToAddr(pPre->maskln);
        pRtCon->dest = pPre->ipaddr;
        pRtCon->nhop = *pNhop;
        pRtCon->ifid = pIntCon->ifid;
        loc_RIB.push_back(pRtCon);
        adj_RIB_in.push_back(pRtCon);
    }

    return pRtCon;
}
