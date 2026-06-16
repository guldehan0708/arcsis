#include "main_window.h"

#include "arac_dialog.h"
#include "kiralama_dialog.h"
#include "musteri_dialog.h"

#include <QApplication>
#include <QChart>
#include <QChartView>
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLegend>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSettings>
#include <QStatusBar>
#include <QStyle>
#include <QThread>
#include <QToolBar>
#include <QVBoxLayout>
#include <QPieSeries>

#include <algorithm>

namespace {

constexpr auto AracDosyasi = "arkis_gui_araclar.bin";
constexpr auto MusteriDosyasi = "arkis_gui_musteriler.bin";
constexpr auto SozlesmeDosyasi = "arkis_gui_sozlesmeler.bin";

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

QJsonObject musteriJson(const Musteri& musteri) {
    return {
        {"tc_no", QString::fromStdString(musteri.tc_no)},
        {"isim", QString::fromStdString(musteri.isim)},
        {"soyisim", QString::fromStdString(musteri.soyisim)},
        {"telefon", QString::fromStdString(musteri.telefon)},
        {"ehliyet_no", QString::fromStdString(musteri.ehliyet_no)}
    };
}

QJsonObject sozlesmeJson(const KiralamaSozlesmesi& sozlesme) {
    return {
        {"sozlesme_id", sozlesme.sozlesme_id},
        {"plaka", QString::fromStdString(sozlesme.plaka)},
        {"tc_no", QString::fromStdString(sozlesme.tc_no)},
        {"baslangic_tarihi", QString::fromStdString(sozlesme.baslangic_tarihi)},
        {"bitis_tarihi", QString::fromStdString(sozlesme.bitis_tarihi.value_or(""))},
        {"toplam_tutar", sozlesme.toplam_tutar}
    };
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    verileriYukle();
    arayuzuKur();
    modelleriYenile();
    ayarlariYukle();
}

MainWindow::~MainWindow() {
    ayarlariKaydet();
}

void MainWindow::arayuzuKur() {
    setWindowTitle("ARKIS - Arac Kiralama Sistemi");

    m_aracModel = new AracModel(this);
    m_musteriModel = new MusteriModel(this);
    m_kiralamaModel = new KiralamaModel(this);

    m_aracProxy = new QSortFilterProxyModel(this);
    m_musteriProxy = new QSortFilterProxyModel(this);
    m_kiralamaProxy = new QSortFilterProxyModel(this);
    m_aracProxy->setSourceModel(m_aracModel);
    m_musteriProxy->setSourceModel(m_musteriModel);
    m_kiralamaProxy->setSourceModel(m_kiralamaModel);
    m_aracProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_musteriProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_kiralamaProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_aracProxy->setFilterKeyColumn(-1);
    m_musteriProxy->setFilterKeyColumn(-1);
    m_kiralamaProxy->setFilterKeyColumn(-1);

    m_tabs = new QTabWidget(this);
    m_aracTable = new QTableView(this);
    m_musteriTable = new QTableView(this);
    m_kiralamaTable = new QTableView(this);
    m_aracTable->setModel(m_aracProxy);
    m_musteriTable->setModel(m_musteriProxy);
    m_kiralamaTable->setModel(m_kiralamaProxy);
    for (auto* table : {m_aracTable, m_musteriTable, m_kiralamaTable}) {
        table->setSortingEnabled(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->horizontalHeader()->setStretchLastSection(true);
    }

    auto* aracArama = new QLineEdit(this);
    auto* musteriArama = new QLineEdit(this);
    auto* kiralamaArama = new QLineEdit(this);
    aracArama->setPlaceholderText("Ara");
    musteriArama->setPlaceholderText("Ara");
    kiralamaArama->setPlaceholderText("Ara");
    connect(aracArama, &QLineEdit::textChanged, m_aracProxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(musteriArama, &QLineEdit::textChanged, m_musteriProxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(kiralamaArama, &QLineEdit::textChanged, m_kiralamaProxy, &QSortFilterProxyModel::setFilterFixedString);

    m_tabs->addTab(tabloSekmesi(m_aracTable, aracArama), "Araclar");
    m_tabs->addTab(tabloSekmesi(m_musteriTable, musteriArama), "Musteriler");
    m_tabs->addTab(tabloSekmesi(m_kiralamaTable, kiralamaArama), "Kiralama Sozlesmeleri");
    m_chartView = new QChartView(this);
    m_tabs->addTab(m_chartView, "Gosterge Paneli");
    setCentralWidget(m_tabs);

    auto* kaydetAction = new QAction(style()->standardIcon(QStyle::SP_DialogSaveButton), "Kaydet", this);
    auto* yukleAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Yukle", this);
    auto* cikisAction = new QAction("Cikis", this);
    auto* aracEkleAction = new QAction(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Arac Ekle", this);
    auto* musteriEkleAction = new QAction("Musteri Ekle", this);
    auto* kiralamaAction = new QAction("Kiralama Olustur", this);
    auto* iadeAction = new QAction("Iade Et", this);
    auto* jsonAction = new QAction("JSON Aktar", this);
    auto* faturaAction = new QAction("Fatura", this);
    auto* raporAction = new QAction("Rapor", this);
    auto* bakimAction = new QAction("Bakim Kontrolu", this);

    connect(kaydetAction, &QAction::triggered, this, &MainWindow::kaydet);
    connect(yukleAction, &QAction::triggered, this, &MainWindow::yukle);
    connect(cikisAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(aracEkleAction, &QAction::triggered, this, &MainWindow::aracEkle);
    connect(musteriEkleAction, &QAction::triggered, this, &MainWindow::musteriEkle);
    connect(kiralamaAction, &QAction::triggered, this, &MainWindow::kiralamaOlustur);
    connect(iadeAction, &QAction::triggered, this, &MainWindow::seciliAraciIadeEt);
    connect(jsonAction, &QAction::triggered, this, &MainWindow::jsonDisaAktar);
    connect(faturaAction, &QAction::triggered, this, &MainWindow::faturaOlustur);
    connect(raporAction, &QAction::triggered, this, &MainWindow::raporUret);
    connect(bakimAction, &QAction::triggered, this, &MainWindow::bakimKontrolEt);

    auto* dosyaMenu = menuBar()->addMenu("Dosya");
    dosyaMenu->addAction(kaydetAction);
    dosyaMenu->addAction(yukleAction);
    dosyaMenu->addAction(jsonAction);
    dosyaMenu->addSeparator();
    dosyaMenu->addAction(cikisAction);

    auto* duzenMenu = menuBar()->addMenu("Duzen");
    duzenMenu->addAction(aracEkleAction);
    duzenMenu->addAction(musteriEkleAction);
    duzenMenu->addAction(kiralamaAction);
    duzenMenu->addAction(iadeAction);

    auto* islemMenu = menuBar()->addMenu("Islemler");
    islemMenu->addAction(faturaAction);
    islemMenu->addAction(raporAction);
    islemMenu->addAction(bakimAction);

    auto* yardimMenu = menuBar()->addMenu("Yardim");
    yardimMenu->addAction("Hakkinda", this, [this] {
        QMessageBox::information(this, "ARKIS", "Araç kiralama sistemi proje uygulamasi.");
    });

    auto* toolbar = addToolBar("Araclar");
    toolbar->addAction(kaydetAction);
    toolbar->addAction(yukleAction);
    toolbar->addSeparator();
    toolbar->addAction(aracEkleAction);
    toolbar->addAction(musteriEkleAction);
    toolbar->addAction(kiralamaAction);
    toolbar->addAction(iadeAction);
}

QWidget* MainWindow::tabloSekmesi(QTableView* tablo, QLineEdit* arama) {
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);
    layout->addWidget(arama);
    layout->addWidget(tablo);
    return widget;
}

void MainWindow::verileriYukle() {
    try {
        m_veri.araclar = arkis_io::dosyadan_oku<std::string, Arac>(AracDosyasi);
        m_veri.musteriler = arkis_io::dosyadan_oku<std::string, Musteri>(MusteriDosyasi);
        m_veri.sozlesmeler = arkis_io::dosyadan_oku<int, KiralamaSozlesmesi>(SozlesmeDosyasi);
    } catch (...) {
        m_veri = {};
    }

    if (m_veri.araclar.boyut() == 0 &&
        m_veri.musteriler.boyut() == 0 &&
        m_veri.sozlesmeler.boyut() == 0)
    {
        ornekVerileriEkle(m_veri);
    }
}

void MainWindow::modelleriYenile() {
    m_aracModel->setAraclar(m_veri.araclar);
    m_musteriModel->setMusteriler(m_veri.musteriler);
    m_kiralamaModel->setSozlesmeler(m_veri.sozlesmeler);
    durumOzetiniYenile();
    grafikYenile();
}

void MainWindow::durumOzetiniYenile() {
    int musait = 0;
    int kirada = 0;
    int bakimda = 0;
    for (const auto& arac : m_veri.araclar.tumunu_al() | std::views::values) {
        switch (arac.durum) {
            case AracDurum::Musait: ++musait; break;
            case AracDurum::Kirada: ++kirada; break;
            case AracDurum::Bakimda: ++bakimda; break;
        }
    }
    statusBar()->showMessage(
        QString("Musait: %1 | Kirada: %2 | Bakimda: %3").arg(musait).arg(kirada).arg(bakimda));
}

void MainWindow::grafikYenile() {
    int musait = 0;
    int kirada = 0;
    int bakimda = 0;
    for (const auto& arac : m_veri.araclar.tumunu_al() | std::views::values) {
        switch (arac.durum) {
            case AracDurum::Musait: ++musait; break;
            case AracDurum::Kirada: ++kirada; break;
            case AracDurum::Bakimda: ++bakimda; break;
        }
    }

    auto* series = new QPieSeries;
    series->append("Musait", musait);
    series->append("Kirada", kirada);
    series->append("Bakimda", bakimda);

    auto* chart = new QChart;
    chart->addSeries(series);
    chart->setTitle("Arac Durumu Dagilimi");
    chart->legend()->setVisible(true);
    m_chartView->setChart(chart);
}

void MainWindow::aracEkle() {
    AracDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto arac = dialog.arac();
    if (!m_veri.araclar.ekle(arac.plaka, arac)) {
        QMessageBox::warning(this, "Kayit Var", "Bu plaka zaten kayitli.");
        return;
    }
    modelleriYenile();
}

void MainWindow::musteriEkle() {
    MusteriDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const auto musteri = dialog.musteri();
    if (!m_veri.musteriler.ekle(musteri.tc_no, musteri)) {
        QMessageBox::warning(this, "Kayit Var", "Bu TC no zaten kayitli.");
        return;
    }
    modelleriYenile();
}

void MainWindow::kiralamaOlustur() {
    KiralamaDialog dialog(m_veri, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const int id = siradakiSozlesmeId(m_veri.sozlesmeler);
    const auto sozlesme = dialog.sozlesme(id);
    m_veri.sozlesmeler.ekle(id, sozlesme);

    if (auto arac = m_veri.araclar.bul(sozlesme.plaka)) {
        arac->durum = sozlesme.bitis_tarihi ? AracDurum::Musait : AracDurum::Kirada;
        m_veri.araclar.sil(arac->plaka);
        m_veri.araclar.ekle(arac->plaka, *arac);
    }
    modelleriYenile();
}

void MainWindow::seciliAraciIadeEt() {
    const auto selection = m_kiralamaTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "Secim Yok", "Iade icin bir sozlesme secin.");
        return;
    }

    const auto sourceIndex = m_kiralamaProxy->mapToSource(selection.first());
    auto sozlesme = m_kiralamaModel->sozlesmeAt(sourceIndex.row());
    if (sozlesme.bitis_tarihi) {
        QMessageBox::information(this, "Tamamlanmis", "Bu sozlesme zaten tamamlanmis.");
        return;
    }

    const auto bugun = QDate::currentDate();
    sozlesme.bitis_tarihi = bugun.toString(Qt::ISODate).toStdString();
    if (const auto arac = m_veri.araclar.bul(sozlesme.plaka)) {
        const auto baslangic = QDate::fromString(QString::fromStdString(sozlesme.baslangic_tarihi), Qt::ISODate);
        const auto gun = std::max<qint64>(1, baslangic.daysTo(bugun));
        sozlesme.toplam_tutar = static_cast<double>(gun) * arac->gunluk_ucret;

        auto guncelArac = *arac;
        guncelArac.durum = AracDurum::Musait;
        m_veri.araclar.sil(guncelArac.plaka);
        m_veri.araclar.ekle(guncelArac.plaka, guncelArac);
    }

    m_veri.sozlesmeler.sil(sozlesme.sozlesme_id);
    m_veri.sozlesmeler.ekle(sozlesme.sozlesme_id, sozlesme);
    modelleriYenile();
}

void MainWindow::kaydet() {
    const bool tamam = arkis_io::dosyaya_kaydet(AracDosyasi, m_veri.araclar) &&
        arkis_io::dosyaya_kaydet(MusteriDosyasi, m_veri.musteriler) &&
        arkis_io::dosyaya_kaydet(SozlesmeDosyasi, m_veri.sozlesmeler);
    statusBar()->showMessage(tamam ? "Veriler kaydedildi." : "Kaydetme basarisiz oldu.", 4000);
}

void MainWindow::yukle() {
    verileriYukle();
    modelleriYenile();
    statusBar()->showMessage("Veriler yuklendi.", 4000);
}

void MainWindow::jsonDisaAktar() {
    const auto hedef = QFileDialog::getSaveFileName(this, "JSON disa aktar", "arkis_export.json", "JSON (*.json)");
    if (hedef.isEmpty()) {
        return;
    }

    QJsonArray araclar;
    QJsonArray musteriler;
    QJsonArray sozlesmeler;
    for (const auto& arac : m_veri.araclar.tumunu_al() | std::views::values) {
        araclar.append(aracJson(arac));
    }
    for (const auto& musteri : m_veri.musteriler.tumunu_al() | std::views::values) {
        musteriler.append(musteriJson(musteri));
    }
    for (const auto& sozlesme : m_veri.sozlesmeler.tumunu_al() | std::views::values) {
        sozlesmeler.append(sozlesmeJson(sozlesme));
    }

    QJsonObject kok{{"araclar", araclar}, {"musteriler", musteriler}, {"sozlesmeler", sozlesmeler}};
    QFile dosya(hedef);
    if (!dosya.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Yazma Hatasi", "JSON dosyasi yazilamadi.");
        return;
    }
    dosya.write(QJsonDocument(kok).toJson(QJsonDocument::Indented));
    statusBar()->showMessage("JSON disa aktarildi.", 4000);
}

void MainWindow::faturaOlustur() {
    arkaPlanGoreviBaslat(ArkaPlanGorevi::Fatura, "Toplu fatura olusturma");
}

void MainWindow::raporUret() {
    arkaPlanGoreviBaslat(ArkaPlanGorevi::Rapor, "Rapor uretimi");
}

void MainWindow::bakimKontrolEt() {
    arkaPlanGoreviBaslat(ArkaPlanGorevi::Bakim, "Bakim takvimi kontrolu");
}

void MainWindow::arkaPlanGoreviBaslat(ArkaPlanGorevi gorev, const QString& baslik) {
    auto* thread = new QThread(this);
    auto* worker = new BackgroundTaskWorker(gorev, m_veri);
    auto* progress = new QProgressDialog(baslik, "Iptal", 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setAutoClose(false);
    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &BackgroundTaskWorker::calistir);
    connect(progress, &QProgressDialog::canceled, worker, &BackgroundTaskWorker::iptalEt);
    connect(worker, &BackgroundTaskWorker::ilerlemeGuncellendi, progress, &QProgressDialog::setValue);
    connect(worker, &BackgroundTaskWorker::tamamlandi, this, [this, progress, thread](const QString& mesaj) {
        progress->setValue(100);
        progress->deleteLater();
        QMessageBox::information(this, "Islem Tamamlandi", mesaj);
        thread->quit();
    });
    connect(worker, &BackgroundTaskWorker::hataOlustu, this, [this, progress, thread](const QString& mesaj) {
        progress->deleteLater();
        QMessageBox::warning(this, "Arka Plan Hatasi", mesaj);
        thread->quit();
    });
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    progress->show();
    thread->start();
}

void MainWindow::ayarlariYukle() {
    QSettings settings("ARKIS", "ARKIS");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::ayarlariKaydet() const {
    QSettings settings("ARKIS", "ARKIS");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}
