#include "assembler.h"

Assembler::Assembler(){
    textStartAddr = DEFAULT_TSA;
}

/*
 * 修改代码段起始地址
 */
void Assembler::changeTSA(QString input) {
    QList<QString>lst = input.simplified().split(" ");
    int tmp = 0;
    // 16进制地址
    if (lst[1].mid(0, 2) == "0x")
        tmp = hex2Int(lst[1]);
    // 10进制地址
    else
        tmp = lst[1].toInt();

    // 检查该偏移量是否为4的整数倍
    if (tmp % 4 == 0)
        textStartAddr = tmp;
    else
        Assembler::error_no = ORIGIN_VERROR;

}

int Assembler::matchType(QString name, int mode){
    // name已做过小写转换
    if (mode == INST_MODE) {
        for (int i = 0; i < TOT_TYPE_NUM; i++) {
            int upbound = inst_type_num[i];
            for (int j = 0; j < upbound; j++) {
                switch (i)
                {
                case 0:
                    if (_3RType[j] == name)
                        return _3R;
                    break;
                case 1:
                    if (_2RType[j] == name)
                        return _2R;
                    break;
                case 2:
                    if (_2R_I8Type[j] == name)
                        return _2R_I8;
                    break;
                case 3:
                    if (_2R_I12Type[j] == name)
                        return _2R_I12;
                    break;
                case 4:
                    if (_2R_I14Type[j] == name)
                        return _2R_I14;
                    break;
                case 5:
                    if (_2R_I16Type[j] == name)
                        return _2R_I16;
                    break;
                case 6:
                    if (_1R_I20Type[j] == name)
                        return _1R_I20;
                    break;
                case 7:
                    if (_2RType[j] == name)
                        return _BAR;
                    break;
                case 8:
                    if (_pseudoType[j] == name)
                        return PSEUDO;
                    break;
                default:
                    return TYPE_ERROR;
                }
            }
        }
        return TYPE_ERROR;
    }
    else if (mode == DATA_MODE) {
        if (name == ".asciz")
            return ASCIZ;
        else if (name == ".word")
            return WORD;
        else if (name == ".space")
            return SPACE;
        else
            return TYPE_ERROR;
    }
    return TYPE_ERROR;
}

/* 负责把指令分发到对应的处理函数
 */
QString Assembler::asm2Machine(instruction input) {
    QString result;
    switch(input.type)
    {
    case _3R:       valid++;    result = _3RTypeASM(input);     break;
    case _2R:       valid++;    result = _2RTypeASM(input);     break;
    case _2R_I8:    valid++;    result = _2RI8TypeASM(input);   break;
    case _2R_I12:   valid++;    result = _2RI12TypeASM(input);  break;
    case _2R_I14:   valid++;    result = _2RI14TypeASM(input);  break;
    case _2R_I16:   valid++;    result = _2RI16TypeASM(input, valid); break;
    case _1R_I20:   valid++;    result = _1RI20TypeASM(input);  break;
    case _BAR:      valid++;    result = _BARTypeASM(input);    break;
    case PSEUDO:    valid++;    result = _pseudoTypeASM(input); break;
    default:                    result =  "UNKNOWN_ERROR";      break;
    }
    return result;
}

