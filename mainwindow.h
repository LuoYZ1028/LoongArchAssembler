#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextDocument>
#include <QTextCursor>
#include <QScrollBar>
#include <QMessageBox>
#include <QTextBlock>
#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QObject>
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
    /*
     * 各Class的主体定义
     */
    Ui::Mainwindow *ui;                 // 整个ui界面的基础
    Assembler assembler;                // 汇编器核心
    Highlighter *highlighter_input;     // 输入窗口的语法高亮器
    Highlighter *highlighter_output;    // 输出窗口的语法高亮器
    Debugger *debugger;                 // 调试器

    /*
     * 输入和输出缓冲区
     */
    std::vector<QString>input;
    std::vector<QString>output;

    /*
     * 汇编处理工序
     */
    void workspaceClear();                              // 清空堆栈
    void readInput();                                   // 读取输入
    void lineProcess(int *error_line, int *error_instno);// 按行处理
    void instProcess(QString input, int lineCnt);        // 指令处理
    void dataProcess(QString input);                     // 数据处理
    void errorDetect(QString result);                    // 错误检测
    int transCode(int *error_instno);                    // ASM转成机器码

    /*
     * 处理结果信息显示
     */
    void showInfo(int error_line, int error_instno);  // 输出信息
    void showOutput();                                 // 显示输出

    /*
     * 文件读取和输出函数
     */
    QString filename;           // 打开文件名
    void fileInput();           // 导入txt或asm格式
    void fileSave();            // 保存当前编辑内容
    void fileInput_txt();       // 导入txt格式
    void fileInput_asm();       // 导入asm格式
    void fileOutput_txt();      // 导出txt格式
    void fileOutput_coe();      // 导出coe格式
    void dataOutput_txt();      // 导出txt格式的数据
    void dataOutput_coe();      // 导出coe格式的数据
    QString GetCorrectUnicode(const QByteArray &buf);   // 调整为合适的编码形式

    /*
     * 杂项功能函数
     */
    QString error_info;                 // 错误信息
    void transPseudoInst();             // 伪指令转换
    void initDebugInst();               // 指令展示初始化
    void initDebugger();                // Debugger初始化
    void refreshDisplay();              // Debugger界面刷新
    void refreshWholeMem();             // Debugger主存刷新
    void refreshMemLine(int addr);      // 刷新某一行内容
    int valid;
    bool has_assembled;
    int data_addr;


public slots:
    void trigerMenu(QAction* act);

private slots:
    void highlightCurrentLine();
    void highlightDebuggerLine_inst();
    void highlightDebuggerLine_data();
    void on_setPCButton_clicked();
    void on_tabWidget_tabBarClicked(int index);
    void on_actionOpenfile_triggered();
    void on_actionSavefile_triggered();
    void on_actionAbout_triggered();
    void on_actionAssemble_triggered();
    void on_actionSavedata_triggered();
    void on_actionSaveresult_triggered();
    void on_actionRun_triggered();
    void on_actionStep_triggered();
    void on_actionReset_triggered();
    void on_actionExit_triggered();
    void on_viewMemoryButton_clicked();
    void on_setMemoryButton_clicked();
    void on_enableBP_toggled(bool checked);
    void on_asm_input_textChanged();
    void on_actionOutputDatacoe_triggered();
    void on_actionOutputDatatxt_triggered();
};

#endif // MAINWINDOW_H
