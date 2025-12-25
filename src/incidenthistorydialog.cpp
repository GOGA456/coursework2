#include "incidenthistorydialog.h"
#include "orm/Incident.h"
#include "orm/IncidentHistory.h"

#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

IncidentHistoryDialog::IncidentHistoryDialog(const QString &incidentTitle,
                                           QWidget *parent)
    : QDialog(parent), m_incidentTitle(incidentTitle) {
    
    setWindowTitle("История изменений: " + incidentTitle);
    setMinimumSize(700, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel(
        QString("<h3>История изменений инцидента:</h3>"
                "<p><b>%1</b></p>")
            .arg(incidentTitle));
    
    mainLayout->addWidget(titleLabel);
    
    historyBrowser = new QTextBrowser();
    historyBrowser->setReadOnly(true);
    mainLayout->addWidget(historyBrowser, 1);
    
    QPushButton *closeButton = new QPushButton("Закрыть");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    mainLayout->addLayout(buttonLayout);
    
    loadHistory();
}

void IncidentHistoryDialog::loadHistory() {
    if (m_incidentTitle.isEmpty()) return;
    
    qDebug() << "\n=== IncidentHistoryDialog::loadHistory ===";
    qDebug() << "Incident title:" << m_incidentTitle;
    
    QString html = "<html><body style='font-family: Arial;'>";
    html += "<h3>История изменений</h3>";
    
    // Ищем историю только по заголовку
    QList<IncidentHistory*> history = IncidentHistory::findByIncident(m_incidentTitle);
    
    qDebug() << "Found" << history.size() << "history entries";
    
    if (history.isEmpty()) {
        html += "<p><i>История изменений отсутствует</i></p>";
        html += "<p><i>Возможные причины:</i></p>";
        html += "<ul>";
        html += "<li>Инцидент не редактировался после создания</li>";
        html += "<li>Изменения не были записаны в историю</li>";
        html += "<li>Проверьте логи приложения</li>";
        html += "</ul>";
    } else {
        html += "<table border='1' cellpadding='5' style='border-collapse: collapse; width: 100%;'>";
        html += "<tr style='background-color: #f0f0f0;'>";
        html += "<th>Дата изменения</th><th>Поле</th><th>Старое значение</th><th>Новое значение</th><th>Кто изменил</th>";
        html += "</tr>";
        
        for (IncidentHistory *item : history) {
            html += "<tr>";
            html += "<td>" + item->getChangeDate().toString("dd.MM.yyyy HH:mm") + "</td>";
            html += "<td>" + item->getChangedField() + "</td>";
            html += "<td>" + item->getOldValue() + "</td>";
            html += "<td>" + item->getNewValue() + "</td>";
            html += "<td>" + item->getChangedBy() + "</td>";
            html += "</tr>";
            
            qDebug() << "History entry:";
            qDebug() << "  Field:" << item->getChangedField();
            qDebug() << "  Old:" << item->getOldValue();
            qDebug() << "  New:" << item->getNewValue();
            qDebug() << "  By:" << item->getChangedBy();
            
            delete item;
        }
        
        html += "</table>";
    }
    
    html += "<h3>Идентификатор инцидента</h3>";
    html += "<table border='1' cellpadding='5' style='border-collapse: collapse; width: 100%;'>";
    html += "<tr><td><b>Заголовок:</b></td><td>" + m_incidentTitle + "</td></tr>";
    html += "</table>";
    
    html += "</body></html>";
    
    historyBrowser->setHtml(html);
}
