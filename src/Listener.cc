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
    return NULL;
}


#define MAX_BACKLOG 10

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
