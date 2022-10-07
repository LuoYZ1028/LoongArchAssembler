#include "debugger.h"
#define I12 12
#define I14 14
#define I16 16
#define I20 20
#define MASK(n, m) (n % (1 << m))

Debugger::Debugger() {
    Debugger(MAX_MEMSIZE);
}

Debugger::~Debugger() {
    delete memory;
}

// 初始化
Debugger::Debugger(int size) {
    if (memsize >= MAX_MEMSIZE)
        memsize = MAX_MEMSIZE;
    else
        memsize = size;
    // memory 稍微设大一些
    memory = new uint[memsize + 64];
    for (uint i = 0; i < memsize; i++)
        memory[i] = 0;
    for (int i = 0; i < 32; i++)
        regfile[i] = 0;
    pc = 0;
    ir = "";
    breakpoint = DEFAULT_BP;
    initPCaddr = 0;
    changedMemAddr = 0;
    memoryText.clear();
    memoryText.resize(memsize);
}

uint Debugger::getChangedMemAddr() {
    uint tmp = changedMemAddr;
    changedMemAddr = 0;
    return tmp;
}

// 直接莽，附加死循环检测
int Debugger::run(Assembler assembler) {
    uint cnt = 0;
    while (step(assembler)) {
        cnt++;
        if (pc == breakpoint || pc / 4 == stdinput.size() - 1)
            break;
        else if (cnt == MAX_INST_NUM)
            break;
    }
    return cnt;
}

