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
Router::Forward(Message * pMsg)
{
}

void
Router::ARPRosp(Message * pMsg)
{

}
