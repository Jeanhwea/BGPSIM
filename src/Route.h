#ifndef ROUTE_J2V5X69X

#define ROUTE_J2V5X69X

#include "global.h"

using namespace std;

#pragma pack(push)
#pragma pack(1)

struct rtcon {
    struct in_addr dest;
    struct in_addr nhop;
    struct in_addr mask;
};

#pragma pack(pop)

class Route {
    private:
        static int rtseq;
        struct in_addr MaskToAddr(int mask);

    public:
        rtcon conf;
        int   netmask;
        int   inter_id;
        Route();
        virtual ~Route();
        
        void LoadKernelRoute();
        char * GetInterface();
};


extern vector<Route *> vRoute;
#endif /* end of include guard: ROUTE_J2V5X69X */
