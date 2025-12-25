#ifndef RUSSIANTABLEMODEL_H
#define RUSSIANTABLEMODEL_H

#include <QSqlQueryModel>

class RussianTableModel : public QSqlQueryModel {
    Q_OBJECT
    
public:
    explicit RussianTableModel(QObject *parent = nullptr);
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

#endif 
