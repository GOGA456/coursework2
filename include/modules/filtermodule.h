#ifndef FILTERMODULE_H
#define FILTERMODULE_H

#include <QWidget>
#include <QDate>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QCheckBox;
class QPushButton;


class AdvancedFilterPanel : public QWidget {
    Q_OBJECT
public:
    AdvancedFilterPanel(QWidget *parent = nullptr);
    
    QString getFilterString() const;
    void resetFilters();
    
signals:
    void filterChanged(const QString &filter);
    
public slots:  
    void onApplyFilter();
    void onResetFilter();
    
private:
    void setupUI();
    
    
    QLineEdit *titleFilter;
    QLineEdit *descriptionFilter;
    QLineEdit *tagsFilter;
    QLineEdit *reporterFilter;
    QLineEdit *assignedFilter;
    
    QComboBox *severityFilter;
    QComboBox *statusFilter;
    
    QDateEdit *dateFromFilter;
    QDateEdit *dateToFilter;
    
    QCheckBox *showClosedCheck;
    QCheckBox *showResolvedCheck;
    
    QPushButton *applyButton;
    QPushButton *resetButton;
};

#endif 
