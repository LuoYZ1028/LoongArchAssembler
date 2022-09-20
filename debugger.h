#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include "ctype.h"

#define MAX_MEMSIZE 65536
#define MAX_INSTNUM 128
#define DEFAULT_BP  0xffff

using namespace std;

typedef unsigned int uint;

class Debugger
{
private:
    uint regfile[32];
    uint pc;
    uint ir;
    int initPCaddr;
    ~Debugger();
    uint breakpoint;

public:
    Debugger();

    int run();      // 返回执行了多少条指令
    int step();     // 成功返回1，否则返回0
    void reset();   // 重置操作

    uint getPC() { return pc; }
    void setPC(uint c) { pc = c; }

    uint getIR() { return ir; }
    uint getReg(int idx) { return regfile[idx]; }

    uint getBreakpoint() { return breakpoint; }
    void setBreakpoint(uint &value) { breakpoint = value; }
};


#endif // DEBUGGER_H
