#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 判断是否有错误，并确认其类型
 */
void Mainwindow::errorDetect(QString result) {
    if (result == "REGNO_ERROR")
        assembler.setErrorno(REGNO_ERROR);
    else if (result == "MISS_AUG")
        assembler.setErrorno(MISS_AUG);
    else if (result == "REDUND_AUG")
        assembler.setErrorno(REDUND_AUG);
    else if (result == "UNKNOWN_ERROR")
        assembler.setErrorno(UNKNOWN_ERROR);
    else if (result == "UNDEFINED_LABEL")
        assembler.setErrorno(UNDEFINED_LABEL);
    else if (result == "UNDEFINED_CONST")
        assembler.setErrorno(UNDEFINED_CONST);
    else if (result == "UNDEFINED_VAR")
        assembler.setErrorno(UNDEFINED_VAR);
    else if (result == "RD_ZERO")
        assembler.setErrorno(RD_ZERO);
}

/*
 * 将错误信息输出到text_output
 */
void Mainwindow::showInfo() {
    switch (assembler.getErrorno())
    {
    case NO_ERROR:
        error_info = "0 Error, Machine Code is generated successfully!\n";
        break;
    case TYPE_ERROR:
        error_info = "Error! Instruction(or Data) TYPE no supported!\n";
        break;
    case REGNO_ERROR:
        error_info = "Error! REGISTER FORM is incorrect!\n";
        break;
    case MISS_AUG:
        error_info = "Error! Missing Augment(s)!\n";
        break;
    case REDUND_AUG:
        error_info = "Error! REDUNDANT Augment(s)!\n";
        break;
    case UNDEFINED_LABEL:
        error_info = "Error! LABEL used without definition!\n";
        break;
    case UNDEFINED_CONST:
        error_info = "Error! CONST used without definition!\n";
        break;
    case UNDEFINED_VAR:
        error_info = "Error! Variable used without definition!\n";
        break;
    case DFORMAT_ERROR:
        error_info = "Error! Some format errors in data section!\n";
        break;
    case RD_ZERO:
        error_info = "Error! Illegal position of $r0 or $zero!\n";
        break;
    case MACRO_NAME_ERROR:
        error_info = "Error! Missing Macro name!\n";
        break;
    case SEG_ERROR:
        error_info = "Error! FIRST LINE must start with \".text\" or \".data\"!\n";
        break;
    case MISS_TEXTSEG:
        error_info = "Error! MISSING Text Segment!\n";
        break;
    case REDUND_TEXTSEG:
        error_info = "Error! REDUNDANT \".text\" mark!\n";
        break;
    case REDUND_DATASEG:
        error_info = "Error! REDUNDANT \".data\" mark!\n";
        break;
    case ORIGIN_ERROR:
        error_info = "Error! MISSING .text before .origin mark!\n";
        break;
    case ORIGIN_VERROR:
        error_info = "Error! OFFSET of Text Segment must be DIBISIBLE by 4!\n";
        break;
    case UNKNOWN_ERROR:
    default:
        error_info = "Unknown Error... It's not supposed to happen....\n";
        break;
    }
    ui->text_output->setText(error_info);
    if (assembler.getErrorno() != NO_ERROR && assembler.getErrorno() > SEG_ERROR) {
        int error_line = assembler.getErrorLine();
        int error_instno = assembler.getErrorInst();

        ui->asm_input->moveCursor(QTextCursor::Start);
        for (int i = 1; i < error_line; i++)
            ui->asm_input->moveCursor(QTextCursor::Down);
        error_info = "Error happend in Line ";
        error_info += QString::number(error_line, 10);
        ui->text_output->append(error_info);
        if (error_instno >= 0)
            ui->text_output->append(debugger->getInst(error_instno));
    }
}

/*
 * 将翻译对照结果，以及数据内容输出到Result页面
 */
