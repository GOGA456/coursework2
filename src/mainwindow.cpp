#include "mainwindow.h"
#include "orm/Incident.h"
#include "orm/User.h"
#include "incidentdetaildialog.h"
#include "russiantablemodel.h"
#include "modules/usermanagement.h"
#include "modules/exportmodule.h"
#include "modules/managedfilterpanel.h"
#include "dashboardwidget.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QSqlRecord>
#include <QTabWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QToolButton>
#include <QProgressBar>
#include <QRadioButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QButtonGroup>

MainWindow::MainWindow(const QString &username, const QString &fullName, 
                       const QString &role, QWidget *parent)
    : QWidget(parent), currentUsername(username), currentFullName(fullName), 
      currentRole(role), dashboardTab(nullptr) {
    
    filterPanel = nullptr;
    incidentsModel = nullptr;
    
    setWindowTitle(QString("Система управления ИБ | %1 [%2]").arg(fullName).arg(role));
    setMinimumSize(1200, 600);
    
    setupUI();
    loadIncidents();
}

MainWindow::~MainWindow() {
    if (incidentsModel) {
        delete incidentsModel;
    }
}

void MainWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QMenuBar *menuBar = new QMenuBar(this);
    
    QMenu *fileMenu = menuBar->addMenu("Файл");
    QAction *exportAction = fileMenu->addAction("Экспорт данных");
    QAction *backupAction = fileMenu->addAction("Резервное копирование");
    QAction *restoreAction = fileMenu->addAction("Восстановление");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("Выход из системы");
    
    QMenu *viewMenu = menuBar->addMenu("Вид");
    QAction *showAllAction = viewMenu->addAction("Показать все");
    QAction *showActiveOnlyAction = viewMenu->addAction("Только активные");
    QAction *showCriticalOnlyAction = viewMenu->addAction("Только критические");
    
    QMenu *toolsMenu = menuBar->addMenu("Инструменты");
    QAction *userManagementAction = toolsMenu->addAction("Управление пользователями");
    
 
    if (currentRole != "admin") {
        userManagementAction->setEnabled(false);
        userManagementAction->setToolTip("Доступно только администраторам");
    }
    
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportData);
    connect(backupAction, &QAction::triggered, this, &MainWindow::onBackupData);
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreData);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    connect(showAllAction, &QAction::triggered, this, &MainWindow::onShowAll);
    connect(showActiveOnlyAction, &QAction::triggered, this, &MainWindow::onShowActiveOnly);
    connect(showCriticalOnlyAction, &QAction::triggered, this, &MainWindow::onShowCriticalOnly);
    
    connect(userManagementAction, &QAction::triggered, this, &MainWindow::onUserManagement);
    
    mainLayout->setMenuBar(menuBar);
    
    tabWidget = new QTabWidget();
    
    QWidget *incidentsTab = new QWidget();
    QVBoxLayout *incidentsLayout = new QVBoxLayout(incidentsTab);
    
    QWidget *userPanel = new QWidget();
    QHBoxLayout *userLayout = new QHBoxLayout(userPanel);
    QLabel *userInfo = new QLabel(
        QString("Пользователь: <b>%1</b> | Роль: <b>%2</b>").arg(currentFullName).arg(currentRole));
    userLayout->addWidget(userInfo);
    userLayout->addStretch();
    
    QWidget *actionsPanel = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsPanel);
    
    QPushButton *btnAdd = new QPushButton("Новый инцидент");
    QPushButton *btnEdit = new QPushButton("Редактировать");
    QPushButton *btnDelete = new QPushButton("Удалить");
    QPushButton *btnRefresh = new QPushButton("Обновить");
    

    if (currentRole == "auditor") {
        btnAdd->setEnabled(false);
        btnEdit->setEnabled(false);
        btnDelete->setEnabled(false);
        btnAdd->setToolTip("Аудиторы не могут создавать инциденты");
        btnEdit->setToolTip("Аудиторы не могут редактировать инциденты");
        btnDelete->setToolTip("Аудиторы не могут удалять инциденты");
    }

    else if (currentRole == "user") {
        btnDelete->setEnabled(false);
        btnDelete->setToolTip("Удаление доступно только администраторам");
    }

    actionsLayout->addWidget(btnAdd);
    actionsLayout->addWidget(btnEdit);
    actionsLayout->addWidget(btnDelete);
    actionsLayout->addStretch();
    actionsLayout->addWidget(btnRefresh);
    
    filterPanel = new ManagedFilterPanel();
    connect(filterPanel, &ManagedFilterPanel::filterChanged, this, &MainWindow::loadIncidents);
    
    QWidget *quickSearchPanel = new QWidget();
    QHBoxLayout *quickSearchLayout = new QHBoxLayout(quickSearchPanel);
    
    QLabel *quickSearchLabel = new QLabel("Быстрый поиск:");
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск по заголовку, описанию или тегам...");
    QPushButton *btnSearch = new QPushButton("Найти");
    QPushButton *btnClear = new QPushButton("Очистить");
    
    quickSearchLayout->addWidget(quickSearchLabel);
    quickSearchLayout->addWidget(searchEdit);
    quickSearchLayout->addWidget(btnSearch);
    quickSearchLayout->addWidget(btnClear);
    
    incidentsTableView = new QTableView();
    incidentsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    incidentsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    incidentsTableView->setAlternatingRowColors(true);
    incidentsTableView->setSortingEnabled(true);
    
    incidentsModel = nullptr;
    
    incidentsLayout->addWidget(userPanel);
    incidentsLayout->addWidget(actionsPanel);
    incidentsLayout->addWidget(filterPanel);
    incidentsLayout->addWidget(quickSearchPanel);
    incidentsLayout->addWidget(incidentsTableView, 1);
    
    tabWidget->addTab(incidentsTab, "Инциденты");
    
    // СОЗДАЕМ И СОХРАНЯЕМ dashboardTab
    dashboardTab = new DashboardWidget();
    tabWidget->addTab(dashboardTab, "Дашборд");
    
    mainLayout->addWidget(tabWidget);
    
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAddIncident);
    connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEditIncident);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDeleteIncident);
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefresh);
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClearSearch);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
    connect(incidentsTableView, &QTableView::doubleClicked, this, &MainWindow::onEditIncident);
    connect(tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::loadIncidents() {
    QList<Incident*> incidents;
    QString searchText = searchEdit->text().trimmed();

    if (!searchText.isEmpty()) {
        incidents = Incident::search(searchText);
    } else if (filterPanel && filterPanel->isExpanded()) {
        QString filter = filterPanel->getFilterString();
        if (filter.isEmpty()) {
            incidents = Incident::findAll();
        } else {
            incidents = Incident::findAll(filter);
        }
    } else {
        incidents = Incident::findAll();
    }

    updateIncidentsTable(incidents);
    
    qDeleteAll(incidents);
}

void MainWindow::updateIncidentsTable(const QList<Incident*> &incidents) {
    if (incidentsModel) {
        delete incidentsModel;
        incidentsModel = nullptr;
    }
    
    incidentsModel = new QStandardItemModel(incidents.size(), 9, this);
    QStringList headers = {"Заголовок", "Описание", "Уровень угрозы", 
                          "Статус", "Сообщил", "Назначен", "Теги", 
                          "Решение", "Дата создания"};
    incidentsModel->setHorizontalHeaderLabels(headers);
    
    for (int i = 0; i < incidents.size(); i++) {
        Incident *incident = incidents[i];

        // Перевод значений на русский:
        QString severity = incident->getSeverity();
        QString severityRu;
        if (severity == "low") severityRu = "Низкий";
        else if (severity == "medium") severityRu = "Средний";
        else if (severity == "high") severityRu = "Высокий";
        else if (severity == "critical") severityRu = "Критический";
        else severityRu = severity;
        
        QString status = incident->getStatus();
        QString statusRu;
        if (status == "new") statusRu = "Новый";
        else if (status == "investigating") statusRu = "Расследуется";
        else if (status == "contained") statusRu = "Локализован";
        else if (status == "resolved") statusRu = "Решен";
        else if (status == "closed") statusRu = "Закрыт";
        else statusRu = status;
        
        incidentsModel->setItem(i, 0, new QStandardItem(incident->getTitle()));
        incidentsModel->setItem(i, 1, new QStandardItem(incident->getDescription()));
        incidentsModel->setItem(i, 2, new QStandardItem(severityRu));
        incidentsModel->setItem(i, 3, new QStandardItem(statusRu));
        incidentsModel->setItem(i, 4, new QStandardItem(incident->getReporter()));
        incidentsModel->setItem(i, 5, new QStandardItem(incident->getAssignedTo()));
        incidentsModel->setItem(i, 6, new QStandardItem(incident->getTags()));
        incidentsModel->setItem(i, 7, new QStandardItem(incident->getResolution()));
        incidentsModel->setItem(i, 8, new QStandardItem(incident->getCreatedDate().toString("dd.MM.yyyy HH:mm")));
    }
    
    incidentsTableView->setModel(incidentsModel);
    
    // Настройка ширины столбцов
    incidentsTableView->setColumnWidth(0, 200);
    incidentsTableView->setColumnWidth(1, 300);
    incidentsTableView->setColumnWidth(2, 120);
    incidentsTableView->setColumnWidth(3, 120);
    incidentsTableView->setColumnWidth(4, 150);
    incidentsTableView->setColumnWidth(5, 150);
    incidentsTableView->setColumnWidth(6, 150);
    incidentsTableView->setColumnWidth(7, 200);
    incidentsTableView->setColumnWidth(8, 150);
}

void MainWindow::onAddIncident() {

    IncidentDetailDialog dialog("", currentRole == "admin", currentRole, 
                               currentFullName, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadIncidents();
    }
}

void MainWindow::onEditIncident() {
    QModelIndexList selection = incidentsTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите инцидент!");
        return;
    }
    
    int row = selection.first().row();
    QAbstractItemModel *model = incidentsTableView->model();
    QString title = model->data(model->index(row, 0)).toString();
    

    IncidentDetailDialog dialog(title, currentRole == "admin", currentRole, 
                               currentFullName, this);
    
    if (dialog.exec() == QDialog::Accepted && currentRole != "auditor") {

        loadIncidents();
    }
}

