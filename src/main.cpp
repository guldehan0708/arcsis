#include "../include/arkis_data.h"

#include <filesystem>
#include <iomanip>
#include <iostream>

namespace {

constexpr auto AracDosyasi = "arkis_araclar.bin";

ArkisVeriSeti veriSetiniHazirla() {
    ArkisVeriSeti veri;

    try {
        veri.araclar = arkis_io::dosyadan_oku<std::string, Arac>(AracDosyasi);
    } catch (const std::exception& hata) {
        std::cerr << "Uyari: " << hata.what() << "\n";
    }

    if (veri.araclar.boyut() == 0) {
        ornekVerileriEkle(veri);
    } else {
        ArkisVeriSeti ornek;
        ornekVerileriEkle(ornek);
        veri.musteriler = ornek.musteriler;
        veri.sozlesmeler = ornek.sozlesmeler;
    }

    return veri;
}

void araclariListele(const Depo<std::string, Arac>& araclar) {
    for (const auto& [plaka, arac] : araclar.tumunu_al()) {
        std::cout << "  " << arac << "\n";
    }
}

void sozlesmeleriListele(const std::vector<KiralamaSozlesmesi>& sozlesmeler) {
    if (sozlesmeler.empty()) {
        std::cout << "  Kayit bulunamadi.\n";
        return;
    }

    for (const auto& sozlesme : sozlesmeler) {
        std::cout << "  " << sozlesme << "\n";
    }
}

} // namespace

int main() {
    std::cout << "=== ARKIS - Arac Kiralama Sistemi ===\n\n";

    auto veri = veriSetiniHazirla();

    std::cout << "Arac sayisi: " << veri.araclar.boyut() << "\n\n";

    std::cout << "--- Arac Katalogu ---\n";
    araclariListele(veri.araclar);
    std::cout << "\n";

    std::cout << "Musteri sayisi: " << veri.musteriler.boyut() << "\n\n";

    std::cout << "--- Kiralama Sozlesmeleri ---\n";
    for (const auto& [id, sozlesme] : veri.sozlesmeler.tumunu_al()) {
        std::cout << "  " << sozlesme << "\n";
    }
    std::cout << "\n";

    std::cout << "--- Musait Araclar ---\n";
    for (const auto& arac : musaitAraclar(veri.araclar)) {
        std::cout << "  " << arac << "\n";
    }

    std::cout << "Benzersiz musait plakalar: ";
    for (const auto& plaka : musaitPlakalar(veri.araclar)) {
        std::cout << plaka << " ";
    }
    std::cout << "\n\n";

    std::cout << "--- Takvim Gorunumu ---\n";
    for (const auto& [tarih, sozlesmeler] : takvimGorunumu(veri.sozlesmeler)) {
        std::cout << tarih << ":\n";
        sozlesmeleriListele(sozlesmeler);
    }
    std::cout << "\n";

    std::cout << "--- Toplam Tamamlanan Kiralama Tutari ---\n";
    std::cout << std::fixed << std::setprecision(2)
              << toplamTamamlananTutar(veri.sozlesmeler) << " TL\n\n";

    const std::string arananTc = "12345678901";
    std::cout << "--- Musteri Kiralama Gecmisi (" << arananTc << ") ---\n";
    sozlesmeleriListele(musteriGecmisi(veri.sozlesmeler, arananTc));
    std::cout << "\n";

    std::cout << "--- En Cok Kiralanan Arac ---\n";
    if (const auto plaka = enCokKiralananArac(veri.sozlesmeler)) {
        std::cout << "  " << *plaka << "\n\n";
    } else {
        std::cout << "  Henuz kiralama yok.\n\n";
    }

    std::cout << "--- Dosya Islemleri ---\n";
    if (arkis_io::dosyaya_kaydet(AracDosyasi, veri.araclar)) {
        const auto okunanAraclar =
            arkis_io::dosyadan_oku<std::string, Arac>(AracDosyasi);
        std::cout << "  " << AracDosyasi << " dosyasina kaydedildi ve "
                  << okunanAraclar.boyut() << " arac geri okundu.\n";
    } else {
        std::cout << "  Dosyaya kaydetme basarisiz oldu.\n";
    }

    std::cout << "\n=== Program sona erdi ===\n";
    return 0;
}
