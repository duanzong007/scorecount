#include "ExcelManager.h"
#include "DatabaseManager.h"
#include "xlsxdocument.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFile>
#include "QXlsx/header/xlsxglobal.h"

ExcelManager::ExcelManager(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), db(dbManager)
{}

bool ExcelManager::exportAllToExcel()
{
    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    // 拼接文件路径
    const QString filePath = desktopPath + "/积分榜.xlsx";  // 文件名为 "导出结果.xlsx"

    QXlsx::Document xlsx;


    //===============================
    // 工作簿 1：GroupInfo
    //===============================
    xlsx.addSheet("小组信息");
    xlsx.selectSheet("小组信息");

    QSqlQuery groupQuery("SELECT group_id, group_name, group_dance, total_score FROM GroupInfo");
    if (!groupQuery.exec()) {
        qDebug() << "[Excel] 导出 GroupInfo 失败:" << groupQuery.lastError().text();
        return false;
    }

    xlsx.write("A1", "组ID");
    xlsx.write("B1", "队长");
    xlsx.write("C1", "队舞");
    xlsx.write("D1", "总分");

    int groupRow = 2;
    while (groupQuery.next()) {
        xlsx.write(groupRow, 1, groupQuery.value(0));  // 组ID
        xlsx.write(groupRow, 2, groupQuery.value(1));  // 队长
        xlsx.write(groupRow, 3, groupQuery.value(2));  // 队舞
        xlsx.write(groupRow, 4, groupQuery.value(3));  // 总分
        groupRow++;
    }

    //===============================
    // 工作簿 2：ScoreRecords
    //===============================
    xlsx.addSheet("积分记录");
    xlsx.selectSheet("积分记录");

    //===============================
    // 设置每个小组的列宽
    //===============================
    int col = 1;
    for (int i = 1; i <= 10; ++i) {
        xlsx.setColumnWidth(col, 10); // 每组的分数列宽度
        xlsx.setColumnWidth(col + 1, 20); // 每组的时间列宽度
        xlsx.setColumnWidth(col + 2, 3); // 每组的时间列宽度
        col += 3;
    }



    // 第一行：每个小组名称占两列，横向排列
    col = 1;
    for (int i = 1; i <= 10; ++i) {
        QString groupName = QString("第%1组").arg(i);
        xlsx.write(1, col, groupName);   // 写入组名
        //lsx.mergeCells(1, col, 1, col + 1);  // 合并每组的两列
        col += 3; // 每个组占两列
    }

    // 第二行：显示“分数”和“时间”
    col = 1;
    for (int i = 1; i <= 10; ++i) {
        xlsx.write(2, col, "分数");
        xlsx.write(2, col + 1, "时间");
        col += 3;  // 每组分数时间各占两列
    }

    // 创建数字格式
    QXlsx::Format numberFormat;
    numberFormat.setNumberFormat("0");  // 设置为整数格式

    // 从第三行开始填充数据，按照小组记录每组的积分
    QSqlQuery scoreQuery("SELECT record_id, group_id, score, description, timestamp FROM ScoreRecords ORDER BY timestamp");
    if (!scoreQuery.exec()) {
        qDebug() << "[Excel] 导出ScoreRecords失败:" << scoreQuery.lastError().text();
        return false;
    }

    // 为每组分配列位置
    int row = 3; // 从第三行开始
    QVector<int> groupRows(10, row); // 每组从第三行开始填充

    while (scoreQuery.next()) {
        int groupId = scoreQuery.value(1).toInt();
        int score = scoreQuery.value(2).toInt();
        QString timestamp = scoreQuery.value(4).toString();  // 取时间戳

        // 计算对应小组的列位置
        int col = (groupId - 1) * 3 + 1; // 每个小组占两列

        // 写入分数和时间
        xlsx.write(groupRows[groupId - 1], col, score); // 写分数（直接存储为数字）
        xlsx.write(groupRows[groupId - 1], col + 1, timestamp); // 写时间

        // 设置分数单元格为数字格式
        xlsx.setColumnFormat(groupRows[groupId - 1], col, numberFormat); // 设置分数列为数字格式

        // 增加该小组的下一行数据位置
        groupRows[groupId - 1]++;
    }


    //===============================
    // 删除默认 Sheet1 并保存
    //===============================
    xlsx.selectSheet("Sheet1"); // 切换到默认
    xlsx.deleteSheet("Sheet1"); // 删除默认工作表

    bool success = xlsx.saveAs(filePath);
    if (success) {
        // 如果导出成功，弹出提示框
        QMessageBox::question(nullptr, "导出成功", "数据已成功导出到桌面！");
    }

    return success;
}




bool ExcelManager::importFromExcel()
{
    // 使用 QFileDialog 弹出文件选择对话框，选择 Excel 文件
    QString filePath = QFileDialog::getOpenFileName(nullptr, "选择 Excel 文件", "", "Excel 文件 (*.xlsx);;所有文件 (*.*)");

    // 如果用户没有选择文件，返回
    if (filePath.isEmpty()) {
        qDebug() << "[Excel] 用户未选择文件，导入操作取消。";
        return false;
    }

    // 调用导入函数，传入选择的文件路径
    return importFromExcel(filePath);
}