// 单步调试
int Debugger::step(Assembler assembler) {
    ir = inst_vec[pc / 4];
    QList<QString> lst = stdinput[pc / 4].valueline.simplified().split(",");
    QString inst_name = stdinput[pc / 4].inst_name;
    int type = stdinput[pc / 4].type;
    // 指令在其类型中的位序
    int idx = 0;
    // 可能用到的寄存器号
    int rd, rj, rk;
    rd = Assembler::getRegID(lst[0].simplified());
    if (inst_name != "syscall" && inst_name != "break" && inst_name != "b" && inst_name != "bl"
            && type != _2R && type != _1R_I20 && type != _BAR) {
        rj = Assembler::getRegID(lst[1].simplified());
        rk = Assembler::getRegID(lst[2].simplified());
    }
    // 可能用到的立即数
    int imm, code;
    if (lst[0].mid(0, 2) == "0x")
        code = Assembler::hex2Int(lst[0].simplified());
    else
        code = lst[0].simplified().toInt();
    // 跳转标识
    bool branch_taken = false;
    // 分类处理
    switch (type)
    {
    case _3R:
        for (int i = 0; i < _3R_NUM; i++) {
            if (Assembler::_3RType[i] == inst_name) {
                idx = i;
                break;
            }
        }
        if (inst_name != "break" && inst_name != "syscall")
            _3R_handler(idx, rd, rj, rk);
        else
            _3R_handler(idx, code, 0, 0);
        break;
    case _2R:
        for (int i = 0; i < _2R_NUM; i++) {
            if (Assembler::_2RType[i] == inst_name) {
                idx = i;
                break;
            }
        }
        _2R_handler(idx, rd, rj);
        break;
    case _2R_I8:
        for (int i = 0; i < _2R_I8_NUM; i++) {
            if (Assembler::_2R_I8Type[i] == inst_name) {
                idx = i;
                break;
            }
        }
        if (lst[2].simplified().mid(0, 2) == "0x")
            imm = Assembler::hex2Int(lst[2].simplified());
        else
            imm = lst[2].simplified().toInt();
        _2RI8_handler(idx, rd, rj, imm);
        break;
    case _2R_I12:
        for (int i = 0; i < _2R_I12_NUM; i++) {
            if (Assembler::_2R_I12Type[i] == inst_name) {
                idx = i;
                break;
            }
        }
        if (lst[2].simplified().mid(0, 2) == "0x")
            imm = Assembler::hex2Int(lst[2]);
        else
            imm = lst[2].simplified().toInt();
        _2RI12_handler(idx, rd, rj, imm);
        break;
    case _2R_I14:
        for (int i = 0; i < _2R_I14_NUM; i++) {
            if (Assembler::_2R_I14Type[i] == inst_name) {
                idx = i;
                break;
            }
        }
        if (lst[2].simplified().mid(0, 2) == "0x")
            imm = Assembler::hex2Int(lst[2].simplified());
        else
            imm = lst[2].simplified().toInt();
        _2RI14_handler(idx, rd, rj, imm);
        break;
    case _2R_I16:
        for (int i = 0; i < _2R_I16_NUM; i++) {
            if (Assembler::_2R_I16Type[i] == inst_name) {
                idx = i;
                break;
            }
        }
        if (inst_name != "b" && inst_name != "bl") {
            char* s;
            QByteArray ascii = lst[2].simplified().toLatin1();
            s = ascii.data();
            if (lst[2].mid(0, 2) == "0x")
                imm = Assembler::hex2Int(lst[2].simplified());
            else if (s[0] <= '9' && s[0] >= '0')
                imm = lst[2].simplified().toInt();
            else
                for (uint i = 0; i < assembler.labellist.size(); i++)
                    if (lst[2].simplified() == assembler.labellist[i].name) {
                        imm = assembler.labellist[i].address - pc / 4;
                        break;
                    }
            _2RI16_handler(idx, rd, rj, imm, &branch_taken);
        }
        else {
            branch_taken = true;
            if (lst[0].simplified().mid(0, 2) == "0x")
                imm = Assembler::hex2Int(lst[0].simplified());
            else
                imm = lst[0].simplified().toInt();
            _2RI16_handler(idx, 0, 0, imm, &branch_taken);
        }
        break;
    case _1R_I20:
        for (int i = 0; i < _1R_I20_NUM; i++) {
            if (Assembler::_1R_I20Type[i] == inst_name) {
                idx = i;
                break;
            }
        }
        if (lst[1].simplified().mid(0, 2) == "0x")
            imm = Assembler::hex2Int(lst[1].simplified());
        else
            imm = lst[1].simplified().toInt();
        _1RI20_handler(idx, rd, imm);
        break;
    case _BAR:
        for (int i = 0; i < _BAR_NUM; i++) {
            if (Assembler::_barType[i] == inst_name) {
                idx = i;
                break;
            }
        }
        _BAR_handler(idx, code);
        break;
    default: return 0;
    }
    // 执行完成后，若无跳转，则正常更新pc
    if (branch_taken == false)
        pc += 4;
    return 1;
}

// 3R类型指令模拟
void Debugger::_3R_handler(int inst_idx, int rd, int rj, int rk) {
    switch (inst_idx)
    {
    // add.w
    case 0:
        regfile[rd] = regfile[rj] + regfile[rk];
        break;
    // sub.w
    case 1:
        regfile[rd] = regfile[rj] - regfile[rk];
        break;
    // slt
    case 2:
        regfile[rd] = int(regfile[rj]) < int(regfile[rk]) ? 1 : 0;
        break;
    // sltu
    case 3:
        regfile[rd] = regfile[rj] < regfile[rk] ? 1 : 0;
        break;
    // nor
    case 4:
        regfile[rd] = ~(regfile[rj] | regfile[rk]);
        break;
    // and
    case 5:
        regfile[rd] = regfile[rj] & regfile[rk];
        break;
    // or
    case 6:
        regfile[rd] = regfile[rj] | regfile[rk];
        break;
    // xor
    case 7:
        regfile[rd] = regfile[rj] ^ regfile[rk];
        break;
    // sll.w
    case 8:
        regfile[rd] = regfile[rj] << (regfile[rk] % 32);
        break;
    // srl.w
    case 9:
        regfile[rd] = regfile[rj] >> (regfile[rk] % 32);
        break;
    // sra.w
    case 10:
        regfile[rd] = int(regfile[rj]) >> (regfile[rk] % 32);
        break;
    // mul.w
    case 11:
        regfile[rd] = uint(int(regfile[rj]) * int(regfile[rk]));
        break;
    // mulh.w todo
    case 12:
    {
        int tmp;
        tmp = int((long long)regfile[rj] * (int)regfile[rk] >> 32);
        regfile[rd] = uint(tmp);
        break;
    }
    // mulh.wu todo
    case 13:
    {
        uint tmp;
        tmp = uint((unsigned long long)regfile[rj] * regfile[rk] >> 32);
        regfile[rd] = tmp;
        break;
    }
    // div.w
    case 14:
        regfile[rd] = int(regfile[rj]) / int(regfile[rk]);
        break;
    // mod.w
    case 15:
        regfile[rd] = int(regfile[rj]) % int(regfile[rk]);
        break;
    // div.wu
    case 16:
        regfile[rd] = regfile[rj] / regfile[rk];
        break;
    // mod.wu
    case 17:
        regfile[rd] = regfile[rj] % regfile[rk];
        break;
    // break
    case 18:
        break;
    // syscall
    case 19:
        break;
    default: ;
    }
}

