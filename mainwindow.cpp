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
//    ui->asm_input->setStyleSheet("background:black;color:white");
//    ui->asm_input->setTabStopWidth(80);
//    ui->text_output->setStyleSheet("background:black;color:white");
    // 间隔行颜色差分
    ui->output_table->setAlternatingRowColors(true);
    ui->output_table->setStyleSheet("alternate-background-color:#455364;");
    ui->data_table->setAlternatingRowColors(true);
    ui->data_table->setStyleSheet("alternate-background-color:#455364");
    // 信号连接
    connect(ui->menuBar,SIGNAL(triggered(QAction*)), this, SLOT(trigerMenu(QAction*)));
    connect(ui->instText, SIGNAL(cursorPositionChanged()), this, SLOT(highlightDebuggerLine_inst()));
    connect(ui->memoryText, SIGNAL(cursorPositionChanged()), this, SLOT(highlightDebuggerLine_data()));
    connect(ui->asm_input, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    has_assembled = false;
    debugger = new Debugger(MAX_MEMSIZE);
    // 视角拉倒第一页，且高亮当前行
    ui->tabWidget->setCurrentIndex(0);
    highlightCurrentLine();

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
    for (int i = 1; i < 32; i++) {
        tmp_info += assembler.getRegName(i, GET_NAME);
        tmp_info += ": ";
        tmp_info += assembler.bi2Hex(assembler.int2Binary(debugger->getReg(i), 32, UNSIGNED));
        tmp_info += "\t";
        // 一行6个,0号寄存器不显示
        if (i % 6 == 5)
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
    tmp_info = "0x";
    if ((ui->enableBP->checkState() == Qt::Checked)
            && (debugger->getBreakPoint() != DEFAULT_BP)) {
        tmp_info += QString::number(debugger->getBreakPoint(), 16);
        ui->breakpointText->setText(tmp_info);
    }
}

// 刷新debugger的主存内容
void Mainwindow::refreshWholeMem() {
    ui->memoryText->clear();
    QString tmp = "Address\t\tHex\t\tContents\n";
    uint list_size = debugger->getMemoTextSize();
    for (uint i = 0; i < list_size; i++) {
        tmp += debugger->getMemoTextAddr(i);
        tmp += "\t";
        tmp += debugger->getMemoTextHex(i);
        tmp += "\t";
        tmp += debugger->getMemoTextAsciz(i);
        tmp += "\n";
    }
    ui->memoryText->setText(tmp);
}

// Editor高亮行
void Mainwindow::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!(ui->asm_input->isReadOnly())) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor;
        if (assembler.getErrorno() == NO_ERROR)
            lineColor = QColor(Qt::darkGreen).darker(200);
        else
            lineColor = QColor(Qt::darkRed);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = ui->asm_input->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
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
    QColor lineColor = QColor(Qt::darkMagenta).darker(100);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = ui->memoryText->textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
    ui->memoryText->setExtraSelections(extraSelections);
}

// 当前内容未汇编时，若切换界面，则弹出提示并强制返回
void Mainwindow::on_tabWidget_tabBarClicked(int index) {
    if ((index == 2 || index == 1) && has_assembled == false) {
        QMessageBox::warning(this, tr("警告"), tr("当前代码尚未Assemble，请先使用Build(F5)！"));
    }
    else if ((index == 2 || index == 1) && assembler.getErrorno() != NO_ERROR) {
        QMessageBox::warning(this, tr("警告"), tr("代码有Error，请先修改您的代码然后重新Build(F5)！"));
    }
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

void Mainwindow::on_actionSaveOther_triggered() {
    fileSaveOther();
    return ;
}

// 按下“build”按钮后的一系列处理
void Mainwindow::on_actionAssemble_triggered() {
    // 将视角拉回编辑页
    ui->tabWidget->setCurrentIndex(0);
    workspaceClear();
    readInput();
    lineProcess();
    if (assembler.getErrorno() == NO_ERROR)
        transCode();
    showInfo();
    showOutput();
    has_assembled = true;
    if (assembler.getErrorno() == NO_ERROR) {
        initDebugInst();
        initDebugger();
        debugger->reset(assembler);
    }
    highlightCurrentLine();
}

// 以coe形式保存结果
void Mainwindow::on_actionSaveresult_triggered() {
    fileOutput_coe();
    return ;
}

// 以txt形式保存数据变量
void Mainwindow::on_actionOutputDatatxt_triggered() {
    dataOutput_txt();
    return ;
}

// 以coe形式保存数据变量
void Mainwindow::on_actionOutputDatacoe_triggered() {
    dataOutput_coe();
    return ;
}

// Run
void Mainwindow::on_actionRun_triggered() {
    uint pre_row = debugger->getPC() / 4;
    ui->tabWidget->setCurrentIndex(2);
    uint num = 0;
    if (debugger->getPC() / 4 < debugger->stdinput.size())
        num = debugger->run(assembler);
    if (num == MAX_INST_NUM) {
        QMessageBox::critical(this, tr("错误"), tr("到达指令计数上限，请检查是否存在死循环！"));
        return ;
    }
    if (pre_row + num + 1 == debugger->getInstVecSize()) {
        QMessageBox::information(this, tr("提示"), tr("已执行完所有指令，请重置Debugger！"));
        return ;
    }
    else if (pre_row + num == debugger->getBreakPoint() / 4)
        ui->outputText->append(QString("已运行到断点处"));

    // 移动光标
    if (debugger->getPC() / 4 < debugger->stdinput.size()) {
        ui->instText->moveCursor(QTextCursor::Start);
        uint cur_row = debugger->getPC() / 4;
        for (uint i = 0; i < cur_row; i++)
            ui->instText->moveCursor(QTextCursor::Down);
    }
    refreshDisplay();
    refreshWholeMem();
}

// Step
void Mainwindow::on_actionStep_triggered() {
    ui->tabWidget->setCurrentIndex(2);
    if (debugger->getPC() == 0 && debugger->stdinput.size()) {
        ui->instText->moveCursor(QTextCursor::Start);
    }
    if (debugger->getPC() / 4 < debugger->stdinput.size()) {
        debugger->step(assembler);
        uint cur_row = debugger->getPC() / 4;
        ui->instText->moveCursor(QTextCursor::Start);
        for (uint i = 0; i < cur_row; i++)
            ui->instText->moveCursor(QTextCursor::Down);
    }
    else {
        QMessageBox::information(this, tr("提示"), tr("已执行完所有指令，Debugger Reset!"));
        debugger->reset(assembler);
        ui->instText->moveCursor(QTextCursor::Start);
        refreshDisplay();
        refreshWholeMem();
        return ;
    }
    refreshDisplay();
    // 获取内存变更的真实地址
    if (debugger->isMemChanged())
        refreshMemLine(debugger->getChangedMemAddr());
    return ;
}

// Reset
void Mainwindow::on_actionReset_triggered() {
    ui->tabWidget->setCurrentIndex(2);
    debugger->reset(assembler);
    refreshDisplay();
    refreshWholeMem();
    ui->outputText->clear();
    ui->instText->moveCursor(QTextCursor::Start);
}

// 退出提示
void Mainwindow::on_actionExit_triggered() {
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, tr("提示"), tr("确实要退出吗?"), QMessageBox::Yes|QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        QMessageBox::StandardButton btn;
        btn = QMessageBox::question(this, tr("提示"), tr("结果保存好了吗?"), QMessageBox::Yes|QMessageBox::No);
        if (btn == QMessageBox::Yes)
            exit(0);
    }
    return ;
}

