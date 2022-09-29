#include "mainwindow.h"
#include "ui_mainwindow.h"

QString Mainwindow::GetCorrectUnicode(const QByteArray &buf) {
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(buf.constData(), buf.size(), &state);
    if (state.invalidChars > 0)
        text = QTextCodec::codecForName("GBK")->toUnicode(buf);
    else
        text = buf;
    return text;
}

/*
 * 读取文件
 */
void Mainwindow::fileInput() {
    ui->asm_input->clear();
    filename = QFileDialog::getOpenFileName(this, tr("选择文件"), TEST_DIR, tr("*.txt;;*.asm"));
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray buf = file.readAll();
        QString str = GetCorrectUnicode(buf);
        ui->asm_input->setPlainText(str);
        file.close();
        str = "Open File " + filename + " success!";
        ui->text_output->append(str);
    } else {
        QString str = "Open File " + filename + " fail!";
        ui->text_output->append(str);
    }
}

/*
 * 保存当前编辑内容
 */
void Mainwindow::fileSave() {
    QString str = ui->asm_input->toPlainText();
    str = str.toUtf8();
    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("选择文件"), TEST_DIR, tr("ASM File(*.asm)"));
    if (fileName == "") {
        QString str = "Save Cancelled!";
        ui->text_output->append(str);
        return ;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return ;
    } else {
        QTextStream textStream(&file);
        textStream << str;
        QMessageBox::information(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
    str = "Save File " + fileName + " success!";
    ui->text_output->append(str);
}

/*
 * 读取txt文件
 */
void Mainwindow::fileInput_txt() {
    // 首先清空输入区
    ui->asm_input->clear();
    // 默认该文件在D盘，套个tr()防止中文乱码
    filename = QFileDialog::getOpenFileName(this, tr("选择文件"), TEST_DIR, tr("*.txt"));
    QFile file(filename);
    // 纯文本+只读模式打开
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 将内容读入buf存储区
        QByteArray buf = file.readAll();
        // 编码格式转化
        QString str = GetCorrectUnicode(buf);
        // 将内容投入文本框
        ui->asm_input->setPlainText(str);
        file.close();
        str = "Open File " + filename + " success!";
        ui->text_output->append(str);
    } else {
        QString str = "Open File " + filename + " fail!";
        ui->text_output->append(str);
    }
}

/*
 * 读取asm文件
 */
void Mainwindow::fileInput_asm() {
    ui->asm_input->clear();
    filename = QFileDialog::getOpenFileName(this, tr("选择文件"), "D:\\", tr("*.asm"));
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray buf = file.readAll();
        QString str = GetCorrectUnicode(buf);
        ui->asm_input->setPlainText(str);
        file.close();
        str = "Open File " + filename + " success!";
        ui->text_output->append(str);
    } else {
        QString str = "Open File " + filename + " fail!";
        ui->text_output->append(str);
    }
}

/*
 * 输出coe文件
 */
