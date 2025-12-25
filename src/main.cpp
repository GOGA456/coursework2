#include "../include/mainwindow.h"
#include "../include/logindialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QTextCodec>

bool setupDatabase() {

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    

    db.setHostName("localhost");
    db.setDatabaseName("ib_incidents_db");
    db.setUserName("ib_user");
    db.setPassword("ib123456"); 
    
    if (!db.open()) {
        qDebug() << "Cannot open PostgreSQL database:" << db.lastError().text();
        QMessageBox::critical(nullptr, "Ошибка подключения", 
            QString("Не удалось подключиться к PostgreSQL:\n%1\n\n"
                   "Проверьте:\n"
                   "1. Запущен ли PostgreSQL: sudo systemctl status postgresql\n"
                   "2. Существует ли БД: sudo -u postgres psql -c '\\l'\n"
                   "3. Пароль PostgreSQL в коде").arg(db.lastError().text()));
        return false;
    }
    
    qDebug() << "PostgreSQL database opened successfully";
    

    QSqlQuery query;
    

    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "username VARCHAR(100) PRIMARY KEY,"
               "password_hash VARCHAR(255) NOT NULL,"
               "full_name VARCHAR(255) NOT NULL,"
               "role VARCHAR(50),"
               "is_active BOOLEAN DEFAULT true,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
    

    query.exec("CREATE TABLE IF NOT EXISTS incidents ("
               "title VARCHAR(255) NOT NULL,"
               "reporter VARCHAR(255) NOT NULL,"
               "created_date TIMESTAMP NOT NULL,"
               "description TEXT,"
               "severity VARCHAR(50),"
               "status VARCHAR(50),"
               "assigned_to VARCHAR(255),"
               "tags TEXT,"
               "resolution TEXT,"
               "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "PRIMARY KEY (title, reporter, created_date))");
    

    query.exec("CREATE TABLE IF NOT EXISTS incident_history ("
               "incident_title VARCHAR(255) NOT NULL,"
               "incident_reporter VARCHAR(255) NOT NULL,"
               "incident_date TIMESTAMP NOT NULL,"
               "changed_field VARCHAR(100),"
               "old_value TEXT,"
               "new_value TEXT,"
               "changed_by VARCHAR(255),"
               "change_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
               "FOREIGN KEY (incident_title, incident_reporter, incident_date) "
               "REFERENCES incidents(title, reporter, created_date))");
    
    return true;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
    if (!setupDatabase()) {
        return 1;
    }
    
    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }
    
    MainWindow mainWindow(
        loginDialog.getUsername(),
        loginDialog.getFullName(),
        loginDialog.getUserRole()
    );
    
    mainWindow.show();
    return app.exec();
}
