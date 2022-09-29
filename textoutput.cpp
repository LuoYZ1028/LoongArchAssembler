#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 判断是否有错误，并确认其类型
 */
void Mainwindow::errorDetect(QString result) {
    if (result == "REGNO_ERROR")
        assembler.error_no = REGNO_ERROR;
    else if (result == "MISS_AUG")
        assembler.error_no = MISS_AUG;
    else if (result == "REDUND_AUG")
        assembler.error_no = REDUND_AUG;
    else if (result == "UNKNOWN_ERROR")
        assembler.error_no = UNKNOWN_ERROR;
    else if (result == "UNDEFINED_LABEL")
        assembler.error_no = UNDEFINED_LABEL;
}

/*
 * 将错误信息输出到text_output
 */
void Mainwindow::showInfo(int error_line, int error_instno) {
    switch (assembler.error_no)
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
    if (assembler.error_no != NO_ERROR && assembler.error_no > SEG_ERROR) {
        ui->asm_input->moveCursor(QTextCursor::Start);
        for (int i = 1; i < error_line; i++)
            ui->asm_input->moveCursor(QTextCursor::Down);
        error_info = "Error happend in Line ";
        error_info += QString::number(error_line, 10);
        ui->text_output->append(error_info);
        ui->text_output->append(debugger->inst_vec[error_instno]);
    }
}

/*
 * 将翻译对照结果，以及数据内容输出到Result页面
 */
void Mainwindow::showOutput() {
    ui->output_table->clearContents();  // 清空脏数据
    // 无错误则输出对照信息
    if (assembler.error_no == NO_ERROR) {
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
            ui->output_table->setItem(i, 2, new QTableWidgetItem(debugger->inst_vec[i]));
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
    if (assembler.error_no == NO_ERROR) {
        // 无误时输出数据变量
        int row_cnt = 0;
        for (uint i = 0; i < assembler.varlist.size(); i++) {
            int row = assembler.varlist[i].size / 4;
            row = assembler.varlist[i].size % 4 ? row + 1 : row;
            row_cnt += row;
        }
        ui->data_table->setRowCount(row_cnt);
        row_cnt = 0;

        for (uint i = 0; i < assembler.varlist.size(); i++) {
            int row = assembler.varlist[i].size / 4;    // 所占行数
            row = assembler.varlist[i].size % 4 ? row + 1 : row;
            ui->data_table->setItem(i + row_cnt, 0, new QTableWidgetItem(assembler.varlist[i].name));
            if (assembler.varlist[i].type == ASCIZ) {
                // 输出ASCIZ类型
                for (int j = 0; j < row; j++) {
                    // 地址
                    ui->data_table->setItem(i + j + row_cnt, 1,
                                            new QTableWidgetItem(assembler.bi2Hex(assembler.int2Binary(
                                                       assembler.varlist[i].addr + 4 * j, 32, UNSIGNED))));
                    // 16进制形式
                    QString tmp = "";
                    for (int k = 0; k < 4; k++){
                        QString tmp_ch = assembler.varlist[i].contents.mid(j * 4 + k, 1);
                        tmp += tmp_ch.toLatin1().toHex();
                        tmp += " ";
                    }
                    ui->data_table->setItem(i + j + row_cnt, 2, new QTableWidgetItem(tmp));
                    // 内容
                    ui->data_table->setItem(i + j + row_cnt, 3,
                                            new QTableWidgetItem(
                                                assembler.varlist[i].contents.mid(j * 4, 4)));
                }
            }
            else if (assembler.varlist[i].type == WORD) {
                // 输出WORD类型
                QList<QString> tmp_lst = assembler.varlist[i].contents.split(" ");
                for (int j = 0; j < row; j++) {
                    ui->data_table->setItem(i + j + row_cnt, 1,
                                            new QTableWidgetItem(assembler.bi2Hex(assembler.int2Binary(
                                                       assembler.varlist[i].addr + 4 * j, 32, UNSIGNED))));
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
                                                       assembler.varlist[i].addr + 4 * j, 32, UNSIGNED))));
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
 * 将汇编完成后的有效指令序列输出
 */
void Mainwindow::initDebugInst() {
    // 清除脏数据
    ui->instText->clear();
    if (assembler.error_no == NO_ERROR) {
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
}
