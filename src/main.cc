#include "main.h"

using namespace std;

Logger    * g_log = NULL;
Simulator * g_sim = NULL;

int
main(int argc, char const *argv[])
{
    if (isDebug)
        cout << "Debug is on ..." << endl;
    g_log = new Logger;
    
    Interface intface;
    intface.LoadInfo();
    g_log->LogIntList();
    
    Route route;
    route.LoadKernelRoute();
    g_log->LogRouteList();
    
    Watcher watch;
    watch.Start();

    g_sim = new Simulator;
    

    g_sim->Start();
    g_sim->Join();

    if (g_sim != NULL)
        delete g_sim;
    if (g_log != NULL)
        delete g_log;
    return 0;
}
