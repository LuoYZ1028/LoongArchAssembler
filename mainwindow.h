#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextDocument>
#include <QTextBlock>
#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <string>
#include "assembler.h"
#include "highlighter.h"
#include "debugger.h"

namespace Ui {
class Mainwindow;
}

class Mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Mainwindow(QWidget *parent = nullptr);
    ~Mainwindow();


private:
    Ui::Mainwindow *ui;                             //整个ui界面的基础
    std::vector<QString>input;                      //输入缓冲区
    Assembler assembler;                            //汇编器核心
    void readInput();                               //读取输入
    void lineProcess(int *error_no);                //按行处理
    void instProcess(int *error_no, QString input, int lineCnt);    //指令处理
    void dataProcess(int *error_no, QString input, int lineCnt);    //数据处理
    void errorDetect(int *error_no, QString result);                //错误检测
    int transCode(int *error_no, int *error_instno);                //ASM转成机器码
    void showInfo(int error_no, int error_line, int error_instno);  //输出信息
    void showOutput(int *error_no);                 //显示输出
    void vectorClear();                             //清空堆栈
    void fileInput_txt();       //导入txt格式
    void fileInput_asm();       //导入asm格式
    void fileOutput_txt();      //导出txt格式
    void fileOutput_coe();      //导出coe格式
    void dataOutput_txt();      //导出txt格式的数据
    QString GetCorrectUnicode(const QByteArray &buf);   //调整为合适的编码形式
    std::vector<Assembler::instruction>stdinput;        //处理后的标准化输入
    std::vector<QString>output;     //输出缓冲区
    std::vector<QString>inst_vec;   //指令语句
    int valid;                      //有效语句条数
    QString error_info;             //错误信息
    QString bi2Hex(QString bin);    //2进制转16进制
    QString filename;           // 打开文件名
    Highlighter *highlighter;   // 语法高亮器
    Debugger *debugger;         // 调试器
    void initDebugger();
    void refreshDisplay();

public slots:
    void trigerMenu(QAction* act);

private slots:
    void on_assembleButton_clicked();
};

#endif // MAINWINDOW_H