QString Assembler::_3RTypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    // 将注释内容删去
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    QString op,rd,rj,rk,code;
    bool isinterrupt = false;

    // 3R类型指令opcode占17bit，默认全0
    op = "00000000000000000";
    if (input.inst_name == "add.w")
        op = "00000000000100000";
    else if (input.inst_name == "sub.w")
        op = "00000000000100010";
    else if (input.inst_name == "slt")
        op = "00000000000100100";
    else if (input.inst_name == "sltu")
        op = "00000000000100101";
    else if (input.inst_name == "nor")
        op = "00000000000101000";
    else if (input.inst_name == "and")
        op = "00000000000101001";
    else if (input.inst_name == "or")
        op = "00000000000101010";
    else if (input.inst_name == "xor")
        op = "00000000000101011";
    else if (input.inst_name == "sll.w")
        op = "00000000000101110";
    else if (input.inst_name == "srl.w")
        op = "00000000000101111";
    else if (input.inst_name == "sra.w")
        op = "00000000000110000";
    else if (input.inst_name == "mul.w")
        op = "00000000000111001";
    else if (input.inst_name == "mulh.w")
        op = "00000000000111010";
    else if (input.inst_name == "div.w")
        op = "00000000001000000";
    else if (input.inst_name == "mod.w")
        op = "00000000001000001";
    else if (input.inst_name == "div.wu")
        op = "00000000001000010";
    else if (input.inst_name == "mod.wu")
        op = "00000000001000011";
    else if (input.inst_name == "break") {
        op = "00000000001010100";
        isinterrupt = true;
    } else if (input.inst_name == "syscall") {
        op = "00000000001010110";
        isinterrupt = true;
    }

    // 非中断和异常指令有3个寄存器参数
    if (!isinterrupt) {
        if (value_num < 3)
            return "MISS_AUG";
        else if (value_num > 3)
            return "REDUND_AUG";
        rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
        rj = int2Binary(getRegID(value[1]), 5, UNSIGNED);
        rk = int2Binary(getRegID(value[2]), 5, UNSIGNED);

        if (rd == ERROR_REG || rj == ERROR_REG || rk == ERROR_REG)
            return "REGNO_ERROR";
        else if (rd == ZERO_REG)
            return "RD_ZERO";
        return op + rk + rj + rd;
    }
    // 中断和异常没有寄存器参数，只有一个code
    else {
        if (value_num < 1)
            return "MISS_AUG";
        else if (value_num > 1)
            return "REDUND_AUG";

        // 15bit整数转化
        int num;
        // 分为16进制和10进制两种情况
        if (value[0].mid(0,2) == "0x")
            num = hex2Int(value[0]);
        else
            num = value[0].toInt();
        code = int2Binary(num, 15, SIGNED);
        return op + code;
    }
}

QString Assembler::_2RTypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 1)
        return "MISS_AUG";
    else if (value_num > 1)
        return "REDUND_AUG";

    QString op,rd,rj;
    // 2R类型指令opcode占22bit，默认全0
    op = "0000000000000000000000";
    if (input.inst_name == "rdcntid.w") {
        op = "0000000000000000011000";
        rd = "00000";
        rj = int2Binary(getRegID(value[0]), 5, UNSIGNED);
        if (rj == ZERO_REG)
            return "RD_ZERO";
    } else if (input.inst_name == "rdcntvl.w") {
        op = "0000000000000000011000";
        rj = "00000";
        rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
        if (rd == ZERO_REG)
            return "RD_ZERO";
    } else if (input.inst_name == "rdcntvh.w") {
        op = "0000000000000000011001";
        rj = "00000";
        rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
        if (rd == ZERO_REG)
            return "RD_ZERO";
    }
    if (rd == ERROR_REG || rj == ERROR_REG)
        return "REGNO_ERROR";

    return op + rj + rd;
}

QString Assembler::_2RI8TypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 3)
        return "MISS_AUG";
    else if (value_num > 3)
        return "REDUND_AUG";

    QString op,rd,rj,ui5;
    // 2RI8类型指令opcode占17bit(将立即数的高三位并入opcode)
    op = "00000000000000000";
    if (input.inst_name == "slli.w")
        op = "00000000010000001";
    else if (input.inst_name == "srli.w")
        op = "00000000010001001";
    else if (input.inst_name == "srai.w")
        op = "00000000010010001";

    // 5bit无符号数转化
    int num;
    char* s;
    QByteArray ascii = value[2].toLatin1(); // 转ASCII码
    s = ascii.data();
    // 分为16进制和10进制两种情况，不排除宏常量的使用
    if(value[2].mid(0,2) == "0x")
        num = hex2Int(value[2]);
    else if (s[0] <= '9' && s[0] >= '0')
        num = value[2].toInt();
    else {
        bool match_flag = false;
        for (uint i = 0; i < equlist.size(); i++) {
            if (value[2].simplified() == equlist[i].name) {
                if (equlist[i].value.mid(0, 2) == "0x")
                    num = hex2Int(equlist[i].value);
                else
                    num = equlist[i].value.toInt();
                match_flag = true;
                break;
            }
        }
        if (!match_flag)
            return "UNDEFINED_CONST";
    }

    ui5 = int2Binary(num, 5, UNSIGNED);
    rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
    rj = int2Binary(getRegID(value[1]), 5, UNSIGNED);

    if (rd == ERROR_REG || rj == ERROR_REG)
        return "REGNO_ERROR";
    else if (rd == ZERO_REG)
        return "RD_ZERO";
    return op + ui5 + rj + rd;
}

