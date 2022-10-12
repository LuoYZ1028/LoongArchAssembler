#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <QString>
#include <QDebug>
#include <vector>
#include <cmath>
#include <QByteArray>

/*
 * 不同类型的指令总数
 */
#define _3R_NUM     20
#define _2R_NUM     3
#define _2R_I8_NUM  3
#define _2R_I12_NUM 16
#define _2R_I14_NUM 2
#define _2R_I16_NUM 9
#define _1R_I20_NUM 2
#define _BAR_NUM    2
#define _PSEUDO_NUM 2

/*
 * 不同类型指令的编号
 */
#define _3R     0
#define _2R     1
#define _2R_I8  2
#define _2R_I12 3
#define _2R_I14 4
#define _2R_I16 5
#define _1R_I20 6
#define _BAR    7
#define PSEUDO  8
#define TOT_TYPE_NUM 9
// 特别定义宏指令类型
#define MACRO   100

/*
 * 数据变量类型
 */
#define ASCIZ   1
#define WORD    2
#define SPACE   3
#define ASCIZ_BYTE  1
#define WORD_BYTE   4
#define SPACE_BYTE  8

/*
 * 错误类型
 */
#define TYPE_ERROR          -1
#define REGNO_ERROR         -2
#define MISS_AUG            -3
#define REDUND_AUG          -4
#define UNKNOWN_ERROR       -5
#define UNDEFINED_LABEL     -6
#define UNDEFINED_CONST     -7
#define UNDEFINED_VAR       -8
#define DFORMAT_ERROR       -9
#define RD_ZERO             -10
#define MACRO_NAME_ERROR    -11
#define SEG_ERROR           -100
#define MISS_TEXTSEG        -101
#define REDUND_TEXTSEG      -102
#define REDUND_DATASEG      -103
#define ORIGIN_ERROR        -104
#define ORIGIN_VERROR       -105

#define NO_ERROR        0

/*
 * 其它宏常量
 */
#define ZERO_REG        "00000"
#define UNSIGNED        0
#define SIGNED          1
#define MAX_LINE_NUM    1000
#define GET_NAME        0
#define GET_ALIAS       1
#define INST_MODE       0
#define DATA_MODE       1
#define TEST_DIR        "D:\\Acadamical_Materials\\SofwareB\\TestDir"
#define DEFAULT_TSA     65535
#define ERROR_REG       "11111"

class Assembler
{
public:
    Assembler();

    // 类型匹配函数
    int matchType(QString name, int mode);

    // 标号结构体，名称+地址
    struct label {
        QString name;
        int     address;
    };

    // 数据变量结构体，名称+类型+地址+大小+内容
    struct var {
        QString name;
        int     type;
        int     addr;
        int     size;
        QString contents;
    };

    // 宏定义结构体，名称+数值
    struct equ {
        QString name;
        QString value;
    };

    // 指令结构体，类型+行号+指令名+值域
    struct instruction {
        int     type;
        int     lineno;
        QString inst_name;
        QString valueline;
    };

    // 宏结构体，宏名+变量个数+指令个数+变量表+指令表
    struct macro {
        int             lineno;
        QString         name;
        int             var_num;
        int             inst_num;
        QList<QString>  var_list;
        QList<QString>  inst_list;
    };

    // 汇编转机器码主函数
    QString asm2Machine(instruction input);

    // 代码段起始地址（TSA）函数
    void changeTSA(QString input);
    uint get_TSA() { return textStartAddr; }

    /*
     * 进制转换函数
     */
    QString int2Binary(int input, int num, int mode);   // 10进制转2进制
    int hex2Int(QString hex);                           // 16进制转10进制
    QString bi2Hex(QString bin);                        // 2进制转16进制

    // 小端转换函数
    QString littleEndian(QString str);

    // 寄存器名获取函数
    QString getRegName(int id, int mode);

    /*
     * 改变or获取私有成员的接口函数
     */
    int getValid()                  { return valid; }
    void incValid()                 { valid++; }
    void clearValid()               { valid = 0; }

    int getErrorno()                { return error_no; }
    void setErrorno(int eno)        { error_no = eno; }

    int getErrorLine()              { return error_line; }
    void setErrorLine(int eline)    { error_line = eline; }

    int getErrorInst()              { return error_instno; }
    void setErrorInst(int einst)    { error_instno = einst; }

    int getLabelSize()              { return labellist.size(); }
    QString getLabelName(int idx)   { return labellist[idx].name; }
    int getLabelAddr(int idx)       { return labellist[idx].address; }
    void clearLabelList()           { labellist.clear(); }
    void pushbackLabel(struct label newlabel);

    int getVarlistSize()            { return varlist.size(); }
    QString getVarName(int idx)     { return varlist[idx].name; }
    int getVarAddr(int idx)         { return varlist[idx].addr; }
    int getVarType(int idx)         { return varlist[idx].type; }
    int getVarSize(int idx)         { return varlist[idx].size; }
    QString getVarContents(int idx) { return varlist[idx].contents; }
    void clearVarlist()             { varlist.clear(); }
    void pushbackVar(struct var newvar);

