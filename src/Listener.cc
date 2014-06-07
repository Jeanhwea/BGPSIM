#include "Listener.h"

using namespace std;

#define BUF_SIZE        8096
#define IP_ADDR_SIZE    64

Listener::Listener()
{
    SetMainSocket();
}

Listener::~Listener()
{

}

bool 
Listener::SetMainSocket()
{
    // htons h:host n:network s:short
    // AF_foo := address family
    // PF_foo := protocol family
    mfd = socket( AF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
    if (mfd < 0) {
        log.Error("failed to add main socket");
        return false;
    }

    return true;
}

sockfd
Listener::Init() 
{
    struct sockaddr_in  sad;
    sockfd              sfd;

    sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sfd == -1) {
        log.Warning("cannot init listen socket");
        return (-1);
    }

    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(BGP_PORT);

    if ( bind(sfd, (struct sockaddr *)&sad, sizeof(sad)) == -1 ) {
        close(sfd);
        log.Error("cannot bind socket");
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
        log.Warning("cannot listen");
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
    struct sockaddr_in  sad;

    len = sizeof(sad);
    connfd = accept(lisfd, (struct sockaddr *)&sad, &len);
    if (connfd == -1) {
        if (errno != EWOULDBLOCK && errno != EINTR) 
            log.Warning("accept bug in Listener");
        return (-1);
    }
    
    UnsetBlock(connfd);

    return connfd;
}

bool
Listener::SetBlock(sockfd sfd) 
{
    int flags;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        log.Fatal("fnctl F_GETFL");
        return false;
    }

    flags &= ~O_NONBLOCK;

    flags = fcntl(sfd, F_SETFL, flags);
    if (flags == -1) {
        log.Fatal("fnctl F_SETFL");
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
        log.Fatal("fnctl F_GETFL");
        return false;
    }

    flags |= O_NONBLOCK;

    flags = fcntl(sfd, F_SETFL, flags);
    if (flags == -1) {
        log.Fatal("fnctl F_SETFL");
        return false;
    }

    return true;
}
