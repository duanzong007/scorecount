#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include "ScoreViewer.h"
#include "ui_mainwindow.h"
#include "DatabaseManager.h"
#include <QAction>
#include <QButtonGroup>
#include <QCloseEvent>
#include "ExcelManager.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onGroupCountChanged(int groupCount); // 接受组数参数
    void onGroupButtonClicked(int groupIndex);
    void onConfirmClicked();
    void onActionViewScoreTriggered();  // 用于处理查看分数按钮的点击

signals:
    void scoreUpdated(QString groupName, const QStringList &selectedOptions);

private:
    Ui::MainWindow *ui;
    QPushButton* groupButtons[10]; // 存储最多10个按钮
    QLabel* groupLabel; // 显示当前选择的组
    QButtonGroup *scoreButtonGroup;
    int currentGroupCount;
    ScoreViewer *scoreViewer;
    DatabaseManager *dbManager = nullptr;
private slots:
    void showAbout();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
