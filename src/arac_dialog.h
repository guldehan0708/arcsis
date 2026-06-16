#pragma once

#include "../include/varliklar.h"

#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>

class AracDialog : public QDialog {
    Q_OBJECT

public:
    explicit AracDialog(QWidget* parent = nullptr);
    Arac arac() const;

private slots:
    void kabulEt();

private:
    QLineEdit* m_plaka{};
    QLineEdit* m_marka{};
    QLineEdit* m_model{};
    QSpinBox* m_yil{};
    QLineEdit* m_yakit{};
    QDoubleSpinBox* m_ucret{};
    QComboBox* m_durum{};
};
