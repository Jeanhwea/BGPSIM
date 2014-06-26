#ifndef WATCHER_KPDU2Z6J

#define WATCHER_KPDU2Z6J

#include "global.h"
#include "Thread.h"

using namespace std;

class Watcher : public Thread {
    private:
        sockfd  sfd;

    public:
        Watcher();
        virtual ~Watcher();
        void * Run();
        
        bool InitMainSocket();
        bool SetPromisc();
        void StartListen();
        
        bool CheckInter(u_char mac[]);
        bool CheckInter(u_int32_t ipaddr);
        bool CheckInter(struct in_addr * pAd);
        
        sockfd GetMainSFD();
};

#endif /* end of include guard: WATCHER_KPDU2Z6J */
