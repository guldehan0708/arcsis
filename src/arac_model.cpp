#include "arac_model.h"

#include <QString>

AracModel::AracModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int AracModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_araclar.size());
}

int AracModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 7;
}

QVariant AracModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rowCount()) {
        return {};
    }

    const auto& arac = m_araclar.at(static_cast<std::size_t>(index.row()));
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return QString::fromStdString(arac.plaka);
            case 1: return QString::fromStdString(arac.marka);
            case 2: return QString::fromStdString(arac.model);
            case 3: return arac.yil;
            case 4: return QString::fromStdString(arac.yakit_tipi);
            case 5: return arac.gunluk_ucret;
            case 6: return QString::fromStdString(durumYazisi(arac.durum));
            default: return {};
        }
    }

    if (role == Qt::BackgroundRole && index.column() == 6) {
        switch (arac.durum) {
            case AracDurum::Musait: return QColor(221, 247, 225);
            case AracDurum::Kirada: return QColor(255, 236, 204);
            case AracDurum::Bakimda: return QColor(244, 221, 221);
        }
    }

    return {};
}

QVariant AracModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case 0: return "Plaka";
        case 1: return "Marka";
        case 2: return "Model";
        case 3: return "Yil";
        case 4: return "Yakit";
        case 5: return "Gunluk Ucret";
        case 6: return "Durum";
        default: return {};
    }
}

void AracModel::setAraclar(const Depo<std::string, Arac>& araclar) {
    beginResetModel();
    m_araclar.clear();
    for (const auto& arac : araclar.tumunu_al() | std::views::values) {
        m_araclar.push_back(arac);
    }
    endResetModel();
}

const Arac& AracModel::aracAt(int row) const {
    return m_araclar.at(static_cast<std::size_t>(row));
}
