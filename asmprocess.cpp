#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 清理工作区
 */
void Mainwindow::workspaceClear() {
    // IO缓冲区清理
    input.clear();
    output.clear();

    // 临时变量重置
    valid = 0;
    lineCnt = 0;
    data_addr = 0;
    text_flag = false;
    data_flag = false;
    has_word = false;
    has_space = false;
    textStartlineNo = MAX_LINE_NUM;
    dataStartlineNo = MAX_LINE_NUM;

    // 汇编器清理
    assembler.clearLabelList();
    assembler.clearVarlist();
    assembler.clearEqulist();
    assembler.clearValid();
    assembler.clearMacrolist();
    assembler.setErrorno(NO_ERROR);

    // debugger清理
    debugger->stdinput.clear();
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
 * 处理指令行，遇到注释行或空行直接跳过
 * 判断：遇到label就记录其名称和位置，然后推入label向量表
 * 否则，将遇到的指令指令语句去掉注释，推入指令向量中
 *  特殊：若遇到宏展开，需要特殊处理
 *  若该指令有问题，则推入后立刻退出
 */
void Mainwindow::instProcess(QString input, int lineCnt) {
    if (input.simplified().isEmpty() || input.simplified().mid(0, 1) == "#")
        return ;

    QList<QString>lst = input.simplified().split(" ");
    if (input.mid(input.length() - 1, 1) == ":") {
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
        QList<QString>tmp;
        QString str = input.simplified();
        tmp = str.split("#");
        str = tmp[0];
        if (type != MACRO) {
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
        }
        else {
            // 宏展开处理
            int idx = 0;
            int size = assembler.getMacrolistSize();
            for (int i = 0; i < size; i++) {
                if (lst[0].toLower() == assembler.getMacroName(i)) {
                    idx = i;
                    break;
                }
            }
            QList<QString> var_list = str.split(",");
            str = var_list[0];
            tmp = str.simplified().split(" ");
            var_list[0] = tmp[1];

            int line_num = assembler.getMacroInstNum(idx);
            for (int i = 0; i < line_num; i++) {
                QString macro_inst = assembler.getMacroInst(idx, i).simplified();
                tmp = macro_inst.split("#");
                macro_inst = tmp[0];

                for (int j = 0; j < var_list.size(); j++) {
                    QString tmp_var = assembler.getMacroVar(idx, j);
                    if (macro_inst.indexOf(tmp_var) != -1) {
                        macro_inst.replace(tmp_var, var_list[j].simplified());
                    }
                }

                QList<QString>tmp_lst = macro_inst.simplified().split(" ");
                int t = assembler.matchType(tmp_lst[0].toLower(), INST_MODE);
                debugger->pushbackInstVec(macro_inst);

                struct Assembler::instruction ins;
                ins.type = t;
                ins.inst_name = tmp_lst[0].toLower(); // 统一做小写转换

                QString vline;
                for (int j = 1; j < tmp_lst.size(); j++) {
                    if (tmp_lst[j].mid(0, 1) != "#") {
                        vline += " ";
                        vline += tmp_lst[j];
                    }
                    else
                        break;  //读到注释则跳过本行
                }
                ins.valueline = vline;
                ins.lineno = lineCnt + 1;
                debugger->stdinput.push_back(ins);
            }
        }

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
 * 遇到注释行或空行直接跳过
 * 否则，进行判断是数据变量还是EQU常量
 * 都不是则类型错误，返回
 */
void Mainwindow::dataProcess(QString input) {
    if (input.simplified().isEmpty() || input.simplified().mid(0, 2) == "#")
        return ;

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
    else if (lst[1].simplified() == QString("EQU")) {
        struct Assembler::equ newEQU;
        newEQU.name = lst[0].simplified();
        newEQU.value = lst[2].simplified();
        assembler.pushbackEqu(newEQU);
    }
    else
        assembler.setErrorno(DFORMAT_ERROR);
}

/*
 * 对出现.text和.data之前的内容进行预处理
 * 主要是跨行注释和宏，遇到上述两标志后即预处理终止
 */
void Mainwindow::preProcess() {
    /*
     * 当前行一直处于预处理阶段
     * 直到碰到.text或.data
     */
    int upper = input.size();
    while ((!input[lineCnt].contains(".text", Qt::CaseSensitive))
           && (!input[lineCnt].contains(".data", Qt::CaseSensitive))
           && (lineCnt < upper))
    {
        /*
         * 存在多行注释时，跳过中间部分，最后行数额外+1
         * 暂不考虑预处理阶段的多行注释同行有其它有效内容
         */
        if (input[lineCnt].contains("/*", Qt::CaseInsensitive)) {
            // 跳过多行注释中间部分
            while (true) {
                if (input[lineCnt].contains("*/", Qt::CaseInsensitive))
                    break;
                lineCnt++;
            }
            lineCnt++;
        }

        /*
         * 存在宏时，开始填写宏结构体
         * 第一行定义宏名、macro关键字、变量表
         * 接下来是其指令行
         * 当遇到endm时表明宏结束
         * 如果macro前面没名称
         * 或指令中出现未定义的参数，都需要进行报错处理
         */
        if (input[lineCnt].contains("macro", Qt::CaseInsensitive)) {
            QList<QString> lst = input[lineCnt].simplified().split(" ");

            int name_idx = lst.indexOf(QString("macro"));
            if (name_idx == 0 || lst[0] == "macro") {
                assembler.setErrorno(MACRO_NAME_ERROR);
                return ;
            }

            // 获取变量表
            QList<QString> var_lst = input[lineCnt].simplified().split(",");
            QString tmp = var_lst[0];
            QList<QString> tmp_lst = tmp.simplified().split(" ");
            var_lst[0] = tmp_lst[name_idx + 1];

            struct Assembler::macro newmacro;
            newmacro.lineno = lineCnt + 1;
            newmacro.name = lst[0].simplified().toLower();
            newmacro.var_num = var_lst.size();
            for (int i = 0; i < var_lst.size(); i++)
                newmacro.var_list.push_back(var_lst[i].simplified());
            lineCnt++;
            newmacro.inst_num = 0;
            while (!input[lineCnt].contains("endm", Qt::CaseInsensitive)) {
                newmacro.inst_list.push_back(input[lineCnt++].simplified());
                newmacro.inst_num++;
            }
            assembler.pushbackMacro(newmacro);
        }

        lineCnt++;
    }

    /*
     * 匹配代码段、数据段的起始标志
     * .text或.data必须独占一行
     * 在多行注释、宏之后，必然是.text或.data，否则格式有误
     */
    QString first_flag = input[lineCnt].simplified().mid(0, 5);
    if (first_flag == ".text") {
        text_flag = true;
        textStartlineNo = lineCnt;
        lineCnt++;
    }
    else if (first_flag == ".data") {
        data_flag = true;
        dataStartlineNo = lineCnt;
        lineCnt++;
    }
    else
        assembler.setErrorno(SEG_ERROR);

    return ;
}

/*
 * 按行读取缓冲区内容，并进行标准化
 * 预处理出错时提前退出
 * 如果后面存在冗余的.text和.data也认为格式有误
 * 正文部分需要考虑跨行注释前后同行中，是否存在有效内容
 */
void Mainwindow::lineProcess() {
    preProcess();
    if (assembler.getErrorno() == MACRO_NAME_ERROR) {
        assembler.setErrorLine(lineCnt + 1);
        assembler.setErrorInst(-1);
        return ;
    }
    else if (assembler.getErrorno() == SEG_ERROR) {
        assembler.setErrorLine(-1);
        return ;
    }

    for (uint i = lineCnt; i < input.size(); i++) {
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

        /*
         * 1. 代码行处理
         * 2. 数据行处理
         */
        if ((text_flag && !data_flag && textStartlineNo < dataStartlineNo)
            || (text_flag && data_flag && textStartlineNo > dataStartlineNo))
            instProcess(input[i], lineCnt);

        else if ((data_flag && !text_flag && dataStartlineNo < textStartlineNo)
                 || (text_flag && data_flag && textStartlineNo < dataStartlineNo))
            dataProcess(input[i]);

        /*
         * 1. 代码行检错
         * 2. 数据行检错
         */
        if (assembler.getErrorno() != NO_ERROR && assembler.getErrorno() != DFORMAT_ERROR) {
            assembler.setErrorLine(debugger->stdinput.back().lineno);
            assembler.setErrorInst(debugger->stdinput.size() - 1);
            return ;
        }
        else if (assembler.getErrorno() == DFORMAT_ERROR) {
            assembler.setErrorLine(lineCnt + 1);
            assembler.setErrorInst(-1);
            return ;
        }
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
 * 该语句无错误才推入输出缓冲区
 *  若发现错误就提前退出，并取出该指令
 */
void Mainwindow::transCode() {
    for (int i = 0; i < valid; i++) {
        QString result = assembler.asm2Machine(debugger->stdinput[i]);
        errorDetect(result);

        if (assembler.getErrorno() == NO_ERROR) {
            output.push_back(result);
            output[i] = assembler.bi2Hex(output[i]);
        }
        else {
            assembler.setErrorInst(i);
            assembler.setErrorLine(debugger->stdinput[i].lineno);
            return ;
        }
    }
    return ;
}
