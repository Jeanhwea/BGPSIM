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
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/icmp.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <queue>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>


#define	BGP_VERSION     4
#define	BGP_PORT        179

#define	MAX_PKTSIZE     4096
#define	MIN_HOLDTIME    3
#define	READ_BUF_SIZE   65535
#define	RT_BUF_SIZE     16384

typedef enum {
    IDLE,
    CONNECT,
    ACTIVE,
    OPENSENT,
    OPENCONFIRM,
    ESTABLISHED
} state_t;

typedef enum {
    BGP_START,
    BGP_STOP,
    BGP_TRANS_CONN_OPEN,
    BGP_TRANS_CONN_CLOSED,
    BGP_TRANS_CONN_OPEN_FAILED,
    BGP_TRANS_FATAL_ERROR,
    CONN_RETRY_TIMER_EXPIRED,
    HOLD_TIMER_EXPIRED,
    KEEPALIVE_TIMER_EXPIRED,
    RECV_OPEN_MSG,
    RECV_KEEPALIVE_MSG,
    RECV_UPDATE_MSG,
    RECV_NOTIFICATION_MSG
} event_t;

typedef int sockfd;

extern int errno;
extern bool isDebug;
extern time_t walltime;

extern FILE * outfd;
extern FILE * errfd;
extern FILE * logfd;

class Logger;
extern Logger * g_log;

class Interface;
extern Interface * g_int;

class Router;
extern Router * g_rtr;

class Watcher;
extern Watcher * g_wtc;

class Timer;
extern Timer * g_tmr;

class Simulator;
extern Simulator * g_sim;
#endif /* end of include guard: GLOBAL_H */
