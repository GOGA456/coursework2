#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include <QWidget>
#include <QDialog>

class QLineEdit;
class QComboBox;
class QTableView;
class QStandardItemModel;
class User;

class UserEditDialog : public QDialog {
    Q_OBJECT
    
public:
    UserEditDialog(const QString &username, QWidget *parent = nullptr);
    ~UserEditDialog();
    
private slots:
    void onSave();
    
private:
    void loadUserData();
    
    QString m_username;
    User *user;
    
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QLineEdit *fullNameEdit;
    QComboBox *roleCombo;
    QComboBox *statusCombo;
};

class UserManagementPanel : public QWidget {
    Q_OBJECT
    
public:
    UserManagementPanel(QWidget *parent = nullptr);
    ~UserManagementPanel();
    
private slots:
    void onAddUser();
    void onEditUser();
    void onDeleteUser();
    void onRefresh();
    void onSearch();
    
private:
    void setupUI();
    void loadUsers();
    
    QTableView *tableView;
    QStandardItemModel *model;
    QLineEdit *searchEdit;
    QList<User*> users;
};

#endif 
