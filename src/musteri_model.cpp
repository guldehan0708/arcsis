#include "musteri_model.h"

#include <QString>

MusteriModel::MusteriModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int MusteriModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_musteriler.size());
}

int MusteriModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 5;
}

QVariant MusteriModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rowCount() || role != Qt::DisplayRole) {
        return {};
    }

    const auto& musteri = m_musteriler.at(static_cast<std::size_t>(index.row()));
    switch (index.column()) {
        case 0: return QString::fromStdString(musteri.tc_no);
        case 1: return QString::fromStdString(musteri.isim);
        case 2: return QString::fromStdString(musteri.soyisim);
        case 3: return QString::fromStdString(musteri.telefon);
        case 4: return QString::fromStdString(musteri.ehliyet_no);
        default: return {};
    }
}

QVariant MusteriModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case 0: return "TC No";
        case 1: return "Isim";
        case 2: return "Soyisim";
        case 3: return "Telefon";
        case 4: return "Ehliyet";
        default: return {};
    }
}

void MusteriModel::setMusteriler(const Depo<std::string, Musteri>& musteriler) {
    beginResetModel();
    m_musteriler.clear();
    for (const auto& musteri : musteriler.tumunu_al() | std::views::values) {
        m_musteriler.push_back(musteri);
    }
    endResetModel();
}
