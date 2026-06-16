#pragma once

#include "../include/varliklar.h"

#include <QDialog>
#include <QLineEdit>

class MusteriDialog : public QDialog {
    Q_OBJECT

public:
    explicit MusteriDialog(QWidget* parent = nullptr);
    Musteri musteri() const;

private slots:
    void kabulEt();

private:
    QLineEdit* m_tc{};
    QLineEdit* m_isim{};
    QLineEdit* m_soyisim{};
    QLineEdit* m_telefon{};
    QLineEdit* m_ehliyet{};
};