void Mainwindow::fileOutput_coe() {
    // coe文件头内容
    QString str = "memory_initialization_radix=16;\nmemory_initialization_vector=\n";
    // 代码段地址之前的内容
    uint cnt = assembler.get_TSA() / 4, i = 0;
    for (; i < cnt; i++) {
        str += "00000000,";
        if (i % 8 == 7)
            str += "\n";
    }
    for (; i < cnt + valid - 1; i++) {
        str += output[i - cnt] + ",";
        if(i % 8 == 7)
            str += "\n";
    }
    str += output[valid - 1] + ";";

    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("选择文件"), TEST_DIR, tr("COE File(*.coe)"));
    if (fileName == "") {
        QString str = "Output Cancelled!";
        ui->text_output->append(str);
        return ;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return ;
    } else {
        QTextStream textStream(&file);
        textStream << str;
        QMessageBox::information(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
    str = "Output File " + fileName + " success!";
    ui->text_output->append(str);
}

/*
 * 输出txt文件（好像没什么用）
 */
void Mainwindow::fileOutput_txt() {
    QString str = "";
    // 代码段地址之前的内容
    uint cnt = assembler.get_TSA() / 4, i = 0;
    for (; i < cnt; i++) {
        str += "00000000,";
        if (i % 8 == 7)
            str += "\n";
    }
    for (; i < cnt + valid - 1; i++) {
        str += output[i - cnt] + ",";
        if(i % 8 == 7)
            str += "\n";
    }
    str += output[valid - 1] + ";";

    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("选择文件"), "D:\\", tr("Text File(*.txt)"));
    if (fileName == "")
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return;
    } else {
        QTextStream textStream(&file);
        textStream << str;
        QMessageBox::information(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
    str = "Output File " + fileName + " success!";
    ui->text_output->append(str);
}

void Mainwindow::dataOutput_txt() {
    QString str = "";
    int cnt = 0;
    for(uint i = 0; i < assembler.varlist.size(); i++) {
        int type = assembler.varlist[i].type;
        QList<QString>tmp_list = assembler.varlist[i].contents.split(" ");
        if (type == ASCIZ) {
            int size = assembler.varlist[i].size;
            int row = size % 4 ? size / 4 + 1 : size / 4;
            for (int j = 0; j < row; j++) {
                for (int k = 0; k < 4; k++){
                    if (j * 4 + k < assembler.varlist[i].size) {
                        // 字符转ascii码
                        QString tmp_ch = assembler.varlist[i].contents.mid(4*j + k, 1);
                        str += tmp_ch.toLatin1().toHex();
                    }
                    else
                        str += "00";    // 补0
                }
                str += ",";
                cnt++;
                if (cnt % 8 == 7)
                    str += "\n";
            }
        }
        else if (type == WORD) {
            for (int j = 0; j < tmp_list.size(); j++) {
                QString tmp = tmp_list[j];
                tmp = assembler.littleEndian(tmp);
                str += tmp;
                str += ",";
                cnt++;
                if (cnt % 8 == 7)
                    str += "\n";
            }
        }
        else {
            for (int j = 0; j < tmp_list.size() * 2; j++) {
                str += "00000000,";
                cnt++;
                if (cnt % 8 == 7)
                    str += "\n";
            }
        }
    }
    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("选择文件"), TEST_DIR, tr("Text File(*.txt)"));
    if (fileName == "")
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return;
    } else {
        QTextStream textStream(&file);
        textStream<<str;
        QMessageBox::information(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
    str = "Output File " + fileName + " success!";
    ui->text_output->append(str);
}

void Mainwindow::dataOutput_coe() {
    QString str = "memory_initialization_radix=16;\nmemory_initialization_vector=\n";
    int cnt = 0;
    for(uint i = 0; i < assembler.varlist.size(); i++) {
        int type = assembler.varlist[i].type;
        QList<QString>tmp_list = assembler.varlist[i].contents.split(" ");
        if (type == ASCIZ) {
            int size = assembler.varlist[i].size;
            int row = size % 4 ? size / 4 + 1 : size / 4;
            for (int j = 0; j < row; j++) {
                for (int k = 0; k < 4; k++){
                    if (j * 4 + k < assembler.varlist[i].size) {
                        // 字符转ascii码
                        QString tmp_ch = assembler.varlist[i].contents.mid(4*j + k, 1);
                        str += tmp_ch.toLatin1().toHex();
                    }
                    else
                        str += "00";    // 补0
                }
                str += ",";
                cnt++;
                if (cnt % 8 == 7)
                    str += "\n";
            }
        }
        else if (type == WORD) {
            for (int j = 0; j < tmp_list.size(); j++) {
                QString tmp = tmp_list[j];
                tmp = assembler.littleEndian(tmp);
                str += tmp;
                str += ",";
                cnt++;
                if (cnt % 8 == 7)
                    str += "\n";
            }
        }
        else {
            for (int j = 0; j < tmp_list.size() * 2; j++) {
                str += "00000000,";
                cnt++;
                if (cnt % 8 == 7)
                    str += "\n";
            }
        }
    }
    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("选择文件"), TEST_DIR, tr("Text File(*.txt)"));
    if (fileName == "")
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return;
    } else {
        QTextStream textStream(&file);
        textStream<<str;
        QMessageBox::information(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
    str = "Output File " + fileName + " success!";
    ui->text_output->append(str);
}
