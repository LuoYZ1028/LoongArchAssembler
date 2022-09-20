#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include<QString>
#include<QDebug>
#include<vector>
#include<cmath>
#include<QByteArray>

/*
 * 不同类型的指令总数
*/
#define _3R_NUM     20
#define _2R_NUM     3
#define _2R_I8_NUM  3
#define _2R_I12_NUM 15
#define _2R_I14_NUM 2
#define _2R_I16_NUM 9
#define _1R_I20_NUM 2
#define _BAR_NUM    2

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
#define ERROR_REG "11111"

/*
 * 数据变量类型
*/
#define ASCIZ   1
#define WORD    2
#define SPACE   3

/*
 * 错误类型
*/
#define TYPE_ERROR      -1
#define REGNO_ERROR     -2
#define MISS_AUG        -3
#define REDUND_AUG      -4
#define UNKNOWN_ERROR   -5
#define UNDEFINED_LABEL -6
#define SEG_ERROR       -7
#define MISS_TEXTSEG    -8
#define REDUND_TEXTSEG  -9
#define REDUND_DATASEG  -10
#define NO_ERROR        0

/*
 * 其它宏常量
*/
#define MAX_LINE_NUM    1000
#define GET_NAME        0
#define GET_ALIAS       1
#define INST_MODE       0
#define DATA_MODE       1
#define TEST_DIR        "D:\\Acadamical_Materials\\SofwareB\\TestDir"

class Assembler
{
public:
    Assembler();
    int matchType(QString name,int mode);
    int inst_type_num[8] = {_3R_NUM, _2R_NUM, _2R_I8_NUM, _2R_I12_NUM,
                           _2R_I14_NUM, _2R_I16_NUM, _1R_I20_NUM, _BAR_NUM};
    QString Hex[16] = {
        "0","1","2","3",
        "4","5","6","7",
        "8","9","A","B",
        "C","D","E","F"
    };
    struct instruction
    {
        int     type;
        int     lineno;
        QString inst_name;
        QString valueline;
    };
    struct label
    {
        QString name;
        int     address;
    };
    std::vector<label> labellist;           //标号向量表
    struct var
    {
        QString name;
        int type;
        int addr;
        int size;
        QString contents;
    };
    std::vector<var> varlist;       // 保存数据变量

    QString asm2Machine(instruction input); //汇编转机器码
    QString int2Binary(int input, int num); //10进制转2进制
    QString getRegName(int id, int mode);   //获取寄存器名
    int hex2Int(QString hex);               //16进制转10进制
    int valid = 0;                          //有效语句条数


private:
    // 寄存器别名
    QString regs_alias[32]             =   {"$r0","$r1","$r2","$r3",
                                            "$r4","$r5","$r6","$r7",
                                            "$r8","$r9","$r10","$r11",
                                            "$r12","$r13","$r14","$r15",
                                            "$r16","$r17","$r18","$r19",
                                            "$r20","$r21","$r22","$r23",
                                            "$r24","$r25","$r26","$r27",
                                            "$r28","$r29","$r30","$r31"};
    // 寄存器名
    QString regs_name[32]              =   {"$zero","$ra","$tp","$sp",
                                            "$a0","$a1","$a2","$a3",
                                            "$a4","$a5","$a6","$a7",
                                            "$t0","$t1","$t2","$t3",
                                            "$t4","$t5","$t6","$t7",
                                            "$t8","$u0","$fp","$s0",
                                            "$s1","$s2","$s3","$s4",
                                            "$s5","$s6","$s7","$s8"};

    // 3R型指令名
    QString _3RType     [_3R_NUM]       =   {"add.w","sub.w","slt","sltu",
                                            "nor","and","or","xor",
                                            "sll.w","srl.w","sra.w","mul.w",
                                            "mulh.w","mulh.wu","div.w","mod.w",
                                            "div.wu","mod.wu","break","syscall"};
    // 2R型指令名
    QString _2RType     [_2R_NUM]       =   {"rdcntid.w","rdcntvl.w","rdcntvh.w"};
    // 2RI8型指令名
    QString _2R_I8Type  [_2R_I8_NUM]    =   {"slli.w","srli.w","srai.w"};
    // 2RI12型指令名
    QString _2R_I12Type [_2R_I12_NUM]   =   {"slti","sltui","addi.w","andi",
                                            "ori","xori","ld.b","ld.h",
                                            "ld.w","st.b","st.h","st.w",
                                            "ld.bu","ld.hu","preld"};
    // 2RI14型指令名
    QString _2R_I14Type [_2R_I14_NUM]   =   {"ll.w","sc.w"};
    // 2RI16型指令名
    QString _2R_I16Type [_2R_I16_NUM]   =   {"jirl","b","bl","beq",
                                            "bne","blt","bge","bltu",
                                            "bgeu"};
    // 1RI20型指令名
    QString _1R_I20Type [_1R_I20_NUM]   =   {"lu12i.w","pcaddu12i"};
    // BAR型指令名
    QString _barType    [_BAR_NUM]      =   {"dbar","ibar"};

    int getRegID(QString reg);                          //获取寄存器号
    QString _3RTypeASM(instruction input);              //3R类型转换
    QString _2RTypeASM(instruction input);              //2R类型转换
    QString _2RI8TypeASM(instruction input);            //2RI8类型转换
    QString _2RI12TypeASM(instruction input);           //2RI12类型转换
    QString _2RI14TypeASM(instruction input);           //2RI14类型转换
    QString _2RI16TypeASM(instruction input, int valid);//2RI16类型转换（跳转指令）
    QString _1RI20TypeASM(instruction input);           //1RI20类型转换
    QString _BARTypeASM(instruction input);             //BAR类型转换

};

#endif // ASSEMBLER_H
