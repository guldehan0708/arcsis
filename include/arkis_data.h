#pragma once

#include "depo.h"
#include "varliklar.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

struct ArkisVeriSeti {
    Depo<std::string, Arac> araclar;
    Depo<std::string, Musteri> musteriler;
    Depo<int, KiralamaSozlesmesi> sozlesmeler;
};

inline void ornekVerileriEkle(ArkisVeriSeti& veri) {
    veri.araclar.ekle("34ABC123", {
        "34ABC123", "Toyota", "Corolla",
        2023, "Benzin", 850.0, AracDurum::Musait
    });
    veri.araclar.ekle("06DEF456", {
        "06DEF456", "Volkswagen", "Passat",
        2022, "Dizel", 1100.0, AracDurum::Kirada
    });
    veri.araclar.ekle("35GHI789", {
        "35GHI789", "Renault", "Clio",
        2024, "Benzin", 650.0, AracDurum::Musait
    });
    veri.araclar.ekle("16JKL012", {
        "16JKL012", "Ford", "Focus",
        2021, "Dizel", 750.0, AracDurum::Bakimda
    });

    veri.musteriler.ekle("12345678901", {
        "12345678901", "Ali", "Yilmaz", "555-0101", "B-123456"
    });
    veri.musteriler.ekle("98765432109", {
        "98765432109", "Zeynep", "Kara", "555-0102", "B-654321"
    });
    veri.musteriler.ekle("11223344556", {
        "11223344556", "Mehmet", "Demir", "555-0103", "B-112233"
    });

    veri.sozlesmeler.ekle(1, {1, "06DEF456", "12345678901",
        "2025-04-01", "2025-04-05", 4400.0});
    veri.sozlesmeler.ekle(2, {2, "34ABC123", "98765432109",
        "2025-04-10", std::nullopt, 0.0});
    veri.sozlesmeler.ekle(3, {3, "35GHI789", "11223344556",
        "2025-03-20", "2025-03-25", 3250.0});
}

inline int siradakiSozlesmeId(const Depo<int, KiralamaSozlesmesi>& sozlesmeler) {
    if (sozlesmeler.tumunu_al().empty()) {
        return 1;
    }
    return sozlesmeler.tumunu_al().rbegin()->first + 1;
}

inline std::vector<Arac> musaitAraclar(const Depo<std::string, Arac>& araclar) {
    return araclar.filtrele([](const Arac& arac) {
        return arac.durum == AracDurum::Musait;
    });
}

inline std::set<std::string> musaitPlakalar(const Depo<std::string, Arac>& araclar) {
    std::set<std::string> plakalar;
    for (const auto& arac : musaitAraclar(araclar)) {
        plakalar.insert(arac.plaka);
    }
    return plakalar;
}

inline std::map<std::string, std::vector<KiralamaSozlesmesi>> takvimGorunumu(
    const Depo<int, KiralamaSozlesmesi>& sozlesmeler)
{
    std::map<std::string, std::vector<KiralamaSozlesmesi>> takvim;
    for (const auto& sozlesme : sozlesmeler.tumunu_al() | std::views::values) {
        takvim[sozlesme.baslangic_tarihi].push_back(sozlesme);
    }
    return takvim;
}

inline double toplamTamamlananTutar(const Depo<int, KiralamaSozlesmesi>& sozlesmeler) {
    return std::accumulate(
        sozlesmeler.tumunu_al().begin(),
        sozlesmeler.tumunu_al().end(),
        0.0,
        [](double toplam, const auto& kayit) {
            const auto& sozlesme = kayit.second;
            return sozlesme.bitis_tarihi ? toplam + sozlesme.toplam_tutar : toplam;
        });
}

inline std::vector<KiralamaSozlesmesi> musteriGecmisi(
    const Depo<int, KiralamaSozlesmesi>& sozlesmeler,
    const std::string& tc_no)
{
    return sozlesmeler.filtrele([&tc_no](const KiralamaSozlesmesi& sozlesme) {
        return sozlesme.tc_no == tc_no;
    });
}

inline std::optional<std::string> enCokKiralananArac(
    const Depo<int, KiralamaSozlesmesi>& sozlesmeler)
{
    std::map<std::string, int> sayac;
    for (const auto& sozlesme : sozlesmeler.tumunu_al() | std::views::values) {
        ++sayac[sozlesme.plaka];
    }

    const auto enCok = std::ranges::max_element(
        sayac,
        [](const auto& lhs, const auto& rhs) {
            return lhs.second < rhs.second;
        });

    if (enCok == sayac.end()) {
        return std::nullopt;
    }
    return enCok->first;
}