QString Assembler::_2RI12TypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 3)
        return "MISS_AUG";
    else if (value_num > 3)
        return "REDUND_AUG";

    QString op,rd,rj,imm12;
    // 2RI12类型指令opcode占10bit
    op = "0000000000";
    if (input.inst_name == "slti")
        op = "0000001000";
    else if (input.inst_name == "sltui")
        op = "0000001001";
    else if (input.inst_name == "addi.w")
        op = "0000001010";
    else if (input.inst_name == "andi" || input.inst_name == "nop")
        op = "0000001101";
    else if (input.inst_name == "ori")
        op = "0000001110";
    else if (input.inst_name == "xori")
        op = "0000001111";
    else if (input.inst_name == "ld.b")
        op = "0010100000";
    else if (input.inst_name == "ld.h")
        op = "0010100001";
    else if (input.inst_name == "ld.w")
        op = "0010100010";
    else if (input.inst_name == "st.b")
        op = "0010100100";
    else if (input.inst_name == "st.h")
        op = "0010100101";
    else if (input.inst_name == "st.w")
        op = "0010100110";
    else if (input.inst_name == "ld.bu")
        op = "0010101000";
    else if (input.inst_name == "ld.hu")
        op = "0010101001";
    else if (input.inst_name == "preld")
        op = "0010101011";
    // nop相当于一条伪指令
    if (input.inst_name == "nop") {
        rd = "00000";
        rj = "00000";
        imm12 = "000000000000";
        return op + imm12 + rj + rd;
    }
    // 12bit整数转化
    int num;
    char* s;
    QByteArray ascii = value[2].toLatin1(); // 转ASCII码
    s = ascii.data();
    // 分为16进制和10进制两种情况，不排除宏常量的使用
    if(value[2].mid(0,2) == "0x")
        num = hex2Int(value[2]);
    else if (s[0] <= '9' && s[0] >= '0')
        num = value[2].toInt();
    else {
        bool match_flag = false;
        for (uint i = 0; i < equlist.size(); i++) {
            if (value[2].simplified() == equlist[i].name) {
                if (equlist[i].value.mid(0, 2) == "0x")
                    num = hex2Int(equlist[i].value);
                else
                    num = equlist[i].value.toInt();
                match_flag = true;
                break;
            }
        }
        if (!match_flag)
            return "UNDEFINED_CONST";
    }

    imm12 = int2Binary(num, 12, SIGNED);
    rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
    rj = int2Binary(getRegID(value[1]), 5, UNSIGNED);

    if (rd == ERROR_REG || rj == ERROR_REG)
        return "REGNO_ERROR";
    else if (rd == ZERO_REG && input.inst_name != "preld" && input.inst_name.mid(0, 2) != "st")
        return "RD_ZERO";

    return op + imm12 + rj + rd;
}

QString Assembler::_2RI14TypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 3)
        return "MISS_AUG";
    else if (value_num > 3)
        return "REDUND_AUG";

    QString op,rd,rj,imm14;
    // 2RI14类型指令opcode占8bit
    op = "00000000";
    if (input.inst_name == "ll.w")
        op = "00100000";
    else if (input.inst_name == "sc.w")
        op = "00100001";

    // 14bit整数转化
    int num;
    char* s;
    QByteArray ascii = value[2].toLatin1(); // 转ASCII码
    s = ascii.data();
    // 分为16进制和10进制两种情况，不排除宏常量的使用
    if(value[2].mid(0,2) == "0x")
        num = hex2Int(value[2]);
    else if (s[0] <= '9' && s[0] >= '0')
        num = value[2].toInt();
    else {
        bool match_flag = false;
        for (uint i = 0; i < equlist.size(); i++) {
            if (value[2].simplified() == equlist[i].name) {
                if (equlist[i].value.mid(0, 2) == "0x")
                    num = hex2Int(equlist[i].value);
                else
                    num = equlist[i].value.toInt();
                match_flag = true;
                break;
            }
        }
        if (!match_flag)
            return "UNDEFINED_CONST";
    }

    imm14 = int2Binary(num, 14, SIGNED);
    rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
    rj = int2Binary(getRegID(value[1]), 5, UNSIGNED);

    if (rd == ERROR_REG || rj == ERROR_REG)
        return "REGNO_ERROR";

    return op + imm14 + rj + rd;
}

