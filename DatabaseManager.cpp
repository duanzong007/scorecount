#include "DatabaseManager.h"
#include <QDir>
#include <QRegularExpression>
#include <QTime>
#include <QMessageBox>
#include <QApplication>
//--------------------------------------------------
// ctor / dtor
//--------------------------------------------------
DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./database.db");

    if (!openDatabase()) {
        qDebug() << "[DB] 打开数据库失败，后续操作无法进行。";
        return;
    }

    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

//--------------------------------------------------
// connection helpers
//--------------------------------------------------
bool DatabaseManager::openDatabase()
{
    if (db.isOpen()) return true;

    if (!db.open()) {
        qDebug() << "[DB] 连接失败:" << db.lastError().text();
        return false;
    }
    QSqlQuery("PRAGMA foreign_keys = ON;");
    return true;
}

bool DatabaseManager::closeDatabase()
{
    if (!db.isOpen()) return true;

    QString name = db.connectionName();
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(name);
    return true;
}

//--------------------------------------------------
// schema & seed
//--------------------------------------------------
void DatabaseManager::initializeDatabase()
{
    QSqlQuery q;

    // GroupInfo
    const char *createGroupInfo =
        "CREATE TABLE IF NOT EXISTS GroupInfo ("
        "  group_id    INTEGER PRIMARY KEY,"
        "  group_name  TEXT,"  // 队长，允许为空
        "  group_dance TEXT,"  // 队舞，允许为空
        "  total_score INTEGER DEFAULT 0"
        ");";
    if (!q.exec(createGroupInfo))
        qDebug() << "[DB] 创建 GroupInfo 失败:" << q.lastError().text();

    // ScoreRecords
    const char *createScore =
        "CREATE TABLE IF NOT EXISTS ScoreRecords ("
        "  record_id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  group_id    INTEGER NOT NULL,"
        "  score       INTEGER NOT NULL,"
        "  description TEXT,"
        "  timestamp   DATETIME DEFAULT (DATETIME('now', '+8 hours'))"  // UTC 转北京时间
        ");";
    if (!q.exec(createScore))
        qDebug() << "[DB] 创建 ScoreRecords 失败:" << q.lastError().text();

    // seed 1~10，队长和队舞可以为空
    q.prepare("INSERT OR IGNORE INTO GroupInfo (group_id, group_name, group_dance) VALUES (?, ?, ?)");
    for(int i = 0; i < 10; ++i){
        q.bindValue(0, i + 1);
        q.bindValue(1, ""); // 队长名称
        q.bindValue(2, ""); // 默认队舞为空
        q.exec();
    }


    // 创建 Settings 表，用于存储总组数
    const char *createSettings =
        "CREATE TABLE IF NOT EXISTS Settings ("
        "  id INTEGER PRIMARY KEY,"
        "  total_group_count INTEGER DEFAULT 4"
        ");";
    if (!q.exec(createSettings))
        qDebug() << "[DB] 创建 Settings 失败:" << q.lastError().text();

    // 插入初始的总组数记录，如果没有的话
    q.prepare("INSERT OR IGNORE INTO Settings (id, total_group_count) VALUES (1, 4)");
    if (!q.exec()) {
        qDebug() << "[DB] 插入 Settings 记录失败:" << q.lastError().text();
    }



    qDebug() << "[DB] 初始化完成。工作目录:" << QDir::currentPath();
}

//--------------------------------------------------
// util : "第n组"->n or plain number
//--------------------------------------------------
int DatabaseManager::extractGroupId(const QString &label)
{
    static const QRegularExpression re("(\\d+)");
    QRegularExpressionMatch m = re.match(label);
    if (m.hasMatch())
        return m.captured(1).toInt();
    return -1;
}

//--------------------------------------------------
// public slot : scoreUpdated
//--------------------------------------------------
void DatabaseManager::scoreUpdated(QString groupName, const QStringList &opts)
{
    if (!openDatabase()) return;

    int gid = extractGroupId(groupName);
    if (gid <= 0) {
        qDebug() << "[DB] 无效组标签:" << groupName;
        return;
    }

    int deltaSum = 0;
    for(const QString &o: opts){
        int delta=0;
        if(o.startsWith("mp"))      delta = -o.mid(2).toInt();
        else if(o.startsWith("p"))  delta =  o.mid(1).toInt();
        else if(o.startsWith("m")){
            delta =  o.mid(1).toInt();
            for(int i = 1;i <= currentGroupCount; ++i){
                if(i != gid && delta != 0){
                    updateTotalScore(i, delta);
                    if(!insertScoreRecord(i, delta)) return;
                }
            }
            return;
        }
        else continue;

        if(!insertScoreRecord(gid, delta)) return;
        deltaSum += delta;
    }
    if(deltaSum!=0) updateTotalScore(gid, deltaSum);
}

