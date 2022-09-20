#include "debugger.h"

Debugger::Debugger()
{
    for (int i = 0; i < 32; ++i)
        regfile[i] = 0;
    pc = 0;
    ir = 0;
    breakpoint = DEFAULT_BP;
    initPCaddr = 0;
}

Debugger::~Debugger()
{
    ;
}

int Debugger::run()
{
    int cnt = 0;
    return cnt;
}

int Debugger::step()
{
    return 0;
}

void Debugger::reset()
{
    for (int i = 0; i < 32; ++i)
        regfile[i] = 0;
    ir = 0;
    pc = initPCaddr;
}