QString Assembler::_2RI16TypeASM(instruction input, int position) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    QString op,rj,rd,imm16,imm10;
    bool longjmp = false;
    // 2RI16类型指令opcode占6bit
    op = "000000";
    if (input.inst_name == "jirl")
        op = "010011";
    else if (input.inst_name == "b") {
        op = "010100";
        longjmp = true;
    } else if (input.inst_name == "bl") {
        op = "010101";
        longjmp = true;
    }
    else if (input.inst_name == "beq")
        op = "010110";
    else if (input.inst_name == "bne")
        op = "010111";
    else if (input.inst_name == "blt")
        op = "011000";
    else if (input.inst_name == "bge")
        op = "011001";
    else if (input.inst_name == "bltu")
        op = "011010";
    else if (input.inst_name == "bgeu")
        op = "011011";

    if (!longjmp) {
        if (value_num < 3)
            return "MISS_AUG";
        else if (value_num > 3)
            return "REDUND_AUG";

        // 16bit整数转化
        char* s;
        QByteArray ascii = value[2].toLatin1(); // 转ASCII码
        s = ascii.data();
        // 直接16进制地址
        if (value[2].mid(0, 2) == "0x")
            imm16 = int2Binary(hex2Int(value[2]), 16, SIGNED);
        // 10进制地址
        else if (s[0] <= '9' && s[0] >= '0')
            imm16 = int2Binary(value[2].toInt(), 16, SIGNED);
        // 如果是借助标号或宏常量的间接跳转
        else {
            bool match_flag = false;
            // 是否是已知标号
            for (uint i = 0; i < labellist.size(); i++) {
                if (value[2].simplified() == labellist[i].name) {
                    // 虽然实际距离是4的整数倍，但此工作应留给硬件完成
                    int distance = labellist[i].address - position;
                    imm16 = int2Binary(distance, 16, SIGNED);
                    match_flag = true;
                    break;
                }
            }
            // 是否是已知宏常量
            if (!match_flag) {
                for (uint i = 0; i < equlist.size(); i++) {
                    if (value[2].simplified() == equlist[i].name) {
                        if (equlist[i].value.mid(0, 2) == "0x")
                            imm16 = int2Binary(hex2Int(equlist[i].value), 16, SIGNED);
                        else
                            imm16 = int2Binary(equlist[i].value.toInt(), 16, SIGNED);
                        match_flag = true;
                        break;
                    }
                }
            }
            if (!match_flag)
                return "UNDEFINED_LABEL";
        }
        rd = int2Binary(getRegID(value[1]), 5, UNSIGNED);
        rj = int2Binary(getRegID(value[0]), 5, UNSIGNED);

        if (rd == ERROR_REG || rj == ERROR_REG)
            return "REGNO_ERROR";

        return op + imm16 + rj + rd;
    }
    else if (longjmp) {
        if (value_num < 1)
            return "MISS_AUG";
        else if (value_num > 1)
            return "REDUND_AUG";

        // 16bit整数转化
        int num_0, num_1;
        char* s;
        QByteArray ascii = value[0].toLatin1(); // 转ASCII码
        s = ascii.data();
        // 分为16进制和10进制两种情况
        if (value[0].mid(0,2) == "0x") {
            num_0 = value[0].toInt(NULL, 16);
            num_1 = num_0 >> 16;
        }
        else if (s[0] <= '9' && s[0] >= '0'){
            num_0 = value[0].toInt();
            num_1 = num_0 >> 16;
        }
        else {
            bool match_flag = false;
            for (uint i = 0; i < equlist.size(); i++) {
                if (value[0].simplified() == equlist[i].name) {
                    if (equlist[i].value.mid(0, 2) == "0x")
                        num_0 = hex2Int(equlist[i].value);
                    else
                        num_0 = equlist[i].value.toInt();
                    match_flag = true;
                    break;
                }
            }
            if (!match_flag)
                return "UNDEFINED_CONST";
            num_1 = num_0 >> 16;
        }
        imm16 = int2Binary(num_0, 16, SIGNED);
        imm10 = int2Binary(num_1, 10, SIGNED);
        return op + imm16 + imm10;
    }
    return "error";
}

