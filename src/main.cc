#include "main.h"

using namespace std;

Logger    * g_log = NULL;
Interface * g_int = NULL;
Router    * g_rtr = NULL;
Simulator * g_sim = NULL;
Watcher   * g_wtc = NULL;
Timer     * g_tmr = NULL;

int
main(int argc, char const *argv[])
{
    if (isDebug)
        cout << "Debug is on ..." << endl;
    
    g_log = new Logger;
    g_int = new Interface;
    g_rtr = new Router;
    g_wtc = new Watcher;
    g_tmr = new Timer();
    
    g_int->LoadInterface();
    g_log->LogIntList();
    
    g_rtr->LoadKernelRoute();
    g_rtr->LoadRouterConf("./config/route.conf");
    g_log->LogRouteList();


    g_tmr->Start();
    g_wtc->Start();
    
    struct in_addr * pAd;
    in_addr_t      Adt;
    Adt = inet_addr("192.168.4.1"),
    pAd = (struct in_addr *) &Adt;
    // g_rtr->ARPReq(pAd);

    //return 0; // test return

    
    g_sim = new Simulator;

    //g_sim->Start();
    g_wtc->Join();
    

    if (g_sim != NULL)
        delete g_sim;
    if (g_wtc != NULL)
        delete g_wtc;
    if (g_int != NULL)
        delete g_int;
    if (g_rtr != NULL)
        delete g_rtr;
    if (g_log != NULL)
        delete g_log;
    return 0;
}
