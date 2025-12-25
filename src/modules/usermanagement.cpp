#include "../include/modules/usermanagement.h"
#include "../include/orm/User.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QDebug>

UserEditDialog::UserEditDialog(const QString &username, QWidget *parent)
    : QDialog(parent), m_username(username), user(nullptr) {
    
    setWindowTitle(m_username.isEmpty() ? "Новый пользователь" : "Редактирование пользователя");
    setMinimumSize(400, 300);
    
    QFormLayout *layout = new QFormLayout(this);
    
    usernameEdit = new QLineEdit();
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    fullNameEdit = new QLineEdit();
    
    roleCombo = new QComboBox();
    roleCombo->addItems(QStringList() << "user" << "admin" << "auditor");
    
    statusCombo = new QComboBox();
    statusCombo->addItems(QStringList() << "Активен" << "Заблокирован");
    
    layout->addRow("Логин *:", usernameEdit);
    layout->addRow("Пароль:", passwordEdit);
    layout->addRow("ФИО *:", fullNameEdit);
    layout->addRow("Роль:", roleCombo);
    layout->addRow("Статус:", statusCombo);
    
    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addRow(buttons);
    
    connect(buttons, &QDialogButtonBox::accepted, this, &UserEditDialog::onSave);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    if (!m_username.isEmpty()) {
        loadUserData();
    }
}

UserEditDialog::~UserEditDialog() {
    delete user;
}

void UserEditDialog::loadUserData() {
    user = User::findByUsername(m_username);
    if (user) {
        usernameEdit->setText(user->getUsername());
        fullNameEdit->setText(user->getFullName());
        roleCombo->setCurrentText(user->getRole());
        statusCombo->setCurrentIndex(user->getIsActive() ? 0 : 1);
        qDebug() << "UserEditDialog::loadUserData() loaded user:" << user->getUsername();
    } else {
        QMessageBox::warning(this, "Ошибка", "Пользователь не найден!");
        reject();
    }
}

void UserEditDialog::onSave() {
    qDebug() << "UserEditDialog::onSave() username:" << m_username;
    
    QString username = usernameEdit->text().trimmed();
    QString fullName = fullNameEdit->text().trimmed();
    
    if (username.isEmpty() || fullName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните обязательные поля!");
        return;
    }
    
    if (m_username.isEmpty()) {
      
        QString password = passwordEdit->text();
        if (password.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите пароль для нового пользователя!");
            return;
        }
        
        if (password.length() < 6) {
            QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов!");
            passwordEdit->clear();
            passwordEdit->setFocus();
            return;
        }
        
        User* existingUser = User::findByUsername(username);
        if (existingUser) {
            QMessageBox::warning(this, "Ошибка", QString("Пользователь с логином '%1' уже существует!").arg(username));
            delete existingUser;
            usernameEdit->setFocus();
            usernameEdit->selectAll();
            return;
        }
        
        User newUser;
        newUser.setUsername(username);
        try {
            newUser.setPassword(password);
        } catch (const std::runtime_error &e) {
            QMessageBox::warning(this, "Ошибка", e.what());
            return;
        }
        newUser.setFullName(fullName);
        newUser.setRole(roleCombo->currentText());
        newUser.setIsActive(statusCombo->currentIndex() == 0);
        
        if (newUser.save()) {
            QMessageBox::information(this, "Успех", "Пользователь создан!");
            accept();
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка сохранения пользователя!");
        }
    } else {
        // Редактирование существующего
        if (!user) {
            qDebug() << "User object is null, reloading...";
            user = User::findByUsername(m_username);
            if (!user) {
                QMessageBox::critical(this, "Ошибка", "Пользователь не найден!");
                return;
            }
        }
        
        qDebug() << "Current user:" << user->getUsername();
        
        QString oldUsername = user->getUsername();
        if (username != oldUsername) {
            User* existingUser = User::findByUsername(username);
            if (existingUser) {
                QMessageBox::warning(this, "Ошибка", QString("Пользователь с логином '%1' уже существует!").arg(username));
                delete existingUser;
                usernameEdit->setText(oldUsername);
                usernameEdit->setFocus();
                usernameEdit->selectAll();
                return;
            }
        }
        
        user->setUsername(username);
        user->setFullName(fullName);
        user->setRole(roleCombo->currentText());
        user->setIsActive(statusCombo->currentIndex() == 0);
        
        QString password = passwordEdit->text();
        if (!password.isEmpty()) {
            if (password.length() < 6) {
                QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов!");
                passwordEdit->clear();
                passwordEdit->setFocus();
                return;
            }
            try {
                user->setPassword(password);
            } catch (const std::runtime_error &e) {
                QMessageBox::warning(this, "Ошибка", e.what());
                return;
            }
        }
        
        qDebug() << "Saving user:" << user->getUsername();
        
        if (user->save()) {
            QMessageBox::information(this, "Успех", "Данные пользователя обновлены!");
            accept();
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка обновления пользователя!");
        }
    }
}

