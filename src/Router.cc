#include "Router.h"
#include "Logger.h"
#include "Interface.h"
#include "Message.h"

using namespace std;

vector<struct arpcon *> vARPConf;
vector<struct rtcon *> vRtConf;

int Router::rtseq = 0;

Router::Router()
{

}


Router::~Router()
{

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



#define BUFSIZE_MAXRT 8096
void 
Router::LoadKernelRoute()
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
            vRtConf.push_back(pRtCon);
            
            pNlhdr = NLMSG_NEXT(pNlhdr, nread);
        }
        
        if (pNlhdr->nlmsg_type == NLMSG_DONE || (pNlhdr->nlmsg_flags & NLM_F_MULTI) == 0) 
            break;
    }
    
}


void
Router::PacketForward(Message * pMsg)
{
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
    
}

void 
Router::ARPReq(struct in_addr * pAd, struct ifcon * pIntCon)
{
    struct arpcon * pArpCon;
    pArpCon = LookupARPCache(pAd);
    if (pArpCon != NULL) 
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
    nsend = sendto(sfd, buf.ReadPos(), 60, 0, (struct sockaddr *) &sadll, sizeof(sadll));
    
    if (nsend < 0)
        g_log->Warning("Arp send failed");
}

struct arpcon * 
Router::LookupARPCache(in_addr* pAd)
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

