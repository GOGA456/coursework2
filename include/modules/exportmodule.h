#ifndef EXPORTMODULE_H
#define EXPORTMODULE_H

#include <QDialog>

class QComboBox;
class QLineEdit;
class QDateEdit;
class QCheckBox;
class QPushButton;
class QGridLayout;
class QGroupBox;      
class QAbstractItemModel;
class QPrinter;
class QScrollArea;    

class ExportDialog : public QDialog {
    Q_OBJECT
    
public:
    ExportDialog(QAbstractItemModel *dataModel, QWidget *parent = nullptr);
    
private slots:
    void onBrowse();
    void onFormatChanged(int index);
    void onSelectAllColumns();
    void onDeselectAllColumns();
    void onExport();
    
private:
    void setupUI();
    void setupColumnCheckboxes();
    QList<int> getSelectedColumns() const;
    QString translateValue(const QString &value, const QString &header) const;
    bool exportToCSV(const QString &filename);
    bool exportToPDF(const QString &filename, bool isPreview = false);
    
    QAbstractItemModel *model;
    
    QComboBox *formatCombo;
    QLineEdit *filenameEdit;
    QPushButton *browseButton;
    
    QGroupBox *fileGroup;      
    QGroupBox *filterGroup;    
    QGroupBox *columnsGroup;   
    QGroupBox *optionsGroup;   
    
    QDateEdit *dateFromEdit;
    QDateEdit *dateToEdit;
    QComboBox *severityCombo;
    QComboBox *statusCombo;
    QLineEdit *searchFilter;
    
    QGridLayout *columnsGridLayout;
    QList<QCheckBox*> columnCheckboxes;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    
    QCheckBox *includeHeaderCheck;
    QCheckBox *translateDataCheck;
    QCheckBox *openAfterExportCheck;
};

#endif 
