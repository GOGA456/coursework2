#include <QPrinter>
#include "../include/modules/exportmodule.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QScrollArea>
#include <QWidget>
#include <QStandardItemModel>
#include <QLineEdit>  
#include <QRegExp> 

ExportDialog::ExportDialog(QAbstractItemModel *dataModel, QWidget *parent)
    : QDialog(parent), model(dataModel) {
    
    setWindowTitle("Экспорт данных");
    setMinimumSize(700, 600);
    
    setupUI();
}

void ExportDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    

    fileGroup = new QGroupBox("Настройки экспорта");  
    QFormLayout *fileLayout = new QFormLayout(fileGroup);
    
    formatCombo = new QComboBox();
    formatCombo->addItems(QStringList() << "CSV (Excel)" << "PDF (документ)");
    
    filenameEdit = new QLineEdit();
    filenameEdit->setPlaceholderText("Выберите файл для сохранения...");
    
    browseButton = new QPushButton("Обзор...");
    
    QHBoxLayout *filenameLayout = new QHBoxLayout();
    filenameLayout->addWidget(filenameEdit);
    filenameLayout->addWidget(browseButton);
    
    fileLayout->addRow("Формат:", formatCombo);
    fileLayout->addRow("Файл:", filenameLayout);
    

    filterGroup = new QGroupBox("Фильтрация данных");
    QFormLayout *filterLayout = new QFormLayout(filterGroup);
    
    dateFromEdit = new QDateEdit();
    dateFromEdit->setCalendarPopup(true);
    dateFromEdit->setDate(QDate::currentDate().addDays(-30));
    
    dateToEdit = new QDateEdit();
    dateToEdit->setCalendarPopup(true);
    dateToEdit->setDate(QDate::currentDate());
    
    severityCombo = new QComboBox();
    severityCombo->addItem("Все уровни", "");
    severityCombo->addItem("Низкий", "low");
    severityCombo->addItem("Средний", "medium");
    severityCombo->addItem("Высокий", "high");
    severityCombo->addItem("Критический", "critical");
    
    statusCombo = new QComboBox();
    statusCombo->addItem("Все статусы", "");
    statusCombo->addItem("Новый", "new");
    statusCombo->addItem("Расследуется", "investigating");
    statusCombo->addItem("Локализован", "contained");
    statusCombo->addItem("Решен", "resolved");
    statusCombo->addItem("Закрыт", "closed");
    
    searchFilter = new QLineEdit();
    searchFilter->setPlaceholderText("Поиск по тексту...");
    
    filterLayout->addRow("С даты:", dateFromEdit);
    filterLayout->addRow("По дату:", dateToEdit);
    filterLayout->addRow("Уровень угрозы:", severityCombo);
    filterLayout->addRow("Статус:", statusCombo);
    filterLayout->addRow("Текстовый поиск:", searchFilter);
    

    columnsGroup = new QGroupBox("Выбор столбцов");
    QVBoxLayout *columnsLayout = new QVBoxLayout(columnsGroup);
    

    QWidget *buttonPanel = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonPanel);
    
    selectAllButton = new QPushButton("Выбрать все");
    deselectAllButton = new QPushButton("Снять все");
    
    buttonLayout->addWidget(selectAllButton);
    buttonLayout->addWidget(deselectAllButton);
    buttonLayout->addStretch();
    
    columnsLayout->addWidget(buttonPanel);
    
   
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMinimumHeight(150);
    
    QWidget *columnsContainer = new QWidget();
    columnsGridLayout = new QGridLayout(columnsContainer);
    columnsGridLayout->setSpacing(10);
    
    scrollArea->setWidget(columnsContainer);
    columnsLayout->addWidget(scrollArea);
    

    setupColumnCheckboxes();
    

    optionsGroup = new QGroupBox("Дополнительные опции");
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);
    
    includeHeaderCheck = new QCheckBox("Включать заголовки столбцов");
    includeHeaderCheck->setChecked(true);
    
    translateDataCheck = new QCheckBox("Переводить данные на русский");
    translateDataCheck->setChecked(true);
    
    openAfterExportCheck = new QCheckBox("Открыть файл после экспорта");
    openAfterExportCheck->setChecked(false);
    
    optionsLayout->addWidget(includeHeaderCheck);
    optionsLayout->addWidget(translateDataCheck);
    optionsLayout->addWidget(openAfterExportCheck);
    

    QWidget *actionPanel = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionPanel);
    
    QPushButton *exportButton = new QPushButton("Экспорт");
    QPushButton *cancelButton = new QPushButton("Отмена");
    QPushButton *previewButton = new QPushButton("Предпросмотр PDF");
    
    actionLayout->addWidget(previewButton);
    actionLayout->addStretch();
    actionLayout->addWidget(exportButton);
    actionLayout->addWidget(cancelButton);
    
   
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(filterGroup);
    mainLayout->addWidget(columnsGroup);
    mainLayout->addWidget(optionsGroup);
    mainLayout->addWidget(actionPanel);
    

    connect(browseButton, &QPushButton::clicked, this, &ExportDialog::onBrowse);
    connect(selectAllButton, &QPushButton::clicked, this, &ExportDialog::onSelectAllColumns);
    connect(deselectAllButton, &QPushButton::clicked, this, &ExportDialog::onDeselectAllColumns);
    connect(exportButton, &QPushButton::clicked, this, &ExportDialog::onExport);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(previewButton, &QPushButton::clicked, [this]() {
        QString tempFile = QDir::tempPath() + "/preview.pdf";
        if (exportToPDF(tempFile, true)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(tempFile));
        }
    });
    
    connect(formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExportDialog::onFormatChanged);
}
void ExportDialog::setupColumnCheckboxes() {
    if (!model) return;
    
    int colCount = model->columnCount();
    columnCheckboxes.clear();
    
    // Очищаем layout
    QLayoutItem* item;
    while ((item = columnsGridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    
    int itemsPerColumn = 5; // 5 чекбоксов в колонке
    int numColumns = (colCount + itemsPerColumn - 1) / itemsPerColumn; // ceil деление
    
    for (int i = 0; i < colCount; i++) {
        QString header = model->headerData(i, Qt::Horizontal).toString();
        QCheckBox *checkBox = new QCheckBox(header);
        checkBox->setChecked(true);
        checkBox->setProperty("columnIndex", i);
        columnCheckboxes.append(checkBox);
        
        int row = i % itemsPerColumn;
        int col = i / itemsPerColumn;
        columnsGridLayout->addWidget(checkBox, row, col);
    }
}

void ExportDialog::onBrowse() {
    QString filter;
    if (formatCombo->currentIndex() == 0) {
        filter = "CSV Files (*.csv);;All Files (*)";
    } else {
        filter = "PDF Files (*.pdf);;All Files (*)";
    }
    
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить файл", 
                                                   "", filter);
    if (!filename.isEmpty()) {
        filenameEdit->setText(filename);
    }
}

void ExportDialog::onFormatChanged(int index) {
    QString currentFile = filenameEdit->text();
    if (!currentFile.isEmpty()) {
        if (index == 0) { // CSV
            currentFile = currentFile.replace(QRegExp("\\.pdf$", Qt::CaseInsensitive), ".csv");
            if (!currentFile.endsWith(".csv", Qt::CaseInsensitive)) {
                currentFile += ".csv";
            }
        } else { // PDF
            currentFile = currentFile.replace(QRegExp("\\.csv$", Qt::CaseInsensitive), ".pdf");
            if (!currentFile.endsWith(".pdf", Qt::CaseInsensitive)) {
                currentFile += ".pdf";
            }
        }
        filenameEdit->setText(currentFile);
    }
}

void ExportDialog::onSelectAllColumns() {
    for (QCheckBox *checkBox : columnCheckboxes) {
        checkBox->setChecked(true);
    }
}

void ExportDialog::onDeselectAllColumns() {
    for (QCheckBox *checkBox : columnCheckboxes) {
        checkBox->setChecked(false);
    }
}

QList<int> ExportDialog::getSelectedColumns() const {
    QList<int> columns;
    for (QCheckBox *checkBox : columnCheckboxes) {
        if (checkBox->isChecked()) {
            columns.append(checkBox->property("columnIndex").toInt());
        }
    }
    return columns;
}

QString ExportDialog::translateValue(const QString &value, const QString &header) const {
    if (header.contains("Уровень", Qt::CaseInsensitive)) {
        if (value == "low") return "Низкий";
        if (value == "medium") return "Средний";
        if (value == "high") return "Высокий";
        if (value == "critical") return "Критический";
    }
    
    if (header.contains("Статус", Qt::CaseInsensitive)) {
        if (value == "new") return "Новый";
        if (value == "investigating") return "Расследуется";
        if (value == "contained") return "Локализован";
        if (value == "resolved") return "Решен";
        if (value == "closed") return "Закрыт";
    }
    
    if (header.contains("Дата", Qt::CaseInsensitive) || 
        header.contains("изменен", Qt::CaseInsensitive)) {
        QDateTime dt = QDateTime::fromString(value, Qt::ISODate);
        if (dt.isValid()) {
            return dt.toString("dd.MM.yyyy HH:mm");
        }
    }
    
    return value;
}

bool ExportDialog::exportToCSV(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Cannot open file for writing:" << filename;
        return false;
    }
    
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    
    QList<int> columns = getSelectedColumns();
    if (columns.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы один столбец для экспорта!");
        return false;
    }
    
    qDebug() << "Exporting to CSV. Selected columns:" << columns;
    
    // Заголовки
    if (includeHeaderCheck->isChecked()) {
        for (int i = 0; i < columns.size(); i++) {
            QString header = model->headerData(columns[i], Qt::Horizontal).toString();
            stream << "\"" << header << "\"";
            if (i < columns.size() - 1) stream << ";";
        }
        stream << "\n";
    }
    

    for (int row = 0; row < model->rowCount(); row++) {
        for (int i = 0; i < columns.size(); i++) {
            QModelIndex index = model->index(row, columns[i]);
            QString value = model->data(index).toString();
            QString header = model->headerData(columns[i], Qt::Horizontal).toString();
            
            if (translateDataCheck->isChecked()) {
                value = translateValue(value, header);
            }
            
      
            value = value.replace("\"", "\"\"")
                        .replace("\n", " ")
                        .replace("\r", " ")
                        .replace(";", ","); // Замена точки с запятой на запятую для безопасности
            
            stream << "\"" << value << "\"";
            if (i < columns.size() - 1) stream << ";";
        }
        stream << "\n";
    }
    
    file.close();
    qDebug() << "CSV export completed successfully to:" << filename;
    return true;
}

