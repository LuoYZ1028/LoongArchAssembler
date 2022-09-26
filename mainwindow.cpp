#include "mainwindow.h"
#include "ui_mainwindow.h"

Mainwindow::Mainwindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::Mainwindow)
{
    ui->setupUi(this);
    // 设置皮肤
    QFile f(":qdarkstyle/dark/darkstyle.qss");
    if (!f.exists()) {
        printf("Unable to set stylesheet, file not found\n");
        exit(-1);
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    ui->output_table->horizontalHeader()->setFont(QFont("Consolas", 12));
    ui->data_table->horizontalHeader()->setFont(QFont("Consolas", 12));
    ui->output_table->horizontalHeader()->setMinimumHeight(30);
    ui->data_table->horizontalHeader()->setMinimumHeight(30);

    highlighter_input = new Highlighter(ui->asm_input->document());
    highlighter_output = new Highlighter(ui->instText->document());
    ui->tabWidget->setCurrentIndex(0);
    QTextCharFormat fmt;
    fmt.setForeground(QBrush(Qt::white));
    ui->asm_input->mergeCurrentCharFormat(fmt);
    ui->text_output->mergeCurrentCharFormat(fmt);
    // 黑底白字
    ui->asm_input->setStyleSheet("background:black;color:white");
    ui->asm_input->setTabStopWidth(80);
    ui->text_output->setStyleSheet("background:black;color:white");
    // 间隔行颜色差分
    ui->output_table->setAlternatingRowColors(true);
    ui->output_table->setStyleSheet("alternate-background-color:#5C8088;");
    ui->data_table->setAlternatingRowColors(true);
    ui->data_table->setStyleSheet("alternate-background-color:#5C8088");
    connect(ui->menuBar,SIGNAL(triggered(QAction*)), this, SLOT(trigerMenu(QAction*)));
    // 高亮当前行
    connect(ui->asm_input, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(ui->instText, SIGNAL(cursorPositionChanged()), this, SLOT(highlightDebuggerLine_inst()));
    connect(ui->memoryText, SIGNAL(cursorPositionChanged()), this, SLOT(highlightDebuggerLine_data()));

    has_assembled = false;
    debugger = new Debugger(MAX_MEMSIZE);
    ui->tabWidget->setCurrentIndex(0);
}

Mainwindow::~Mainwindow() {
    delete ui;
}

void Mainwindow::trigerMenu(QAction* act) {
    if (act->text() == "读取*.txt文件") {
        fileInput_txt();
        ui->tabWidget->setCurrentIndex(0);
        return;
    }
    if (act->text() == "读取*.asm文件") {
        fileInput_asm();
        ui->tabWidget->setCurrentIndex(0);
        return;
    }
    if (act->text() == "另存为*.txt") {
        fileOutput_txt();
        ui->tabWidget->setCurrentIndex(0);
        return;
    }
    if (act->text() == "另存为*.coe") {
        fileOutput_coe();
        ui->tabWidget->setCurrentIndex(0);
        return;
    }
    if (act->text() == "导出数据为.txt") {
        dataOutput_txt();
        ui->tabWidget->setCurrentIndex(0);
        return;
    }
}

void Mainwindow::initDebugger() {
    debugger->loadData(assembler);
    refreshDisplay();
    refreshWholeMem();
}

// 刷新debuger的一些输出显示
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
        // 一行8个
        if (i % 8 == 7 || i == 0)
            tmp_info += "\n";
    }
    ui->registerText->setText(tmp_info);

    // 输出PC，IR
    tmp_info = "0x";
    tmp_info += QString::number(debugger->getPC(), 16);
    ui->PCtext->setText(tmp_info);
    tmp_info = debugger->getIR();
    ui->IRtext->setStyleSheet("color:cyan");
    ui->IRtext->setText(tmp_info);

    // 输出断点
    tmp_info = "";
    if ((ui->enableBP->checkState() == Qt::Checked)
            && (debugger->getBreakpoint() != DEFAULT_BP)) {
        tmp_info += QString::number(debugger->getBreakpoint());
        ui->breakpointText->setText(tmp_info);
    }

    // 输出数据内存
}

