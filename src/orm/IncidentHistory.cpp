#include "../../include/orm/IncidentHistory.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QDebug>

IncidentHistory::IncidentHistory(QObject *parent) : BaseModel(parent) {
    changeDate = QDateTime::currentDateTime();
}

QHash<QString, QVariant> IncidentHistory::toHash() const {
    QHash<QString, QVariant> hash;
    hash["incident_title"] = incidentTitle;
    hash["incident_reporter"] = incidentReporter;
    hash["incident_date"] = incidentDate;
    hash["changed_field"] = changedField;
    hash["old_value"] = oldValue;
    hash["new_value"] = newValue;
    hash["changed_by"] = changedBy;
    hash["change_date"] = changeDate;
    return hash;
}

void IncidentHistory::fromHash(const QHash<QString, QVariant> &hash) {
    incidentTitle = hash.value("incident_title").toString();
    incidentReporter = hash.value("incident_reporter").toString();
    incidentDate = hash.value("incident_date").toDateTime();
    changedField = hash.value("changed_field").toString();
    oldValue = hash.value("old_value").toString();
    newValue = hash.value("new_value").toString();
    changedBy = hash.value("changed_by").toString();
    changeDate = hash.value("change_date").toDateTime();
}

QStringList IncidentHistory::fields() const {
    return { "incident_title", "incident_reporter", "incident_date", 
             "changed_field", "old_value", "new_value", 
             "changed_by", "change_date" };
}

bool IncidentHistory::save() {
    QHash<QString, QVariant> data = toHash();
    
    qDebug() << "=== SAVING HISTORY ENTRY ===";
    qDebug() << "Incident:" << incidentTitle;
    qDebug() << "Reporter:" << incidentReporter;
    qDebug() << "Date:" << incidentDate.toString("yyyy-MM-dd HH:mm:ss");
    qDebug() << "Field:" << changedField;
    qDebug() << "Old value:" << oldValue;
    qDebug() << "New value:" << newValue;
    qDebug() << "Changed by:" << changedBy;

    QString sql = "INSERT INTO incident_history "
                  "(incident_title, incident_reporter, incident_date, "
                  "changed_field, old_value, new_value, changed_by, change_date) "
                  "VALUES (:incident_title, :incident_reporter, :incident_date, "
                  ":changed_field, :old_value, :new_value, :changed_by, :change_date)";
    
    bool result = BaseModel::executeNonQuery(sql, data);
    qDebug() << "Save result:" << (result ? "SUCCESS" : "FAILED");
    
    return result;
}

QList<IncidentHistory*> IncidentHistory::findAll(const QString &condition) {
    QList<IncidentHistory*> history;
    QString sql = "SELECT * FROM incident_history";

    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }
    sql += " ORDER BY change_date DESC";

    QList<QHash<QString, QVariant>> results = BaseModel::executeQuerySimple(sql);
    for (const auto &row : results) {
        IncidentHistory *item = new IncidentHistory();
        item->fromHash(row);
        history.append(item);
    }
    return history;
}

QList<IncidentHistory*> IncidentHistory::findByIncident(const QString &title) {
    qDebug() << "\n=== IncidentHistory::findByIncident ===";
    qDebug() << "Searching history for title:" << title;
    
    QString sql = "SELECT * FROM incident_history WHERE incident_title = :title ORDER BY change_date DESC";

    QHash<QString, QVariant> params;
    params["title"] = title;

    QList<IncidentHistory*> history;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    qDebug() << "Found" << results.size() << "history entries";
    
    for (const auto &row : results) {
        IncidentHistory *item = new IncidentHistory();
        item->fromHash(row);
        history.append(item);
        
        qDebug() << "History entry - Field:" << item->changedField
                 << "Old:" << item->oldValue
                 << "New:" << item->newValue
                 << "By:" << item->changedBy;
    }
    return history;
}

QList<IncidentHistory*> IncidentHistory::findByUser(const QString &username) {
    QString condition = QString("changed_by = '%1'").arg(username);
    return findAll(condition);
}

int IncidentHistory::count() {
    QString sql = "SELECT COUNT(*) FROM incident_history";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

bool IncidentHistory::clearHistory(const QString &title, const QString &reporter, const QDateTime &date) {
    QString sql = "DELETE FROM incident_history WHERE incident_title = :title AND incident_reporter = :reporter AND incident_date = :date";
    QHash<QString, QVariant> params;
    params["title"] = title;
    params["reporter"] = reporter;
    params["date"] = date;
    
    return BaseModel::executeNonQuery(sql, params);
}

void IncidentHistory::logChange(const QString &title, const QString &field, 
                               const QString &oldValue, const QString &newValue,
                               const QString &changedBy) {
    
    qDebug() << "\n=== IncidentHistory::logChange ===";
    qDebug() << "Title:" << title;
    qDebug() << "Field:" << field;
    qDebug() << "Old:" << oldValue;
    qDebug() << "New:" << newValue;
    qDebug() << "Changed by:" << changedBy;

    QString getIncidentSql = "SELECT reporter, created_date FROM incidents WHERE title = :title LIMIT 1";
    QHash<QString, QVariant> incidentParams;
    incidentParams["title"] = title;

    QList<QHash<QString, QVariant>> incidentResults = BaseModel::executeQuery(getIncidentSql, incidentParams);
    
    if (incidentResults.isEmpty()) {
        qDebug() << "ERROR: Could not find incident" << title << "in database!";
        return;
    }
    
    QString reporter = incidentResults.first().value("reporter").toString();
    QDateTime incidentDate = incidentResults.first().value("created_date").toDateTime();
    
    qDebug() << "Found incident - Reporter:" << reporter << "Date:" << incidentDate.toString("yyyy-MM-dd HH:mm:ss");
    
    IncidentHistory history;
    history.setIncidentTitle(title);
    history.setIncidentReporter(reporter);
    history.setIncidentDate(incidentDate);
    history.setChangedField(field);
    history.setOldValue(oldValue);
    history.setNewValue(newValue);
    history.setChangedBy(changedBy);
    history.setChangeDate(QDateTime::currentDateTime());
    
    bool saved = history.save();
    qDebug() << "History saved:" << (saved ? "SUCCESS" : "FAILED");
}

// ДОБАВЬТЕ ЭТОТ МЕТОД:
QDateTime IncidentHistory::findStatusChangeDate(const QString &incidentTitle, const QString &toStatus)
{
    // Создаем копии строк для экранирования
    QString safeTitle = incidentTitle;
    safeTitle.replace("'", "''");
    QString safeStatus = toStatus;
    safeStatus.replace("'", "''");
    
    // Ищем когда статус изменился НА указанный (toStatus)
    QString condition = QString("incident_title = '%1' AND changed_field = 'status' AND new_value = '%2'")
                        .arg(safeTitle)
                        .arg(safeStatus);
    
    QList<IncidentHistory*> history = findAll(condition);
    
    QDateTime latestDate;
    for (IncidentHistory* item : history) {
        QDateTime changeDate = item->getChangeDate();
        if (changeDate.isValid() && (latestDate.isNull() || changeDate > latestDate)) {
            latestDate = changeDate;
        }
        delete item;
    }
    
    return latestDate;
}
