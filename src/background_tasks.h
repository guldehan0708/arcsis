#pragma once

#include "../include/arkis_data.h"

#include <QObject>
#include <atomic>

enum class ArkaPlanGorevi {
    Fatura,
    Rapor,
    Bakim
};

class BackgroundTaskWorker : public QObject {
    Q_OBJECT

public:
    BackgroundTaskWorker(ArkaPlanGorevi gorev, ArkisVeriSeti veri);

public slots:
    void calistir();
    void iptalEt();

signals:
    void ilerlemeGuncellendi(int yuzde);
    void tamamlandi(const QString& mesaj);
    void hataOlustu(const QString& mesaj);

private:
    ArkaPlanGorevi m_gorev;
    ArkisVeriSeti m_veri;
    std::atomic_bool m_iptal{false};
};
