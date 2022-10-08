#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 清理工作区
 */
void Mainwindow::workspaceClear() {
    input.clear();
    debugger->stdinput.clear();
    assembler.clearLabelList();
    assembler.clearVarlist();
    assembler.clearEqulist();
    assembler.clearValid();
    data_addr = 0;
    assembler.setErrorno(NO_ERROR);
    valid = 0;
    output.clear();
    debugger->clearInstVec();
    debugger->clearMemoText();
}

/*
 * 读取缓冲区内容
 */
void Mainwindow::readInput() {
    QTextDocument* doc = ui->asm_input->document();
    int lines = doc->lineCount();
    for (int x = 0; x < lines; x++) {
        QTextBlock textLine = doc->findBlockByNumber(x);    // 获取单独一行
        QString text = textLine.text();
        input.push_back(text);   // 推入输入缓冲区
    }
}

/*
 * 处理指令行
 */
void Mainwindow::instProcess(QString input, int lineCnt) {
    if (input.simplified().isEmpty() || input.simplified().mid(0, 1) == "#")
        return ;    //遇到注释行或空行则跳过
    QList<QString>lst = input.simplified().split(" ");
    if (input.mid(input.length() - 1, 1) == ":") {
        //遇到label就记录其名称和位置
        struct Assembler::label newLabel;
        newLabel.name = input.mid(0, input.length() - 1);
        if (valid == 0)
            newLabel.address = 0;
        else
            newLabel.address = valid + 1;
        assembler.pushbackLabel(newLabel);
    }
    else {
        int type = assembler.matchType(lst[0].toLower(), INST_MODE);
        // 将该条指令语句去掉注释，推入指令向量中
        QList<QString>tmp;
        QString str = input.simplified();
        tmp = str.split("#");
        str = tmp[0];
        debugger->pushbackInstVec(str);

        // 指令语句标准化处理
        struct Assembler::instruction ins;
        ins.type = type;
        ins.inst_name = lst[0].toLower(); // 统一做小写转换
        QString vline;
        uint len = assembler.getEqulistSize();
        for (int i = 1; i < lst.size(); i++) {
            if (lst[i].mid(0, 1) != "#") {
                for (uint j = 0; j < len; j++) {
                    if (lst[i].simplified() == assembler.getEquName(j)) {
                        lst[i] = assembler.getEquValue(j);
                        break;
                    }
                }
                vline += " ";
                vline += lst[i];
            }
            else
                break;//读到注释则跳过本行
        }
        ins.valueline = vline;
        ins.lineno = lineCnt + 1;
        debugger->stdinput.push_back(ins);
        // 若该指令有问题，则推入后立刻退出
        if (type == TYPE_ERROR) {
            assembler.setErrorno(TYPE_ERROR);
            return ;
        }
        else
            valid++;
    }
}

/*
 * 处理数据行
 */
void Mainwindow::dataProcess(QString input) {
    static bool has_word = false;
    static bool has_space = false;
    if (input.simplified().isEmpty() || input.simplified().mid(0, 2) == "#")
        return ;    //遇到注释行或空行则跳过

    QList<QString>lst = input.simplified().split(" ");
    // 定义的是数据变量
    if (lst[0].mid(lst[0].length() - 1, 1) == ":") {
        struct Assembler::var newVar;
        newVar.type = assembler.matchType(lst[1].toLower(), DATA_MODE); // 判断变量类型
        if (newVar.type != TYPE_ERROR) {
            newVar.name = lst[0].mid(0, lst[0].length() - 1);
            // ASCIZ类型去掉双引号
            if (newVar.type == ASCIZ) {
                for (int i = 2; i < lst.size(); i++) {
                    if (lst[i].mid(0, 1) != "#" && lst[i].mid(0, 1) == "\"") {
                        newVar.contents += lst[i].mid(1, lst[i].length() - 1);
                    } else if (lst[i].mid(0, 1) != "#" && lst[i].mid(lst[i].length() - 1, 1) == "\"") {
                        newVar.contents += " ";
                        newVar.contents += lst[i].mid(0, lst[i].length() - 1);
                    } else if (lst[i].mid(0, 1) != "#") {
                        newVar.contents += " ";
                        newVar.contents += lst[i];
                    } else break;//读到注释则跳过本行
                }
            }
            // WORD类型处理逗号分割符
            else if (newVar.type == WORD) {
                for (int i = 2; i < lst.size(); i++) {
                    lst[i].replace(",", " ");
                    QList<QString>tmp = lst[i].split(" ");
                    for (int j = 0; j < tmp.size(); j++) {
                        if (tmp[j] != "")
                            newVar.contents += assembler.bi2Hex(assembler.int2Binary(tmp[j].toInt(), 32, SIGNED));
                    }
                    newVar.contents += " ";
                }
            }
            // SPACE类型直接赋0
            else {
                int blk = lst[2].simplified().toInt();
                for (int i = 0; i < blk; i++)
                    newVar.contents += "0 ";
            }
            int ele_size = 0;
            switch (newVar.type)
            {
            case ASCIZ: ele_size = 1; break;
            case WORD: ele_size = 4; has_word = true; break;
            case SPACE: ele_size = 8; has_space = true; break;
            default: ele_size = 0;
            }
            // 处理变量所占空间，单位是B
            if (newVar.type == ASCIZ)
                newVar.size = ele_size * newVar.contents.length();
            else if (newVar.type == SPACE)
                newVar.size = ele_size * lst[2].simplified().toInt();
            else
                newVar.size = ele_size * (lst.size() - 2);
            // 内部对齐问题
            if (has_space && data_addr % SPACE_BYTE)
                data_addr = (data_addr / SPACE_BYTE + 1) * SPACE_BYTE;
            else if (has_word && data_addr % WORD_BYTE)
                data_addr = (data_addr / WORD_BYTE + 1) * WORD_BYTE;
            newVar.addr = data_addr;
            assembler.pushbackVar(newVar);
            // 计算新地址
            data_addr += newVar.size;
        }
        else
            assembler.setErrorno(DFORMAT_ERROR);
    }
    // 定义的是宏常量
    else if (lst[1].simplified() == QString("EQU")) {
        struct Assembler::equ newEQU;
        newEQU.name = lst[0].simplified();
        newEQU.value = lst[2].simplified();
        assembler.pushbackEqu(newEQU);
    }
    // 否则，类型错误
    else
        assembler.setErrorno(DFORMAT_ERROR);
}