QString Assembler::_1RI20TypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 2)
        return "MISS_AUG";
    else if (value_num > 2)
        return "REDUND_AUG";

    QString op,rd,imm20;
    // 1RI20类型指令opcode占7bit
    op = "0000000";
    if (input.inst_name == "lu12i.w")
        op = "0001010";
    else if (input.inst_name == "pcaddu12i.w")
        op = "0001110";

    // 20bit整数转化
    int num;
    char* s;
    QByteArray ascii = value[1].toLatin1(); // 转ASCII码
    s = ascii.data();
    // 分为16进制和10进制两种情况，不排除宏常量的使用
    if(value[1].mid(0,2) == "0x")
        num = hex2Int(value[1]);
    else if (s[0] <= '9' && s[0] >= '0')
        num = value[1].toInt();
    else {
        bool match_flag = false;
        for (uint i = 0; i < equlist.size(); i++) {
            if (value[1].simplified() == equlist[i].name) {
                if (equlist[i].value.mid(0, 2) == "0x")
                    num = hex2Int(equlist[i].value);
                else
                    num = equlist[i].value.toInt();
                match_flag = true;
                break;
            }
        }
        if (!match_flag)
            return "UNDEFINED_CONST";
    }
    imm20 = int2Binary(num, 20, SIGNED);
    rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);

    if (rd == ERROR_REG)
        return "REGNO_ERROR";
    else if (rd == ZERO_REG)
        return "RD_ZERO";

    return op + imm20 + rd;
}

QString Assembler::_BARTypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 1)
        return "MISS_AUG";
    else if (value_num > 1)
        return "REDUND_AUG";

    QString op, hint;
    // BAR类型指令opcode占17bit
    op = "00000000000000000";
    if (input.inst_name == "dbar")
        op = "00111000011100100";
    else if (input.inst_name == "ibar")
        op = "00111000011100101";

    // 15bit整数转化
    int num;
    char* s;
    QByteArray ascii = value[0].toLatin1(); // 转ASCII码
    s = ascii.data();
    // 分为16进制和10进制两种情况，不排除宏常量的使用
    if(value[0].mid(0,2) == "0x")
        num = hex2Int(value[0]);
    else if (s[0] <= '9' && s[0] >= '0')
        num = value[0].toInt();
    else {
        bool match_flag = false;
        for (uint i = 0; i < equlist.size(); i++) {
            if (value[0].simplified() == equlist[i].name) {
                if (equlist[i].value.mid(0, 2) == "0x")
                    num = hex2Int(equlist[i].value);
                else
                    num = equlist[i].value.toInt();
                match_flag = true;
                break;
            }
        }
        if (!match_flag)
            return "UNDEFINED_CONST";
    }
    hint = int2Binary(num, 15, SIGNED);
    return op + hint;
}

