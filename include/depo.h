#pragma once

/**
 * depo.h -- Genel Amaçlı Depo Şablon Sınıfı
 *
 * Anahtar-değer çifti olarak veri saklayan şablon sınıf.
 * Temel CRUD ve filtreleme islemlerini saglayan genel depo sinifi.
 *
 * Derleme: g++ -std=c++20 -o arkis src/main.cpp
 */

#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <vector>

template <typename Anahtar, typename Deger>
class Depo {
public:
    // Yeni kayıt ekler. Anahtar zaten varsa false döner.
    bool ekle(const Anahtar& anahtar, const Deger& deger) {
        auto [it, eklendi] = m_veriler.insert({anahtar, deger});
        return eklendi;
    }

    // Verilen anahtari map'te arar.
    std::optional<Deger> bul(const Anahtar& anahtar) const {
        const auto it = m_veriler.find(anahtar);
        if (it == m_veriler.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // Verilen anahtari map'ten siler.
    bool sil(const Anahtar& anahtar) {
        return m_veriler.erase(anahtar) > 0;
    }

    // Tüm kayıtları döner (salt okunur)
    const std::map<Anahtar, Deger>& tumunu_al() const {
        return m_veriler;
    }

    // Kayıt sayısını döner
    std::size_t boyut() const {
        return m_veriler.size();
    }

    // Depoyu temizler
    void temizle() {
        m_veriler.clear();
    }

    // Kosulu saglayan tum degerleri vector olarak dondurur.
    std::vector<Deger> filtrele(
        std::function<bool(const Deger&)> kosul) const
    {
        std::vector<Deger> sonuc;
        for (const auto& deger : m_veriler | std::views::values) {
            if (kosul(deger)) {
                sonuc.push_back(deger);
            }
        }
        return sonuc;
    }

private:
    std::map<Anahtar, Deger> m_veriler;
};
