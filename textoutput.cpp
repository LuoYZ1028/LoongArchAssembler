#include "mainwindow.h"
#include "ui_mainwindow.h"

QString Mainwindow::bi2Hex(QString bin) {
    QString out = "";
    for (int i = 0; i < 8; i++) {
        int b3, b2, b1, b0;
        b3 = bin.mid(4*i, 1).toInt();
        b2 = bin.mid(4*i + 1, 1).toInt();
        b1 = bin.mid(4*i + 2, 1).toInt();
        b0 = bin.mid(4*i + 3, 1).toInt();
        int tmp = b3*8 + b2*4 + b1*2 + b0;
        out += assembler.Hex[tmp];
    }
    return out;
}

/*---------------------------------------------*/
/* 判断是否有错误，并确认其类型
 */
void Mainwindow::errorDetect(int *error_no, QString result) {
    if (result == "REGNO_ERROR")
        *error_no = REGNO_ERROR;
    else if (result == "MISS_AUG")
        *error_no = MISS_AUG;
    else if (result == "REDUND_AUG")
        *error_no = REDUND_AUG;
    else if (result == "UNKNOWN_ERROR")
        *error_no = UNKNOWN_ERROR;
    else if (result == "UNDEFINED_LABEL")
        *error_no = UNDEFINED_LABEL;
}

/*---------------------------------------------*/
/* 将错误信息输出到text_output
 */
void Mainwindow::showInfo(int error_no, int error_line, int error_instno) {
    switch (error_no)
    {
    case NO_ERROR:
        error_info = "0 Error, Machine Code is generated successfully!\n";
        break;
    case TYPE_ERROR:
        error_info = "Error! Instruction(or Data) Type no supported!\n";
        break;
    case REGNO_ERROR:
        error_info = "Error! Register's Form is incorrect!\n";
        break;
    case MISS_AUG:
        error_info = "Error! Augment(s) Missing!\n";
        break;
    case REDUND_AUG:
        error_info = "Error! Redundant Augment(s)!\n";
        break;
    case UNDEFINED_LABEL:
        error_info = "Error! Label used without definition!\n";
        break;
    case SEG_ERROR:
        error_info = "Error! First line must start with \".text\" or \".data\"!\n";
        break;
    case MISS_TEXTSEG:
        error_info = "Error! Text Segment missing!\n";
        break;
    case REDUND_TEXTSEG:
        error_info = "Error! Redundant \".text\" mark!\n";
        break;
    case REDUND_DATASEG:
        error_info = "Error! Redundant \".data\" mark!\n";
        break;
    case UNKNOWN_ERROR:
    default:
        error_info = "Unknown Error... It's not supposed to happen....\n";
        break;
    }
    ui->text_output->setText(error_info);
    if (error_no != NO_ERROR && error_no > SEG_ERROR) {
        error_info = "Error happend in Line ";
        error_info += QString::number(error_line, 10);
        ui->text_output->append(error_info);
        ui->text_output->append(inst_vec[error_instno]);
    }
}

/*---------------------------------------------*/
/* 将翻译对照结果，以及数据内容输出到Result页面
 */
void Mainwindow::showOutput(int *error_no) {
    // 清空脏数据
    ui->output_table->clearContents();
    // 无错误则输出对照信息
    if (*error_no == NO_ERROR) {
        ui->output_table->setRowCount(valid);
        int pc = 0;
        for (int i = 0; i < valid; i++) {
            // 输出指令地址
            QString address = bi2Hex(assembler.int2Binary(pc, 32));
            address += ":";
            ui->output_table->setItem(i, 0, new QTableWidgetItem(address));
            // 输出机器码
            ui->output_table->setItem(i, 1, new QTableWidgetItem(output[i]));
            // 输出原指令
            ui->output_table->setItem(i, 2, new QTableWidgetItem(inst_vec[i]));
            // 地址+4
            pc += 4;
        }
    }
    // 有误时进行提示
    else {
        error_info += "Please fix your error(s)!\n";
        ui->output_table->setItem(0, 0, new QTableWidgetItem(error_info));
    }
    ui->output_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 清空脏数据
    ui->data_table->clearContents();
    // 无误时输出数据变量
    if (*error_no == NO_ERROR) {
        int row_cnt = 0;
        for (uint i = 0; i < assembler.varlist.size(); i++) {
            int row = assembler.varlist[i].size / 4;
            row = assembler.varlist[i].size % 4 ? row + 1 : row;
            row_cnt += row;
        }
        ui->data_table->setRowCount(row_cnt);

        for (uint i = 0; i < assembler.varlist.size(); i++) {
            // 变量名
            ui->data_table->setItem(i, 0, new QTableWidgetItem(assembler.varlist[i].name));
            if (assembler.varlist[i].type == ASCIZ) {
                int row = assembler.varlist[i].size / 4;
                row = assembler.varlist[i].size % 4 ? row + 1 : row;
                for (int j = 0; j < row; j++) {
                    // 地址
                    ui->data_table->setItem(i + j, 1,
                                            new QTableWidgetItem(bi2Hex(assembler.int2Binary(
                                                       assembler.varlist[i].addr + 4 * j, 32))));
                    // 16进制形式
                    QString tmp = "";
                    for (int k = 0; k < 4; k++){
                        QString tmp_ch = assembler.varlist[i].contents.mid(j * 4 + k, 1);
                        tmp += tmp_ch.toLatin1().toHex();
                        tmp += " ";
                    }
                    ui->data_table->setItem(i + j, 2, new QTableWidgetItem(tmp));
                    // 内容
                    ui->data_table->setItem(i + j, 3,
                                            new QTableWidgetItem(
                                                assembler.varlist[i].contents.mid(j * 4, 4)));
                }
            }
        }
    }
    ui->data_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}
