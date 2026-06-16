#include "../include/arkis_data.h"

#include <QCoreApplication>
#include <QDate>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>

#include <algorithm>
#include <iostream>
#include <mutex>

namespace {

QJsonObject aracJson(const Arac& arac) {
    return {
        {"plaka", QString::fromStdString(arac.plaka)},
        {"marka", QString::fromStdString(arac.marka)},
        {"model", QString::fromStdString(arac.model)},
        {"yil", arac.yil},
        {"yakit_tipi", QString::fromStdString(arac.yakit_tipi)},
        {"gunluk_ucret", arac.gunluk_ucret},
        {"durum", QString::fromStdString(durumYazisi(arac.durum))}
    };
}

void yanitYolla(QTcpSocket* socket, const QJsonObject& yanit) {
    socket->write(QJsonDocument(yanit).toJson(QJsonDocument::Compact));
    socket->write("\n");
}

} // namespace

class ArkisServer : public QTcpServer {
    Q_OBJECT

public:
    explicit ArkisServer(QObject* parent = nullptr)
        : QTcpServer(parent)
    {
        ornekVerileriEkle(m_veri);
    }

protected:
    void incomingConnection(qintptr handle) override {
        auto* socket = new QTcpSocket(this);
        socket->setSocketDescriptor(handle);
        socket->setProperty("yetkili", false);
        m_soketler.push_back(socket);

        connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
            while (socket->canReadLine()) {
                const auto belge = QJsonDocument::fromJson(socket->readLine().trimmed());
                if (!belge.isObject()) {
                    yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Gecersiz JSON"}});
                    continue;
                }
                istegiIsle(socket, belge.object());
            }
        });

        connect(socket, &QTcpSocket::disconnected, this, [this, socket] {
            std::erase(m_soketler, socket);
            socket->deleteLater();
        });

        yanitYolla(socket, {{"durum", "basarili"}, {"mesaj", "ARKIS sunucusuna baglandiniz"}});
    }

private:
    void istegiIsle(QTcpSocket* socket, const QJsonObject& istek) {
        const auto tip = istek.value("tip").toString();

        if (tip == "kimlik") {
            const bool yetkili = istek.value("token").toString() == "arkis";
            socket->setProperty("yetkili", yetkili);
            yanitYolla(socket, {
                {"durum", yetkili ? "basarili" : "hata"},
                {"mesaj", yetkili ? "Kimlik dogrulandi" : "Kimlik dogrulanamadi"}
            });
            return;
        }

        if (!socket->property("yetkili").toBool()) {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Once kimlik dogrulayin"}});
            return;
        }

        if (tip == "arac_listele") {
            aracListele(socket);
        } else if (tip == "arac_kirala") {
            aracKirala(socket, istek);
        } else if (tip == "arac_iade") {
            aracIade(socket, istek);
        } else {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Bilinmeyen istek tipi"}});
        }
    }

    void aracListele(QTcpSocket* socket) {
        std::lock_guard kilit(m_mutex);
        QJsonArray araclar;
        for (const auto& arac : m_veri.araclar.tumunu_al() | std::views::values) {
            araclar.append(aracJson(arac));
        }
        yanitYolla(socket, {{"durum", "basarili"}, {"araclar", araclar}});
    }

    void aracKirala(QTcpSocket* socket, const QJsonObject& istek) {
        std::lock_guard kilit(m_mutex);
        const auto plaka = istek.value("plaka").toString().toStdString();
        const auto tc = istek.value("tc_no").toString().toStdString();

        auto arac = m_veri.araclar.bul(plaka);
        if (!arac) {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Arac bulunamadi"}});
            return;
        }
        if (!m_veri.musteriler.bul(tc)) {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Musteri bulunamadi"}});
            return;
        }
        if (arac->durum != AracDurum::Musait) {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Arac musait degil"}});
            return;
        }

        const int id = siradakiSozlesmeId(m_veri.sozlesmeler);
        const KiralamaSozlesmesi sozlesme{
            id, plaka, tc, QDate::currentDate().toString(Qt::ISODate).toStdString(),
            std::nullopt, 0.0
        };
        m_veri.sozlesmeler.ekle(id, sozlesme);
        arac->durum = AracDurum::Kirada;
        m_veri.araclar.sil(plaka);
        m_veri.araclar.ekle(plaka, *arac);

        yanitYolla(socket, {{"durum", "basarili"}, {"mesaj", "Arac kiralandi"}, {"sozlesme_id", id}});
        durumYayinla(plaka, *arac);
    }

    void aracIade(QTcpSocket* socket, const QJsonObject& istek) {
        std::lock_guard kilit(m_mutex);
        const auto plaka = istek.value("plaka").toString().toStdString();
        auto arac = m_veri.araclar.bul(plaka);
        if (!arac) {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Arac bulunamadi"}});
            return;
        }

        std::optional<KiralamaSozlesmesi> aktif;
        for (const auto& sozlesme : m_veri.sozlesmeler.tumunu_al() | std::views::values) {
            if (sozlesme.plaka == plaka && !sozlesme.bitis_tarihi) {
                aktif = sozlesme;
                break;
            }
        }

        if (!aktif) {
            yanitYolla(socket, {{"durum", "hata"}, {"mesaj", "Aktif sozlesme yok"}});
            return;
        }

        const auto bugun = QDate::currentDate();
        const auto baslangic = QDate::fromString(QString::fromStdString(aktif->baslangic_tarihi), Qt::ISODate);
        const auto gun = std::max<qint64>(1, baslangic.daysTo(bugun));
        aktif->bitis_tarihi = bugun.toString(Qt::ISODate).toStdString();
        aktif->toplam_tutar = static_cast<double>(gun) * arac->gunluk_ucret;
        m_veri.sozlesmeler.sil(aktif->sozlesme_id);
        m_veri.sozlesmeler.ekle(aktif->sozlesme_id, *aktif);

        arac->durum = AracDurum::Musait;
        m_veri.araclar.sil(plaka);
        m_veri.araclar.ekle(plaka, *arac);

        yanitYolla(socket, {{"durum", "basarili"}, {"mesaj", "Arac iade edildi"},
                            {"toplam_tutar", aktif->toplam_tutar}});
        durumYayinla(plaka, *arac);
    }

    void durumYayinla(const std::string& plaka, const Arac& arac) {
        const QJsonObject olay{
            {"tip", "arac_guncellendi"},
            {"plaka", QString::fromStdString(plaka)},
            {"arac", aracJson(arac)}
        };
        for (auto* socket : m_soketler) {
            if (socket->property("yetkili").toBool()) {
                yanitYolla(socket, olay);
            }
        }
    }

    ArkisVeriSeti m_veri;
    std::mutex m_mutex;
    std::vector<QTcpSocket*> m_soketler;
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    const quint16 port = argc > 1 ? QString::fromLocal8Bit(argv[1]).toUShort() : 45454;

    ArkisServer server;
    if (!server.listen(QHostAddress::Any, port)) {
        std::cerr << "Sunucu baslatilamadi: " << server.errorString().toStdString() << "\n";
        return 1;
    }

    std::cout << "ARKIS sunucu " << port << " portunda calisiyor.\n";
    return app.exec();
}

#include "arkis_server_main.moc"
