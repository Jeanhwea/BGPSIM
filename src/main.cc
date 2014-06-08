#include "main.h"

using namespace std;

Simulator * g_sim;

int
main(int argc, char const *argv[])
{
    if (isDebug) printf("Debug is on ... ...\n");
    g_sim = new Simulator();
    g_sim->Run();
    g_sim->Join();
    return 0;
}
