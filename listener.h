#ifndef LISTENER_FCV7JYL9

#define LISTENER_FCV7JYL9

#include "global.h"
#include "thread.h"

using namespace std;

class Listener : public Thread {
    public:
        Listener ();
        virtual ~Listener ();
        bool SetMainSocket();
        bool ListenAll();
        bool SetPromisc(string ifname);
        bool UnsetPromisc(string ifname);
        bool SetPromisc(string ifname, int sockfd);
        bool UnsetPromisc(string ifname, int sockfd);
        void OutputPacket(struct iphdr * );

        void * Run() {
            ListenAll();
            return NULL;
        }

        // tips: > ifconfig eth0 promisc # set eth0 to promisc model
        // tips: > ifconfig eth0 -promisc # set eth0 to none promisc model

    private:
        vector<int> vSockFd;

};

#endif /* end of include guard: LISTENER_FCV7JYL9 */
