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
        int                 ifid;
        char                name[IFNAMSIZ];
};
#pragma pack(pop)

struct _prefix;

class Interface {
    private:

    public:
 
        Interface();
        virtual ~Interface();
        void LoadInterface();

        static char * GetIfNameById(int ifid);
        static struct ifcon * GetIfconById(int ifid);
        static int GetIfidByName(char *);
        static struct ifcon * GetIfByAddr(struct in_addr * pAd);
        static struct ifcon * GetIfByAddr(struct _prefix * pPre);
};

extern vector<struct ifcon *> vIntConf;
#endif /* end of include guard: INTERFACE_870ABUR8 */
