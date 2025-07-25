#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QStringList>

struct GroupInfo {
    int id;           // 小组ID
    QString name; // 小组名称（即队长）
    QString dance; // 队舞
    int totalScore;   // 小组总分
};


class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool openDatabase();
    bool closeDatabase();
    void initializeDatabase();
    QList<GroupInfo> fetchGroups(int maxId);  // 获取小组信息
    void setGroupCount(int a);
    bool updateTotalGroupCount(int count);
    int getTotalGroupCount();

public slots:
    void scoreUpdated(QString groupName, const QStringList &selectedOptions);
    void clearAllData();  // 清空所有数据

private:
    static int extractGroupId(const QString &groupLabel);

    bool insertScoreRecord(int gid, int score);
    bool insertScoreRecord(int gid, int score, const QString &desc);

    bool updateTotalScore(int groupId, int score);

    QSqlDatabase db;

    int currentGroupCount;
};

#endif // DATABASEMANAGER_H
