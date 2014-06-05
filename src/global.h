#ifndef GLOBAL_H

#define GLOBAL_H

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#define BGP_PORT 179

typedef int sockfd;

extern int errno;
extern bool isDebug;

extern FILE * outfd;
extern FILE * errfd;
extern FILE * logfd;

#endif /* end of include guard: GLOBAL_H */
