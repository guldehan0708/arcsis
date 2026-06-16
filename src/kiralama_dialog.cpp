#include "kiralama_dialog.h"

#include <QDate>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>

KiralamaDialog::KiralamaDialog(const ArkisVeriSeti& veri, QWidget* parent)
    : QDialog(parent),
      m_veri(veri)
{
    setWindowTitle("Kiralama Sozlesmesi");

    m_arac = new QComboBox(this);
    m_musteri = new QComboBox(this);
    m_baslangic = new QDateEdit(QDate::currentDate(), this);
    m_bitis = new QDateEdit(QDate::currentDate().addDays(1), this);
    m_tamamlandi = new QCheckBox(this);
    m_tutar = new QDoubleSpinBox(this);

    for (const auto& [plaka, arac] : m_veri.araclar.tumunu_al()) {
        m_arac->addItem(QString::fromStdString(plaka + " - " + arac.marka + " " + arac.model),
                        QString::fromStdString(plaka));
    }
    for (const auto& [tc, musteri] : m_veri.musteriler.tumunu_al()) {
        m_musteri->addItem(QString::fromStdString(tc + " - " + musteri.isim + " " + musteri.soyisim),
                           QString::fromStdString(tc));
    }

    m_baslangic->setCalendarPopup(true);
    m_bitis->setCalendarPopup(true);
    m_bitis->setEnabled(false);
    m_tutar->setRange(0.0, 100000000.0);
    m_tutar->setSuffix(" TL");
    m_tutar->setDecimals(2);

    auto* form = new QFormLayout(this);
    form->addRow("Arac", m_arac);
    form->addRow("Musteri", m_musteri);
    form->addRow("Baslangic", m_baslangic);
    form->addRow("Tamamlandi", m_tamamlandi);
    form->addRow("Bitis", m_bitis);
    form->addRow("Toplam Tutar", m_tutar);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);

    connect(m_arac, &QComboBox::currentIndexChanged, this, &KiralamaDialog::tutariGuncelle);
    connect(m_baslangic, &QDateEdit::dateChanged, this, &KiralamaDialog::tutariGuncelle);
    connect(m_bitis, &QDateEdit::dateChanged, this, &KiralamaDialog::tutariGuncelle);
    connect(m_tamamlandi, &QCheckBox::toggled, m_bitis, &QDateEdit::setEnabled);
    connect(m_tamamlandi, &QCheckBox::toggled, this, &KiralamaDialog::tutariGuncelle);
    connect(buttons, &QDialogButtonBox::accepted, this, &KiralamaDialog::kabulEt);
    connect(buttons, &QDialogButtonBox::rejected, this, &KiralamaDialog::reject);

    tutariGuncelle();
}

KiralamaSozlesmesi KiralamaDialog::sozlesme(int id) const {
    std::optional<std::string> bitis;
    if (m_tamamlandi->isChecked()) {
        bitis = m_bitis->date().toString(Qt::ISODate).toStdString();
    }

    return {
        id,
        m_arac->currentData().toString().toStdString(),
        m_musteri->currentData().toString().toStdString(),
        m_baslangic->date().toString(Qt::ISODate).toStdString(),
        bitis,
        m_tutar->value()
    };
}

void KiralamaDialog::tutariGuncelle() {
    const auto plaka = m_arac->currentData().toString().toStdString();
    const auto arac = m_veri.araclar.bul(plaka);
    if (!arac || !m_tamamlandi->isChecked()) {
        m_tutar->setValue(0.0);
        return;
    }

    const auto gun = std::max<qint64>(1, m_baslangic->date().daysTo(m_bitis->date()));
    m_tutar->setValue(static_cast<double>(gun) * arac->gunluk_ucret);
}

void KiralamaDialog::kabulEt() {
    if (m_arac->currentIndex() < 0 || m_musteri->currentIndex() < 0) {
        QMessageBox::warning(this, "Eksik Bilgi", "Arac ve musteri secin.");
        return;
    }

    if (m_tamamlandi->isChecked() && m_bitis->date() < m_baslangic->date()) {
        QMessageBox::warning(this, "Gecersiz Tarih", "Bitis tarihi baslangictan once olamaz.");
        return;
    }

    accept();
}