// 设置PC值
void Mainwindow::on_setPCButton_clicked() {
    QString pc_text = ui->PCtext->text().simplified();
    uint pc_set = 0;
    if (pc_text.mid(0, 2) == "0x")
        pc_set = pc_text.toInt(NULL, 16);
    else
        pc_set = pc_text.toInt();
    if (pc_set % 4) {
        QMessageBox::warning(this, tr("警告"), tr("PC值应该是4的整数倍（包括0）！"));
        return ;
    }
    debugger->setPC(pc_set);
    ui->instText->moveCursor(QTextCursor::Start);
    for (uint i = 0; i < pc_set; i += 4)
        ui->instText->moveCursor(QTextCursor::Down);
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
    tc.removeSelectedText();

    // 再将新的内容插入
    QString tmp = "";
    tmp += debugger->getMemoTextAddr(addr);
    tmp += "\t";
    tmp += debugger->getMemoTextHex(addr);
    tmp += "\t";
    tmp += debugger->getMemoTextAsciz(addr);
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
    ui->memoryText->verticalScrollBar()->setValue(int(float(addr*4) / debugger->getMemSize() * height));
}

// 勾选（取消）断点
void Mainwindow::on_enableBP_toggled(bool checked) {
    if (checked == false) {
        debugger->clearBreakPoint();
        QMessageBox::information(this, tr("提示"), tr("断点设置为失效"));
        return ;
    }
    else if (checked == true) {
        QString bp_text = ui->breakpointText->text();
        uint bp = 0;
        if (bp_text.mid(0, 2) == "0x")
            bp = bp_text.toInt(NULL, 16);
        else
            bp = bp_text.toInt();
        if (bp % 4) {
            QMessageBox::warning(this, tr("警告"), tr("断点地址应该是4的整数倍（包括0）！"));
            return ;
        }
        if (bp / 4 >= debugger->getInstVecSize()) {
            QMessageBox::critical(this, tr("错误"), tr("断点地址越界！"));
            return ;
        }
        debugger->setBreakPoint(bp);
        QMessageBox::information(this, tr("提示"), tr("断点设置为生效"));
    }
}

// 文本改变时，撤销之前的已汇编标志
void Mainwindow::on_asm_input_textChanged() {
    has_assembled = false;
    ui->tabWidget->setTabText(0, QString("Edit *"));
}

// 关于我的信息
void Mainwindow::on_actionAbout_triggered() {
    QMessageBox::about(this, tr("About LoongArch Assembler"),
                    tr("<p><b>LoongArch Assembler</b></p> " \
                       "<p><strong>Created by 罗翊洲 at 2022</strong></p>" \
                       "<p>哈尔滨工业大学（深圳）计算机科学与技术学院</p>" \
                       "<p>Email: 1713700496@qq.com</p>" \
                       "<a href=\"https://github.com/LuoYZ1028/LoongArchAssembler\" target=\"_red\">仓库链接</a>" \
                       ""));
}

void Mainwindow::on_actionDocument_triggered() {
    QMessageBox::about(this, tr("User's Document Link"),
                       tr("<p><b><a href=\"https://luoyz1028.github.io/myBlog/2022/10/11/" \
                          "LoongArch32%E4%BD%8D%E6%B1%87%E7%BC%96%E5%99%A8%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3/\"" \
                          "target=\"_red\">文档连接</a></b></p>"));
}
