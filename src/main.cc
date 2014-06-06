#include "main.h"

using namespace std;

int 
main(int argc, char const *argv[])
{
    if (isDebug) printf("Debug is on ... ...\n");
    Timer tim;
    Simulator sim;
    Listener lis;
    Event eve;
    Message msg;

    tim.Start();
    tim.Join();
            

    // lis.Start();
    // lis.Join();

    return 0;
}
