#include "ScoreViewer.h"
#include "ui_ScoreViewer.h"
#include <QDebug>
ScoreViewer::ScoreViewer(int groupCount, QWidget *parent)
    : QMainWindow(parent), scoreGroupCount(groupCount), ui(new Ui::ScoreViewer)
{
    ui->setupUi(this);  // 从 UI 文件加载布局
    this->setWindowFlags(Qt::Window);
    setWindowIcon(QIcon("://resource/logo.ico"));

    groupInfo[0] = ui->infogroup1; groupInfo[1] = ui->infogroup2; groupInfo[2] = ui->infogroup3; groupInfo[3] = ui->infogroup4; groupInfo[4] = ui->infogroup5;
    groupInfo[5] = ui->infogroup6; groupInfo[6] = ui->infogroup7; groupInfo[7] = ui->infogroup8; groupInfo[8] = ui->infogroup9; groupInfo[9] = ui->infogroup10;

    groupScoreLabel[0] = ui->labelscore1; groupScoreLabel[1] = ui->labelscore2; groupScoreLabel[2] = ui->labelscore3; groupScoreLabel[3] = ui->labelscore4; groupScoreLabel[4] = ui->labelscore5;
    groupScoreLabel[5] = ui->labelscore6; groupScoreLabel[6] = ui->labelscore7; groupScoreLabel[7] = ui->labelscore8; groupScoreLabel[8] = ui->labelscore9; groupScoreLabel[9] = ui->labelscore10;

    groupMameLabel[0]  = ui->labelname1;  groupMameLabel[1]  = ui->labelname2;  groupMameLabel[2]  = ui->labelname3;  groupMameLabel[3]  = ui->labelname4;  groupMameLabel[4]  = ui->labelname5;
    groupMameLabel[5]  = ui->labelname6;  groupMameLabel[6]  = ui->labelname7;  groupMameLabel[7]  = ui->labelname8;  groupMameLabel[8]  = ui->labelname9;  groupMameLabel[9]  = ui->labelname10;

    groupDanceLabel[0] = ui->labeldance1; groupDanceLabel[1] = ui->labeldance2; groupDanceLabel[2] = ui->labeldance3; groupDanceLabel[3] = ui->labeldance4; groupDanceLabel[4] = ui->labeldance5;
    groupDanceLabel[5] = ui->labeldance6; groupDanceLabel[6] = ui->labeldance7; groupDanceLabel[7] = ui->labeldance8; groupDanceLabel[8] = ui->labeldance9; groupDanceLabel[9] = ui->labeldance10;

    refresh(4);  // 初始化时加载数据
}

ScoreViewer::~ScoreViewer()
{
    delete ui;
}

void ScoreViewer::refresh(int groupCount)
{
    scoreGroupCount = groupCount;
    loadGroupInfo();  // 每次刷新都重新加载小组信息

}

void ScoreViewer::loadGroupInfo()
{
    // 获取小组信息
    QList<GroupInfo> groups = dbManager->fetchGroups(scoreGroupCount);

    // // 更新 UI，显示小组信息
    for (int i = 0; i < scoreGroupCount; ++i) {
        if (i < groups.size()) {
            groupScoreLabel[i]->setText("分数: " + QString::number(groups[i].totalScore));
            groupMameLabel[i]->setText("队长: " + groups[i].name);
            groupDanceLabel[i]->setText("队舞: " + groups[i].dance);
        }
    }
    for(int i = 0; i < 10; ++i){
        if(i<scoreGroupCount) groupInfo[i]->setVisible(true);
        else                  groupInfo[i]->setVisible(false);
    }
}

void ScoreViewer::setdb(DatabaseManager *db){
    dbManager = db;
}

