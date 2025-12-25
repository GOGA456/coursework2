#ifndef INCIDENT_HISTORY_H
#define INCIDENT_HISTORY_H

#include "../../include/orm/BaseModel.h"
#include <QObject>
#include <QVariant>
#include <QDateTime>
#include <QList>
#include <QString>

class IncidentHistory : public BaseModel {
    Q_OBJECT

public:
    explicit IncidentHistory(QObject *parent = nullptr);
    
    QString getIncidentTitle() const { return incidentTitle; }
    QString getIncidentReporter() const { return incidentReporter; }
    QDateTime getIncidentDate() const { return incidentDate; }
    QString getChangedField() const { return changedField; }
    QString getOldValue() const { return oldValue; }
    QString getNewValue() const { return newValue; }
    QString getChangedBy() const { return changedBy; }
    QDateTime getChangeDate() const { return changeDate; }
    
    void setIncidentTitle(const QString &title) { incidentTitle = title; }
    void setIncidentReporter(const QString &reporter) { incidentReporter = reporter; }
    void setIncidentDate(const QDateTime &date) { incidentDate = date; }
    void setChangedField(const QString &field) { changedField = field; }
    void setOldValue(const QString &value) { oldValue = value; }
    void setNewValue(const QString &value) { newValue = value; }
    void setChangedBy(const QString &by) { changedBy = by; }
    void setChangeDate(const QDateTime &date) { changeDate = date; }
    
    QHash<QString, QVariant> toHash() const;
    void fromHash(const QHash<QString, QVariant> &hash);
    QStringList fields() const;
    
    bool save();
    static QList<IncidentHistory*> findAll(const QString &condition = "");
    static QList<IncidentHistory*> findByIncident(const QString &title);
    static QList<IncidentHistory*> findByUser(const QString &username);
    static int count();
    static bool clearHistory(const QString &title, const QString &reporter, const QDateTime &date);
    static void logChange(const QString &title, const QString &field, const QString &oldValue, 
                          const QString &newValue, const QString &changedBy);
    
    
    static QDateTime findStatusChangeDate(const QString &incidentTitle, const QString &toStatus);

private:
    QString incidentTitle;
    QString incidentReporter;
    QDateTime incidentDate;
    QString changedField;
    QString oldValue;
    QString newValue;
    QString changedBy;
    QDateTime changeDate;
};

#endif 
