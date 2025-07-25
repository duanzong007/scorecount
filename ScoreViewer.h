#ifndef SCOREVIEWER_H
#define SCOREVIEWER_H

#include <QMainWindow>
#include <QLabel>
#include <QList>
#include "DatabaseManager.h"
#include <QWidget>
namespace Ui {
class ScoreViewer;
}

class ScoreViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScoreViewer(int groupCount, QWidget *parent = nullptr);
    ~ScoreViewer();
    void setdb(DatabaseManager *db);

public slots:
    void refresh(int groupCount);      // 外部调用刷新数据

private:
    void loadGroupInfo();              // 加载小组信息
    int scoreGroupCount;                 // 当前组数
    QLabel*  groupScoreLabel[10];           // 每个小组对应的 label
    QLabel*  groupMameLabel[10];
    QLabel*  groupDanceLabel[10];
    QWidget* groupInfo[10];

    Ui::ScoreViewer *ui;               // UI 设计文件生成的类指针
    DatabaseManager *dbManager;


};

#endif // SCOREVIEWER_H
