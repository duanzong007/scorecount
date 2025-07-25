#ifndef EXCELMANAGER_H
#define EXCELMANAGER_H

#include <QObject>
#include <QString>

class DatabaseManager;

class ExcelManager : public QObject
{
    Q_OBJECT
public:
    explicit ExcelManager(DatabaseManager *dbManager, QObject *parent = nullptr);

public slots:
    bool exportAllToExcel(); // 导出两个表到一个 Excel 文件（两个工作簿）
    bool importFromExcel();
private:
    DatabaseManager *db; // 数据库指针，不负责创建和销毁
    bool importFromExcel(const QString &filePath);
};

#endif // EXCELMANAGER_H

