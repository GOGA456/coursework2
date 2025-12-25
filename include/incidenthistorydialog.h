#ifndef INCIDENTHISTORYDIALOG_H
#define INCIDENTHISTORYDIALOG_H

#include <QDialog>
#include <QString>
#include <QDateTime>

class QTextBrowser;
class QLabel;

class IncidentHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    
    explicit IncidentHistoryDialog(const QString &incidentTitle,
                                  QWidget *parent = nullptr);
    
private:
    void loadHistory();
    
    QString m_incidentTitle;
    QTextBrowser *historyBrowser;
};

#endif 