// 2R类型指令模拟
void Debugger::_2R_handler(int inst_idx, int rd, int rj) {
    // 此类型指令暂不处理
    rd = rd;
    rj = rj;
    switch (inst_idx)
    {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    default: ;
    }
}

// 2RI8类型指令模拟
void Debugger::_2RI8_handler(int inst_idx, int rd, int rj, int imm) {
    switch (inst_idx)
    {
    // slli.w
    case 0:
        regfile[rd] = regfile[rj] << (uint(imm) % 32);
        break;
    // srli.w
    case 1:
        regfile[rd] = regfile[rj] >> (uint(imm) % 32);
        break;
    // srai.w
    case 2:
        regfile[rd] = int(regfile[rj]) >> (uint(imm) % 32);
        break;
    default: ;
    }
}

// 2RI12类型指令模拟
void Debugger::_2RI12_handler(int inst_idx, int rd, int rj, int imm) {
    switch (inst_idx)
    {
    // slti
    case 0:
        regfile[rd] = int(regfile[rj]) < int(MASK(imm, I12)) ? 1 : 0;
        break;
    // sltui
    case 1:
        regfile[rd] = regfile[rj] < uint(MASK(imm, I12)) ? 1 : 0;
        break;
    // addi.w
    case 2:
        regfile[rd] = regfile[rj] + MASK(imm, I12);
        break;
    // andi
    case 3:
        regfile[rd] = regfile[rj] & MASK(imm, I12);
        break;
    // nop
    case 4:
        regfile[0] = 0;
        break;
    // ori
    case 5:
        regfile[rd] = regfile[rj] | MASK(imm, I12);
        break;
    // xori
    case 6:
        regfile[rd] = regfile[rj] ^ MASK(imm, I12);
        break;
    // ld.b
    case 7:
        regfile[rd] = MASK(int(memory[regfile[rj] + MASK(imm, I12)]), 8);
        break;
    // ld.h
    case 8:
        regfile[rd] = MASK(int(memory[regfile[rj] + MASK(imm, I12)]), 16);
        break;
    // ld.w
    case 9:
        regfile[rd] = int(memory[regfile[rj] + MASK(imm, I12)]);
        break;
    // st.b
    case 10:
        setMemory(regfile[rj] + MASK(imm, I12), MASK(regfile[rd], 8));
        break;
    // st.h
    case 11:
        setMemory(regfile[rj] + MASK(imm, I12), MASK(regfile[rd], 16));
        break;
    // st.w
    case 12:
        setMemory(regfile[rj] + MASK(imm, I12), regfile[rd]);
        break;
    // ld.bu
    case 13:
        regfile[rd] = MASK(memory[regfile[rj] + MASK(imm, I12)], 8);
        break;
    // ld.hu
    case 14:
        regfile[rd] = MASK(memory[regfile[rj] + MASK(imm, I12)], 16);
        break;
    // preld
    case 15:
        break;
    default: ;
    }
}