void MainWindow::onDeleteIncident() {

    if (currentRole != "admin") {
        QMessageBox::warning(this, "Доступ запрещен", 
            "Удаление инцидентов доступно только администраторам!");
        return;
    }
    
    QModelIndexList selection = incidentsTableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите инцидент для удаления!");
        return;
    }
    
    int row = selection.first().row();
    QAbstractItemModel *model = incidentsTableView->model();
    QString title = model->data(model->index(row, 0)).toString();
    
    qDebug() << "\n=== DELETE ===";
    qDebug() << "Title:" << title;
    
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение", 
                                 QString("Удалить инцидент:\n\"%1\"?").arg(title),
                                 QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        Incident incident;
        incident.setTitle(title);
        
        if (incident.remove()) {
            QMessageBox::information(this, "Успех", "Инцидент удален!");
            loadIncidents();
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка при удалении инцидента!");
        }
    }
}

void MainWindow::onRefresh() {
    loadIncidents();
}

void MainWindow::onSearch() {
    loadIncidents();
}

void MainWindow::onClearSearch() {
    searchEdit->clear();
    loadIncidents();
}

void MainWindow::onTabChanged(int index) {
    if (index == 1 && dashboardTab) {
        dashboardTab->refreshDashboard();
    }
}

void MainWindow::loadDashboard() {
    if (dashboardTab) {
        dashboardTab->refreshDashboard();
    }
}

