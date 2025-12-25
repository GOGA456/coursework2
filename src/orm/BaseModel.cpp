#include "../../include/orm/BaseModel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>  
#include <QDate>

BaseModel::BaseModel(QObject *parent)
    : QObject(parent)
{
}

void BaseModel::bindParameters(QSqlQuery &query, const QHash<QString, QVariant> &params)
{
    for (auto it = params.begin(); it != params.end(); ++it) {
        QString placeholder = ":" + it.key();
        query.bindValue(placeholder, it.value());
    }
}

QHash<QString, QVariant> BaseModel::extractRowFromQuery(QSqlQuery &query)
{
    QHash<QString, QVariant> row;
    QSqlRecord record = query.record();

    for (int i = 0; i < record.count(); ++i) {
        QString fieldName = record.fieldName(i);
        row[fieldName] = query.value(i);
    }

    return row;
}

QList<QHash<QString, QVariant>> BaseModel::executeQuery(const QString &sql, const QHash<QString, QVariant> &params)
{
    QList<QHash<QString, QVariant>> results;
    QSqlQuery query;

    if (!query.prepare(sql)) {
        qDebug() << "SQL Prepare error:" << query.lastError().text();
        qDebug() << "SQL:" << sql;
        return results;
    }

    bindParameters(query, params);

    if (!query.exec()) {
        qDebug() << "SQL Execute error:" << query.lastError().text();
        qDebug() << "SQL:" << sql;
        return results;
    }

    while (query.next()) {
        results.append(extractRowFromQuery(query));
    }

    return results;
}

QList<QHash<QString, QVariant>> BaseModel::executeQuerySimple(const QString &sql)
{
    QList<QHash<QString, QVariant>> results;
    QSqlQuery query;

    if (!query.exec(sql)) {
        qDebug() << "SQL Execute error:" << query.lastError().text();
        qDebug() << "SQL:" << sql;
        return results;
    }

    while (query.next()) {
        results.append(extractRowFromQuery(query));
    }

    return results;
}

QVariant BaseModel::executeScalar(const QString &sql, const QHash<QString, QVariant> &params)
{
    QSqlQuery query;

    if (!query.prepare(sql)) {
        qDebug() << "Scalar Prepare error:" << query.lastError().text();
        return QVariant();
    }

    bindParameters(query, params);

    if (!query.exec()) {
        qDebug() << "Scalar Execute error:" << query.lastError().text();
        return QVariant();
    }

    if (query.next()) {
        return query.value(0);
    }

    return QVariant();
}

QVariant BaseModel::executeScalarSimple(const QString &sql)
{
    QSqlQuery query;

    if (!query.exec(sql)) {
        qDebug() << "Scalar Execute error:" << query.lastError().text();
        return QVariant();
    }

    if (query.next()) {
        return query.value(0);
    }

    return QVariant();
}

bool BaseModel::executeNonQuery(const QString &sql, const QHash<QString, QVariant> &params)
{
    QSqlQuery query;

    if (!query.prepare(sql)) {
        qDebug() << "NonQuery Prepare error:" << query.lastError().text();
        return false;
    }

    bindParameters(query, params);

    return query.exec();
}

bool BaseModel::executeNonQuerySimple(const QString &sql)
{
    QSqlQuery query;
    return query.exec(sql);
}

bool BaseModel::clearTable(const QString &tableName)
{
    QString sql = QString("DELETE FROM %1").arg(tableName);
    
    if (!executeNonQuerySimple(sql)) {
        qDebug() << "Failed to clear table:" << tableName;
        return false;
    }

    return true;
}

