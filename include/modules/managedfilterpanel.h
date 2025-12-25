#ifndef MANAGEDFILTERPANEL_H
#define MANAGEDFILTERPANEL_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QToolButton;

class ManagedFilterPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ManagedFilterPanel(QWidget *parent = nullptr);
    ~ManagedFilterPanel();

    QString getFilterString() const;
    bool isExpanded() const;
    void resetFilters();

signals:
    void filterChanged(const QString &filter);

private slots:
    void toggleFilterPanel(bool expanded);
    void onApplyClicked();
    void onResetClicked();
    void onFilterChanged();
    


private:
    void setupUI();
    QString buildFilter() const;

    QToolButton *toggleButton;
    QWidget *filterPanel;
    
    QLineEdit *titleFilter;
    QComboBox *statusFilter;
    QComboBox *severityFilter;
    QDateEdit *dateFromFilter;
    QDateEdit *dateToFilter;
};

#endif