// 刷新debugger的主存内容
void Mainwindow::refreshWholeMem() {
    ui->memoryText->clear();
    QString tmp = "Address\t\tHex\t\tContents\n";
    for (uint i = 0; i < debugger->memoryText.size(); i++) {
        tmp += debugger->memoryText[i].addr;
        tmp += "\t";
        tmp += debugger->memoryText[i].hex;
        tmp += "\t";
        tmp += debugger->memoryText[i].asciz;
        tmp += "\n";
    }
    ui->memoryText->setText(tmp);
}

// 高亮当前行
void Mainwindow::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::darkGreen).darker(200);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = ui->asm_input->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    ui->asm_input->setExtraSelections(extraSelections);
}

// Debugger高亮行
void Mainwindow::highlightDebuggerLine_inst() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::darkMagenta).darker(100);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = ui->instText->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    ui->instText->setExtraSelections(extraSelections);
}

void Mainwindow::highlightDebuggerLine_data() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::darkRed).darker(100);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = ui->memoryText->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    ui->memoryText->setExtraSelections(extraSelections);
}

// 切换到debugger界面
void Mainwindow::on_tabWidget_tabBarClicked(int index) {
    index = index;
}

// 打开文件按钮
void Mainwindow::on_actionOpenfile_triggered() {
    fileInput();
    return ;
}

// 保存文件按钮
void Mainwindow::on_actionSavefile_triggered() {
    fileSave();
    return ;
}

// 按下“build”按钮后的一系列处理函数
void Mainwindow::on_actionAssemble_triggered() {
    // 将视角拉回编辑页
    ui->tabWidget->setCurrentIndex(0);
    vectorClear();
    readInput();
    int error_no = NO_ERROR;
    lineProcess(&error_no);
    int error_line = 0, error_instno = 0;
    if (error_no == NO_ERROR)
        error_line = transCode(&error_no, &error_instno);
    showInfo(error_no, error_line, error_instno);
    showOutput(&error_no);
    has_assembled = true;
    initDebugInst(&error_no);
    initDebugger();
}

// 以coe形式保存文件
void Mainwindow::on_actionSaveresult_triggered() {
    fileOutput_coe();
    return ;
}

// 以txt形式保存数据变量
void Mainwindow::on_actionSavedata_triggered() {
    dataOutput_txt();
    return ;
}

/*
 * Debugger的三个按钮：Run，Step，Reset
 */
void Mainwindow::on_actionRun_triggered() {
    ui->tabWidget->setCurrentIndex(2);
    uint num = 0;
    if (debugger->getPC() / 4 < debugger->stdinput.size())
        num = debugger->run(assembler);
    QMessageBox::information(this, tr("提示"), tr("已执行完所有指令，请重置Debugger！"));
    refreshDisplay();
    QString tmp = "Total ";
    tmp += QString::number(num, 10);
    tmp += " instructions had executed!";
    ui->outputText->append(tmp);
}

void Mainwindow::on_actionStep_triggered() {
    uint pre_row = 0, cur_row = 0;
    pre_row = debugger->getPC() / 4;
    ui->tabWidget->setCurrentIndex(2);
    if (debugger->getPC() == 0 && debugger->stdinput.size()) {
        ui->instText->moveCursor(QTextCursor::Start);
    }
    if (debugger->getPC() / 4 < debugger->stdinput.size()) {
        debugger->step(assembler);
        cur_row = debugger->getPC() / 4;
        if (pre_row < cur_row)
            for (uint i = 0; i < cur_row - pre_row; i++)
                ui->instText->moveCursor(QTextCursor::Down);
        else
            for (uint i = 0; i < pre_row - cur_row; i++)
                ui->instText->moveCursor(QTextCursor::Up);
    }
    else {
        QMessageBox::information(this, tr("提示"), tr("已执行完所有指令，Debugger Reset!"));
        debugger->reset(assembler);
    }
    refreshDisplay();
    // 获取内存变更的真实地址
    refreshMemLine(debugger->getChangedMemAddr());
}

void Mainwindow::on_actionReset_triggered() {
    ui->tabWidget->setCurrentIndex(2);
    debugger->reset(assembler);
    refreshDisplay();
    ui->outputText->clear();
    ui->instText->moveCursor(QTextCursor::Start);
}

// 退出提示
void Mainwindow::on_actionExit_triggered() {
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, tr("提示"), tr("确实要退出吗?"), QMessageBox::Yes|QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        QMessageBox::StandardButton btn;
        btn = QMessageBox::question(this, tr("警告"), tr("结果保存好了吗?"), QMessageBox::Yes|QMessageBox::No);
        if (btn == QMessageBox::Yes)
            exit(0);
    }
    return ;
}

