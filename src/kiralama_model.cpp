#include "kiralama_model.h"

#include <QColor>
#include <QString>

KiralamaModel::KiralamaModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int KiralamaModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_sozlesmeler.size());
}

int KiralamaModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 6;
}

QVariant KiralamaModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= rowCount()) {
        return {};
    }

    const auto& sozlesme = m_sozlesmeler.at(static_cast<std::size_t>(index.row()));
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return sozlesme.sozlesme_id;
            case 1: return QString::fromStdString(sozlesme.plaka);
            case 2: return QString::fromStdString(sozlesme.tc_no);
            case 3: return QString::fromStdString(sozlesme.baslangic_tarihi);
            case 4: return QString::fromStdString(sozlesme.bitis_tarihi.value_or("---"));
            case 5: return sozlesme.toplam_tutar;
            default: return {};
        }
    }

    if (role == Qt::BackgroundRole && !sozlesme.bitis_tarihi) {
        return QColor(255, 248, 213);
    }

    return {};
}

QVariant KiralamaModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case 0: return "ID";
        case 1: return "Plaka";
        case 2: return "TC No";
        case 3: return "Baslangic";
        case 4: return "Bitis";
        case 5: return "Tutar";
        default: return {};
    }
}

void KiralamaModel::setSozlesmeler(const Depo<int, KiralamaSozlesmesi>& sozlesmeler) {
    beginResetModel();
    m_sozlesmeler.clear();
    for (const auto& sozlesme : sozlesmeler.tumunu_al() | std::views::values) {
        m_sozlesmeler.push_back(sozlesme);
    }
    endResetModel();
}

const KiralamaSozlesmesi& KiralamaModel::sozlesmeAt(int row) const {
    return m_sozlesmeler.at(static_cast<std::size_t>(row));
}
