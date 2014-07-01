#include "main.h"

using namespace std;

Logger    * g_log = NULL;
Interface * g_int = NULL;
Router    * g_rtr = NULL;
Simulator * g_sim = NULL;
Watcher   * g_wtc = NULL;
Timer     * g_timer = NULL;

int
main(int argc, char const *argv[])
{
    if (isDebug)
        cout << "Debug is on ..." << endl;
    
    g_log = new Logger;
    g_int = new Interface;
    g_rtr = new Router;
    g_wtc = new Watcher;
    g_timer = new Timer();
    
    g_int->LoadInterface();
    g_log->LogIntList();
    
    g_rtr->LoadKernelRouter();
    g_rtr->LoadRouterConf("./config/route.conf");
    g_log->LogRouteList();

    g_log->LogARPCache();
    
    g_sim = new Simulator;

    g_timer->Start();
    g_wtc->Start();
    g_sim->Start();
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
