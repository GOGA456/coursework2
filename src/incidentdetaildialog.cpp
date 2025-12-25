#include "incidentdetaildialog.h"
#include "orm/Incident.h"
#include "orm/User.h"
#include "incidenthistorydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

IncidentDetailDialog::IncidentDetailDialog(const QString &incidentIdentifier, bool isAdmin,
                                           const QString &userRole, 
                                           const QString &currentFullName,
                                           QWidget *parent)
    : QDialog(parent), incidentIdentifier(incidentIdentifier), isAdmin(isAdmin),
      userRole(userRole), fullName(currentFullName) {
    
    setWindowTitle(incidentIdentifier.isEmpty() ? "Новый инцидент" : "Редактирование инцидента");
    setMinimumSize(600, 500);
    
    setupUI();
    
    if (!incidentIdentifier.isEmpty()) {
        loadIncidentData();
    }
}

IncidentDetailDialog::~IncidentDetailDialog() {
}

void IncidentDetailDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    titleEdit = new QLineEdit();
    titleEdit->setPlaceholderText("Краткое описание инцидента");
    
    descriptionEdit = new QTextEdit();
    descriptionEdit->setPlaceholderText("Подробное описание...");
    
    severityCombo = new QComboBox();
    severityCombo->addItems(QStringList() << "Низкий" << "Средний" << "Высокий" << "Критический");
    
    statusCombo = new QComboBox();
    statusCombo->addItems(QStringList() << "Новый" << "Расследуется" << "Локализован" << "Решен" << "Закрыт");
    
    reporterEdit = new QLineEdit();
    reporterEdit->setText(fullName);
    reporterEdit->setReadOnly(true);
    
    assignedCombo = new QComboBox();
    assignedCombo->addItem("Не назначен", "");
    

    QList<User*> users = User::findAll("is_active = true");
    for (User *user : users) {
        assignedCombo->addItem(user->getFullName(), user->getFullName());
    }
    
    tagsEdit = new QLineEdit();
    tagsEdit->setPlaceholderText("через запятую: phishing, malware, ddos");
    
    resolutionEdit = new QTextEdit();
    resolutionEdit->setPlaceholderText("Описание решения...");
    
    formLayout->addRow("Заголовок *:", titleEdit);
    formLayout->addRow("Описание:", descriptionEdit);
    formLayout->addRow("Уровень угрозы:", severityCombo);
    formLayout->addRow("Статус:", statusCombo);
    formLayout->addRow("Сообщил:", reporterEdit);
    formLayout->addRow("Назначить:", assignedCombo);
    formLayout->addRow("Теги:", tagsEdit);
    formLayout->addRow("Решение:", resolutionEdit);
    
    mainLayout->addLayout(formLayout);
    
    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    saveButton = new QPushButton("Сохранить");
    cancelButton = new QPushButton("Отмена");
    historyButton = new QPushButton("История изменений");
    
    buttonLayout->addWidget(historyButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    

    historyButton->setVisible(!incidentIdentifier.isEmpty());
    

    if (userRole == "auditor") {
        titleEdit->setReadOnly(true);
        descriptionEdit->setReadOnly(true);
        severityCombo->setEnabled(false);
        statusCombo->setEnabled(false);
        reporterEdit->setReadOnly(true);
        assignedCombo->setEnabled(false);
        tagsEdit->setReadOnly(true);
        resolutionEdit->setReadOnly(true);
        saveButton->setEnabled(false);
        saveButton->setText("Только просмотр");
        
      
        historyButton->setEnabled(true);
    }
 
    else if (userRole == "user") {
       
        statusCombo->setEnabled(false);
        assignedCombo->setEnabled(false);
        
       
        if (!incidentIdentifier.isEmpty()) {
          
            Incident* incident = Incident::findOneByTitle(incidentIdentifier);
            if (incident) {
                if (incident->getReporter() != fullName) {
              
                    titleEdit->setReadOnly(true);
                    descriptionEdit->setReadOnly(true);
                    severityCombo->setEnabled(false);
                    tagsEdit->setReadOnly(true);
                    resolutionEdit->setReadOnly(true);
                    saveButton->setEnabled(false);
                    saveButton->setText("Только просмотр (не ваш инцидент)");
                }
                delete incident;
            }
        }
    }
 
    
    connect(saveButton, &QPushButton::clicked, this, &IncidentDetailDialog::onSave);
    connect(cancelButton, &QPushButton::clicked, this, &IncidentDetailDialog::onCancel);
    connect(historyButton, &QPushButton::clicked, this, &IncidentDetailDialog::onShowHistory);
}

