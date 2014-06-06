#ifndef GLOBAL_H

#define GLOBAL_H

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#define	BGP_VERSION			4
#define	BGP_PORT			179
#define	BGPD_USER			"_bgpd"
#define	PEER_DESCR_LEN			32
#define	PFTABLE_LEN			16

#define	MAX_PKTSIZE			4096
#define	MIN_HOLDTIME			3
#define	READ_BUF_SIZE			65535
#define	RT_BUF_SIZE			16384

typedef int sockfd;

extern int errno;
extern bool isDebug;

extern FILE * outfd;
extern FILE * errfd;
extern FILE * logfd;

#endif /* end of include guard: GLOBAL_H */
