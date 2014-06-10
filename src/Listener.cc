#include "Listener.h"

using namespace std;

#define BUF_SIZE        8096
#define IP_ADDR_SIZE    64

Listener::Listener(in_addr & l, in_addr & r)
{
    afd = lfd = -1;
    memcpy(&la, &l, sizeof(l));
    memcpy(&ra, &r, sizeof(r));
}


Listener::~Listener()
{

}

void * Listener::Run()
{
    if ( !InitConn(la) )
        return NULL;
    while (true) {
        if (!TryAccept(ra))
            continue;
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
    sad.sin_addr = lisaddr;
    sad.sin_port = htons(BGP_PORT);

    if (bind(lfd, (struct sockaddr *)&sad, sizeof(sad)) == -1) {
        close(lfd);
        lfd = -1;
        g_log->Error("Cannot bind peer socket");
        return false;
    }

    UnsetBlock(lfd);

    if (listen(lfd, MAX_BACKLOG) == -1) {
        g_log->Error("Cannot listen peer socket");
        assert(false);
        return false;
    }

    return true;
}

bool
Listener::TryAccept(struct in_addr & addr)
{
    struct sockaddr_in  sad;
    socklen_t           len;
    len = sizeof(struct sockaddr_in);
    sad.sin_family = AF_INET;
    sad.sin_addr = addr;
    sad.sin_port = htons(BGP_PORT);
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
