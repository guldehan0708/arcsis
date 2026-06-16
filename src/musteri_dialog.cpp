#include "musteri_dialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QRegularExpression>

MusteriDialog::MusteriDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Musteri Ekle");

    m_tc = new QLineEdit(this);
    m_isim = new QLineEdit(this);
    m_soyisim = new QLineEdit(this);
    m_telefon = new QLineEdit(this);
    m_ehliyet = new QLineEdit(this);

    auto* form = new QFormLayout(this);
    form->addRow("TC No", m_tc);
    form->addRow("Isim", m_isim);
    form->addRow("Soyisim", m_soyisim);
    form->addRow("Telefon", m_telefon);
    form->addRow("Ehliyet No", m_ehliyet);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &MusteriDialog::kabulEt);
    connect(buttons, &QDialogButtonBox::rejected, this, &MusteriDialog::reject);
}

Musteri MusteriDialog::musteri() const {
    return {
        m_tc->text().trimmed().toStdString(),
        m_isim->text().trimmed().toStdString(),
        m_soyisim->text().trimmed().toStdString(),
        m_telefon->text().trimmed().toStdString(),
        m_ehliyet->text().trimmed().toStdString()
    };
}

void MusteriDialog::kabulEt() {
    const QRegularExpression tcRegex("^[0-9]{11}$");
    if (!tcRegex.match(m_tc->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "Gecersiz TC", "TC no 11 rakamdan olusmalidir.");
        return;
    }

    if (m_isim->text().trimmed().isEmpty() ||
        m_soyisim->text().trimmed().isEmpty() ||
        m_telefon->text().trimmed().isEmpty() ||
        m_ehliyet->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Eksik Bilgi", "Tum alanlari doldurun.");
        return;
    }

    accept();
}
