#include "arac_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QRegularExpression>

AracDialog::AracDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Arac Ekle");

    m_plaka = new QLineEdit(this);
    m_marka = new QLineEdit(this);
    m_model = new QLineEdit(this);
    m_yil = new QSpinBox(this);
    m_yakit = new QLineEdit(this);
    m_ucret = new QDoubleSpinBox(this);
    m_durum = new QComboBox(this);

    m_plaka->setPlaceholderText("34ABC123");
    m_yil->setRange(1980, 2100);
    m_yil->setValue(2024);
    m_ucret->setRange(0.0, 100000.0);
    m_ucret->setSuffix(" TL");
    m_ucret->setDecimals(2);
    m_durum->addItems({"Musait", "Kirada", "Bakimda"});

    auto* form = new QFormLayout(this);
    form->addRow("Plaka", m_plaka);
    form->addRow("Marka", m_marka);
    form->addRow("Model", m_model);
    form->addRow("Yil", m_yil);
    form->addRow("Yakit Tipi", m_yakit);
    form->addRow("Gunluk Ucret", m_ucret);
    form->addRow("Durum", m_durum);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &AracDialog::kabulEt);
    connect(buttons, &QDialogButtonBox::rejected, this, &AracDialog::reject);
}

Arac AracDialog::arac() const {
    return {
        m_plaka->text().trimmed().toUpper().toStdString(),
        m_marka->text().trimmed().toStdString(),
        m_model->text().trimmed().toStdString(),
        m_yil->value(),
        m_yakit->text().trimmed().toStdString(),
        m_ucret->value(),
        static_cast<AracDurum>(m_durum->currentIndex())
    };
}

void AracDialog::kabulEt() {
    const auto plaka = m_plaka->text().trimmed().toUpper();
    const QRegularExpression plakaRegex("^[0-9]{2}[A-Z]{1,3}[0-9]{2,4}$");

    if (plaka.isEmpty() || m_marka->text().trimmed().isEmpty() ||
        m_model->text().trimmed().isEmpty() || m_yakit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Eksik Bilgi", "Tum alanlari doldurun.");
        return;
    }

    if (!plakaRegex.match(plaka).hasMatch()) {
        QMessageBox::warning(this, "Gecersiz Plaka",
                             "Plaka 34ABC123 bicimine benzemelidir.");
        return;
    }

    accept();
}
