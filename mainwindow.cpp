#include "mainwindow.h"
#include "ui_mainwindow.h"

Mainwindow::Mainwindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::Mainwindow)
{
    ui->setupUi(this);

    highlighter = new Highlighter(ui->asm_input->document());
    ui->tabWidget->setCurrentIndex(0);
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(Qt::white));
    ui->asm_input->mergeCurrentCharFormat(fmt);
    ui->text_output->mergeCurrentCharFormat(fmt);
    // 黑底白字
    ui->asm_input->setStyleSheet("background:black;color:white");
    ui->text_output->setStyleSheet("background:black;color:white");
    // 间隔行颜色差分
    ui->output_table->setAlternatingRowColors(true);
    ui->output_table->setStyleSheet("alternate-background-color:#C1CBD7;background:white");
    ui->data_table->setAlternatingRowColors(true);
    ui->data_table->setStyleSheet("alternate-background-color:#C1CBD7;background:white");

    connect(ui->menuBar,SIGNAL(triggered(QAction*)), this, SLOT(trigerMenu(QAction*)));
    initDebugger();
}

Mainwindow::~Mainwindow() {
    delete ui;
}

void Mainwindow::trigerMenu(QAction* act) {
    if (act->text() == "读取*.txt文件") {
        fileInput_txt();
        return;
    }
    if (act->text() == "读取*.asm文件") {
        fileInput_asm();
        return;
    }
    if (act->text() == "另存为*.txt") {
        fileOutput_txt();
        return;
    }
    if (act->text() == "另存为*.coe") {
        fileOutput_coe();
        return;
    }
    if (act->text() == "导出数据为.txt") {
        dataOutput_txt();
        return;
    }
}

void Mainwindow::initDebugger() {
    refreshDisplay();
}

/*---------------------------------------------*/
/* 刷新debger的一些输出显示
 */
void Mainwindow::refreshDisplay() {
    // 输出寄存器内容
    ui->registerText->clear();
    ui->PCtext->clear();
    QString tmp_info = "";
    for (int i = 0; i < 32; i++) {
        tmp_info += assembler.getRegName(i, GET_NAME);
        tmp_info += ": ";
        tmp_info += QString::number(debugger->getReg(i), 16);
        tmp_info += "\t";
        if (i % 3 == 2)
            tmp_info += "\n";
    }
    ui->registerText->setText(tmp_info);

    // 输出PC，IR
    tmp_info = "";
    tmp_info += QString::number(debugger->getPC(), 16);
    ui->PCtext->setText(tmp_info);
    tmp_info = "IR content: ";
    tmp_info += QString::number(debugger->getIR(), 16);
    ui->labelIR->setText(tmp_info);

    // 输出断点
    tmp_info = "";
    if ((ui->enableBP->checkState() == Qt::Checked)
            && (debugger->getBreakpoint() != DEFAULT_BP)) {
        tmp_info += QString::number(debugger->getBreakpoint());
        ui->breakpointText->setText(tmp_info);
    }
}

/*---------------------------------------------*/
/* 按下“汇编”按钮后的一系列处理函数
 */
void Mainwindow::on_assembleButton_clicked() {
    vectorClear();

    readInput();

    int error_no = NO_ERROR;
    lineProcess(&error_no);

    int error_line = 0;
    int error_instno = 0;
    if (error_no == NO_ERROR)
        error_line = transCode(&error_no, &error_instno);
    showInfo(error_no, error_line, error_instno);
    showOutput(&error_no);
}
