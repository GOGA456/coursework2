#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QList>
#include <QString>
#include <QSqlQuery>     
#include <QTextStream> 

class BaseModel : public QObject
{
    Q_OBJECT

public:
    explicit BaseModel(QObject *parent = nullptr);
    virtual ~BaseModel() = default;

    static QList<QHash<QString, QVariant>> executeQuery(const QString &sql, const QHash<QString, QVariant> &params);
    static QList<QHash<QString, QVariant>> executeQuerySimple(const QString &sql);
    
    static QVariant executeScalar(const QString &sql, const QHash<QString, QVariant> &params);
    static QVariant executeScalarSimple(const QString &sql);
    
    static bool executeNonQuery(const QString &sql, const QHash<QString, QVariant> &params);
    static bool executeNonQuerySimple(const QString &sql);
    
    static bool backupDatabase(const QString &filename);
    static bool restoreDatabase(const QString &filename);
    
    virtual bool remove() { return false; }

protected:
    static QList<QString> getAllTableNames();

private:
    static void bindParameters(QSqlQuery &query, const QHash<QString, QVariant> &params);
    static QHash<QString, QVariant> extractRowFromQuery(QSqlQuery &query);
    static bool clearTable(const QString &tableName);
    static bool executeSqlFromStream(QTextStream &stream);
    static void logTableStatistics(const QList<QString> &tables);
};

#endif
