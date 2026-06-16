#pragma once

#include "../include/arkis_data.h"

#include <QAbstractTableModel>
#include <vector>

class KiralamaModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit KiralamaModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setSozlesmeler(const Depo<int, KiralamaSozlesmesi>& sozlesmeler);
    const KiralamaSozlesmesi& sozlesmeAt(int row) const;

private:
    std::vector<KiralamaSozlesmesi> m_sozlesmeler;
};
