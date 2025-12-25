#ifndef MANAGEDFILTERPANEL_H
#define MANAGEDFILTERPANEL_H

#include <QWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>

class ManagedFilterPanel : public QWidget {
    Q_OBJECT
    
private:
    bool isExpanded_;
    QToolButton *toggleButton;
    QWidget *filterPanel;
    
    
    QLineEdit *titleFilter;
    QComboBox *statusFilter;
    QComboBox *severityFilter;
    QDateEdit *dateFromFilter;
    QDateEdit *dateToFilter;
    
public:
    ManagedFilterPanel(QWidget *parent = nullptr);
    ~ManagedFilterPanel();
    
    QString getFilterString() const;
    void resetFilters();
    
signals:
    void filterChanged(const QString &filter);
    
private slots:
    void setupUI();
    void toggleFilterPanel(bool expanded);
    void onApplyFilter();
};

#endif