UserManagementPanel::UserManagementPanel(QWidget *parent)
    : QWidget(parent), model(nullptr) {
    
    setupUI();
    loadUsers();
}

UserManagementPanel::~UserManagementPanel() {
    qDeleteAll(users);
}

void UserManagementPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QWidget *controlPanel = new QWidget();
    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);
    
    QPushButton *btnAdd = new QPushButton("Добавить");
    QPushButton *btnEdit = new QPushButton("Редактировать");
    QPushButton *btnDelete = new QPushButton("Удалить");
    QPushButton *btnRefresh = new QPushButton("Обновить");
    
    controlLayout->addWidget(btnAdd);
    controlLayout->addWidget(btnEdit);
    controlLayout->addWidget(btnDelete);
    controlLayout->addStretch();
    controlLayout->addWidget(btnRefresh);
    
    QWidget *searchPanel = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchPanel);
    
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск по логину или ФИО...");
    QPushButton *btnSearch = new QPushButton("Найти");
    QPushButton *btnClear = new QPushButton("Очистить");
    
    searchLayout->addWidget(new QLabel("Поиск:"));
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(btnSearch);
    searchLayout->addWidget(btnClear);
    
    tableView = new QTableView();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << "Логин" << "ФИО" << "Роль" << "Статус");
    
    tableView->setModel(model);
    
    tableView->setColumnWidth(0, 150);
    tableView->setColumnWidth(1, 250);
    tableView->setColumnWidth(2, 100);
    tableView->setColumnWidth(3, 100);
    
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(searchPanel);
    mainLayout->addWidget(tableView, 1);
    
    connect(btnAdd, &QPushButton::clicked, this, &UserManagementPanel::onAddUser);
    connect(btnEdit, &QPushButton::clicked, this, &UserManagementPanel::onEditUser);
    connect(btnDelete, &QPushButton::clicked, this, &UserManagementPanel::onDeleteUser);
    connect(btnRefresh, &QPushButton::clicked, this, &UserManagementPanel::onRefresh);
    connect(btnSearch, &QPushButton::clicked, this, &UserManagementPanel::onSearch);
    connect(btnClear, &QPushButton::clicked, [this]() {
        searchEdit->clear();
        loadUsers();
    });
}

void UserManagementPanel::loadUsers() {
    qDeleteAll(users);
    users.clear();
    if (model) {
        model->removeRows(0, model->rowCount());
    }
    
    users = User::findAll();
    
    for (User *user : users) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(user->getUsername()));
        row.append(new QStandardItem(user->getFullName()));
        row.append(new QStandardItem(user->getRole()));
        row.append(new QStandardItem(user->getIsActive() ? "Активен" : "Заблокирован"));
        model->appendRow(row);
    }
}

void UserManagementPanel::onAddUser() {
    UserEditDialog dialog("", this);
    if (dialog.exec() == QDialog::Accepted) {
        loadUsers();
    }
}

void UserManagementPanel::onEditUser() {
    QModelIndexList selection = tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите пользователя для редактирования!");
        return;
    }
    
    int row = selection.first().row();
    QString username = model->item(row, 0)->text();
    
    UserEditDialog dialog(username, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadUsers();
    }
}

void UserManagementPanel::onDeleteUser() {
    QModelIndexList selection = tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите пользователя для удаления!");
        return;
    }
    
    int row = selection.first().row();
    QString username = model->item(row, 0)->text();
    QString fullName = model->item(row, 1)->text();
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", 
                                 QString("Удалить пользователя:\n%1 (%2)?").arg(fullName).arg(username),
                                 QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        User *user = User::findByUsername(username);
        if (user && user->remove()) {
            delete user;
            QMessageBox::information(this, "Успех", "Пользователь удален!");
            loadUsers();
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка удаления пользователя!");
        }
    }
}

void UserManagementPanel::onRefresh() {
    loadUsers();
}

void UserManagementPanel::onSearch() {
    QString searchText = searchEdit->text().trimmed();
    if (!searchText.isEmpty()) {
        qDeleteAll(users);
        users.clear();
        model->removeRows(0, model->rowCount());
        
        QString condition = QString("username LIKE '%%1%' OR full_name LIKE '%%1%'")
            .arg(searchText);
        users = User::findAll(condition);
        
        for (User *user : users) {
            QList<QStandardItem*> row;
            row.append(new QStandardItem(user->getUsername()));
            row.append(new QStandardItem(user->getFullName()));
            row.append(new QStandardItem(user->getRole()));
            row.append(new QStandardItem(user->getIsActive() ? "Активен" : "Заблокирован"));
            model->appendRow(row);
        }
    } else {
        loadUsers();
    }
}
