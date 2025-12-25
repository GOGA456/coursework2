#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;

class LoginDialog : public QDialog {
    Q_OBJECT
    
public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    
    QString getUsername() const { return m_username; }
    QString getFullName() const { return m_fullName; }
    QString getUserRole() const { return m_userRole; }
    
private slots:
    void onLogin();
    
private:
    void setupUI();
    
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    
    QString m_username;
    QString m_fullName;
    QString m_userRole;
};

#endif 
