#include "../include/modules/filtermodule.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QDate>

AdvancedFilterPanel::AdvancedFilterPanel(QWidget *parent)
    : QWidget(parent) {
    
    setupUI();
}

void AdvancedFilterPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QGroupBox *mainFilterGroup = new QGroupBox("Основные фильтры");
    QFormLayout *mainLayoutForm = new QFormLayout(mainFilterGroup);
    
    titleFilter = new QLineEdit();
    titleFilter->setPlaceholderText("По заголовку...");
    
    descriptionFilter = new QLineEdit();
    descriptionFilter->setPlaceholderText("По описанию...");
    
    tagsFilter = new QLineEdit();
    tagsFilter->setPlaceholderText("По тегам...");
    
    reporterFilter = new QLineEdit();
    reporterFilter->setPlaceholderText("Кто сообщил...");
    
    assignedFilter = new QLineEdit();
    assignedFilter->setPlaceholderText("Кому назначен...");
    
    mainLayoutForm->addRow("Заголовок:", titleFilter);
    mainLayoutForm->addRow("Описание:", descriptionFilter);
    mainLayoutForm->addRow("Теги:", tagsFilter);
    mainLayoutForm->addRow("Сообщил:", reporterFilter);
    mainLayoutForm->addRow("Назначен:", assignedFilter);
    
    QGroupBox *advancedFilterGroup = new QGroupBox("Дополнительные фильтры");
    QFormLayout *advancedLayoutForm = new QFormLayout(advancedFilterGroup);
    
    severityFilter = new QComboBox();
    severityFilter->addItem("Все уровни", "");
    severityFilter->addItem("Низкий", "low");
    severityFilter->addItem("Средний", "medium");
    severityFilter->addItem("Высокий", "high");
    severityFilter->addItem("Критический", "critical");
    
    statusFilter = new QComboBox();
    statusFilter->addItem("Все статусы", "");
    statusFilter->addItem("Новый", "new");
    statusFilter->addItem("Расследуется", "investigating");
    statusFilter->addItem("Локализован", "contained");
    statusFilter->addItem("Решен", "resolved");
    statusFilter->addItem("Закрыт", "closed");
    
    dateFromFilter = new QDateEdit();
    dateFromFilter->setCalendarPopup(true);
    dateFromFilter->setDate(QDate::currentDate().addDays(-30));
    
    dateToFilter = new QDateEdit();
    dateToFilter->setCalendarPopup(true);
    dateToFilter->setDate(QDate::currentDate());
    
    showClosedCheck = new QCheckBox("Показывать закрытые");
    showResolvedCheck = new QCheckBox("Показывать решенные");
    showClosedCheck->setChecked(true);
    showResolvedCheck->setChecked(true);
    
    advancedLayoutForm->addRow("Уровень угрозы:", severityFilter);
    advancedLayoutForm->addRow("Статус:", statusFilter);
    advancedLayoutForm->addRow("С даты:", dateFromFilter);
    advancedLayoutForm->addRow("По дату:", dateToFilter);
    advancedLayoutForm->addRow("", showClosedCheck);
    advancedLayoutForm->addRow("", showResolvedCheck);
    
    QWidget *buttonPanel = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonPanel);
    
    applyButton = new QPushButton("Применить фильтры");
    resetButton = new QPushButton("Сбросить фильтры");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addWidget(mainFilterGroup);
    mainLayout->addWidget(advancedFilterGroup);
    mainLayout->addWidget(buttonPanel);
    
    connect(applyButton, &QPushButton::clicked, this, &AdvancedFilterPanel::onApplyFilter);
    connect(resetButton, &QPushButton::clicked, this, &AdvancedFilterPanel::onResetFilter);
}

QString AdvancedFilterPanel::getFilterString() const {
    QStringList filters;
    
    if (!titleFilter->text().isEmpty()) {
        filters << QString("title ILIKE '%%1%'").arg(titleFilter->text());
    }
    if (!descriptionFilter->text().isEmpty()) {
        filters << QString("description ILIKE '%%1%'").arg(descriptionFilter->text());
    }
    if (!tagsFilter->text().isEmpty()) {
        filters << QString("tags ILIKE '%%1%'").arg(tagsFilter->text());
    }
    if (!reporterFilter->text().isEmpty()) {
        filters << QString("reporter ILIKE '%%1%'").arg(reporterFilter->text());
    }
    if (!assignedFilter->text().isEmpty()) {
        filters << QString("assigned_to ILIKE '%%1%'").arg(assignedFilter->text());
    }
    
    QString severity = severityFilter->currentData().toString();
    if (!severity.isEmpty()) {
        filters << QString("severity = '%1'").arg(severity);
    }
    
    QString status = statusFilter->currentData().toString();
    if (!status.isEmpty()) {
        filters << QString("status = '%1'").arg(status);
    }
    
    QDate fromDate = dateFromFilter->date();
    QDate toDate = dateToFilter->date();
    if (fromDate.isValid()) {
        filters << QString("created_date >= '%1'").arg(fromDate.toString("yyyy-MM-dd"));
    }
    if (toDate.isValid()) {
        filters << QString("created_date <= '%1'").arg(toDate.toString("yyyy-MM-dd"));
    }
    
    if (!showClosedCheck->isChecked()) {
        filters << "status != 'closed'";
    }
    if (!showResolvedCheck->isChecked()) {
        filters << "status != 'resolved'";
    }
    
    return filters.join(" AND ");
}

void AdvancedFilterPanel::resetFilters() {
    titleFilter->clear();
    descriptionFilter->clear();
    tagsFilter->clear();
    reporterFilter->clear();
    assignedFilter->clear();
    
    severityFilter->setCurrentIndex(0);
    statusFilter->setCurrentIndex(0);
    
    dateFromFilter->setDate(QDate::currentDate().addDays(-30));
    dateToFilter->setDate(QDate::currentDate());
    
    showClosedCheck->setChecked(true);
    showResolvedCheck->setChecked(true);
}

void AdvancedFilterPanel::onApplyFilter() {
    QString filter = getFilterString();
    emit filterChanged(filter);
}

void AdvancedFilterPanel::onResetFilter() {
    resetFilters();
    emit filterChanged("");
}
