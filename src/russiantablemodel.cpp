#include "../include/russiantablemodel.h"
#include <QDateTime>
#include <QDebug>

RussianTableModel::RussianTableModel(QObject *parent)
    : QSqlQueryModel(parent) {}

QVariant RussianTableModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QVariant value = QSqlQueryModel::data(index, role);
        
        
        if (index.column() == 2) {
            QString severity = value.toString();
            if (severity == "low") return "Низкий";
            if (severity == "medium") return "Средний";
            if (severity == "high") return "Высокий";
            if (severity == "critical") return "Критический";
            return severity;
        }
        
      
        if (index.column() == 3) {
            QString status = value.toString();
            if (status == "new") return "Новый";
            if (status == "investigating") return "Расследуется";
            if (status == "contained") return "Локализован";
            if (status == "resolved") return "Решен";
            if (status == "closed") return "Закрыт";
            return status;
        }
        
        
        if (index.column() == 8) {
            QDateTime dt = value.toDateTime();
            if (dt.isValid()) {
                return dt.toString("dd.MM.yyyy HH:mm");
            }
        }
    }
    
    return QSqlQueryModel::data(index, role);
}

QVariant RussianTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      
        switch(section) {
            case 0: return "Заголовок";
            case 1: return "Описание";
            case 2: return "Уровень угрозы";
            case 3: return "Статус";
            case 4: return "Сообщил";
            case 5: return "Назначен";
            case 6: return "Теги";
            case 7: return "Решение";
            case 8: return "Дата создания";
            default: return QSqlQueryModel::headerData(section, orientation, role);
        }
    }
    return QSqlQueryModel::headerData(section, orientation, role);
}
