#ifndef USER_H
#define USER_H

#include "../../include/orm/BaseModel.h"
#include <QObject>
#include <QVariant>
#include <QDateTime>
#include <QList>
#include <QString>

class User : public BaseModel {
    Q_OBJECT

public:
    explicit User(QObject *parent = nullptr);
    
    QHash<QString, QVariant> toHash() const;
    void fromHash(const QHash<QString, QVariant> &hash);
    QStringList fields() const;
    
    bool save();
    bool remove();
    
    static QList<User*> findAll(const QString &condition = "");
    static User* findByUsername(const QString &username);
    static User* findByFullName(const QString &fullName);
    static QList<User*> findActiveUsers();
    static QList<User*> findByRole(const QString &role);
    static bool userExists(const QString &username);
    static int count();
    static int countActive();
    
    void setPassword(const QString &password);
    bool checkPassword(const QString &username, const QString &password);
    static bool deleteAll();
    static bool deactivate(const QString &username);
    static bool activate(const QString &username);
    static bool updateRole(const QString &username, const QString &role);
    static QString getUserNameByFullName(const QString &fullName);
    
    
    QString getUsername() const { return username; }
    QString getFullName() const { return fullName; }
    QString getRole() const { return role; }
    bool getIsActive() const { return isActive; }

    
    void setUsername(const QString &newUsername) { username = newUsername; }
    void setFullName(const QString &newFullName) { fullName = newFullName; }
    void setRole(const QString &newRole) { role = newRole; }
    void setIsActive(bool active) { isActive = active; }

private:
    QString username;
    QString passwordHash;
    QString fullName;
    QString role;
    bool isActive;
    QDateTime createdAt;
};

#endif