/*
 * 按行读取缓冲区内容，并进行标准化
 */
void Mainwindow::lineProcess() {
    valid = 0;
    bool text_flag = false;
    bool data_flag = false;
    int textStartlineNo = MAX_LINE_NUM;
    int dataStartlineNo = MAX_LINE_NUM;
    int lineCnt = 0;    // 表示正准备处理的行号，从0开始计数
    int annote_cnt = 0; // 开头注释行所占行数
    // 存在多行注释时
    if (input[0].contains("/*", Qt::CaseInsensitive)) {
        // 跳过多行注释中间部分
        while (true) {
            if (input[annote_cnt].contains("*/", Qt::CaseInsensitive))
                break;
            annote_cnt++;
        }
    }

    // .text或.data必须单独一行
    lineCnt = annote_cnt ? annote_cnt + 1 : 0;
    // 匹配代码段、数据段的起始标志
    if (input[lineCnt].mid(0, 5) == ".text") {
        text_flag = true;
        textStartlineNo = lineCnt;
        lineCnt++;
    }
    else if (input[lineCnt].mid(0, 5) == ".data") {
        data_flag = true;
        dataStartlineNo = lineCnt;
        lineCnt++;
    }
    else {
        // 第一行必然是.text或.data，否则格式有误
        assembler.setErrorno(SEG_ERROR);
        return ;
    }

    for (uint i = lineCnt; i < input.size(); i++) {
        // 标志检测，若出现冗余则有错
        if (!text_flag && input[i].mid(0, 5) == ".text") {
            text_flag = true;
            textStartlineNo = lineCnt;
            lineCnt++;
            continue;
        } else if (!data_flag && input[i].mid(0, 5) == ".data") {
            data_flag = true;
            dataStartlineNo = lineCnt;
            lineCnt++;
            continue;
        } else if (text_flag && input[i].mid(0, 5) == ".text") {
            assembler.setErrorno(REDUND_TEXTSEG);
            return ;
        } else if (data_flag && input[i].mid(0, 5) == ".data") {
            assembler.setErrorno(REDUND_DATASEG);
            return ;
        }
        // .origin标志检测，若在.text前出现则有错
        if (text_flag && input[i].mid(0, 7) == ".origin") {
            assembler.changeTSA(input[i]);
            lineCnt++;
            continue;
        }
        else if (!text_flag && input[i].mid(0, 7) == ".origin") {
            assembler.setErrorno(ORIGIN_ERROR);
            return ;
        }

        // 存在多行注释时
        // 跳过多行注释中间部分
        if (input[i].contains("/*", Qt::CaseInsensitive)) {
            int idx = input[i].indexOf("/*");
            input[i] = input[i].left(idx);
            uint tmp = i;
            while (true) {
                if (input[tmp].contains("*/", Qt::CaseInsensitive) || tmp == input.size()) {
                    int idx = input[tmp].indexOf("*/");
                    input[tmp] = input[tmp].mid(idx+2, input[tmp].length() - (idx+2));
                    break;
                }
                if (tmp > i)
                    input[tmp] = "";
                tmp++;
            }
        }
        // 代码段处理
        if ((text_flag && !data_flag && textStartlineNo < dataStartlineNo)
            || (text_flag && data_flag && textStartlineNo > dataStartlineNo))
            instProcess(input[i], lineCnt);
        // 数据段处理
        else if ((data_flag && !text_flag && dataStartlineNo < textStartlineNo)
                 || (text_flag && data_flag && textStartlineNo < dataStartlineNo))
            dataProcess(input[i]);
        // 指令行出错
        if (assembler.getErrorno() != NO_ERROR && assembler.getErrorno() != DFORMAT_ERROR) {
            assembler.setErrorLine(debugger->stdinput.back().lineno);
            assembler.setErrorInst(debugger->stdinput.size() - 1);
            return ;
        }
        // 数据行出错
        else if (assembler.getErrorno() == DFORMAT_ERROR) {
            assembler.setErrorLine(lineCnt + 1);
            assembler.setErrorInst(-1);
            return ;
        }
        // 否则总行数+1
        else
            lineCnt++;
    }
    // 程序里必须有.text，否则有错
    if (text_flag == false) {
        assembler.setErrorno(MISS_TEXTSEG);
        return ;
    }
}

/*
 * 对标准化后的内容进行翻译
 */
void Mainwindow::transCode() {
    for (int i = 0; i < valid; i++) {
        QString result = assembler.asm2Machine(debugger->stdinput[i]);
        errorDetect(result);
        // 该语句无错误才推入输出缓冲区
        if (assembler.getErrorno() == NO_ERROR) {
            output.push_back(result);
            output[i] = assembler.bi2Hex(output[i]);
        }
        // 翻译过程发现错误就提前退出，并取出该指令
        else {
            assembler.setErrorInst(i);
            assembler.setErrorLine(debugger->stdinput[i].lineno);
            return ;
        }
    }
    return ;
}
