#include "main.h"

using namespace std;

Simulator * g_sim = NULL;
Logger    * g_log = NULL;

int
main(int argc, char const *argv[])
{
    if (isDebug) printf("Debug is on ... ...\n");
    g_sim = new Simulator;
    g_log = new Logger;
    g_sim->Start();
    g_sim->Join();
    return 0;
}
