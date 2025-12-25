#include "../../include/orm/Incident.h"
#include "../../include/orm/IncidentHistory.h"

Incident::Incident(QObject *parent) : BaseModel(parent) {
    status = "new";
    severity = "medium";
    createdDate = QDateTime::currentDateTime();
    createdAt = QDateTime::currentDateTime();
    updatedAt = QDateTime::currentDateTime();
    currentUser = "";
}

bool Incident::updateStatus(const QString &newStatus, const QString &changedBy) {
    QString oldStatus = status;
    status = newStatus;
    currentUser = changedBy;
    
    return save();
}

bool Incident::save() {
    QHash<QString, QVariant> data = toHash();
    updatedAt = QDateTime::currentDateTime();
    data["updated_at"] = updatedAt;

    QString checkSql = "SELECT COUNT(*) FROM incidents WHERE title = :title";
    QHash<QString, QVariant> checkParams;
    checkParams["title"] = title;

    QVariant exists = BaseModel::executeScalar(checkSql, checkParams);

    if (exists.toInt() > 0) {
        QString getOldSql = "SELECT * FROM incidents WHERE title = :title";
        QList<QHash<QString, QVariant>> oldResults = BaseModel::executeQuery(getOldSql, checkParams);

        if (!oldResults.isEmpty()) {
            QHash<QString, QVariant> oldRow = oldResults.first();
            QString oldSeverity = oldRow.value("severity").toString();
            QString oldStatus = oldRow.value("status").toString();
            QString oldAssignedTo = oldRow.value("assigned_to").toString();
            QString oldTags = oldRow.value("tags").toString();
            QString oldResolution = oldRow.value("resolution").toString();
            QDateTime oldCreatedDate = oldRow.value("created_date").toDateTime();
            QString oldReporter = oldRow.value("reporter").toString();

            QString sql = "UPDATE incidents SET "
                           "description = :description, "
                           "severity = :severity, "
                           "status = :status, "
                           "reporter = :reporter, "
                           "assigned_to = :assigned_to, "
                           "tags = :tags, "
                           "resolution = :resolution, "
                           "updated_at = :updated_at, "
                           "created_date = :created_date "
                           "WHERE title = :title";

            bool result = BaseModel::executeNonQuery(sql, data);

            if (result && !currentUser.isEmpty()) {
                if (oldSeverity != severity) {
                    Incident::logChange(title, "severity", oldSeverity, severity, currentUser);
                }
                if (oldStatus != status) {
                    Incident::logChange(title, "status", oldStatus, status, currentUser);
                }
                if (oldAssignedTo != assignedTo) {
                    Incident::logChange(title, "assigned_to", oldAssignedTo, assignedTo, currentUser);
                }
                if (oldTags != tags) {
                    Incident::logChange(title, "tags", oldTags, tags, currentUser);
                }
                if (oldResolution != resolution) {
                    Incident::logChange(title, "resolution", oldResolution, resolution, currentUser);
                }
                if (oldCreatedDate != createdDate) {
                    Incident::logChange(title, "created_date", 
                                        oldCreatedDate.toString("yyyy-MM-dd HH:mm:ss"), 
                                        createdDate.toString("yyyy-MM-dd HH:mm:ss"), 
                                        currentUser);
                }
                if (oldReporter != reporter) {
                    Incident::logChange(title, "reporter", oldReporter, reporter, currentUser);
                }
            }
            return result;
        }
    } else {
        if (createdAt.isNull() || !createdAt.isValid()) {
            createdAt = QDateTime::currentDateTime();
            data["created_at"] = createdAt;
        }

        QString sql = "INSERT INTO incidents (title, description, severity, status, reporter, "
                       "assigned_to, tags, resolution, created_date, created_at, updated_at) "
                       "VALUES (:title, :description, :severity, :status, :reporter, "
                       ":assigned_to, :tags, :resolution, :created_date, :created_at, :updated_at)";
        return BaseModel::executeNonQuery(sql, data);
    }
    
    return false; // Добавьте эту строку
}

bool Incident::remove() {
    if (title.isEmpty()) {
        return false;
    }

    QString sql = "DELETE FROM incidents WHERE title = :title";
    QHash<QString, QVariant> params;
    params["title"] = title;

    bool result = BaseModel::executeNonQuery(sql, params);

    if (result) {
        QString historySql = "DELETE FROM incident_history WHERE incident_title = :title";
        BaseModel::executeNonQuery(historySql, params);
    }
    
    return result;
}

QHash<QString, QVariant> Incident::toHash() const {
    QHash<QString, QVariant> hash;
    
    hash["title"] = title;
    hash["description"] = description;
    hash["severity"] = severity;
    hash["status"] = status;
    hash["reporter"] = reporter;
    hash["assigned_to"] = assignedTo;
    hash["tags"] = tags;
    hash["resolution"] = resolution;
    hash["created_date"] = createdDate;
    hash["created_at"] = createdAt;
    hash["updated_at"] = updatedAt;
    
    return hash;
}

