#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QString>
#include <vector>
#include <cstdio>
#include <cstring>
#include "qglobal.h"
#include "assembler.h"

// 其它宏定义
#define MAX_INST_NUM    1000
#define MAX_MEMSIZE     2048
#define DEFAULT_BP      0xffff

using namespace std;

struct meminfo {
    QString addr;
    QString hex;
    QString asciz;
};

class Debugger : public Assembler
{
private:
    uint regfile[32];   // 模拟32个寄存器
    uint memsize;       // 主存大小
    uint *memory;       // 主存内容
    uint pc;            // PC值
    QString ir;         // IR内容
    int initPCaddr;     // PC初始值
    uint breakpoint;    // 断点表
    uint changedMemAddr;// 被修改的内存地址
    vector<meminfo>memoryText;  // 内存信息向量
     std::vector<QString>inst_vec;  //指令存储区
    ~Debugger();

public:
    Debugger();
    Debugger(int size);

    std::vector<Assembler::instruction>stdinput;    //处理后的标准化输入
    /*
     * 私有成员接口函数
     */
    void clearMemoText()                            { memoryText.clear(); }
    void resizeMemoText(int size)                   { memoryText.resize(size); }
    int getMemoTextSize()                           { return memoryText.size(); }
    QString getMemoTextHex(int idx)                 { return memoryText[idx].hex; }
    QString getMemoTextAddr(int idx)                { return memoryText[idx].addr; }
    QString getMemoTextAsciz(int idx)               { return memoryText[idx].asciz; }
    void setMemoTextHex(int idx, QString hex)       { memoryText[idx].hex = hex; }
    void setMemoTextAddr(int idx, QString addr)     { memoryText[idx].addr = addr; }
    void setMemoTextAsciz(int idx, QString asciz)   { memoryText[idx].asciz = asciz; }
    void pushbackMemoText(struct meminfo mt)        { memoryText.push_back(mt); }

    void clearInstVec()                             { inst_vec.clear(); }
    uint getInstVecSize()                            { return inst_vec.size(); }
    QString getInst(int idx)                        { return inst_vec[idx]; }
    void pushbackInstVec(QString inst)              { inst_vec.push_back(inst); }

    /*
     * 三大基本功能，执行、单步、重置
     */
    int run(Assembler assembler);      // 返回执行了多少条指令
    int step(Assembler assembler);     // 成功返回1，否则返回0
    void reset(Assembler assembler);

    /*
     * PC处理函数
     */
    uint getPC() { return pc; }
    void setPC(uint c) { pc = c; }

    /*
     * IR和寄存器堆处理函数
     */
    QString getIR() { return ir; }
    uint getReg(int idx) { return regfile[idx]; }

    /*
     * 主存处理函数
     */
    uint getMemSize() { return memsize; }
    void setMemory(uint address, uint data);
    uint getMem(int addr) { return memory[addr]; }
    uint getChangedMemAddr();
    void loadData(Assembler assembler);

    /*
     * 断点处理函数
     */
    void clearBreakPoint() { breakpoint = DEFAULT_BP; }
    void setBreakPoint(uint value) { breakpoint = value; }
    uint getBreakPoint() { return breakpoint; }

    /*
     * 各类指令的单周期CPU模拟处理函数
     */
    void _3R_handler(int inst_idx, int rd, int rj, int rk);
    void _2R_handler(int inst_idx, int rd, int rj);
    void _2RI8_handler(int inst_idx, int rd, int rj, int imm);
    void _2RI12_handler(int inst_idx, int rd, int rj, int imm);
    void _2RI14_handler(int inst_idx, int rd, int rj, int imm);
    void _2RI16_handler(int inst_idx, int rd, int rj, int imm, bool *branch_taken);
    void _1RI20_handler(int inst_idx, int rd, int imm);
    void _BAR_handler(int inst_idx, int hint);
};


#endif // DEBUGGER_H