// 设置PC值
void Mainwindow::on_setPCButton_clicked() {
    uint pre_pc = debugger->getPC();
    uint pc_set = ui->PCtext->text().toInt(NULL, 16);
    if (pc_set % 4) {
        QMessageBox::warning(this, tr("警告"), tr("PC值应该是4的整数倍（包括0）！"));
        return ;
    }
    debugger->setPC(pc_set);
    if (pc_set > pre_pc)
        for (uint i = 0; i < pc_set - pre_pc; i += 4)
            ui->instText->moveCursor(QTextCursor::Down);
    else if (pc_set < pre_pc)
        for (uint i = 0; i < pre_pc - pc_set; i += 4)
            ui->instText->moveCursor(QTextCursor::Up);
}

// 查看内存内容
void Mainwindow::on_viewMemoryButton_clicked() {
    QString addr_text = ui->memoryAddEdit->text();
    uint addr = 0;
    if (addr_text.mid(0, 2) == "0x")
        addr = addr_text.toInt(NULL, 16);
    else
        addr = addr_text.toInt();
    if (addr % 4) {
        QMessageBox::warning(this, tr("警告"), tr("内存地址值应该是4的整数倍（包括0）！"));
        return ;
    }
    else if (addr > debugger->getMemSize()) {
        QMessageBox::critical(this, tr("错误"), tr("内存地址越界！"));
        return ;
    }

    // 光标移动到第一行
    ui->memoryText->moveCursor(QTextCursor::Start);
    ui->memoryText->moveCursor(QTextCursor::Down);
    // 再移动到特定行
    for (uint i = 0; i < addr; i += 4)
        ui->memoryText->moveCursor(QTextCursor::Down);
    int height = ui->memoryText->verticalScrollBar()->maximum();
    ui->memoryText->verticalScrollBar()->setSliderPosition(int(float(addr) / debugger->getMemSize() * height));
}

// 设置内存内容
void Mainwindow::on_setMemoryButton_clicked() {
    QString addr_text = ui->memoryAddEdit->text();
    uint addr = 0;
    if (addr_text.mid(0, 2) == "0x")
        addr = addr_text.toInt(NULL, 16);
    else
        addr = addr_text.toInt();
    if (addr % 4) {
        QMessageBox::warning(this, tr("警告"), tr("内存地址应该是4的整数倍（包括0）！"));
        return ;
    }
    else if (addr > debugger->getMemSize()) {
        QMessageBox::critical(this, tr("错误"), tr("内存地址越界！"));
        return ;
    }
    QString data_text = ui->memdataEdit->text();
    uint data = 0;
    if (data_text.mid(0, 2) == "0x")
        data = data_text.toInt(NULL, 16);
    else
        data = data_text.toInt();
    debugger->setMemory(addr, data);
    refreshMemLine(addr);
}

// 刷新内存的某一行
void Mainwindow::refreshMemLine(int addr) {
    addr /= 4;
    ui->memoryText->moveCursor(QTextCursor::Start);
    ui->memoryText->moveCursor(QTextCursor::Down);

    // 利用光标移除特定行
    QTextCursor tc = ui->memoryText->textCursor();
    tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, addr);
    tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    tc.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
    tc.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    qDebug() << tc.selectedText() << endl;
    tc.removeSelectedText();

    // 再将新的内容插入
    QString tmp = "";
    tmp += debugger->memoryText[addr].addr;
    tmp += "\t";
    tmp += debugger->memoryText[addr].hex;
    tmp += "\t";
    tmp += debugger->memoryText[addr].asciz;
    tmp += "\n";
    tc.insertText(tmp);

    // 光标移动到第一行
    ui->memoryText->moveCursor(QTextCursor::Start);
    ui->memoryText->moveCursor(QTextCursor::Down);
    // 再移动到特定行
    for (int i = 0; i < addr; i++)
        ui->memoryText->moveCursor(QTextCursor::Down);
    // 滚轮滚到改行
    int height = ui->memoryText->verticalScrollBar()->maximum();
    ui->memoryText->verticalScrollBar()->setSliderPosition(int(float(addr*4) / debugger->getMemSize() * height));
}
