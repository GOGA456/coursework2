#ifndef INCIDENTDETAILDIALOG_H
#define INCIDENTDETAILDIALOG_H

#include <QDialog>

class QLineEdit;
class QTextEdit;
class QComboBox;
class QPushButton;

class IncidentDetailDialog : public QDialog
{
    Q_OBJECT

public:
    IncidentDetailDialog(const QString &incidentIdentifier, bool isAdmin,
                        const QString &userRole, 
                        const QString &currentFullName,
                        QWidget *parent = nullptr);
    ~IncidentDetailDialog();

private slots:
    void onSave();
    void onCancel();
    void onShowHistory();

private:
    void setupUI();
    void loadIncidentData();

    QString incidentIdentifier;
    bool isAdmin;
    QString userRole;
    QString fullName;
    
    QLineEdit *titleEdit;
    QTextEdit *descriptionEdit;
    QComboBox *severityCombo;
    QComboBox *statusCombo;
    QLineEdit *reporterEdit;
    QComboBox *assignedCombo;
    QLineEdit *tagsEdit;
    QTextEdit *resolutionEdit;
    QPushButton *saveButton;
    QPushButton *cancelButton;
    QPushButton *historyButton;
};

#endif 
