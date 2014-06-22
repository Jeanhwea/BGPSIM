#ifndef INTERFACE_870ABUR8

#define INTERFACE_870ABUR8

#include "global.h"

using namespace std;

#pragma pack(push)
#pragma pack(1)
struct ifcon {
        u_char              mac[ETH_ALEN];
        struct in_addr      netmask;
        struct in_addr      ipaddr;
        struct in_addr      broadcast;
        int                 id;
        char                name[128];
};
#pragma pack(pop)

class Interface {
    private:

    public:
        ifcon  conf;
        
        Interface();
        virtual ~Interface();
        void LoadInfo();
};

extern vector<Interface *> vInt;
#endif /* end of include guard: INTERFACE_870ABUR8 */
