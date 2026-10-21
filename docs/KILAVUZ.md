# FUTBOL13 Kullanım Kılavuzu

FUTBOL13, PC için çıkmış *Pro Evolution Soccer 2013* (Konami, 2012) oyununun **kendi
kopyanızı**, jailbreak yapılmış bir PS5'te ana ekranda kendi kutucuğu olan bir homebrew
uygulaması olarak çalıştırır (`PPSA99405`). Oyunun PC sürümünü
[prospero-win](https://github.com/mpereiraesaa/prospero-win) üzerinden, yani Wine, DXVK ve
PS5'in Vulkan sürücüsüyle çalıştırır. Blur için aynı işi yapan
[Sahin](https://github.com/7tg/sahin) uygulamasından yapılmıştır.

**Oyunun kendi kopyanız gerekir. FUTBOL13 hiçbir oyun dosyası içermez.** Uygulamada yalnızca
kendi kodumuz, yamalarımız, ayarlarımız ve çizimlerimiz ile kaynak koddan derlenen açık
kaynaklı projeler bulunur. Uygulama hiçbir şey indirmez. FUTBOL13'ün Konami veya Sony ile bir
bağlantısı yoktur.

## Neler yapar

- PES 2013'ü kendi klasörünüzden, kendine ait bir Wine önekine (prefix) olduğu gibi kurar.
  Kopyanız hiçbir zaman değiştirilmez.
- Kendi kopyanızın klasörünü PC'de nasıl hazırladıysanız (kadro güncellemesi, Türkçe spiker
  gibi eklemelerle) öyle kurar. Yalnızca `pes2013.exe` ve `img/` bulunmalıdır; eklenen tek
  dosya bizim `dxvk.conf` dosyamızdır.
- İsterseniz kayıtlarınızı ve kadrolarınızı PC'den taşır (aşağıya bakın).
- PES'in `settings.exe` programı konsolda çalışmadığı için oyunun `settings.dat` dosyasını
  kendisi yazar: tam ekran, en yüksek kalite, kumanda XInput olarak ve sayfada seçilen
  çözünürlük.
- Kutucuğa bastığınızda oyun doğrudan açılır.
- **Yerel maç:** oturum açmış her PS5 kullanıcısı için bir kumanda.
- Dokunmatik yüzey fare gibi çalışır.

## Gerekenler

- Homebrew çalıştırabilen bir PS5: bir FTP sunucusu, 9021 numaralı yerel portta elfldr
  çalışan bir ELF yükleyici ve `/data/homebrew` klasöründen uygulama kuran
  [ShadowMount+](https://github.com/drakmor/ShadowMountPlus). Uygulama, kstuff-lite,
  ShadowMount+ ve Payload Manager kurulu 13.60 yazılımlı bir konsolda geliştirildi.
- **PES 2013 klasörünüz**, yani içinde `pes2013.exe` ve `img/` bulunan klasör, PC'de
  istediğiniz gibi hazırlanmış olarak. PC'de Wine altında açılmayan bir kopya burada da
  açılmaz: oyun açılır açılmaz kapanırsa FUTBOL13 bir sonraki açılışta bunu size söyler.
  FUTBOL13 kopya korumasını kaldırmaz, aşmaz ve bunu yapan bir şeye yönlendirmez.

## Kurulum

1. [Sürümler](https://github.com/7tg/futbol13/releases) sayfasından `PPSA99405.zip`
   dosyasını indirin ve açın.
2. İçindeki `PPSA99405` klasörünü FTP ile konsolda `/data/homebrew/PPSA99405/` konumuna
   kopyalayın. ShadowMount+ kutucuğu ekler; kutucuk görünmezse konsolu yeniden başlatın.
3. PES 2013 klasörünüzün tamamını FTP ile `/data/futbol13/base-games/PES2013` konumuna
   kopyalayın. Ya da bir USB belleğe `PES2013` adlı bir klasör olarak koyun; USB'deki kopya
   kurulum sırasında konsola aktarılır.
4. İsterseniz kayıt klasörünüzü de kopyalayın (aşağıya bakın). Bunu **kurulumdan önce**
   yapın.
5. FUTBOL13'ü açın ve **Yükle**'ye basın. FUTBOL13 oyununuzu öneke kopyalar, `dxvk.conf` ve
   `settings.dat` dosyalarını yazar ve kayıt dosyalarınızı yerleştirir. Hiçbir şey
   indirilmez.

Bundan sonra kutucuk oyunu doğrudan başlatır.

### Kayıtlarınızı ve kadronuzu taşımak

PES, PC'de ayarlarını, düzenlediğiniz kadroları, Master League'i ve bir kadro güncellemesinin
getirdiği `EDIT` dosyasını `Belgeler\KONAMI\Pro Evolution Soccer 2013\save\` klasöründe
(`Documents\KONAMI\Pro Evolution Soccer 2013\save\`) tutar. PC'de uyguladığınız bir kadro
güncellemesi çoğu zaman oyun klasöründe değil, bu klasörde durur; yani yalnızca oyun
klasörünü kopyalarsanız yeni kadrolar konsola gelmez.

Kadroyu ve kayıtlarınızı taşımak için **kurulumdan önce** bu `save` klasörünü FTP ile
`/data/futbol13/saves/PES2013/save/` konumuna kopyalayın. Kurulum içindeki dosyaları, önekin
her Windows kullanıcısı için oyunun kaydını tuttuğu yere, yalnızca orada oyunun kendi dosyası
yoksa yerleştirir. Oyunun kendi kayıt dosyaları oluştuktan sonra kurulum ya da onarım bunları
**asla** değiştirmez, yani ilerlemeniz kaybolmaz.

## Ayarlar sayfası

Kutucuk açılırken **□ (Kare)** tuşunu basılı tutarsanız oyun yerine FUTBOL13'ün sayfası
açılır:

- **Çözünürlük:** 3840×2160, 2560×1440 veya 1920×1080
- **Performans katmanı:** ekranda kare hızı ve kare süreleri (DXVK HUD)
- **Onar (△):** kurulumu onarır; kayıtlarınıza dokunmaz
- **Oyna:** oyunu başlatır

## Kontroller

- Oyunda kumanda bir Xbox 360 kumandası gibi görünür.
- **Dokunmatik yüzey** fare imlecini hareket ettirir, basınca tıklar.
- **Çıkmak için** Options tuşunu ve dokunmatik yüzeyi birlikte yaklaşık bir saniye basılı
  tutun.
- **Titreşim:** oyun istediğinde DualSense titrer.
- **Dil:** oyunun dili kendi menülerinden seçilir.
- **FUTBOL13'ün sayfası:** PS5'in dili Türkçeyse Türkçe, başka bir dilse İngilizce açılır.

## Yerel maç

1. Her ek kumandayı açın ve PS tuşuna basın.
2. O kumanda için bir kullanıcı ya da misafir seçerek oturum açın. Oturum açılmamış bir
   kumanda oyuna görünmez.
3. PES 2013'te maçı başlatın ve kumandaları takımlara dağıtın.

Dokunmatik yüzeyle fare kullanımı ve çıkış tuş kombinasyonu yalnızca birinci oyuncuya aittir.

## Sorun giderme

- **Kutucuk görünmüyor:** konsolu yeniden başlatın ve klasörün tam olarak
  `/data/homebrew/PPSA99405/` konumunda olduğunu kontrol edin.
- **Oyun açılır açılmaz kapanıyor:** kopyanızı önce PC'de Wine altında deneyin. Orada
  açılmayan bir kopya burada da açılmaz. FUTBOL13 bir sonraki açılışta bunu söyler.
- **Kurulum hata veriyor:** PES 2013 klasörünün `/data/futbol13/base-games/PES2013`
  konumunda olduğunu ve `pes2013.exe` dosyasını ve `img` klasörünü içerdiğini kontrol edin.
  Sorun sürerse sayfadaki Onar (△) ile onarın.
- **Yeni kadrolar oyunda yok:** kadro güncellemesi büyük olasılıkla PC'deki `save`
  klasöründeydi. Yukarıdaki adımla kopyalayın. Kurulum yalnızca oyunda henüz olmayan
  dosyaları ekler; var olanları değiştirmez. Oyunun kendi dosyaları
  `/data/prospero-win/prefixes/futbol13/drive_c/users/<kullanıcı>/Documents/KONAMI/Pro Evolution Soccer 2013/save/`
  konumundadır; onarımdan önce oradaki eski dosyaları FTP ile silebilirsiniz.
- **İkinci kumanda görünmüyor:** o kumandayla bir kullanıcı ya da misafir olarak oturum
  açın.
- **Çözünürlük değişmiyor:** ayarlar sayfasından seçin; FUTBOL13 bunu oyunun kendi
  `settings.dat` dosyasına da yazar.

## Bilinen sınırlamalar

- PES 2013, 32 bit x86 çevirmeni üzerinden çalıştığı için kare hızları bir PC'dekinden
  düşük olabilir.
- Çevrimiçi oyun yoktur; Konami'nin sunucuları kapalıdır.
- Konsol testleri sürüyor. Sorunları depo sayfasındaki Issues bölümüne bildirin.

## Lisans

GPL-3.0 (`LICENSE`). Derleme, katkıda bulunanlar ve dosyaların yeri için İngilizce
[README](../README.md) dosyasına bakın.