    int getEqulistSize()            { return equlist.size(); }
    QString getEquName(int idx)     { return equlist[idx].name; }
    QString getEquValue(int idx)    { return equlist[idx].value; }
    void clearEqulist()             { equlist.clear(); }
    void pushbackEqu(struct equ newequ);

    int getMacrolistSize()          { return macrolist.size(); }
    void clearMacrolist()           { macrolist.clear(); }
    QString getMacroName(int idx)   { return macrolist[idx].name; }
    int getMacroVarNum(int idx)     { return macrolist[idx].var_num; }
    int getMacroInstNum(int idx)    { return macrolist[idx].inst_num; }
    QString getMacroInst(int i, int j)  { return macrolist[i].inst_list[j]; }
    QString getMacroVar(int i, int j)   { return macrolist[i].var_list[j]; }
    int getMacroLineno(int idx)     { return macrolist[idx].lineno; }
    void pushbackMacro(struct macro newmacro);


protected:
    uint textStartAddr; // 记录代码段起始地址
    int getRegID(QString reg);  // 获取寄存器号
    /*
     * 寄存器名存储
     */
    QString regs_alias[32]  =  {"$r0","$r1","$r2","$r3",
                                "$r4","$r5","$r6","$r7",
                                "$r8","$r9","$r10","$r11",
                                "$r12","$r13","$r14","$r15",
                                "$r16","$r17","$r18","$r19",
                                "$r20","$r21","$r22","$r23",
                                "$r24","$r25","$r26","$r27",
                                "$r28","$r29","$r30","$r31"};

    QString regs_name[32]   =  {"$zero","$ra","$tp","$sp",
                                "$a0","$a1","$a2","$a3",
                                "$a4","$a5","$a6","$a7",
                                "$t0","$t1","$t2","$t3",
                                "$t4","$t5","$t6","$t7",
                                "$t8","$u0","$fp","$s0",
                                "$s1","$s2","$s3","$s4",
                                "$s5","$s6","$s7","$s8"};

    /*
     * 各类指令的指令名存储
     */
    QString _3RType     [_3R_NUM]       =   {"add.w","sub.w","slt","sltu",
                                            "nor","and","or","xor",
                                            "sll.w","srl.w","sra.w","mul.w",
                                            "mulh.w","mulh.wu","div.w","mod.w",
                                            "div.wu","mod.wu","break","syscall"};
    QString _2RType     [_2R_NUM]       =   {"rdcntid.w","rdcntvl.w","rdcntvh.w"};
    QString _2R_I8Type  [_2R_I8_NUM]    =   {"slli.w","srli.w","srai.w"};
    QString _2R_I12Type [_2R_I12_NUM]   =   {"slti","sltui","addi.w","andi",
                                            "nop","ori","xori","ld.b",
                                            "ld.h","ld.w","st.b","st.h",
                                            "st.w","ld.bu","ld.hu","preld"};
    QString _2R_I14Type [_2R_I14_NUM]   =   {"ll.w","sc.w"};
    QString _2R_I16Type [_2R_I16_NUM]   =   {"jirl","b","bl","beq",
                                            "bne","blt","bge","bltu",
                                            "bgeu"};
    QString _1R_I20Type [_1R_I20_NUM]   =   {"lu12i.w","pcaddu12i"};
    QString _barType    [_BAR_NUM]      =   {"dbar","ibar"};
    QString _pseudoType [_PSEUDO_NUM]   =   {"li.w","la"};

    int inst_type_num[TOT_TYPE_NUM] = { _3R_NUM, _2R_NUM, _2R_I8_NUM, _2R_I12_NUM,
                                        _2R_I14_NUM, _2R_I16_NUM, _1R_I20_NUM,
                                        _BAR_NUM, _PSEUDO_NUM };
    QString Hex[16] = { "0","1","2","3",
                        "4","5","6","7",
                        "8","9","A","B",
                        "C","D","E","F" };

private:
    /*
     * 各类指令的翻译功能函数
     */
    QString _3RTypeASM(instruction input);              // 3R类型转换
    QString _2RTypeASM(instruction input);              // 2R类型转换
    QString _2RI8TypeASM(instruction input);            // 2RI8类型转换
    QString _2RI12TypeASM(instruction input);           // 2RI12类型转换
    QString _2RI14TypeASM(instruction input);           // 2RI14类型转换
    QString _2RI16TypeASM(instruction input, int valid);// 2RI16类型转换（跳转指令）
    QString _1RI20TypeASM(instruction input);           // 1RI20类型转换
    QString _BARTypeASM(instruction input);             // BAR类型转换
    QString _pseudoTypeASM(instruction input);          // 伪指令转换

    /*
     * 4个私有成员
     * 分别表示有效语句数,错误侦测号,错误发生行,错误指令号
     */
    int valid           = 0;
    int error_no        = NO_ERROR;
    int error_line      = 0;
    int error_instno    = 0;

    /*
     * 3个结构体向量私有成员，分别是：
     * 标号表，向量表，宏定义表
     */
    std::vector<label>  labellist;
    std::vector<var>    varlist;
    std::vector<equ>    equlist;
    std::vector<macro>  macrolist;
};

#endif // ASSEMBLER_H
