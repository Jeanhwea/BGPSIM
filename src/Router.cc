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
Router::LoadKernelRouter()
{
    // create kernel socket
    sockfd sfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    
    if (sfd == -1) 
        g_log->Error("cannot create kernel route socket");
    
    u_char  buf[BUFSIZE_MAXRT];
    struct nlmsghdr * pNlhdr;
    
    // fill socket data
    memset(buf, 0, sizeof(buf));
    pNlhdr = (struct nlmsghdr *) buf;
    pNlhdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    pNlhdr->nlmsg_type = RTM_GETROUTE;
    pNlhdr->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    pNlhdr->nlmsg_seq = ++ rtseq;
    pNlhdr->nlmsg_pid = getpid();
    
    // write kernel socket
    int ret = write(sfd, pNlhdr, pNlhdr->nlmsg_len);
    
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
        
        pNlhdr = (struct nlmsghdr *) buf;
        
        struct rtmsg    * pRtmsg;
        struct rtattr   * pRtattr;
        
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
    
    if (pRtCon == NULL)
        return false;
    
    // try to forward the packet
    // or just push the packet in the waiting message queue
    //      to let it be forwarded in next schedule
    struct arpcon * pArpCon;
    
    if (isDefaultAddr(pIphdr->daddr)) {
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
    struct sockaddr_ll sadll;
    memset(&sadll, 0, sizeof(sadll));
    
    struct ifcon * pIntCon;
    pIntCon = Interface::GetIfconById(pRtCon->ifid);
    if (pIntCon == NULL)
        return false;
    
    sadll.sll_ifindex = pIntCon->ifid;
    sadll.sll_pkttype = PACKET_OUTGOING;
    
    memcpy(&pEthhdr->h_dest, pArpCon->mac, ETH_ALEN);
    memcpy(&pEthhdr->h_source, pIntCon->mac, ETH_ALEN);
    pIphdr->ttl --;
    if (pIphdr->ttl <= 0) {
        // ignore ... 
        return false;
    }
    CalIpChecksum(pIphdr);
    
    // do send the message
    if (g_wtc->GetMainSFD() < 0) {
        g_log->Error("router detect watcher didnot set main socket fd");
        return false;
    }
    
    size_t nsend;
    nsend = sendto(g_wtc->GetMainSFD(), pMsg->ReadPos(), pMsg->Length(), 0, (struct sockaddr *) &sadll, sizeof(sadll));
    
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
    u_int32_t flag_mask = 0;
    for (rit = loc_RIB.begin(); rit != loc_RIB.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        u_int32_t mask;
        mask = pRtCon->mask.s_addr;
        if (InAddrCmp(pAd, &pRtCon->dest, &pRtCon->mask)) {
            if (mask > flag_mask) {
                ret = pRtCon;
                flag_mask = mask;
            } else if (mask == flag_mask) {
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
    u_int32_t flag_mask = 0;
    for (rit = loc_RIB.begin(); rit != loc_RIB.end(); ++rit) {
        pRtCon = *rit;
        assert(pRtCon != NULL);
        u_int32_t mask;
        mask = 0xffffffff;
        mask = mask << (32 - pPre->maskln);
        if (InAddrCmp(&pRtCon->dest, pPre)) {
            if (mask > flag_mask) {
                ret = pRtCon;
                flag_mask = mask;
            } else if (mask == flag_mask) {
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
Router::ARPRos(Message * pMsg)
{
    if (pMsg == NULL)
        return;
    struct eth_arphdr * pArphdr;
    pArphdr = (eth_arphdr *) pMsg->ReadPos();
    assert(pArphdr != NULL);
    
    if (pArphdr->hdr.ar_op != htons(ARPOP_REPLY)) {
        return ;
    }
    
    struct arpcon * pArpCon;
    pArpCon = (struct arpcon *) malloc( sizeof(struct arpcon));
    assert(pArpCon != NULL);
    
    // set target ip address
    memcpy(&pArpCon->ipadd, &pArphdr->ar_sip, sizeof(pArpCon->ipadd));
    // set target hardware address
    memcpy(&pArpCon->mac, &pArphdr->ar_sha, ETH_ALEN);
    vARPConf.push_back(pArpCon);
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
    
    struct rtcon * pRtCon;
    pRtCon = LookupRoutingTable(pAd);
    if (pRtCon == NULL)
        return;
    
    struct ifcon * pIntCon;
    pIntCon = Interface::GetIfconById(pRtCon->ifid);
    if (pIntCon == NULL)
        return;
    
    struct sockaddr_ll  sadll;
    struct timeval      timev;
    Buffer buf(1024);
    
    memset(&sadll, 0, sizeof(struct sockaddr_ll));
    sadll.sll_ifindex = pIntCon->ifid;
    sadll.sll_pkttype = PACKET_OUTGOING;
    
    timev.tv_sec = 1;
    timev.tv_usec = 0;
    
    sockfd sfd;
    sfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if ( setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev)) != 0) {
        g_log->Error("failed set socket optine in arp");
        return;
    }
    
    struct ethhdr * pEthhdr;
    pEthhdr = (struct ethhdr *) buf.ReadPos();
    
    struct eth_arphdr * pArphdr;
    pArphdr = (struct eth_arphdr *) buf.Reserve(sizeof(struct ethhdr));
    
    // fill ether header
    memcpy(pEthhdr->h_source, pIntCon->mac, ETH_ALEN);
    memset(pEthhdr->h_dest, 0xff, ETH_ALEN);
    pEthhdr->h_proto = htons(ETH_P_ARP);
    
    // fill arp header
    pArphdr->hdr.ar_hrd = htons((__be16)ARPHRD_ETHER);
    pArphdr->hdr.ar_pro = htons(ETH_P_IP);
    pArphdr->hdr.ar_hln = ETH_ALEN;
    pArphdr->hdr.ar_pln = 4;
    pArphdr->hdr.ar_op  = htons(ARPOP_REQUEST);
    memcpy(pArphdr->ar_sha, pIntCon->mac, ETH_ALEN);
    memset(pArphdr->ar_tha, 0, ETH_ALEN);
    memcpy(&pArphdr->ar_sip, &pIntCon->ipaddr, sizeof(pArphdr->ar_sip));
    memcpy(&pArphdr->ar_tip, pAd, sizeof(pArphdr->ar_tip));
    
    size_t nsend;
    nsend = sendto(sfd, buf.ReadPos(), buf.Length(), 0, (struct sockaddr *) &sadll, sizeof(sadll));
    
    if (nsend < 0)
        g_log->Warning("Arp send failed");
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
    pRtCon = (struct rtcon *) malloc(sizeof(struct rtcon));
    for (vector<struct _prefix *>::iterator pIt = pUpInfo->nlri.begin();
        pIt != pUpInfo->nlri.end();
            ++ pIt) {
        struct _prefix  * pPre = *pIt;
        pRtCon = LookupRoutingTable(pPre);
    }
    g_log->LogRouteList();
}
