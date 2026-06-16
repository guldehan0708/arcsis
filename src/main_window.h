#pragma once

#include "../include/arkis_data.h"
#include "arac_model.h"
#include "background_tasks.h"
#include "kiralama_model.h"
#include "musteri_model.h"

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QTableView>

class QChartView;
class QLineEdit;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void aracEkle();
    void musteriEkle();
    void kiralamaOlustur();
    void seciliAraciIadeEt();
    void kaydet();
    void yukle();
    void jsonDisaAktar();
    void faturaOlustur();
    void raporUret();
    void bakimKontrolEt();

private:
    void arayuzuKur();
    QWidget* tabloSekmesi(QTableView* tablo, QLineEdit* arama);
    void verileriYukle();
    void modelleriYenile();
    void durumOzetiniYenile();
    void grafikYenile();
    void ayarlariYukle();
    void ayarlariKaydet() const;
    void arkaPlanGoreviBaslat(ArkaPlanGorevi gorev, const QString& baslik);

    ArkisVeriSeti m_veri;
    AracModel* m_aracModel{};
    MusteriModel* m_musteriModel{};
    KiralamaModel* m_kiralamaModel{};
    QSortFilterProxyModel* m_aracProxy{};
    QSortFilterProxyModel* m_musteriProxy{};
    QSortFilterProxyModel* m_kiralamaProxy{};
    QTabWidget* m_tabs{};
    QTableView* m_aracTable{};
    QTableView* m_musteriTable{};
    QTableView* m_kiralamaTable{};
    QChartView* m_chartView{};
};
