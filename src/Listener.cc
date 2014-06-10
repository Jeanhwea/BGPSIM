#include "Listener.h"

using namespace std;

#define BUF_SIZE        8096
#define IP_ADDR_SIZE    64

Listener::Listener()
{
}

Listener::~Listener()
{

}

void * Listener::Run()
{
    g_log->Tips("Listener runs ...");
    SetMainSocket();
    Init();
    Listen();

    sockfd  ac_sfd;
    size_t nread = 0;
    u_char  buf[40960];
    for (;;) {
        ac_sfd = Accept(mfd);
        if (ac_sfd != -1) {
            nread = read(ac_sfd, buf, 40960);
            g_log->LogDumpMsg(buf, nread);
        }
        close(ac_sfd);
    }
    return NULL;
}


bool
Listener::SetMainSocket()
{
    // htons h:host n:network s:short
    // AF_foo := address family
    // PF_foo := protocol family
    mfd = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
    if (mfd < 0) {
        g_log->Error("failed to add main socket");
        return false;
    } else
        g_log->Tips("add main socket in listener");

    return true;
}

sockfd
Listener::Init()
{
    struct sockaddr_in  sad;
    sockfd              sfd;

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        g_log->Warning("cannot init listen socket");
        return (-1);
    }

    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(BGP_PORT);

    if ( bind(sfd, (struct sockaddr *)&sad, sizeof(sad)) == -1 ) {
        close(sfd);
        g_log->Error("cannot bind socket");
        return (-1);
    }

    UnsetBlock(sfd);
    mfd = sfd;

    return sfd;
}

#define MAX_BACKLOG     10

sockfd
Listener::Listen()
{
    if (listen(mfd, MAX_BACKLOG) == -1) {
        g_log->Error("cannot listen");
        assert(false);
        return (-1);
    }

    return mfd;
}

void
Listener::Shutdown()
{
    close(mfd);
}

sockfd
Listener::Accept(sockfd lisfd)
{
    sockfd              connfd;
    socklen_t           len;
    struct sockaddr_in *sad = NULL;

    len = sizeof(struct sockaddr_in);
//     sad.sin_addr.s_addr = htonl(INADDR_ANY);
//     sad.sin_family = AF_INET;
//     sad.sin_port = htons(BGP_PORT);
    connfd = accept(lisfd, (struct sockaddr *)sad, &len);
    if (connfd == -1) {
        if (errno != EWOULDBLOCK && errno != EINTR)
            g_log->Warning("accept bug in Listener");
        return (-1);
    } else {
        g_log->Tips("Accept successfully !!! ");
        UnsetBlock(connfd);
    }

    return connfd;
}

bool
Listener::InitPeerConn(Peer * pPeer)
{
    struct sockaddr_in  sad;
    sockfd              sfd;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) {
        g_log->Error("Cannot init peer socket1");
        return false;
    }

    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr = pPeer->conf.local_addr;
    sad.sin_port = htons(BGP_PORT);

    if (bind(sfd, (struct sockaddr *)&sad, sizeof(sad)) == -1) {
        close(sfd);
        g_log->Error("Cannot bind peer socket");
        return false;
    }

    UnsetBlock(sfd);

    if (listen(sfd, MAX_BACKLOG) == -1) {
        g_log->Error("Cannot listen peer socket");
        assert(false);
        return false;
    }

    sockfd      acfd;
    socklen_t   len;
    len = sizeof(struct sockaddr_in);
    sad.sin_family = AF_INET;
    sad.sin_addr = pPeer->conf.remote_addr;
    sad.sin_port = htons(BGP_PORT);
    size_t      nread = 0;
    u_char      buf[40960];
    for (;;) {
        acfd = accept(sfd, (struct sockaddr *)&sad, &len);
        if (acfd == -1) continue;
        pPeer->sfd = acfd;
        nread = read(acfd, buf, 40960);
        g_log->LogDumpMsg(buf, nread);
        close(acfd);
        return true;
    }

    return true;
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
