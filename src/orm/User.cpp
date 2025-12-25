#include "../../include/orm/User.h"
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QDebug>

User::User(QObject *parent) : BaseModel(parent), isActive(true) { }

QHash<QString, QVariant> User::toHash() const {
    QHash<QString, QVariant> hash;
    hash["username"] = username;
    hash["password_hash"] = passwordHash;
    hash["full_name"] = fullName;
    hash["role"] = role;
    hash["is_active"] = isActive;
    hash["created_at"] = createdAt;
    return hash;
}

void User::fromHash(const QHash<QString, QVariant> &hash) {
    username = hash.value("username").toString();
    passwordHash = hash.value("password_hash").toString();
    fullName = hash.value("full_name").toString();
    role = hash.value("role", "user").toString();
    isActive = hash.value("is_active", true).toBool();
    createdAt = hash.value("created_at").toDateTime();
}

QStringList User::fields() const {
    return {"username", "password_hash", "full_name", "role", "is_active", "created_at"};
}

bool User::save() {
    QHash<QString, QVariant> data = toHash();
    
    QString checkSql = "SELECT COUNT(*) FROM users WHERE username = :username";
    QHash<QString, QVariant> checkParams;
    checkParams["username"] = username;

    QVariant exists = BaseModel::executeScalar(checkSql, checkParams);
    
    if (exists.toInt() > 0) {
        QString sql = "UPDATE users SET "
                      "password_hash = :password_hash, "
                      "full_name = :full_name, "
                      "role = :role, "
                      "is_active = :is_active "
                      "WHERE username = :username";
        
        return BaseModel::executeNonQuery(sql, data);
    } else {
        QString sql = "INSERT INTO users (username, password_hash, full_name, role, is_active, created_at) "
                      "VALUES (:username, :password_hash, :full_name, :role, :is_active, :created_at)";
        
        return BaseModel::executeNonQuery(sql, data);
    }
}

bool User::remove() {
    QString sql = "DELETE FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;

    return BaseModel::executeNonQuery(sql, params);
}

QList<User*> User::findAll(const QString &condition) {
    QList<User*> users;
    QString sql = "SELECT * FROM users";

    if (!condition.isEmpty()) {
        sql += " WHERE " + condition;
    }
    
    sql += " ORDER BY full_name";

    QList<QHash<QString, QVariant>> results = BaseModel::executeQuerySimple(sql);
    
    for (const auto &row : results) {
        User *user = new User();
        user->fromHash(row);
        users.append(user);
    }
    
    return users;
}

User* User::findByUsername(const QString &username) {
    QString sql = "SELECT * FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;

    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        User *user = new User();
        user->fromHash(results.first());
        return user;
    }
    
    return nullptr;
}

User* User::findByFullName(const QString &fullName) {
    QString sql = "SELECT * FROM users WHERE full_name = :full_name";
    QHash<QString, QVariant> params;
    params["full_name"] = fullName;

    QList<QHash<QString, QVariant>> results = BaseModel::executeQuery(sql, params);
    
    if (!results.isEmpty()) {
        User *user = new User();
        user->fromHash(results.first());
        return user;
    }
    
    return nullptr;
}

QList<User*> User::findActiveUsers() {
    return findAll("is_active = true");
}

QList<User*> User::findByRole(const QString &role) {
    return findAll("role = '" + role + "'");
}

bool User::userExists(const QString &username) {
    QString sql = "SELECT COUNT(*) FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;

    QVariant result = BaseModel::executeScalar(sql, params);
    return result.toInt() > 0;
}

int User::count() {
    QString sql = "SELECT COUNT(*) FROM users";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

int User::countActive() {
    QString sql = "SELECT COUNT(*) FROM users WHERE is_active = true";
    QVariant result = BaseModel::executeScalarSimple(sql);
    return result.toInt();
}

void User::setPassword(const QString &password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    passwordHash = hash.toHex();
}

bool User::checkPassword(const QString &username, const QString &password) {
    QString sql = "SELECT password_hash FROM users WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;

    QVariant result = BaseModel::executeScalar(sql, params);
    if (result.isNull()) {
        return false;
    }

    QString storedHash = result.toString();
    QByteArray inputHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString inputHashHex = QString(inputHash.toHex());

    return storedHash == inputHashHex;
}

bool User::deleteAll() {
    QString sql = "DELETE FROM users";
    return BaseModel::executeNonQuerySimple(sql);
}

bool User::deactivate(const QString &username) {
    QString sql = "UPDATE users SET is_active = false WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;
    return BaseModel::executeNonQuery(sql, params);
}

bool User::activate(const QString &username) {
    QString sql = "UPDATE users SET is_active = true WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;
    return BaseModel::executeNonQuery(sql, params);
}

bool User::updateRole(const QString &username, const QString &role) {
    QString sql = "UPDATE users SET role = :role WHERE username = :username";
    QHash<QString, QVariant> params;
    params["username"] = username;
    params["role"] = role;
    return BaseModel::executeNonQuery(sql, params);
}

QString User::getUserNameByFullName(const QString &fullName) {
    if (fullName.isEmpty()) return "Не назначен";

    QString sql = "SELECT username FROM users WHERE full_name = :full_name";
    QHash<QString, QVariant> params;
    params["full_name"] = fullName;

    QVariant result = BaseModel::executeScalar(sql, params);
    if (result.isValid() && !result.isNull()) {
        return result.toString();
    }
    return "Не найдено";
}

