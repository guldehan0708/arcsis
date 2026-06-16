#pragma once

#include "../include/arkis_data.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDoubleSpinBox>

class KiralamaDialog : public QDialog {
    Q_OBJECT

public:
    KiralamaDialog(const ArkisVeriSeti& veri, QWidget* parent = nullptr);
    KiralamaSozlesmesi sozlesme(int id) const;

private slots:
    void tutariGuncelle();
    void kabulEt();

private:
    const ArkisVeriSeti& m_veri;
    QComboBox* m_arac{};
    QComboBox* m_musteri{};
    QDateEdit* m_baslangic{};
    QDateEdit* m_bitis{};
    QCheckBox* m_tamamlandi{};
    QDoubleSpinBox* m_tutar{};
};
