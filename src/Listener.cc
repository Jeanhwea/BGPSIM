#include "Listener.h"
#include "Simulator.h"

using namespace std;

#define BUF_SIZE        8096
#define IP_ADDR_SIZE    64

Listener::Listener(struct in_addr & l, struct in_addr & r)
{
    afd = lfd = -1;
    memcpy(&la, &l, sizeof(l));
    memcpy(&ra, &r, sizeof(r));
}

Listener::Listener(struct in_addr & l)
{
    afd = lfd = -1;
    memcpy(&la, &l, sizeof(l));
}

Listener::~Listener()
{

}

void * Listener::Run()
{
    Peer *              pPeer;
    struct sockaddr_in  sad;
    socklen_t           len;

    if ( !InitConn(la) )
        return NULL;
    while (true) {
        if (!TryAccept(sad))
            continue;
        assert(afd != -1);
        if (getpeername(afd, (struct sockaddr *) &sad, &len) > 0)
            g_log->Tips("accept a socket");
        pPeer = g_sim->GetPeerByAddr(sad);
        pPeer->sfd = afd;
        g_sim->FSM(pPeer, BGP_TRANS_CONN_OPEN);
        cout << pPeer->Self() << endl;
    }
    return NULL;
}



bool
Listener::InitConn(struct in_addr & lisaddr)
{
    struct sockaddr_in  sad;
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        g_log->Error("Cannot init peer socket1");
        return false;
    }

    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = lisaddr.s_addr;
    sad.sin_port = htons(BGP_PORT);

    if (bind(lfd, (struct sockaddr *)&sad, sizeof(sad)) == -1) {
        g_log->ShowErrno();
        close(lfd);
        lfd = -1;
        g_log->Error("Cannot bind peer socket");
        return false;
    } else
        g_log->Tips("Bind success");

    UnsetBlock(lfd);

    if (listen(lfd, MAX_BACKLOG) == -1) {
        g_log->Error("Cannot listen peer socket");
        assert(false);
        return false;
    }

    return true;
}

bool
Listener::TryAccept(struct sockaddr_in & sad)
{
    socklen_t           len;
    len = sizeof(struct sockaddr_in);
    memset(&sad, 0, sizeof(sad));
//     sad.sin_family = AF_INET;
//     sad.sin_addr = htonl(INADDR_ANY);
//     sad.sin_port = htons(BGP_PORT);
    if (lfd == -1)
        g_log->Error("Cannot accept socket with -1 listen fd");
    afd = accept(lfd, (struct sockaddr *)&sad, &len);
    return (afd != -1);
}


bool
Listener::SetBlock(sockfd sfd)
{
    int flags;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        g_log->Fatal("fnctl F_GETFL");
        return false;
    }

    flags &= ~O_NONBLOCK;

    flags = fcntl(sfd, F_SETFL, flags);
    if (flags == -1) {
        g_log->Fatal("fnctl F_SETFL");
        return false;
    }

    return true;
}

bool
Listener::UnsetBlock(sockfd sfd)
{
    int flags;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        g_log->Fatal("fnctl F_GETFL");
        return false;
    }

    flags |= O_NONBLOCK;

    flags = fcntl(sfd, F_SETFL, flags);
    if (flags == -1) {
        g_log->Fatal("fnctl F_SETFL");
        return false;
    }

    return true;
}
