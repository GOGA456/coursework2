#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class QTabWidget;
class QTableView;
class QLineEdit;
class QStandardItemModel;
class RussianTableModel;  
class ManagedFilterPanel;
class DashboardWidget;

class MainWindow : public QWidget {
    Q_OBJECT
    
public:
    MainWindow(const QString &username, const QString &fullName, 
               const QString &role, QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void onAddIncident();
    void onEditIncident();
    void onDeleteIncident();
    void onRefresh();
    void onSearch();
    void onClearSearch();
    void onTabChanged(int index);
    void onExportData();
    void onBackupData();
    void onRestoreData();
    void onShowAll();
    void onShowActiveOnly();
    void onShowCriticalOnly();
    void onUserManagement();
    void loadDashboard();
    
private:
    void setupUI();
    void loadIncidents();
    void updateIncidentsTable(const QList<class Incident*> &incidents);
    void onFilterChanged(const QString &filter);
    
    QString currentUsername;
    QString currentFullName;
    QString currentRole;
    
    QTabWidget *tabWidget;
    QTableView *incidentsTableView;
    QLineEdit *searchEdit;
    QStandardItemModel *incidentsModel;  
    ManagedFilterPanel *filterPanel;
    DashboardWidget *dashboardTab;
};

#endif 
