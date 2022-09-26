#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 清理向量
 */
void Mainwindow::vectorClear() {
    input.clear();
    debugger->stdinput.clear();
    assembler.labellist.clear();
    assembler.varlist.clear();
    assembler.valid = 0;
    valid = 0;
    output.clear();
    debugger->inst_vec.clear();
    debugger->memoryText.clear();
}

/*
 * 读取缓冲区内容
 */
void Mainwindow::readInput() {
    QTextDocument* doc = ui->asm_input->document();
    int lines = doc->lineCount();
    for (int x = 0; x < lines; x++) {
        QTextBlock textLine = doc->findBlockByNumber(x);
        input.push_back(textLine.text());
    }
}

/*
 * 处理指令行
 */
void Mainwindow::instProcess(int *error_no, QString input, int lineCnt) {
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
//        qDebug() << newLabel.name << newLabel.address;
        assembler.labellist.push_back(newLabel);
    }
    else {
        int type = assembler.matchType(lst[0].toLower(), INST_MODE);
        if (type != TYPE_ERROR) {
            // 将该条指令语句去掉注释，推入指令向量中
            QList<QString>tmp;
            QString str = input.simplified();
            tmp = str.split("#");
            str = tmp[0];
            debugger->inst_vec.push_back(str);

            // 指令语句标准化处理
            struct Assembler::instruction ins;
            ins.type = type;
            ins.inst_name = lst[0].toLower(); // 统一做小写转换
            QString vline;
            for (int i = 1; i < lst.size(); i++) {
                if (lst[i].mid(0, 1) != "#") {
                    vline += " ";
                    vline += lst[i];
                }
                else
                    break;//读到注释则跳过本行
            }
            ins.valueline = vline;
            ins.lineno = lineCnt + 1;
            debugger->stdinput.push_back(ins);
            valid++;
        }
        else {
            *error_no = TYPE_ERROR;
            return;
        }
    }
}

/*
 * 处理数据行
 */
void Mainwindow::dataProcess(int *error_no, QString input) {
    static uint address = 0;
    if (has_assembled == true)
        address = 0;
    static bool has_word = false;
    static bool has_space = false;
    if (input.simplified().isEmpty() || input.simplified().mid(0, 2) == "#")
        return ;    //遇到注释行或空行则跳过

    QList<QString>lst = input.simplified().split(" ");
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
                            newVar.contents += assembler.bi2Hex(assembler.int2Binary(tmp[j].toInt(), 32));
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
            if (has_space && address % SPACE_BYTE)
                address = (address / SPACE_BYTE + 1) * SPACE_BYTE;
            else if (has_word && address % WORD_BYTE)
                address = (address / WORD_BYTE + 1) * WORD_BYTE;
            newVar.addr = address;
            assembler.varlist.push_back(newVar);
            // 计算新地址
            address += newVar.size;
        }
        else
            *error_no = TYPE_ERROR;
    }
}

/*
 * 按行读取缓冲区内容，并进行标准化
 */
void Mainwindow::lineProcess(int *error_no) {
    valid = 0;
    bool text_flag = false;
    bool data_flag = false;
    int textStartlineNo = MAX_LINE_NUM;
    int dataStartlineNo = MAX_LINE_NUM;
    int lineCnt = 0;

    // 匹配代码段、数据段的起始标志
    if (input[0].mid(0, 5) == ".text") {
        text_flag = true;
        textStartlineNo = lineCnt;
        lineCnt++;
    } else if (input[0].mid(0, 5) == ".data") {
        data_flag = true;
        dataStartlineNo = lineCnt;
        lineCnt++;
    } else {
        // 第一行必然是.text或.data，否则格式有误
        *error_no = SEG_ERROR;
        return ;
    }

    for (uint i = 1; i < input.size(); i++) {
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
            *error_no = REDUND_TEXTSEG;
            return ;
        } else if (data_flag && input[i].mid(0, 5) == ".data") {
            *error_no = REDUND_DATASEG;
            return ;
        }
        // .origin标志检测，若在.text前出现则有错
        if (text_flag && input[i].mid(0, 7) == ".origin") {
            assembler.changeTSA(input[i], error_no);
            lineCnt++;
            continue;
        }
        else if (!text_flag && input[i].mid(0, 7) == ".origin") {
            *error_no = ORIGIN_ERROR;
            return ;
        }

        // 代码段处理
        if ((text_flag && textStartlineNo < dataStartlineNo)
            || (text_flag && data_flag && textStartlineNo > dataStartlineNo))
            instProcess(error_no, input[i], lineCnt);
        // 数据段处理
        else if ((data_flag && dataStartlineNo < textStartlineNo)
                 || (text_flag && data_flag && textStartlineNo < dataStartlineNo))
            dataProcess(error_no, input[i]);
        lineCnt++; // 总行数+1
    }
    // 程序里必须有.text，否则有错
    if (text_flag == false) {
        *error_no = MISS_TEXTSEG;
        return ;
    }
}

/*
 * 对标准化后的内容进行翻译
 */
int Mainwindow::transCode(int *error_no, int *error_instno) {
    for (int i = 0; i < valid; i++) {
        QString result = assembler.asm2Machine(debugger->stdinput[i]);
        errorDetect(error_no, result);
        // 该语句无错误才推入输出缓冲区
        if (*error_no == NO_ERROR) {
            output.push_back(result);
            output[i] = assembler.bi2Hex(output[i]);
        }
        // 翻译过程发现错误就提前退出，并取出该指令
        else {
            *error_instno = i;
            return debugger->stdinput[i].lineno;
        }
    }
    return -1;
}