//--------------------------------------------------
// DB helpers
//--------------------------------------------------
bool DatabaseManager::insertScoreRecord(int gid, int score, const QString &desc)
{
    QString currentTimestamp = QDateTime::currentDateTimeUtc().addSecs(8 * 3600).toString("yyyy-MM-dd hh:mm:ss");
    QSqlQuery q;
    q.prepare("INSERT INTO ScoreRecords (group_id, score, description, timestamp) VALUES (:gid, :s, :d, :timestamp)");
    q.bindValue(":gid", gid);
    q.bindValue(":s", score);
    q.bindValue(":d", desc);  // 确保 desc 是有意义的
    q.bindValue(":timestamp", currentTimestamp);

    if (!q.exec()) {
        qDebug() << "[DB] 记录插入失败:" << q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::insertScoreRecord(int gid,int score)
{
    QString currentTimestamp = QDateTime::currentDateTimeUtc().addSecs(8 * 3600).toString("yyyy-MM-dd hh:mm:ss");
    QSqlQuery q;
    q.prepare("INSERT INTO ScoreRecords (group_id, score, timestamp) VALUES (:gid,:s, :timestamp)");
    q.bindValue(":gid",gid);
    q.bindValue(":s",score);
    q.bindValue(":timestamp", currentTimestamp);
    if(!q.exec()){
        qDebug()<<"[DB] 记录插入失败:"<<q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateTotalScore(int gid,int delta)
{
    QSqlQuery q;
    q.prepare("UPDATE GroupInfo SET total_score = total_score + :d WHERE group_id = :gid");
    q.bindValue(":d",delta);
    q.bindValue(":gid",gid);
    if(!q.exec()){
        qDebug()<<"[DB] 更新总分失败:"<<q.lastError().text();
        return false;
    }
    return true;
}


QList<GroupInfo> DatabaseManager::fetchGroups(int maxId){
    QList<GroupInfo> groups;
    QSqlQuery query;

    query.prepare("SELECT group_id, group_name, group_dance, total_score FROM GroupInfo WHERE group_id <= :maxId");
    query.bindValue(":maxId", maxId);

    if (query.exec()) {
        while (query.next()) {
            GroupInfo group;
            group.id = query.value(0).toInt();           // 获取小组ID
            group.name = query.value(1).toString(); // 获取小组名称（队长）
            group.dance = query.value(2).toString(); // 获取队舞
            group.totalScore = query.value(3).toInt();   // 获取小组总分
            groups.append(group);
        }
    } else {
        qDebug() << "Error fetching groups:" << query.lastError().text();
    }

    return groups;
}



void DatabaseManager::setGroupCount(int a){
    currentGroupCount = a;
}


// 获取总组数
int DatabaseManager::getTotalGroupCount()
{
    QSqlQuery query;
    query.prepare("SELECT total_group_count FROM Settings WHERE id = 1");

    if (query.exec()) {
        if (query.next()) {
            return query.value(0).toInt();
        }
    } else {
        qDebug() << "[DB] 获取总组数失败:" << query.lastError().text();
    }

    return -1;  // 如果没有获取到，返回 -1
}

// 更新总组数
bool DatabaseManager::updateTotalGroupCount(int count)
{
    QSqlQuery query;
    query.prepare("UPDATE Settings SET total_group_count = :count WHERE id = 1");
    query.bindValue(":count", count);

    if (!query.exec()) {
        qDebug() << "[DB] 更新总组数失败:" << query.lastError().text();
        return false;
    }
    return true;
}


// 清空所有数据：清空 GroupInfo 和 ScoreRecords 表
void DatabaseManager::clearAllData()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "确认清空数据", "确定要清空所有数据吗？\n清空将退出程序",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        // 用户取消清空操作
        return;
    }
    // 打开数据库
    if (!openDatabase()) {
        qDebug() << "[DB] 数据库未打开，无法清空数据";
        return;
    }

    // 清空 GroupInfo 表
    QSqlQuery clearGroupQuery("DELETE FROM GroupInfo");
    if (!clearGroupQuery.exec()) {
        qDebug() << "[DB] 清空 GroupInfo 表失败:" << clearGroupQuery.lastError().text();
    }

    // 清空 ScoreRecords 表
    QSqlQuery clearScoreQuery("DELETE FROM ScoreRecords");
    if (!clearScoreQuery.exec()) {
        qDebug() << "[DB] 清空 ScoreRecords 表失败:" << clearScoreQuery.lastError().text();
    }

    qDebug() << "[DB] 所有数据已清空";

    QCoreApplication::quit();  // 退出应用程序
}
