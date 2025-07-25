#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon("://resource/logo.ico"));

    // 创建DatabaseManager对象
    dbManager = new DatabaseManager(this);
    dbManager->setGroupCount(4);
    scoreViewer = new ScoreViewer(currentGroupCount, nullptr);
    scoreViewer->setdb(dbManager);
    ExcelManager *excelMgr = new ExcelManager(dbManager, this);
    // 打开数据库连接
    if (!dbManager->openDatabase()) {
        qDebug() << "Failed to connect to the database!";
        return;
    }

    // 连接信号与槽
    connect(this,             &MainWindow::scoreUpdated, dbManager, &DatabaseManager::scoreUpdated);
    connect(ui->chakanfenshu, &QAction::triggered,       this,      &MainWindow::onActionViewScoreTriggered);
    connect(ui->viewscore,    &QPushButton::clicked,     this,      &MainWindow::onActionViewScoreTriggered);
    connect(ui->daochu_excel, &QAction::triggered,       excelMgr,  &ExcelManager::exportAllToExcel);
    connect(ui->daoru_excel,  &QAction::triggered,       excelMgr,  static_cast<bool (ExcelManager::*)()>(&ExcelManager::importFromExcel));
    connect(ui->qingkong,     &QAction::triggered,       dbManager, &DatabaseManager::clearAllData);
    connect(ui->banben,       &QAction::triggered,       this,      &MainWindow::showAbout);
    // 初始化按钮数组
    groupButtons[0] = ui->pbzu1; groupButtons[1] = ui->pbzu2; groupButtons[2] = ui->pbzu3; groupButtons[3] = ui->pbzu4; groupButtons[4] = ui->pbzu5;
    groupButtons[5] = ui->pbzu6; groupButtons[6] = ui->pbzu7; groupButtons[7] = ui->pbzu8; groupButtons[8] = ui->pbzu9; groupButtons[9] = ui->pbzu10;

    groupLabel = ui->groupshow;

    // 创建QButtonGroup，并将勾选框添加到其中
    scoreButtonGroup = new QButtonGroup(this);
    scoreButtonGroup->addButton(ui->p500);
    scoreButtonGroup->addButton(ui->p1000);
    scoreButtonGroup->addButton(ui->p1500);
    scoreButtonGroup->addButton(ui->p2000);
    scoreButtonGroup->addButton(ui->p2500);
    scoreButtonGroup->addButton(ui->p3000);
    scoreButtonGroup->addButton(ui->p5000);
    scoreButtonGroup->addButton(ui->p10000);
    scoreButtonGroup->addButton(ui->m500);
    scoreButtonGroup->addButton(ui->m1000);
    scoreButtonGroup->addButton(ui->m1500);
    scoreButtonGroup->addButton(ui->m2000);
    scoreButtonGroup->addButton(ui->m2500);
    scoreButtonGroup->addButton(ui->m3000);
    scoreButtonGroup->addButton(ui->m5000);
    scoreButtonGroup->addButton(ui->m10000);
    scoreButtonGroup->addButton(ui->checkpdiy);
    scoreButtonGroup->addButton(ui->checkmdiy);
    scoreButtonGroup->addButton(ui->checkmpdiy);
    // 设置为互斥模式，保证只有一个勾选框被选中
    scoreButtonGroup->setExclusive(true);

    connect(ui->checkbuttom, &QPushButton::clicked, this, &MainWindow::onConfirmClicked);

    // 连接菜单项选择组数
    connect(ui->action1, &QAction::triggered, this, [this]() { onGroupCountChanged(1); });
    connect(ui->action2, &QAction::triggered, this, [this]() { onGroupCountChanged(2); });
    connect(ui->action3, &QAction::triggered, this, [this]() { onGroupCountChanged(3); });
    connect(ui->action4, &QAction::triggered, this, [this]() { onGroupCountChanged(4); });
    connect(ui->action5, &QAction::triggered, this, [this]() { onGroupCountChanged(5); });
    connect(ui->action6, &QAction::triggered, this, [this]() { onGroupCountChanged(6); });
    connect(ui->action7, &QAction::triggered, this, [this]() { onGroupCountChanged(7); });
    connect(ui->action8, &QAction::triggered, this, [this]() { onGroupCountChanged(8); });
    connect(ui->action9, &QAction::triggered, this, [this]() { onGroupCountChanged(9); });
    connect(ui->action10, &QAction::triggered, this, [this]() { onGroupCountChanged(10); });
    onGroupCountChanged(dbManager->getTotalGroupCount());
    onGroupButtonClicked(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onGroupCountChanged(int groupCount)
{
    currentGroupCount = groupCount;
    scoreViewer->refresh(currentGroupCount);
    dbManager->setGroupCount(currentGroupCount);
    dbManager->updateTotalGroupCount(currentGroupCount);
    // 显示对应数量的组按钮
    for (int i = 0; i < 10; ++i) {
        if (i < groupCount) {
            groupButtons[i]->setVisible(true); // 显示按钮
            connect(groupButtons[i], &QPushButton::clicked, this, [this, i]() { onGroupButtonClicked(i); });
        } else {
            groupButtons[i]->setVisible(false); // 隐藏按钮
        }
    }

    // 获取 "组数" 菜单项
    QMenu* groupMenu = ui->menubar->findChild<QMenu*>("bianji")->findChild<QMenu*>("zushu");

    // 更新菜单项的勾选状态
    for (int i = 0; i < 10; ++i) {
        QAction* action = groupMenu->actions().at(i);  // 获取对应的 action
        if (action) {
            if (i + 1 == groupCount) {
                action->setChecked(true);  // 只有当前选中的组才勾选
            } else {
                action->setChecked(false); // 取消其他组的勾选
            }
        }
    }

    onGroupButtonClicked(0);
}

void MainWindow::onGroupButtonClicked(int groupIndex)
{
    // 高亮显示已选择的组并更新label
    for (int i = 0; i < 10; ++i) {
        if (i == groupIndex) {
            groupButtons[i]->setStyleSheet("background-color: lightblue"); // 高亮显示
            groupLabel->setText("第" + QString::number(groupIndex + 1) + "组");
        } else {
            groupButtons[i]->setStyleSheet(""); // 重置其他按钮样式
        }
    }
}

void MainWindow::onConfirmClicked()
{
    // 获取当前选择的小组
    QString currentGroupName = groupLabel->text();  // 获取当前显示的组名

    // 初始化选中的选项列表
    QStringList selectedOptions;

    // 处理给当前组加分（p系列）并加入选项列表
    if (ui->p500->isChecked()) selectedOptions.append("p500");
    if (ui->p1000->isChecked()) selectedOptions.append("p1000");
    if (ui->p1500->isChecked()) selectedOptions.append("p1500");
    if (ui->p2000->isChecked()) selectedOptions.append("p2000");
    if (ui->p2500->isChecked()) selectedOptions.append("p2500");
    if (ui->p3000->isChecked()) selectedOptions.append("p3000");
    if (ui->p5000->isChecked()) selectedOptions.append("p5000");
    if (ui->p10000->isChecked()) selectedOptions.append("p10000");

    // 处理给其他小组加分（m系列）并加入选项列表
    if (ui->m500->isChecked()) selectedOptions.append("m500");
    if (ui->m1000->isChecked()) selectedOptions.append("m1000");
    if (ui->m1500->isChecked()) selectedOptions.append("m1500");
    if (ui->m2000->isChecked()) selectedOptions.append("m2000");
    if (ui->m2500->isChecked()) selectedOptions.append("m2500");
    if (ui->m3000->isChecked()) selectedOptions.append("m3000");
    if (ui->m5000->isChecked()) selectedOptions.append("m5000");
    if (ui->m10000->isChecked()) selectedOptions.append("m10000");

    // 处理给当前组扣分（pm系列）并加入选项列表
    if (ui->checkmpdiy->isChecked()) {
        selectedOptions.append("mp" + QString::number(ui->mpdiyscore->value()));

    }

    // 处理自定义分数（pdiy和mdiy）并加入选项列表
    if (ui->checkpdiy->isChecked()) {
        selectedOptions.append("p" + QString::number(ui->pdiyscore->value()));
    }
    if (ui->checkmdiy->isChecked()) {
        selectedOptions.append("m" + QString::number(ui->mdiyscore->value()));
    }

    QString b = selectedOptions.join(", ");
    // 打印当前选择的选项
    qDebug() << "当前小组：" << currentGroupName;
    qDebug() << "选择的选项：" << b;

    QString a = "";
    if (b.startsWith("mp")) {
        a = currentGroupName + "扣掉" + b.mid(2) + "分";
    } else if (b.startsWith("p")) {
        a = currentGroupName + "加上" + b.mid(1) + "分";
    } else if (b.startsWith("m")) {
        a = "除了" + currentGroupName + "，每组加上" + b.mid(1) + "分";
    }

    if(a != ""){
        // 创建 QLabel 用于显示消息
        QLabel *messageLabel = new QLabel(a, this);

        // 将 QLabel 添加到状态栏的右侧
        ui->statusbar->addPermanentWidget(messageLabel);

        // 设置定时器，1.5秒后自动隐藏并删除 messageLabel
        QTimer::singleShot(2500, this, [this, messageLabel]() {
            // 从状态栏移除 messageLabel
            ui->statusbar->removeWidget(messageLabel);

            // 删除 messageLabel 控件
            delete messageLabel;
        });
    }




    // 发出信号，将加分信息传递给后端
    emit scoreUpdated(currentGroupName, selectedOptions);


    QTimer::singleShot(100, this, [this]() {
        scoreViewer->refresh(currentGroupCount);  // 延迟刷新
    });
}


void MainWindow::onActionViewScoreTriggered()
{
    scoreViewer->refresh(currentGroupCount);
    scoreViewer->show();  // 显示 ScoreViewer 窗口
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (scoreViewer) {
        delete scoreViewer;  // 手动删除 ScoreViewer
    }

    // 让事件继续传播（允许窗口关闭）
    event->accept();
}

void MainWindow::showAbout(){
    QMessageBox::question(this, "关于", "软件版本：v1.0.0");
}
