#ifndef LISTENER_FCV7JYL9

#define LISTENER_FCV7JYL9

#include "global.h"
#include "Thread.h"

using namespace std;

class Listener : public Thread {
    private:
        vector<int> vSockFd;

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
};

#endif /* end of include guard: LISTENER_FCV7JYL9 */