namespace arkis_io {

inline void yaz(std::ostream& os, int deger) {
    os.write(reinterpret_cast<const char*>(&deger), sizeof(deger));
}

inline void yaz(std::ostream& os, std::uint64_t deger) {
    os.write(reinterpret_cast<const char*>(&deger), sizeof(deger));
}

inline void yaz(std::ostream& os, double deger) {
    os.write(reinterpret_cast<const char*>(&deger), sizeof(deger));
}

inline void yaz(std::ostream& os, const std::string& deger) {
    yaz(os, static_cast<std::uint64_t>(deger.size()));
    os.write(deger.data(), static_cast<std::streamsize>(deger.size()));
}

inline void yaz(std::ostream& os, AracDurum durum) {
    yaz(os, static_cast<int>(durum));
}

inline void yaz(std::ostream& os, const std::optional<std::string>& deger) {
    const int var = deger.has_value() ? 1 : 0;
    yaz(os, var);
    if (deger) {
        yaz(os, *deger);
    }
}

inline void yaz(std::ostream& os, const Arac& arac) {
    yaz(os, arac.plaka);
    yaz(os, arac.marka);
    yaz(os, arac.model);
    yaz(os, arac.yil);
    yaz(os, arac.yakit_tipi);
    yaz(os, arac.gunluk_ucret);
    yaz(os, arac.durum);
}

inline void yaz(std::ostream& os, const Musteri& musteri) {
    yaz(os, musteri.tc_no);
    yaz(os, musteri.isim);
    yaz(os, musteri.soyisim);
    yaz(os, musteri.telefon);
    yaz(os, musteri.ehliyet_no);
}

inline void yaz(std::ostream& os, const KiralamaSozlesmesi& sozlesme) {
    yaz(os, sozlesme.sozlesme_id);
    yaz(os, sozlesme.plaka);
    yaz(os, sozlesme.tc_no);
    yaz(os, sozlesme.baslangic_tarihi);
    yaz(os, sozlesme.bitis_tarihi);
    yaz(os, sozlesme.toplam_tutar);
}

template <typename T>
T oku(std::istream& is);

template <>
inline int oku<int>(std::istream& is) {
    int deger{};
    is.read(reinterpret_cast<char*>(&deger), sizeof(deger));
    return deger;
}

template <>
inline std::uint64_t oku<std::uint64_t>(std::istream& is) {
    std::uint64_t deger{};
    is.read(reinterpret_cast<char*>(&deger), sizeof(deger));
    return deger;
}

template <>
inline double oku<double>(std::istream& is) {
    double deger{};
    is.read(reinterpret_cast<char*>(&deger), sizeof(deger));
    return deger;
}

template <>
inline std::string oku<std::string>(std::istream& is) {
    const auto uzunluk = oku<std::uint64_t>(is);
    std::string deger(uzunluk, '\0');
    is.read(deger.data(), static_cast<std::streamsize>(uzunluk));
    return deger;
}

template <>
inline AracDurum oku<AracDurum>(std::istream& is) {
    return static_cast<AracDurum>(oku<int>(is));
}

template <>
inline std::optional<std::string> oku<std::optional<std::string>>(std::istream& is) {
    const bool var = oku<int>(is) != 0;
    if (!var) {
        return std::nullopt;
    }
    return oku<std::string>(is);
}

template <>
inline Arac oku<Arac>(std::istream& is) {
    return {
        oku<std::string>(is),
        oku<std::string>(is),
        oku<std::string>(is),
        oku<int>(is),
        oku<std::string>(is),
        oku<double>(is),
        oku<AracDurum>(is)
    };
}

template <>
inline Musteri oku<Musteri>(std::istream& is) {
    return {
        oku<std::string>(is),
        oku<std::string>(is),
        oku<std::string>(is),
        oku<std::string>(is),
        oku<std::string>(is)
    };
}

template <>
inline KiralamaSozlesmesi oku<KiralamaSozlesmesi>(std::istream& is) {
    return {
        oku<int>(is),
        oku<std::string>(is),
        oku<std::string>(is),
        oku<std::string>(is),
        oku<std::optional<std::string>>(is),
        oku<double>(is)
    };
}

template <typename Anahtar, typename Deger>
bool dosyaya_kaydet(const std::filesystem::path& dosya_yolu,
                    const Depo<Anahtar, Deger>& depo)
{
    std::ofstream dosya(dosya_yolu, std::ios::binary);
    if (!dosya) {
        return false;
    }

    yaz(dosya, static_cast<std::uint64_t>(depo.boyut()));
    for (const auto& [anahtar, deger] : depo.tumunu_al()) {
        yaz(dosya, anahtar);
        yaz(dosya, deger);
    }
    return static_cast<bool>(dosya);
}

template <typename Anahtar, typename Deger>
Depo<Anahtar, Deger> dosyadan_oku(const std::filesystem::path& dosya_yolu)
{
    Depo<Anahtar, Deger> depo;
    if (!std::filesystem::exists(dosya_yolu)) {
        return depo;
    }

    std::ifstream dosya(dosya_yolu, std::ios::binary);
    if (!dosya) {
        throw std::runtime_error("Dosya acilamadi: " + dosya_yolu.string());
    }

    const auto kayitSayisi = oku<std::uint64_t>(dosya);
    for (std::uint64_t i = 0; i < kayitSayisi; ++i) {
        const auto anahtar = oku<Anahtar>(dosya);
        const auto deger = oku<Deger>(dosya);
        depo.ekle(anahtar, deger);
    }

    if (!dosya) {
        throw std::runtime_error("Dosya okunurken hata olustu: " + dosya_yolu.string());
    }
    return depo;
}

} // namespace arkis_io
