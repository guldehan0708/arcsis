#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSocketNotifier>
#include <QTcpSocket>
#include <QTextStream>
#include <QTimer>

#include <iostream>
#include <utility>
#include <cstdio>

class ArkisClient : public QObject {
    Q_OBJECT

public:
    ArkisClient(QString host, quint16 port, QObject* parent = nullptr)
        : QObject(parent),
          m_host(std::move(host)),
          m_port(port)
    {
        connect(&m_socket, &QTcpSocket::connected, this, &ArkisClient::baglandi);
        connect(&m_socket, &QTcpSocket::readyRead, this, &ArkisClient::oku);
        connect(&m_socket, &QTcpSocket::disconnected, this, &ArkisClient::yenidenBaglan);
        connect(&m_socket, &QTcpSocket::errorOccurred, this, [this] {
            std::cerr << "Baglanti hatasi: " << m_socket.errorString().toStdString() << "\n";
        });
    }

    void baslat() {
        baglan();
        m_stdin = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
        connect(m_stdin, &QSocketNotifier::activated, this, &ArkisClient::komutOku);
        yardimYaz();
    }

private slots:
    void baglandi() {
        std::cout << "Sunucuya baglandi.\n";
        yolla({{"tip", "kimlik"}, {"token", "arkis"}});
    }

    void oku() {
        while (m_socket.canReadLine()) {
            const auto satir = m_socket.readLine().trimmed();
            const auto belge = QJsonDocument::fromJson(satir);
            std::cout << QJsonDocument(belge.object()).toJson(QJsonDocument::Indented).toStdString();
        }
    }

    void komutOku() {
        QTextStream stream(stdin);
        const auto satir = stream.readLine().trimmed();
        const auto parcalar = satir.split(' ', Qt::SkipEmptyParts);
        if (parcalar.isEmpty()) {
            return;
        }

        const auto komut = parcalar[0].toLower();
        if (komut == "listele") {
            yolla({{"tip", "arac_listele"}});
        } else if (komut == "kirala" && parcalar.size() == 3) {
            yolla({{"tip", "arac_kirala"}, {"tc_no", parcalar[1]}, {"plaka", parcalar[2].toUpper()}});
        } else if (komut == "iade" && parcalar.size() == 2) {
            yolla({{"tip", "arac_iade"}, {"plaka", parcalar[1].toUpper()}});
        } else if (komut == "yardim") {
            yardimYaz();
        } else if (komut == "cikis") {
            m_kapanis = true;
            m_socket.disconnectFromHost();
            QCoreApplication::quit();
        } else {
            std::cout << "Bilinmeyen komut. 'yardim' yazin.\n";
        }
    }

    void yenidenBaglan() {
        if (m_kapanis) {
            return;
        }
        std::cout << "Baglanti koptu, 3 saniye sonra yeniden denenecek.\n";
        QTimer::singleShot(3000, this, &ArkisClient::baglan);
    }

private:
    void baglan() {
        if (m_socket.state() == QAbstractSocket::ConnectedState ||
            m_socket.state() == QAbstractSocket::ConnectingState)
        {
            return;
        }
        m_socket.connectToHost(m_host, m_port);
    }

    void yolla(const QJsonObject& istek) {
        if (m_socket.state() != QAbstractSocket::ConnectedState) {
            std::cout << "Sunucuya bagli degil.\n";
            return;
        }
        m_socket.write(QJsonDocument(istek).toJson(QJsonDocument::Compact));
        m_socket.write("\n");
    }

    void yardimYaz() const {
        std::cout << "Komutlar:\n"
                  << "  listele\n"
                  << "  kirala <tc_no> <plaka>\n"
                  << "  iade <plaka>\n"
                  << "  cikis\n";
    }

    QString m_host;
    quint16 m_port;
    QTcpSocket m_socket;
    QSocketNotifier* m_stdin{};
    bool m_kapanis{false};
};

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    const QString host = argc > 1 ? QString::fromLocal8Bit(argv[1]) : "127.0.0.1";
    const quint16 port = argc > 2 ? QString::fromLocal8Bit(argv[2]).toUShort() : 45454;

    ArkisClient client(host, port);
    client.baslat();
    return app.exec();
}

#include "arkis_client_main.moc"
