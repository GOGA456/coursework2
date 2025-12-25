#ifndef INCIDENT_H
#define INCIDENT_H

#include "../../include/orm/BaseModel.h"
#include <QObject>
#include <QDateTime>
#include <QString>
#include <QList>

class Incident : public BaseModel {
    Q_OBJECT

public:
    explicit Incident(QObject *parent = nullptr);
    

    QString getTitle() const { return title; }
    QString getDescription() const { return description; }
    QString getSeverity() const { return severity; }
    QString getStatus() const { return status; }
    QString getReporter() const { return reporter; }
    QString getAssignedTo() const { return assignedTo; }
    QString getTags() const { return tags; }
    QString getResolution() const { return resolution; }
    QDateTime getCreatedDate() const { return createdDate; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getUpdatedAt() const { return updatedAt; }  
    

    void setTitle(const QString &newTitle) { title = newTitle; }
    void setDescription(const QString &newDescription) { description = newDescription; }
    void setSeverity(const QString &newSeverity) { severity = newSeverity; }
    void setStatus(const QString &newStatus) { status = newStatus; }
    void setReporter(const QString &newReporter) { reporter = newReporter; }
    void setAssignedTo(const QString &newAssignedTo) { assignedTo = newAssignedTo; }
    void setTags(const QString &newTags) { tags = newTags; }
    void setResolution(const QString &newResolution) { resolution = newResolution; }
    void setCreatedDate(const QDateTime &newDate) { createdDate = newDate; }
    void setCreatedAt(const QDateTime &newDate) { createdAt = newDate; }
    void setUpdatedAt(const QDateTime &newDate) { updatedAt = newDate; }
    void setCurrentUser(const QString &user) { currentUser = user; }
    
    
    bool updateStatus(const QString &newStatus, const QString &changedBy);
    bool save();
    bool remove();
    
    QHash<QString, QVariant> toHash() const;
    void fromHash(const QHash<QString, QVariant> &hash);
    QStringList fields() const;
    
    
    static QList<Incident*> findAll(const QString &condition = "");
    static QList<Incident*> search(const QString &text);
    static QList<Incident*> findByDateRange(const QDate &fromDate, const QDate &toDate);
    static QList<Incident*> findRecent(int limit);
    static int countActive();
    static int countToday();
    static int countBySeverity(const QString &severity);
    static int countTotal();
    static QList<Incident*> findByStatus(const QString &status);
    static QList<Incident*> findBySeverity(const QString &severity);
    static QList<Incident*> findByAssignedTo(const QString &assignedTo);
    static QList<Incident*> findByTitle(const QString &title);
    static Incident* findOneByTitle(const QString &title);
    static QList<Incident*> findByTitleAndReporter(const QString &title, const QString &reporter);
    static QList<Incident*> findByTitleAndDate(const QString &title, const QDateTime &date);
    
    static void logChange(const QString &title, const QString &field, 
                         const QString &oldValue, const QString &newValue, 
                         const QString &changedBy);

private:
    QString title;
    QString description;
    QString severity;
    QString status;
    QString reporter;
    QString assignedTo;
    QString tags;
    QString resolution;
    QDateTime createdDate;
    QDateTime createdAt;
    QDateTime updatedAt;  
    QString currentUser;
};

#endif 
