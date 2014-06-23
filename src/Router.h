#ifndef ROUTER_J2V5X69X

#define ROUTER_J2V5X69X

#include "global.h"

using namespace std;

#pragma pack(push)
#pragma pack(1)

struct rtcon {
    struct in_addr  dest;
    struct in_addr  nhop;
    struct in_addr  mask;
    int             ifid;
};

#pragma pack(pop)

class Router {
    private:
        static int rtseq;
        struct in_addr MaskToAddr(int mask);

    public:
        Router();
        virtual ~Router();
        
        void LoadKernelRoute();
};


extern vector<struct rtcon *> vRtConf;
#endif /* end of include guard: ROUTE_J2V5X69X */
