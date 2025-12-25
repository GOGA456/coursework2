#include "../include/modules/managedfilterpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QToolButton>
#include <QDate>
#include <QDebug>

ManagedFilterPanel::ManagedFilterPanel(QWidget *parent)
    : QWidget(parent) {
    setupUI();
}

ManagedFilterPanel::~ManagedFilterPanel() {
}

void ManagedFilterPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    toggleButton = new QToolButton();
    toggleButton->setText("▼ Расширенный поиск");
    toggleButton->setCheckable(true);
    toggleButton->setChecked(false);
    toggleButton->setStyleSheet("QToolButton { text-align: left; padding: 5px; }");
    
    mainLayout->addWidget(toggleButton);
    
    // Панель фильтров
    filterPanel = new QWidget();
    filterPanel->setVisible(false);
    QFormLayout *formLayout = new QFormLayout(filterPanel);
    formLayout->setContentsMargins(10, 10, 10, 10);
    
    // Создаем элементы
    titleFilter = new QLineEdit();
    titleFilter->setPlaceholderText("Поиск по заголовку...");
    
    statusFilter = new QComboBox();
    statusFilter->addItem("Все статусы", "");
    statusFilter->addItem("Новый", "new");
    statusFilter->addItem("Расследуется", "investigating");
    statusFilter->addItem("Локализован", "contained");
    statusFilter->addItem("Решен", "resolved");
    statusFilter->addItem("Закрыт", "closed");
    
    severityFilter = new QComboBox();
    severityFilter->addItem("Все уровни", "");
    severityFilter->addItem("Низкий", "low");
    severityFilter->addItem("Средний", "medium");
    severityFilter->addItem("Высокий", "high");
    severityFilter->addItem("Критический", "critical");
    
    dateFromFilter = new QDateEdit();
    dateFromFilter->setCalendarPopup(true);
    dateFromFilter->setDisplayFormat("dd.MM.yyyy");
    dateFromFilter->setDate(QDate::currentDate().addDays(-30));
    
    dateToFilter = new QDateEdit();
    dateToFilter->setCalendarPopup(true);
    dateToFilter->setDisplayFormat("dd.MM.yyyy");
    dateToFilter->setDate(QDate::currentDate());
    
    // Добавляем на форму
    formLayout->addRow("Заголовок:", titleFilter);
    formLayout->addRow("Статус:", statusFilter);
    formLayout->addRow("Уровень:", severityFilter);
    formLayout->addRow("С даты:", dateFromFilter);
    formLayout->addRow("По дату:", dateToFilter);
    
    // Кнопки действий
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnApply = new QPushButton("Применить");
    QPushButton *btnReset = new QPushButton("Сбросить");
    QPushButton *btnClose = new QPushButton("Закрыть");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnReset);
    buttonLayout->addWidget(btnApply);
    buttonLayout->addWidget(btnClose);
    
    formLayout->addRow("", buttonLayout);
    
    mainLayout->addWidget(filterPanel);
    
 
    connect(toggleButton, &QToolButton::toggled, this, &ManagedFilterPanel::toggleFilterPanel);
    connect(btnApply, &QPushButton::clicked, this, &ManagedFilterPanel::onApplyClicked);
    connect(btnReset, &QPushButton::clicked, this, &ManagedFilterPanel::onResetClicked);
    connect(btnClose, &QPushButton::clicked, [this]() {
        toggleButton->setChecked(false);
        toggleFilterPanel(false);
    });
    
    
    connect(titleFilter, &QLineEdit::textChanged, this, &ManagedFilterPanel::onFilterChanged);
    connect(statusFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ManagedFilterPanel::onFilterChanged);
    connect(severityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &ManagedFilterPanel::onFilterChanged);
    connect(dateFromFilter, &QDateEdit::dateChanged, this, &ManagedFilterPanel::onFilterChanged);
    connect(dateToFilter, &QDateEdit::dateChanged, this, &ManagedFilterPanel::onFilterChanged);
}

void ManagedFilterPanel::toggleFilterPanel(bool expanded) {
    filterPanel->setVisible(expanded);
    
    if (expanded) {
        toggleButton->setText("▲ Скрыть фильтры");
    } else {
        toggleButton->setText("▼ Расширенный поиск");
    }
}

void ManagedFilterPanel::onApplyClicked() {
    QString filter = buildFilter();
    qDebug() << "Применен фильтр:" << filter;
    emit filterChanged(filter);
}

void ManagedFilterPanel::onResetClicked() {
    titleFilter->clear();
    statusFilter->setCurrentIndex(0);
    severityFilter->setCurrentIndex(0);
    dateFromFilter->setDate(QDate::currentDate().addDays(-30));
    dateToFilter->setDate(QDate::currentDate());
    
    emit filterChanged("");
}

void ManagedFilterPanel::onFilterChanged() {
    // Автоматически применяем при изменении
    onApplyClicked();
}

QString ManagedFilterPanel::buildFilter() const {
    QStringList conditions;
    
    // Заголовок
    QString title = titleFilter->text().trimmed();
    if (!title.isEmpty()) {
        conditions << QString("title ILIKE '%%1%'").arg(title.replace("'", "''"));
    }
    
    // Статус
    QString status = statusFilter->currentData().toString();
    if (!status.isEmpty()) {
        conditions << QString("status = '%1'").arg(status);
    }
    
    // Уровень угрозы
    QString severity = severityFilter->currentData().toString();
    if (!severity.isEmpty()) {
        conditions << QString("severity = '%1'").arg(severity);
    }
    
  
    QDate from = dateFromFilter->date();
    QDate to = dateToFilter->date();
    if (from.isValid() && to.isValid() && from <= to) {
    
        conditions << QString("created_date >= '%1 00:00:00' AND created_date <= '%2 23:59:59'")
                      .arg(from.toString("yyyy-MM-dd"))
                      .arg(to.toString("yyyy-MM-dd"));
        
        qDebug() << "Date filter: from" << from.toString("yyyy-MM-dd") 
                 << "to" << to.toString("yyyy-MM-dd");
    }
    
    
    if (conditions.isEmpty()) {
        return "";
    }
    
    QString filter = conditions.join(" AND ");
    qDebug() << "Built filter:" << filter;
    return filter;
}

bool ManagedFilterPanel::isExpanded() const {
    return toggleButton->isChecked();
}

QString ManagedFilterPanel::getFilterString() const {
    return buildFilter();
}

void ManagedFilterPanel::resetFilters() {
    onResetClicked();
}