void Incident::fromHash(const QHash<QString, QVariant> &hash) {
    title = hash.value("title").toString();
    description = hash.value("description").toString();
    severity = hash.value("severity", "medium").toString();
    status = hash.value("status", "new").toString();
    reporter = hash.value("reporter").toString();
    assignedTo = hash.value("assigned_to").toString();
    tags = hash.value("tags").toString();
    resolution = hash.value("resolution").toString();
    
    QDateTime dbDateTime = hash.value("created_date").toDateTime();
    createdDate = dbDateTime.isValid() ? dbDateTime : QDateTime::currentDateTime();
    
    createdAt = hash.value("created_at").toDateTime();
    updatedAt = hash.value("updated_at").toDateTime();
}

QStringList Incident::fields() const {
    return {"title", "description", "severity", "status", "reporter", 
            "assigned_to", "tags", "resolution", "created_date", 
            "created_at", "updated_at"};
}

QList<Incident*> Incident::findAll(const QString &condition) {
    QList<Incident*> incidents;
    QString sql = "SELECT * FROM incidents";
    
    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }
    
    sql += " ORDER BY created_date DESC, updated_at DESC";
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuerySimple(sql);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

QList<Incident*> Incident::search(const QString &text) {
    if (text.trimmed().isEmpty()) return findAll();
    
    QString sql = "SELECT * FROM incidents WHERE "
                  "title ILIKE :search OR " 
                  "description ILIKE :search OR "
                  "tags ILIKE :search "
                  "ORDER BY created_date DESC, updated_at DESC";
                  
    QHash<QString, QVariant> params;
    params["search"] = "%" + text + "%";
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

QList<Incident*> Incident::findByDateRange(const QDate &fromDate, const QDate &toDate) {
    QString sql = "SELECT * FROM incidents WHERE DATE(created_date) BETWEEN :frodate AND :to_date "
                  "ORDER BY created_date DESC";
                  
    QHash<QString, QVariant> params;
    params["frodate"] = fromDate.toString("yyyy-MM-dd");
    params["to_date"] = toDate.toString("yyyy-MM-dd");
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

QList<Incident*> Incident::findRecent(int limit) {
    QString sql = QString("SELECT * FROM incidents ORDER BY created_date DESC, updated_at DESC LIMIT %1").arg(limit);
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuerySimple(sql);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

int Incident::countActive() {
    QString sql = "SELECT COUNT(*) FROM incidents WHERE status NOT IN ('closed', 'resolved')";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int Incident::countToday() {
    QString sql = "SELECT COUNT(*) FROM incidents WHERE DATE(created_date) = CURRENT_DATE";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int Incident::countBySeverity(const QString &severity) {
    QString sql = "SELECT COUNT(*) FROM incidents WHERE severity = :severity";
    QHash<QString, QVariant> params;
    params["severity"] = severity;
    
    QVariant result = BaseModel::executeScalar(sql, params);
    return result.toInt();
}

int Incident::countTotal() {
    QString sql = "SELECT COUNT(*) FROM incidents";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

QList<Incident*> Incident::findByStatus(const QString &status) {
    return findAll("status = '" + status + "'");
}

QList<Incident*> Incident::findBySeverity(const QString &severity) {
    return findAll("severity = '" + severity + "'");
}

QList<Incident*> Incident::findByAssignedTo(const QString &assignedTo) {
    return findAll("assigned_to = '" + assignedTo + "'");
}

QList<Incident*> Incident::findByTitle(const QString &title) {
    QString sql = "SELECT * FROM incidents WHERE title = :title ORDER BY created_date DESC";
    QHash<QString, QVariant> params;
    params["title"] = title;
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

Incident* Incident::findOneByTitle(const QString &title) {
    QString sql = "SELECT * FROM incidents WHERE title = :title ORDER BY created_date DESC LIMIT 1";
    QHash<QString, QVariant> params;
    params["title"] = title;
    
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        Incident *incident = new Incident();
        incident->fromHash(results.first());
        return incident;
    }
    
    return nullptr;
}

QList<Incident*> Incident::findByTitleAndReporter(const QString &title, const QString &reporter) {
    QString sql = "SELECT * FROM incidents WHERE title = :title AND reporter = :reporter ORDER BY created_date DESC";
    QHash<QString, QVariant> params;
    params["title"] = title;
    params["reporter"] = reporter;
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

QList<Incident*> Incident::findByTitleAndDate(const QString &title, const QDateTime &date) {
    QString sql = "SELECT * FROM incidents WHERE title = :title AND created_date >= :date_start AND created_date <= :date_end ORDER BY created_date DESC";
    
    QDateTime dateStart = date.addSecs(-60);
    QDateTime dateEnd = date.addSecs(60);
    
    QHash<QString, QVariant> params;
    params["title"] = title;
    params["date_start"] = dateStart;
    params["date_end"] = dateEnd;
    
    QList<Incident*> incidents;
    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    for (const auto &row : results) {
        Incident *incident = new Incident();
        incident->fromHash(row);
        incidents.append(incident);
    }
    
    return incidents;
}

void Incident::logChange(const QString &title, const QString &field, const QString &oldValue, const QString &newValue, const QString &changedBy) {
    IncidentHistory::logChange(title, field, oldValue, newValue, changedBy);
}
