#ifndef LISTENER_FCV7JYL9

#define LISTENER_FCV7JYL9

#include "global.h"
#include "Logger.h"
#include "Thread.h"

using namespace std;

class Listener : public Thread {
    private:
        sockfd      mfd; // member sockfd

        Logger      log;
    public:
        Listener();
        virtual ~Listener();
        void * Run();
        bool SetMainSocket();

        // control operations
        sockfd Init();
        sockfd Listen();
        void Shutdown();
        sockfd Accept(sockfd lisfd);

        // socket helper
        bool SetBlock(sockfd sfd);
        bool UnsetBlock(sockfd sfd);
};

#endif /* end of include guard: LISTENER_FCV7JYL9 */