void IncidentDetailDialog::loadIncidentData() {
    if (incidentIdentifier.isEmpty()) return;

    QString title = incidentIdentifier;

    qDebug() << "Loading incident data for title:" << title;

    Incident* incident = Incident::findOneByTitle(title);
    
    if (!incident) {
        QMessageBox::critical(this, "Ошибка", "Инцидент не найден!\nЗаголовок: " + title);
        reject();
        return;
    }

    titleEdit->setText(incident->getTitle());
    descriptionEdit->setText(incident->getDescription());
    
    QString severity = incident->getSeverity();
    if (severity == "low") severityCombo->setCurrentIndex(0);
    else if (severity == "medium") severityCombo->setCurrentIndex(1);
    else if (severity == "high") severityCombo->setCurrentIndex(2);
    else if (severity == "critical") severityCombo->setCurrentIndex(3);
    
    QString status = incident->getStatus();
    if (status == "new") statusCombo->setCurrentIndex(0);
    else if (status == "investigating") statusCombo->setCurrentIndex(1);
    else if (status == "contained") statusCombo->setCurrentIndex(2);
    else if (status == "resolved") statusCombo->setCurrentIndex(3);
    else if (status == "closed") statusCombo->setCurrentIndex(4);

    reporterEdit->setText(incident->getReporter());
    
    QString assigned = incident->getAssignedTo();
    for (int i = 0; i < assignedCombo->count(); i++) {
        if (assignedCombo->itemData(i).toString() == assigned) {
            assignedCombo->setCurrentIndex(i);
            break;
        }
    }

    tagsEdit->setText(incident->getTags());
    resolutionEdit->setText(incident->getResolution());

    delete incident;
}

void IncidentDetailDialog::onSave() {

    if (userRole == "auditor") {
        QMessageBox::warning(this, "Доступ запрещен", 
            "Аудиторы могут только просматривать инциденты и историю изменений!");
        return;
    }
    

    if (userRole == "user" && !incidentIdentifier.isEmpty()) {
        Incident* incident = Incident::findOneByTitle(incidentIdentifier);
        if (incident) {
            if (incident->getReporter() != fullName) {
                QMessageBox::warning(this, "Доступ запрещен", "Вы можете редактировать только свои инциденты!");
                delete incident;
                return;
            }
            delete incident;
        }
    }
    
    QString title = titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заголовок обязателен для заполнения!");
        titleEdit->setFocus();
        return;
    }
    
    Incident incident;
 
    if (!incidentIdentifier.isEmpty()) {

        Incident* existingIncident = Incident::findOneByTitle(incidentIdentifier);
        
        if (existingIncident) {
            incident.setTitle(title);
            incident.setDescription(descriptionEdit->toPlainText());
            
            QString severityText = severityCombo->currentText();
            QString severity;
            if (severityText == "Низкий") severity = "low";
            else if (severityText == "Средний") severity = "medium";
            else if (severityText == "Высокий") severity = "high";
            else if (severityText == "Критический") severity = "critical";
            else severity = "medium";
            incident.setSeverity(severity);
            
            // Пользователи не могут менять статус
            QString statusText = statusCombo->currentText();
            QString status;
            if (userRole == "admin") {
                // Админы могут менять статус
                if (statusText == "Новый") status = "new";
                else if (statusText == "Расследуется") status = "investigating";
                else if (statusText == "Локализован") status = "contained";
                else if (statusText == "Решен") status = "resolved";
                else if (statusText == "Закрыт") status = "closed";
                else status = "new";
            } else {
        
                status = existingIncident->getStatus();
            }
            incident.setStatus(status);
            
            incident.setReporter(reporterEdit->text());
            
       
            if (userRole == "admin") {
                incident.setAssignedTo(assignedCombo->currentData().toString());
            } else {
                incident.setAssignedTo(existingIncident->getAssignedTo());
            }
            
            incident.setTags(tagsEdit->text());
            incident.setResolution(resolutionEdit->toPlainText());
            incident.setCreatedDate(existingIncident->getCreatedDate());
            
         
            incident.setCurrentUser(fullName);
            
            qDebug() << "Editing existing incident:" << incidentIdentifier;
            
            delete existingIncident;
        } else {
            QMessageBox::critical(this, "Ошибка", "Инцидент не найден!");
            return;
        }
    } else {
      
        incident.setTitle(title);
        incident.setDescription(descriptionEdit->toPlainText());
        
        QString severityText = severityCombo->currentText();
        QString severity;
        if (severityText == "Низкий") severity = "low";
        else if (severityText == "Средний") severity = "medium";
        else if (severityText == "Высокий") severity = "high";
        else if (severityText == "Критический") severity = "critical";
        else severity = "medium";
        incident.setSeverity(severity);
        
        incident.setStatus("new"); 
        incident.setReporter(reporterEdit->text());
        incident.setAssignedTo(assignedCombo->currentData().toString());
        incident.setTags(tagsEdit->text());
        incident.setResolution(resolutionEdit->toPlainText());
        incident.setCurrentUser(fullName);
        
        qDebug() << "Creating new incident";
    }
    
    if (incident.save()) {
        QMessageBox::information(this, "Успех", 
            incidentIdentifier.isEmpty() ? "Инцидент создан!" : "Инцидент обновлен!");
        accept();
    } else {
        if (incidentIdentifier.isEmpty()) {
            QMessageBox::critical(this, "Ошибка", 
                "Ошибка создания инцидента!\nВозможно, инцидент с таким заголовком уже существует.");
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка обновления инцидента!");
        }
    }
}

void IncidentDetailDialog::onCancel() {
    reject();
}

void IncidentDetailDialog::onShowHistory() {
    if (incidentIdentifier.isEmpty()) return;
    
    QString title = incidentIdentifier;
    
    Incident* incident = Incident::findOneByTitle(title);
    
    if (incident) {
        QString displayTitle = incident->getTitle();
        delete incident;
        
        IncidentHistoryDialog historyDialog(title, this);
        historyDialog.exec();
    } else {
        QMessageBox::warning(this, "Ошибка", "Инцидент не найден!");
    }
}
