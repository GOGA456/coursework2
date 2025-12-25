#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>

class QLabel;
class QTableWidget;
class QPushButton;

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    
  
    void refreshDashboard();
    
private:
    // Метрики
    QLabel *lblTotalIncidents;
    QLabel *lblActiveIncidents;
    QLabel *lblCriticalIncidents;
    QLabel *lblAvgResolutionTime;
    QLabel *lblSLACompliance;
    QLabel *lblIncidentsToday;
    
    // Таблицы
    QTableWidget *recentIncidentsTable;
    QTableWidget *statsTable;
    

    QPushButton *btnRefresh;
    
private:
    void setupUI();
    QWidget* createMetricWidget(const QString &title, QLabel *valueLabel);
    void updateMetrics();
    void loadRecentIncidents();
    void loadStatisticsTable();
    
   
    double calculateAverageResolutionTime();
    double calculateSLACompliance();
};

#endif
