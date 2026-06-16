#pragma once

#include "../include/arkis_data.h"

#include <QAbstractTableModel>
#include <vector>

class MusteriModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit MusteriModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setMusteriler(const Depo<std::string, Musteri>& musteriler);

private:
    std::vector<Musteri> m_musteriler;
};
