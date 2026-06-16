#pragma once

#include "../include/arkis_data.h"

#include <QAbstractTableModel>
#include <QColor>
#include <vector>

class AracModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit AracModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setAraclar(const Depo<std::string, Arac>& araclar);
    const Arac& aracAt(int row) const;

private:
    std::vector<Arac> m_araclar;
};