QString Assembler::_pseudoTypeASM(instruction input) {
    QList<QString>value;
    value = input.valueline.split(",");

    QList<QString>tmp;
    int value_num = value.size();
    QString str = value[value_num - 1].remove(QRegExp("\\s"));
    tmp = str.split("#");
    value[value_num - 1] = tmp[0];

    if (value_num < 2)
        return "MISS_AUG";
    else if (value_num > 2)
        return "REDUND_AUG";

    QString op, rd, rj, imm;
    // li.w和la都当成addi.w处理，源寄存器为r0
    op = "0000001010";
    rj = "00000";
    rd = int2Binary(getRegID(value[0]), 5, UNSIGNED);
    if (rd == ERROR_REG)
        return "REGNO_ERROR";

    // 区别在imm的值，li.w可直接赋值，la则需要比对每个数据变量
    if (input.inst_name == "li.w") {
        int num;
        char* s;
        QByteArray ascii = value[0].toLatin1(); // 转ASCII码
        s = ascii.data();
        if(value[1].mid(0,2) == "0x")
            num = hex2Int(value[1]);
        else if (s[0] <= '9' && s[0] >= '0')
            num = value[1].toInt();
        else {
            bool match_flag = false;
            for (uint i = 0; i < equlist.size(); i++) {
                if (value[1].simplified() == equlist[i].name) {
                    if (equlist[i].value.mid(0, 2) == "0x")
                        num = hex2Int(equlist[i].value);
                    else
                        num = equlist[i].value.toInt();
                    match_flag = true;
                    break;
                }
            }
            if (!match_flag)
                return "UNDEFINED_CONST";
        }
        imm = int2Binary(num, 12, SIGNED);
    }
    else {
        uint size = varlist.size();
        bool match_flag = false;
        for (uint i = 0; i < size; i++) {
            if (value[1].simplified() == varlist[i].name) {
                imm = int2Binary(varlist[i].addr, 12, SIGNED);
                match_flag = true;
                break;
            }
        }
        if (!match_flag)
            return "UNDEFINED_VAR";
    }
    return op + imm + rj + rd;
}

// 16进制转10进制
int Assembler::hex2Int(QString hex) {
    char *str;
    QByteArray ba = hex.toLatin1();
    str = ba.data();
    int count = hex.length();
    int sum = 0;
    for (int i = count - 1; i >= 2; i--)//从十六进制个位开始，每位都转换成十进制
    {
        if (str[i] >= '0' && str[i] <= '9') //数字转换
            sum += (str[i] - 48) * pow(16, count - i - 1);
        else if (str[i] >= 'A' && str[i] <= 'F') //大写字母转换
            sum += (str[i] - 55) * pow(16, count - i - 1);
        else if (str[i] >= 'a' && str[i] <= 'f') //小写字母转换
            sum += (str[i] - 55 - 32) * pow(16, count - i - 1);
    }
    return sum;
}

// 查询寄存器名
QString Assembler::getRegName(int id, int mode) {
    return mode == GET_NAME ? regs_name[id] : regs_alias[id];
}

// 查询寄存器号
int Assembler::getRegID(QString reg) {
    for (int i = 0; i < 32; i++)
        if (regs_name[i] == reg.simplified() || regs_alias[i] == reg.simplified())
            return i;
    return -1;
}

// 10进制转二进制
QString Assembler::int2Binary(int input, int num, int mode) {
    QString ans = "";
    if (mode == UNSIGNED) {
        uint tmp = (uint)input;
        for (int i = 0; i < num; i++) {
            int bit = (tmp % 2) ? 1 : 0;
            ans = QString::number(bit) + ans;
            tmp = tmp / 2;
        }
        return ans;
    }
    else {
        // 非负数时直接转换
        if (input >= 0) {
            for (int i = 0; i < num; i++) {
                int bit = (input % 2) ? 1 : 0;
                ans = QString::number(bit) + ans;
                input = input / 2;
            }
        }
        // 负数时转成补码
        else {
            input = (int)((long long)pow(2, num) + input);
            for (int i = 0; i < num; i++) {
                int bit = (input % 2) ? 1 : 0;
                ans = QString::number(bit) + ans;
                input = input / 2;
            }
        }
    }
    return ans;
}

// 2进制转16进制
QString Assembler::bi2Hex(QString bin) {
    QString out = "";
    for (int i = 0; i < 8; i++) {
        int b3, b2, b1, b0;
        b3 = bin.mid(4*i, 1).toInt();
        b2 = bin.mid(4*i + 1, 1).toInt();
        b1 = bin.mid(4*i + 2, 1).toInt();
        b0 = bin.mid(4*i + 3, 1).toInt();
        int tmp = b3*8 + b2*4 + b1*2 + b0;
        out += Assembler::Hex[tmp];
    }
    return out;
}

// 大小端转换
QString Assembler::littleEndian(QString str) {
    QString byte1 = str.mid(0, 2);
    QString byte2 = str.mid(2, 2);
    QString byte3 = str.mid(4, 2);
    QString byte4 = str.mid(6, 2);
    return byte4 + byte3 + byte2 + byte1;
}