bool ExcelManager::importFromExcel(const QString &filePath)
{
    QXlsx::Document xlsx(filePath);
    if (!xlsx.isLoadPackage()) {
        qDebug() << "[Excel] 打开文件失败:" << filePath;
        return false;
    }

    QString currentTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // =======================
    // 清空旧数据
    // =======================
    QSqlQuery clearGroupQuery("DELETE FROM GroupInfo");
    if (!clearGroupQuery.exec()) {
        qDebug() << "[Excel] 清空 GroupInfo 表失败:" << clearGroupQuery.lastError().text();
        return false;
    }

    QSqlQuery clearScoreQuery("DELETE FROM ScoreRecords");
    if (!clearScoreQuery.exec()) {
        qDebug() << "[Excel] 清空 ScoreRecords 表失败:" << clearScoreQuery.lastError().text();
        return false;
    }

    // =======================
    // 导入 GroupInfo 数据
    // =======================
    QXlsx::Worksheet *groupWorksheet = dynamic_cast<QXlsx::Worksheet*>(xlsx.sheet("小组信息"));
    if (!groupWorksheet) {
        qDebug() << "[Excel] 获取 '小组信息' 工作表失败";
        return false;
    }

    QSqlQuery groupQuery;
    groupQuery.prepare("INSERT INTO GroupInfo (group_id, group_name, group_dance) VALUES (?, ?, ?)");

    for (int row = 2; row <= 11; ++row) { // 读取第2行到第11行数据，假设第1行为表头
        QVariant groupId = groupWorksheet->read(row, 1);  // 组ID（应该是1到10）
        QVariant groupName = groupWorksheet->read(row, 2);  // 队长
        QVariant groupDance = groupWorksheet->read(row, 3);  // 队舞

        // 如果groupId为空，则跳过该行
        if (groupId.isNull()) {
            continue;
        }

        groupQuery.bindValue(0, groupId.toInt());
        groupQuery.bindValue(1, groupName.toString().isEmpty() ? "" : groupName.toString());
        groupQuery.bindValue(2, groupDance.toString().isEmpty() ? "" : groupDance.toString());

        if (!groupQuery.exec()) {
            qDebug() << "[Excel] 插入 GroupInfo 数据失败:" << groupQuery.lastError().text();
            return false;
        }
    }

    // =======================
    // 导入 ScoreRecords 数据
    // =======================
    QXlsx::Worksheet *scoreWorksheet = dynamic_cast<QXlsx::Worksheet*>(xlsx.sheet("积分记录"));
    if (!scoreWorksheet) {
        qDebug() << "[Excel] 获取 '积分记录' 工作表失败";
        return false;
    }

    QSqlQuery scoreQuery;
    scoreQuery.prepare("INSERT INTO ScoreRecords (score, timestamp, group_id) VALUES (?, ?, ?)");

    int row = 3;  // 从第3行开始读取数据

    // 获取工作表的最大行数
    int lastRow = scoreWorksheet->dimension().lastRow();
    if (lastRow < row) {
        qDebug() << "[Excel] 工作表没有足够的行数据";
        return false;
    }

    // 定义一个QMap来存储每个小组的总分
    QMap<int, int> groupScores;

    // 持续读取每行数据直到没有更多数据
    while (row <= lastRow) {

        for (int col = 1; col <= 10; ++col) {  // 每列代表一个小组
            // 每列有三列数据: 分数，时间戳，空列
            QVariant score = scoreWorksheet->read(row, (col - 1) * 3 + 1);  // 获取分数

            // 如果有分数，就插入数据库并计算总分
            if (!score.isNull()) {
                scoreQuery.bindValue(0, score.toInt());  // 分数
                scoreQuery.bindValue(1, currentTimestamp);  // 使用当前时间戳
                scoreQuery.bindValue(2, col);  // group_id 从列号推算

                if (!scoreQuery.exec()) {
                    qDebug() << "[Excel] 插入 ScoreRecords 数据失败:" << scoreQuery.lastError().text();
                    return false;
                }

                // 计算该小组的总分
                if (!groupScores.contains(col)) {
                    groupScores[col] = 0;
                }
                groupScores[col] += score.toInt();  // 累加分数
            }
        }
        row++;  // 读取下一行
    }

    // =======================
    // 更新 GroupInfo 表中的总分
    // =======================
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE GroupInfo SET total_score = ? WHERE group_id = ?");

    for (auto it = groupScores.begin(); it != groupScores.end(); ++it) {
        int groupId = it.key();
        int totalScore = it.value();

        updateQuery.bindValue(0, totalScore);  // 更新总分
        updateQuery.bindValue(1, groupId);  // 按照group_id更新

        if (!updateQuery.exec()) {
            qDebug() << "[Excel] 更新 GroupInfo 总分失败:" << updateQuery.lastError().text();
            return false;
        }
    }

    QMessageBox::question(nullptr, "导入成功", "数据已成功导入到数据库！");
    return true;
}