// 2RI14类型指令模拟
void Debugger::_2RI14_handler(int inst_idx, int rd, int rj, int imm) {
    switch (inst_idx)
    {
    // ll.w
    case 0:
        regfile[rd] = int(memory[regfile[rj] + (MASK(imm, I14) << 2)]);
        break;
    // sc.w
    case 1:
        memory[regfile[rj] + (MASK(imm, I14) << 2)] = regfile[rd];
        break;
    default: ;
    }
}

// 2RI16类型指令模拟
void Debugger::_2RI16_handler(int inst_idx, int rd, int rj, int imm, bool *branch_taken) {
    int inc = MASK(imm, I16) << 2;
    switch (inst_idx)
    {
    // jirl
    case 0:
        regfile[rd] = pc + 4;
        pc = int(regfile[rj]) + inc;
        *branch_taken = true;
        break;
    // b
    case 1:
        pc = int(pc) + (MASK(imm, 26) << 2);
        *branch_taken = true;
        break;
    // bl
    case 2:
        regfile[1] = pc + 4;
        pc = int(pc) + (MASK(imm, 26) << 2);
        *branch_taken = true;
        break;
    // beq
    case 3:
        *branch_taken = (regfile[rd] == regfile[rj]);
        pc = *branch_taken == true ? int(pc) + inc : pc;
        break;
    // bne
    case 4:
        *branch_taken = (regfile[rd] != regfile[rj]);
        pc = *branch_taken ? int(pc) + inc : pc;
        break;
    // blt
    case 5:
        *branch_taken = (int(regfile[rd]) < int(regfile[rj]));
        pc = *branch_taken ? int(pc) + inc : pc;
        break;
    // bge
    case 6:
        *branch_taken = (int(regfile[rd]) >= int(regfile[rj]));
        pc = *branch_taken ? int(pc) + inc : pc;
        break;
    // bltu
    case 7:
        *branch_taken = (regfile[rd] < regfile[rj]);
        pc = *branch_taken ? int(pc) + inc : pc;
        break;
    // bgeu
    case 8:
        *branch_taken = (regfile[rd] >= regfile[rj]);
        pc = *branch_taken ? int(pc) + inc : pc;
        break;
    default: ;
    }
}

// 1RI20类型指令模拟
void Debugger::_1RI20_handler(int inst_idx, int rd, int imm) {
    switch(inst_idx)
    {
    // lu12i.w
    case 0:
        regfile[rd] = MASK(imm, I20) << 12;
        break;
    // pcaddu12i
    case 1:
        regfile[rd] = pc + (MASK(imm, I20) << 12);
        break;
    default: ;
    }
}

// BAR类型指令模拟
void Debugger::_BAR_handler(int inst_idx, int hint) {
    // 此类指令暂不处理
    hint = hint;
    switch (inst_idx)
    {
    case 0:
        break;
    case 1:
        break;
    default: ;
    }
}

// 设置数据
void Debugger::setMemory(uint address, uint data) {
    address /= 4;
    memory[address] = data;
    // 反馈到memoryText向量中，地址已存在，所以无需重申
    QString hex_str = Assembler::bi2Hex(Assembler::int2Binary(data, 32, UNSIGNED));
    hex_str = hex_str.insert(2, QString(" "));
    hex_str = hex_str.insert(5, QString(" "));
    hex_str = hex_str.insert(8, QString(" "));
    memoryText[address].hex = hex_str;

    QList<QString> lst = hex_str.split(" ");
    memoryText[address].asciz = "";
    for (int i = 0; i < lst.size(); i++) {
        if (isascii(lst[i].toInt(NULL, 16)) && isprint(lst[i].toInt(NULL, 16)))
            memoryText[address].asciz += char(lst[i].toInt(NULL, 16));
        else
            memoryText[address].asciz += ".";
    }
    changedMemAddr = address * 4;
}

