#include "../include/logindialog.h"
#include "../include/orm/User.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent) {
    setupUI();
}

LoginDialog::~LoginDialog() {}

void LoginDialog::setupUI() {
    setWindowTitle("Вход в систему");
    setMinimumWidth(300);
    
    QFormLayout *formLayout = new QFormLayout(this);
    
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("admin");
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    
    formLayout->addRow("Логин:", usernameEdit);
    formLayout->addRow("Пароль:", passwordEdit);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    formLayout->addRow(buttonBox);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::onLogin);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    usernameEdit->setFocus();
}

void LoginDialog::onLogin() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }
    
    qDebug() << "=== Login attempt ===";
    qDebug() << "Username:" << username;
    
    User* user = User::findByUsername(username);
    
    if (user) {
        qDebug() << "User found. Username:" << user->getUsername() 
                 << "Active:" << user->getIsActive()
                 << "Role:" << user->getRole();
        
        if (!user->getIsActive()) {
            QMessageBox::warning(this, "Отказано", "Пользователь деактивирован!");
            delete user;
            return;
        }
        
        qDebug() << "Checking password...";
        if (user->checkPassword(username, password)) {
            qDebug() << "Password correct!";
            m_username = user->getUsername();
            m_fullName = user->getFullName();
            m_userRole = user->getRole();
            qDebug() << "Login successful! User:" << m_fullName << "Role:" << m_userRole;
            delete user;
            accept();
        } else {
            qDebug() << "Password incorrect!";
            QMessageBox::warning(this, "Ошибка", "Неверный пароль!");
            passwordEdit->clear();
            passwordEdit->setFocus();
            delete user;
        }
    } else {
        qDebug() << "User not found!";
        if (username.toLower() == "admin") {
            qDebug() << "Creating admin user...";
            
            User newUser;
            newUser.setUsername(username);
            newUser.setPassword(password);
            newUser.setFullName("Администратор");
            newUser.setRole("admin");
            newUser.setIsActive(true);
            
            if (newUser.save()) {
                m_username = newUser.getUsername();
                m_fullName = "Администратор";
                m_userRole = "admin";
                qDebug() << "Admin user created! Username:" << m_username;
                QMessageBox::information(this, "Успех", 
                    "Создан пользователь admin!\nДобро пожаловать!");
                accept();
            } else {
                qDebug() << "Failed to create admin user";
                QMessageBox::critical(this, "Ошибка", 
                    "Не удалось создать пользователя!");
            }
        } else {
            QMessageBox::warning(this, "Ошибка", 
                "Пользователь не найден!\nТолько admin может быть создан автоматически.");
        }
    }
}