bool ExportDialog::exportToPDF(const QString &filename, bool isPreview) {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filename);
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    
    QPainter painter;
    if (!painter.begin(&printer)) {
        qDebug() << "Cannot start PDF painter";
        return false;
    }
    
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    QFontMetrics fm(font);
    int rowHeight = fm.height() + 4;
    int pageWidth = printer.width();
    int pageHeight = printer.height();
    int currentY = 50;
    
    // Заголовок
    painter.setFont(QFont(font.family(), 12, QFont::Bold));
    painter.drawText(50, currentY, "Отчет по инцидентам ИБ");
    currentY += rowHeight;
    
    painter.setFont(QFont(font.family(), 10));
    painter.drawText(50, currentY, 
        QString("Дата формирования: %1").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm")));
    currentY += rowHeight * 2;
    
    QList<int> columns = getSelectedColumns();
    if (columns.isEmpty()) {
        painter.drawText(50, currentY, "ОШИБКА: Не выбраны столбцы для экспорта!");
        painter.end();
        return false;
    }
    

    QVector<int> columnWidths(columns.size());
    int totalWidth = 0;
    
    for (int i = 0; i < columns.size(); i++) {
        QString header = model->headerData(columns[i], Qt::Horizontal).toString();
        columnWidths[i] = fm.horizontalAdvance(header) + 20;
        
        for (int row = 0; row < model->rowCount(); row++) {
            QString value = model->data(model->index(row, columns[i])).toString();
            if (translateDataCheck->isChecked()) {
                value = translateValue(value, header);
            }
            int width = fm.horizontalAdvance(value) + 20;
            if (width > columnWidths[i]) {
                columnWidths[i] = width;
            }
        }
        

        if (columnWidths[i] > pageWidth / 3) {
            columnWidths[i] = pageWidth / 3;
        }
        
        totalWidth += columnWidths[i];
    }
    

    if (totalWidth > pageWidth - 100) {
        float scale = (pageWidth - 100) / (float)totalWidth;
        for (int i = 0; i < columnWidths.size(); i++) {
            columnWidths[i] = columnWidths[i] * scale;
        }
    }
    
 
    if (includeHeaderCheck->isChecked()) {
        painter.setPen(Qt::black);
        painter.setBrush(QBrush(QColor(240, 240, 240)));
        
        int x = 50;
        for (int i = 0; i < columns.size(); i++) {
            QString header = model->headerData(columns[i], Qt::Horizontal).toString();
            painter.drawRect(x, currentY, columnWidths[i], rowHeight);
            painter.drawText(x + 5, currentY + fm.ascent() + 2, header);
            x += columnWidths[i];
        }
        currentY += rowHeight;
    }
    
  
    painter.setBrush(Qt::NoBrush);
    int rowsPerPage = 0;
    
    for (int row = 0; row < model->rowCount(); row++) {
    
        if (currentY + rowHeight > pageHeight - 50) {
            if (!isPreview) {
                printer.newPage();
            }
            currentY = 50;
            rowsPerPage = 0;
            
            if (includeHeaderCheck->isChecked()) {
                painter.setBrush(QBrush(QColor(240, 240, 240)));
                int x = 50;
                for (int i = 0; i < columns.size(); i++) {
                    QString header = model->headerData(columns[i], Qt::Horizontal).toString();
                    painter.drawRect(x, currentY, columnWidths[i], rowHeight);
                    painter.drawText(x + 5, currentY + fm.ascent() + 2, header);
                    x += columnWidths[i];
                }
                currentY += rowHeight;
                painter.setBrush(Qt::NoBrush);
            }
        }
        
        int x = 50;
        for (int i = 0; i < columns.size(); i++) {
            QString value = model->data(model->index(row, columns[i])).toString();
            QString header = model->headerData(columns[i], Qt::Horizontal).toString();
            
            if (translateDataCheck->isChecked()) {
                value = translateValue(value, header);
            }
            
       
            QString displayValue = value;
            if (fm.horizontalAdvance(displayValue) > columnWidths[i] - 10) {
                displayValue = fm.elidedText(displayValue, Qt::ElideRight, columnWidths[i] - 10);
            }
            
            painter.drawRect(x, currentY, columnWidths[i], rowHeight);
            painter.drawText(x + 5, currentY + fm.ascent() + 2, displayValue);
            x += columnWidths[i];
        }
        currentY += rowHeight;
        rowsPerPage++;
        
       
        if (isPreview && rowsPerPage >= 20) {
            break;
        }
    }
    
    
    currentY += rowHeight;
    painter.drawText(50, currentY, 
        QString("Всего записей: %1").arg(model->rowCount()));
    
    painter.end();
    return true;
}

void ExportDialog::onExport() {
    QString filename = filenameEdit->text();
    if (filename.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Укажите имя файла!");
        return;
    }
    
    QList<int> columns = getSelectedColumns();
    if (columns.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы один столбец!");
        return;
    }
    
    bool success = false;
    if (formatCombo->currentIndex() == 0) { // CSV
        success = exportToCSV(filename);
    } else { // PDF
        success = exportToPDF(filename, false);
    }
    
    if (success) {
        QMessageBox::information(this, "Успех", 
            QString("Экспорт завершен:\n%1").arg(filename));
        
       
        if (openAfterExportCheck->isChecked()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
        }
        
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Ошибка экспорта!");
    }
}