void MainWindow::onExportData() {
    QAbstractItemModel *currentModel = incidentsTableView->model();
    if (!currentModel) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта!");
        return;
    }
    
    ExportDialog exportDialog(currentModel, this);
    exportDialog.exec();
}

void MainWindow::onBackupData() {
    QString fileName = QFileDialog::getSaveFileName(this, "Резервное копирование БД", 
        "ib_backup_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".sql", 
        "SQL Files (*.sql);;All Files (*)");

    if (fileName.isEmpty()) return;

    BaseModel baseModel;  // Создаём экземпляр BaseModel
    if (baseModel.backupDatabase(fileName)) {
        QMessageBox::information(this, "Успех", "Резервная копия создана!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка создания резервной копии!");
    }
}

void MainWindow::onRestoreData() {
    QString fileName = QFileDialog::getOpenFileName(this, "Восстановление БД", 
        "", "SQL Files (*.sql);;All Files (*)");

    if (fileName.isEmpty()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(this, "Предупреждение", 
        "Восстановление БД удалит все текущие данные!\nПродолжить?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    BaseModel baseModel;  
    if (baseModel.restoreDatabase(fileName)) {
        QMessageBox::information(this, "Успех", "База данных восстановлена!");
        loadIncidents();
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка восстановления базы данных!");
    }
}

void MainWindow::onShowAll() {
    searchEdit->clear();
    if (filterPanel) {
        filterPanel->resetFilters();
    }
    loadIncidents();
    QMessageBox::information(this, "Информация", "Показаны все инциденты");
}

void MainWindow::onShowActiveOnly() {
    searchEdit->clear();

    QList<Incident*> incidents = Incident::findAll("status NOT IN ('closed', 'resolved')");
    updateIncidentsTable(incidents);
    qDeleteAll(incidents);
    QMessageBox::information(this, "Информация", "Показаны только активные инциденты");
}

void MainWindow::onShowCriticalOnly() {
    searchEdit->clear();

    QList<Incident*> incidents = Incident::findAll("severity = 'critical'");
    updateIncidentsTable(incidents);
    qDeleteAll(incidents);
    QMessageBox::information(this, "Информация", "Показаны только критические инциденты");
}

void MainWindow::onUserManagement() {

    if (currentRole != "admin") {
        QMessageBox::warning(this, "Доступ запрещен", 
            "Управление пользователями доступно только администраторам!");
        return;
    }
    
    UserManagementPanel *userPanel = new UserManagementPanel();
    userPanel->setAttribute(Qt::WA_DeleteOnClose);
    userPanel->setWindowModality(Qt::ApplicationModal);
    userPanel->resize(800, 600);
    userPanel->setWindowTitle("Управление пользователями");
    userPanel->show();
}