// 载入数据
void Debugger::loadData(Assembler assembler) {
    uint row_acc = 0;
    for (uint i = 0; i < assembler.varlist.size(); i++) {
        int row = assembler.varlist[i].size / 4;   // 4字节一行，计算变量所占行数
        for (int j = 0; j < row; j++) {
            // 先处理memory，用于指令模拟
            QString tmp = "";
            if (assembler.varlist[i].type == ASCIZ)
                for (int k = 0; k < 4; k++) {
                    QString ch = assembler.varlist[i].contents.mid(j*4 + k, 1);
                    tmp += ch.toLatin1().toHex();
                }
            else if (assembler.varlist[i].type == WORD) {
                QList<QString> lst = assembler.varlist[i].contents.split(" ");
                tmp = Assembler::littleEndian(lst[j]);
            }
            else
                tmp = "000000";
            memory[j + row_acc] = tmp.toInt(NULL, 16);
            // 再处理memoryText，用于展示
            // 地址
            struct meminfo tmp_info;
            tmp_info.addr = Assembler::bi2Hex(Assembler::int2Binary(
                                assembler.varlist[i].addr + 4*(j + row_acc), 32, UNSIGNED));
            // 16进制形式
            if (assembler.varlist[i].type == ASCIZ) {
                for (int k = 0; k < 4; k++) {
                    if (j*4 + k < assembler.varlist[i].size) {
                        QString ch = assembler.varlist[i].contents.mid(j*4 + k, 1);
                        tmp_info.hex += ch.toLatin1().toHex();
                    }
                    else
                        tmp_info.hex += "00";    // 补0
                    if (k < 3)
                        tmp_info.hex += " ";
                }
            }
            else if (assembler.varlist[i].type == WORD) {
                QList<QString> lst = assembler.varlist[i].contents.split(" ");
                lst[j] = Assembler::littleEndian(lst[j]);
                tmp_info.hex = lst[j].insert(2, QString(" "));
                tmp_info.hex = tmp_info.hex.insert(5, QString(" "));
                tmp_info.hex = tmp_info.hex.insert(8, QString(" "));
            }
            else
                tmp_info.hex = "00 00 00 00";
            // 内容
            if (assembler.varlist[i].type == ASCIZ)
                tmp_info.asciz = assembler.varlist[i].contents.mid(j*4, 4);
            else if (assembler.varlist[i].type == WORD) {
                QList<QString> lst = tmp_info.hex.split(" ");
                for (int k = 0; k < lst.size(); k++) {
                    if (isascii(lst[k].toInt(NULL, 16)) && isprint(lst[k].toInt(NULL, 16)))
                        tmp_info.asciz += char(lst[k].toInt(NULL, 16));
                    else
                        tmp_info.asciz += ".";
                }
            }
            else
                tmp_info.asciz = "....";
            // 一条完整的结果推入vector中
            memoryText.push_back(tmp_info);
        }
        // 更新累计行数
        row_acc += row;
    }
    // 剩余地址内填全0
    if (row_acc*4 < memsize) {
        uint row = (memsize - row_acc*4) / 4;
        for (uint i = 0; i < row; i++) {
            // 填memory
            memory[row + row_acc] = 0;
            // 填memoryText
            struct meminfo tmp_info;
            tmp_info.addr = Assembler::bi2Hex(Assembler::int2Binary(4*(i + row_acc), 32, UNSIGNED));
            tmp_info.hex = "00 00 00 00";
            tmp_info.asciz = "....";
            memoryText.push_back(tmp_info);
        }
    }
    changedMemAddr = 0;
}

// 重置Debugger
void Debugger::reset(Assembler assembler) {
    for (int i = 0; i < 32; ++i)
        regfile[i] = 0;
    ir = "";
    pc = initPCaddr;
    loadData(assembler);
}