void Mainwindow::showOutput() {
    ui->output_table->clearContents();  // 清空脏数据
    // 无错误则输出对照信息
    if (assembler.getErrorno() == NO_ERROR) {
        ui->output_table->setRowCount(valid);
        int pc = 0;
        for (int i = 0; i < valid; i++) {
            // 输出指令地址
            QString address = assembler.bi2Hex(assembler.int2Binary(pc, 32, UNSIGNED));
            address += ":";
            ui->output_table->setItem(i, 0, new QTableWidgetItem(address));
            // 输出机器码
            ui->output_table->setItem(i, 1, new QTableWidgetItem(output[i]));
            // 输出原指令
            ui->output_table->setItem(i, 2, new QTableWidgetItem(debugger->getInst(i)));
            // 地址+4
            pc += 4;
        }
    }
    else {
        // 有误时进行提示
        error_info += "Please fix your error(s)!\n";
        ui->output_table->setItem(0, 0, new QTableWidgetItem(error_info));
    }
    ui->output_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->data_table->clearContents();    // 清空脏数据
    if (assembler.getErrorno() == NO_ERROR) {
        // 无误时输出数据变量
        int row_cnt = 0;
        uint list_size = assembler.getVarlistSize();
        for (uint i = 0; i < list_size; i++) {
            int row = assembler.getVarSize(i) / 4;
            row = assembler.getVarSize(i) % 4 ? row + 1 : row;
            row_cnt += row;
        }
        ui->data_table->setRowCount(row_cnt);
        row_cnt = 0;

        for (uint i = 0; i < list_size; i++) {
            int row = assembler.getVarSize(i) / 4;
            row = assembler.getVarSize(i) % 4 ? row + 1 : row;
            ui->data_table->setItem(i + row_cnt, 0, new QTableWidgetItem(assembler.getVarName(i)));
            int type = assembler.getVarType(i);
            int addr = assembler.getVarAddr(i);
            QString contents = assembler.getVarContents(i);
            if (type == ASCIZ) {
                // 输出ASCIZ类型
                for (int j = 0; j < row; j++) {
                    // 地址
                    ui->data_table->setItem(i + j + row_cnt, 1,
                                            new QTableWidgetItem(assembler.bi2Hex(assembler.int2Binary(
                                                       addr + 4 * j, 32, UNSIGNED))));
                    // 16进制形式
                    QString tmp = "";
                    for (int k = 0; k < 4; k++){
                        QString tmp_ch = contents.mid(j * 4 + k, 1);
                        tmp += tmp_ch.toLatin1().toHex();
                        tmp += " ";
                    }
                    ui->data_table->setItem(i + j + row_cnt, 2, new QTableWidgetItem(tmp));
                    // 内容
                    ui->data_table->setItem(i + j + row_cnt, 3,
                                            new QTableWidgetItem(
                                                contents.mid(j * 4, 4)));
                }
            }
            else if (type == WORD) {
                // 输出WORD类型
                QList<QString> tmp_lst = contents.split(" ");
                for (int j = 0; j < row; j++) {
                    ui->data_table->setItem(i + j + row_cnt, 1,
                                            new QTableWidgetItem(assembler.bi2Hex(assembler.int2Binary(
                                                       addr + 4 * j, 32, UNSIGNED))));
                    // 转成小端模式
                    tmp_lst[j] = assembler.littleEndian(tmp_lst[j]);
                    QString tmp = tmp_lst[j].insert(2, QString(" "));
                    tmp = tmp.insert(5, QString(" "));
                    tmp = tmp.insert(8, QString(" "));
                    ui->data_table->setItem(i + j + row_cnt, 2, new QTableWidgetItem(tmp));
                    // 内容
                    QList<QString>t_lst = tmp.split(" ");
                    tmp = "";
                    for (int k = 0; k < t_lst.size(); k++) {
                        if (isascii(t_lst[k].toInt(NULL, 16)) && isprint(t_lst[k].toInt(NULL, 16)))
                            tmp += char(t_lst[k].toInt(NULL, 16));
                        else
                            tmp += ".";
                    }
                    ui->data_table->setItem(i + j + row_cnt, 3, new QTableWidgetItem(tmp));
                }
            }
            else {
                // 输出SAPCE类型
                for (int j = 0; j < row; j++) {
                    ui->data_table->setItem(i + j + row_cnt, 1,
                                            new QTableWidgetItem(assembler.bi2Hex(assembler.int2Binary(
                                                       addr + 4 * j, 32, UNSIGNED))));
                    // 16进制形式
                    QString tmp = "00 00 00 00";
                    ui->data_table->setItem(i + j + row_cnt, 2, new QTableWidgetItem(tmp));
                    // 内容
                    tmp = "....";
                    ui->data_table->setItem(i + j + row_cnt, 3, new QTableWidgetItem(tmp));
                }
            }
            row_cnt += row;
        }
    }
    ui->data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

/*
 * 将li.w和la转换为对应的addi.w
 * 借助反汇编思想似乎更有效？
 */
void Mainwindow::transPseudoInst() {
    uint num = debugger->stdinput.size();
    for (uint i = 0; i < num; i++) {
        if (debugger->stdinput[i].type == PSEUDO) {
            debugger->stdinput[i].type = _2R_I12;
            QString bin = assembler.int2Binary(output[i].toInt(NULL, 16), 32, UNSIGNED);
            QString rd, rj, imm;
            rj = "$zero";
            rd = assembler.getRegName(bin.mid(27, 5).toInt(NULL, 2), GET_NAME);
            int tmp = bin.mid(10, 12).toInt(NULL, 2);
            imm = QString::number(tmp, 16);
            debugger->stdinput[i].inst_name = "addi.w";
            debugger->stdinput[i].valueline = " " + rd + ", " + rj + ", 0x" + imm;
        }
    }
}

/*
 * 将汇编完成后的有效指令序列输出
 */
void Mainwindow::initDebugInst() {
    // 清除脏数据
    ui->instText->clear();
    // 伪指令转换
    transPseudoInst();
    if (assembler.getErrorno() == NO_ERROR) {
        QString tmp = "Address\t\tInstruction\n";
        int addr = 0;
        for (uint i = 0; i < debugger->stdinput.size(); i++) {
            tmp += assembler.bi2Hex(assembler.int2Binary(addr, 32, UNSIGNED));
            tmp += "\t";
            tmp += debugger->stdinput[i].inst_name;
            tmp += debugger->stdinput[i].valueline;
            tmp += "\n";
            addr += 4;
        }
        ui->instText->append(tmp);  // 输出指令
    }
    else {
        QString tmp = "You need to finish your Program and fix all Errors first!";
        ui->instText->append(tmp);  // 错误提示
    }
    ui->instText->moveCursor(QTextCursor::Start);
}
