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

/*---------------------------------------------*/
/* 读取txt文件
 */
void Mainwindow::fileInput_txt() {
    // 首先清空输入区
    ui->asm_input->clear();
    // 默认该文件在D盘，套个tr()防止中文乱码
    filename = QFileDialog::getOpenFileName(this, tr("选择文件"), TEST_DIR, tr("*.txt"));
    QFile file(filename);
    // 纯文本+只读模式打开
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    // 将内容读入buf存储区
    QByteArray buf = file.readAll();
    // 编码格式转化
    QString str = GetCorrectUnicode(buf);
    // 将内容投入文本框
    ui->asm_input->setPlainText(str);
    file.close();
}

/*---------------------------------------------*/
/* 读取asm文件
 */
void Mainwindow::fileInput_asm() {
    ui->asm_input->clear();
    filename = QFileDialog::getOpenFileName(this, tr("选择文件"), "D:\\", tr("*.asm"));
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray buf = file.readAll();
    QString str = GetCorrectUnicode(buf);
    ui->asm_input->setPlainText(str);
    file.close();
}

/*---------------------------------------------*/
/* 输出coe文件
 */
void Mainwindow::fileOutput_coe() {
    // coe文件头内容
    QString str = "memory_initialization_radix=16;\nmemory_initialization_vector=\n";
    for (int i = 0; i < valid - 1; i++)
    {
        str = str + output[i] + ",";
        if(i % 8 == 7)
            str = str + "\n";
    }
    str = str + output[valid-1] + ";";

    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("open File"), TEST_DIR, tr("COE File(*.coe)"));
    if (fileName == "")
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return;
    }
    else
    {
        QTextStream textStream(&file);
        textStream<<str;
        QMessageBox::warning(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
}

/*---------------------------------------------*/
/* 输出txt文件（好像没什么用）
 */
void Mainwindow::fileOutput_txt() {
    QString str = "";
    for(int i = 0; i < valid; i++)
        str = str + output[i] + "\n";

    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this, tr("open File"), "D:\\", tr("Text File(*.txt)"));
    if (fileName == "")
        return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("错误"), tr("打开文件失败"));
        return;
    }
    else
    {
        QTextStream textStream(&file);
        textStream<<str;
        QMessageBox::warning(this, tr("提示"), tr("保存文件成功"));
        file.close();
    }
}

void Mainwindow::dataOutput_txt() {

}
