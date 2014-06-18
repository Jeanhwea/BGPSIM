#include "Listener.h"
#include "Simulator.h"

using namespace std;

#define BUF_SIZE        8096
#define IP_ADDR_SIZE    64

vector<Listener *>          vListeners;

Listener::Listener(struct in_addr * pL, struct in_addr * pR)
{
    assert(pL != NULL);
    assert(pR != NULL);
    afd = lfd = -1;
    pLAdd = (struct in_addr *) malloc(sizeof(struct in_addr));
    pRAdd = (struct in_addr *) malloc(sizeof(struct in_addr));
    assert(pLAdd != NULL);
    assert(pRAdd != NULL);
    memcpy(pLAdd, pL, sizeof(struct in_addr));
    memcpy(pRAdd, pR, sizeof(struct in_addr));
}

Listener::Listener(struct in_addr * pL)
{
    assert(pL != NULL);
    afd = lfd = -1;
    pLAdd = (struct in_addr *) malloc(sizeof(struct in_addr));
    pRAdd = (struct in_addr *) malloc(sizeof(struct in_addr));
    assert(pLAdd != NULL);
    assert(pRAdd != NULL);
    memcpy(pLAdd, pL, sizeof(struct in_addr));
    pRAdd->s_addr = htonl(INADDR_ANY);
}

Listener::~Listener()
{
    cout << "listen deconstruct" << endl;
    if (pLAdd != NULL)
        free(pLAdd);
    if (pRAdd != NULL)
        free(pRAdd);
}

void * Listener::Run()
{
    g_log->Tips("Listener runs ... ");
    if ( !InitConn(pLAdd) )
        return NULL;


    Peer *                  pPeer;
    struct sockaddr_in      sad;
    socklen_t               len;
    while (true) {
        if (!TryAccept(&sad))
            continue;
        len = sizeof(struct sockaddr_in);
        if (getpeername(afd, (struct sockaddr *)&sad, &len) == 0)
            g_log->Tips("successfully accept a socket");
        pPeer = g_sim->GetPeerByAddr(&sad);
        if (pPeer == NULL) {
            g_log->Warning("Cannot find a peer");
            shutdown(afd, SHUT_RDWR);
            close(afd);
            continue;
        }
        pPeer->Lock();
        if (pPeer->GetPeerState()==CONNECT || pPeer->GetPeerState()==ACTIVE){
            if (pPeer->sfd != -1) {
                if (pPeer->GetPeerState() == CONNECT) {
                    g_sim->SimColseConnect(pPeer);
                } else {
                    g_log->Tips("shutdown accept socket fd");
                    shutdown(afd, SHUT_RDWR);
                    close(afd);
                }
            }
            pPeer->sfd = afd;
            if (!g_sim->SimSetupSocket(pPeer)) {
                    shutdown(afd, SHUT_RDWR);
                    close(afd);
            }
            pPeer->conf.passive = true;
            pPeer->Start(BGP_TRANS_CONN_OPEN);
        }
        pPeer->Unlock();
    }
    return NULL;
}



bool
Listener::InitConn(struct in_addr * pAd)
{
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        g_log->Error("Cannot init peer socket1");
        return false;
    }

    struct sockaddr_in  sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = pAd->s_addr;
    sad.sin_port = htons(BGP_PORT);
    if (bind(lfd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        g_log->ShowErrno();
        close(lfd);
        lfd = -1;
        return false;
    } else
        g_log->Tips("Bind success");

    //SetNonBlock(lfd);

    if (listen(lfd, MAX_BACKLOG) == -1) {
        g_log->Error("Cannot listen peer socket");
        return false;
    } else
        g_log->Tips("Listen success");

    return true;
}

bool
Listener::TryAccept(struct sockaddr_in * pSad)
{
    socklen_t           len;
    len = sizeof(struct sockaddr_in);
    assert(pSad != NULL);
    memset(pSad, 0, sizeof(struct sockaddr_in));
    pSad->sin_family = AF_INET;
    pSad->sin_addr.s_addr = htonl(INADDR_ANY);
    pSad->sin_port = htons(BGP_PORT);
    if (lfd == -1)
        g_log->Error("Cannot accept socket with -1 listen fd");
    afd = accept(lfd, (struct sockaddr *)pSad, &len);
    return (afd != -1);
}

struct in_addr *
Listener::GetLisAddr()
{
    return pLAdd;
}

bool
Listener::SetNonBlock(sockfd sfd)
{
    int nodelay = 1 ;
    if (setsockopt( sfd, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(int)) == -1) {
        g_log->Error("Cannot set non-block");
        return false;
    }
    return true;
}

bool
Listener::UnsetNonBlock(sockfd sfd)
{
    return true;
}

bool
Listener::SetTTL(sockfd sfd, int ttl)
{
    int _ttl = ttl;
    if (setsockopt(sfd, IPPROTO_IP, IP_TTL, &_ttl, sizeof(_ttl)) == -1) {
        g_log->Error("failed to set TTL");
        return false;
    }
    return true;
}
