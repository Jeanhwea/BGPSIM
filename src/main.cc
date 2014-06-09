#include "main.h"

using namespace std;

Logger    * g_log = NULL;
Simulator * g_sim = NULL;

int
main(int argc, char const *argv[])
{
    if (isDebug)
        printf("Debug is on ... ...\n");

    g_log = new Logger;
    g_sim = new Simulator;

    g_sim->Start();
    g_sim->Join();

    return 0;
}