bool BaseModel::backupDatabase(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for backup:" << filename;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    QList<QString> tables = getAllTableNames();

    // Добавляем заголовок
    stream << "-- Резервная копия базы данных\n";
    stream << "-- Создана: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";

    for (const QString &table : tables) {
        qDebug() << "Backing up table:" << table;
        
        // Получаем структуру таблицы (опционально)
        stream << "\n-- Таблица: " << table << "\n";
        
        // Сначала удаляем существующие данные (для очистки при восстановлении)
        stream << "-- DELETE FROM " << table << ";\n\n";
        
        // Получаем данные
        QString dataSql = QString("SELECT * FROM %1").arg(table);
        QList<QHash<QString, QVariant>> rows = executeQuerySimple(dataSql);
        
        qDebug() << "  Found" << rows.size() << "rows";
        
        for (const auto &row : rows) {
            QStringList columns;
            QStringList values;

            for (auto it = row.begin(); it != row.end(); ++it) {
                columns << it.key();
                QVariant value = it.value();
                QString valueStr;
                
                if (value.isNull()) {
                    valueStr = "NULL";
                } else if (value.type() == QVariant::String) {
                    valueStr = "'" + value.toString().replace("'", "''") + "'";
                } else if (value.type() == QVariant::DateTime) {
                    QDateTime dt = value.toDateTime();
                    valueStr = "'" + dt.toString("yyyy-MM-dd HH:mm:ss") + "'";
                } else if (value.type() == QVariant::Date) {
                    QDate d = value.toDate();
                    valueStr = "'" + d.toString("yyyy-MM-dd") + "'";
                } else if (value.type() == QVariant::Bool) {
                    valueStr = value.toBool() ? "TRUE" : "FALSE";
                } else {
                    valueStr = value.toString();
                }
                
                values << valueStr;
            }

            stream << "INSERT INTO " << table << " (";
            stream << columns.join(", ");
            stream << ") VALUES (";
            stream << values.join(", ");
            stream << ");\n";
        }
    }

    file.close();
    qDebug() << "Backup completed to:" << filename;
    return true;
}

bool BaseModel::executeSqlFromStream(QTextStream &stream)
{
    QString currentStatement;

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        if (line.startsWith("--") || line.startsWith("/*") || line.isEmpty()) {
            continue;
        }

        currentStatement += line + " ";

        if (line.endsWith(";")) {
            currentStatement = currentStatement.trimmed();
            
            qDebug() << "Executing SQL:" << currentStatement;

            if (!executeNonQuerySimple(currentStatement)) {
                qDebug() << "Failed to execute SQL:" << currentStatement;
                return false;
            }

            currentStatement.clear();
        }
    }

    return true;
}

void BaseModel::logTableStatistics(const QList<QString> &tables)
{
    qDebug() << "Restore completed, checking data...";

    for (const QString &table : tables) {
        QString sql = QString("SELECT COUNT(*) FROM %1").arg(table);
        QVariant count = executeScalarSimple(sql);
        qDebug() << "Table" << table << "has" << count.toInt() << "records";
    }
}

bool BaseModel::restoreDatabase(const QString &filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open backup file:" << filename;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    QList<QString> tables = getAllTableNames();

    // Очищаем таблицы в правильном порядке (сначала зависимые)
    // Очищаем incident_history
    if (tables.contains("incident_history")) {
        if (!clearTable("incident_history")) {
            file.close();
            return false;
        }
    }
    
    // Очищаем остальные таблицы
    for (const QString &table : tables) {
        if (table == "incident_history") continue;
        
        if (!clearTable(table)) {
            file.close();
            return false;
        }
    }

    // Выполняем SQL из файла
    if (!executeSqlFromStream(stream)) {
        file.close();
        return false;
    }

    file.close();
    logTableStatistics(tables);
    
    return true;
}

QList<QString> BaseModel::getAllTableNames()
{
    QList<QString> tables;

    QString sql = 
        "SELECT table_name FROM information_schema.tables "
        "WHERE table_schema = 'public' AND table_type = 'BASE TABLE' "
        "ORDER BY table_name";
    
    QList<QHash<QString, QVariant>> results = executeQuerySimple(sql);

    for (const auto &row : results) {
        QString tableName = row.value("table_name").toString();  // ИСПРАВЛЕНО: table_name вместо tablename
        if (!tableName.isEmpty()) {
            tables.append(tableName);
        }
    }

    qDebug() << "Found tables in database:" << tables;
    return tables;
}
