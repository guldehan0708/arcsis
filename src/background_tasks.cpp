#include "background_tasks.h"

#include <QThread>

namespace {

int oran(std::size_t index, std::size_t toplam) {
    if (toplam == 0) {
        return 100;
    }
    return static_cast<int>(((index + 1) * 100) / toplam);
}

} // namespace

BackgroundTaskWorker::BackgroundTaskWorker(ArkaPlanGorevi gorev, ArkisVeriSeti veri)
    : m_gorev(gorev),
      m_veri(std::move(veri))
{
}

void BackgroundTaskWorker::calistir() {
    try {
        switch (m_gorev) {
            case ArkaPlanGorevi::Fatura: {
                const auto toplam = m_veri.sozlesmeler.boyut();
                int aktif = 0;
                std::size_t index = 0;
                for (const auto& sozlesme : m_veri.sozlesmeler.tumunu_al() | std::views::values) {
                    if (m_iptal) {
                        emit tamamlandi("Fatura olusturma iptal edildi.");
                        return;
                    }
                    if (!sozlesme.bitis_tarihi) {
                        ++aktif;
                    }
                    QThread::msleep(120);
                    emit ilerlemeGuncellendi(oran(index++, toplam));
                }
                emit tamamlandi(QString("%1 aktif sozlesme icin fatura hazirlandi.").arg(aktif));
                break;
            }
            case ArkaPlanGorevi::Rapor: {
                const auto toplam = m_veri.sozlesmeler.boyut();
                double tutar = 0.0;
                std::size_t index = 0;
                for (const auto& sozlesme : m_veri.sozlesmeler.tumunu_al() | std::views::values) {
                    if (m_iptal) {
                        emit tamamlandi("Rapor uretimi iptal edildi.");
                        return;
                    }
                    tutar += sozlesme.toplam_tutar;
                    QThread::msleep(120);
                    emit ilerlemeGuncellendi(oran(index++, toplam));
                }
                emit tamamlandi(QString("Rapor hazir: toplam ciro %1 TL.").arg(tutar, 0, 'f', 2));
                break;
            }
            case ArkaPlanGorevi::Bakim: {
                const auto toplam = m_veri.araclar.boyut();
                int bakimda = 0;
                std::size_t index = 0;
                for (const auto& arac : m_veri.araclar.tumunu_al() | std::views::values) {
                    if (m_iptal) {
                        emit tamamlandi("Bakim takvimi kontrolu iptal edildi.");
                        return;
                    }
                    if (arac.durum == AracDurum::Bakimda) {
                        ++bakimda;
                    }
                    QThread::msleep(120);
                    emit ilerlemeGuncellendi(oran(index++, toplam));
                }
                emit tamamlandi(QString("%1 arac bakim takibinde.").arg(bakimda));
                break;
            }
        }
    } catch (const std::exception& hata) {
        emit hataOlustu(QString::fromStdString(hata.what()));
    }
}

void BackgroundTaskWorker::iptalEt() {
    m_iptal = true;
}
