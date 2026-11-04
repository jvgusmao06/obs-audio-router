# OBS Audio Router Plugin

A native, ultra-lightweight plugin for [OBS Studio](https://obsproject.com/) that enables streamers and content creators to separate and route application audio into dedicated thematic channels, functioning similarly to dedicated hardware mixers with near-zero CPU and RAM overhead.

---

> [!IMPORTANT]
> ### 🚀 Super Easy Installation (Only 1 File: `obs-audio-router.dll`)
>
> 1. **Download**:
>    - Go to the **[Releases](../../releases)** tab.
>    - Download **`obs-audio-router.dll`** (or `obs-audio-router-windows-x64.zip`).
>
> 2. **Drop into your OBS plugins folder**:
>    - Close OBS Studio if open.
>    - Copy `obs-audio-router.dll` directly into:
>      ```text
>      C:\Program Files\obs-studio\obs-plugins\64bit\
>      ```
>
> 3. **Open OBS Studio**:
>    - Launch OBS Studio.
>    - Open top menu: **Docks (Painéis)** → **Audio Router**.
>    - Done! The plugin is 100% self-contained in that single `.dll` (embedded translations for Portuguese, English, Spanish, French, and Russian included).

---

## 🎯 Goal & Features

Eliminate complex external virtual audio cables. This plugin manages Windows audio sessions internally and routes them into categorized buses directly inside OBS.

- 📦 **100% Standalone Single DLL**: Zero external runtime files or drivers required. Just drop `obs-audio-router.dll` into OBS!
- ⚡ **Ultra-Lightweight**: Built with native C++20 and Qt, consuming **less than 5 MB of RAM** and **~0.0% CPU** (compared to 300MB–600MB+ of background software suites).
- ✨ **Smart Out-of-the-Box Auto-Routing**:
  - 🎮 **Game**: Automatically detects games (`cs2.exe`, `valorant.exe`, etc.). Game launchers (Steam, Epic, Riot Client) default to System to keep your game channel clean.
  - 💬 **Voice / Chat**: Pre-configured for communication tools (`discord.exe`, `slack.exe`, `teamspeak.exe`, `whatsapp.exe`, etc.).
  - 🎵 **Media / Music**: Pre-configured for music players and streaming apps (`spotify.exe`, `vlc.exe`, `tidal.exe`, `music.ui.exe`, etc.).
  - ⚙️ **System**: Fallback channel for miscellaneous apps and Windows system sounds.
- 👂 **Stream Monitoring ("Ouvir Stream")**: One-click toggle in the top bar to listen in your headphones exactly to what is being broadcasted to the stream.
- 🎛️ **ChatMix Slider**: Fast attenuator slider between Game audio and Chat audio with double-click center reset.
- 🔄 **Real-Time WASAPI Session Tracking**: Instantly detects when any program starts outputting sound.
- 🖱️ **Drag & Drop Reassignment**: Move active applications between channel columns directly on the dock.
- 🚫 **Ignored Applications**: Exclude specific apps from routing with a simple right-click ("Ignore"), keeping them completely untouched and playing normally directly to Windows. OBS Studio itself is ignored out-of-the-box, and a top-bar dialog allows viewing and restoring ignored apps at any time.
- 🎚️ **Dual Bus Mixing (Stream vs. Monitor)**: Independent volume sliders and mute controls for your live broadcast output and your headphones.
- 🌐 **Embedded Multi-Language Support (i18n)**: Automatically adapts to OBS language (Portuguese, English, Spanish, French, and Russian) with zero external files needed.
- 💾 **Persistent User Rules**: Remembers custom application assignments across sessions in a lightweight JSON format.

---

## ⚠️ Known Architecture Limitations & Troubleshooting (Limitações Conhecidas e Funcionamento)

### 1. The Windows WASAPI Loopback Pipeline (A Limitação Fundamental do Windows)
No Windows, o plugin utiliza a API nativa de captura por processo (`AUDIOCLIENT_ACTIVATION_TYPE_PROCESS_LOOPBACK`). Essa API possui uma característica arquitetural intrínseca do Windows:

1. **Atenuação Pré-Loopback:** O Windows aplica o volume da sessão de áudio do aplicativo (`ISimpleAudioVolume`, controlado pelo fader **🎧 Fone**) **antes** de entregar os dados para o loopback de captura do OBS.
2. **Impacto no Mudo / Volume Baixo:**
   - Se um aplicativo toca diretamente no seu fone físico e você coloca o fader de **Fone em 0% (ou Mudo)**, o Windows simplesmente interrompe o envio de áudio desse programa na placa de som.
   - Como consequência, o OBS recebe silêncio absoluto. **Não é possível, no modo nativo puro do Windows, silenciar o som nos ouvidos e manter 100% na live se ambos estiverem no mesmo dispositivo físico.**
   - Se o fone estiver em 20%, o sinal capturado pelo OBS também terá sua dinâmica reduzida na origem.

### 2. Por que softwares como SteelSeries Sonar ou Elgato Wave Link conseguem isso?
Programas proprietários conseguem isolamento absoluto (ex: música tocando na live e 0% nos fones) porque instalam **drivers virtuais de kernel (`.sys`) assinados com certificados EV/WHQL da Microsoft**. Eles criam de 4 a 5 placas de som virtuais no Windows, forçam todas as aplicações a tocarem nessas placas virtuais a 100%, e o software (pesando centenas de MB de RAM em segundo plano) redistribui o som para o fone físico.

Tentativas de usar cabos virtuais genéricos (como VB-Cable) embutidos em plugins trazem complexidade excessiva para o usuário comum:
- Criam múltiplos dispositivos confusos no menu de som do Windows (`CABLE Input`, `CABLE 16ch`).
- Se não forem manualmente configurados e monitorados pelo usuário, causam perda total do áudio no computador.
- Exigem permissões de administrador e podem gerar conflitos de latência e taxa de amostragem (44.1kHz vs 48kHz).

### 3. Melhores Práticas de Uso (Best Practices)
Para tirar o melhor proveito do **OBS Audio Router**:

- **Para mixagem de live padrão:** Mantenha o volume dos aplicativos no Windows e os faders de **Fone** em um nível confortável (70%–100%). Utilize os faders de **Live** no Audio Router para balancear a altura das músicas, jogos e Discord para os espectadores.
- **Caso precise tocar áudio na live sem ouvir no fone (Silêncio no fone):**
  - No Windows, vá em **Configurações → Som → Mixer de volume**.
  - Direcione a saída daquele aplicativo específico (ex: Spotify ou navegador) para um dispositivo secundário não utilizado (ex: saída HDMI do monitor, saída óptica ou `Steam Streaming Speakers`).
  - Como o aplicativo estará gerando áudio a 100% nesse dispositivo secundário, o OBS continuará capturando o processo perfeitamente para a transmissão, enquanto os seus fones de ouvido principais permanecerão em silêncio absoluto.
- **Botão "Ouvindo Stream":** Caso queira inspecionar exatamente o que está saindo para o público (mix de Live), ative o botão **`🎧 Ouvindo Stream`** no topo do dock. Lembre-se de desativá-lo para que os faders de **Fone** voltem a atuar sobre sua escuta pessoal.

---

## 📂 Repository Structure

```text
obs-audio-router-plugin/
├── CMakeLists.txt
├── LICENSE
├── README.md
├── BUILDING.md
├── .gitignore
├── data/
│   └── locale/
│       ├── en-US.ini
│       ├── pt-BR.ini
│       ├── es-ES.ini
│       ├── fr-FR.ini
│       └── ru-RU.ini
├── src/
│   ├── plugin-main.cpp
│   ├── plugin-macros.hpp
│   ├── plugin-loader.cpp
│   ├── core/
│   │   ├── audio-session-watcher.hpp / .cpp
│   │   ├── channel-audio-engine.hpp / .cpp
│   │   ├── routing-rules.hpp / .cpp
│   │   └── process-utils.hpp / .cpp
│   └── ui/
│       ├── audio-router-dock.hpp / .cpp
│       ├── chatmix-slider.hpp / .cpp
│       ├── channel-strip-widget.hpp / .cpp
│       ├── app-badge-widget.hpp / .cpp
│       └── localization-helper.hpp
└── tests/
    ├── test_channel_audio_engine.cpp
    ├── test_chatmix_and_volumes.cpp
    └── test_routing_rules.cpp
```

---

## 🛠️ Building from Source

See [BUILDING.md](BUILDING.md) for detailed instructions on compiling with CMake, Ninja, and Qt 6 on Windows 64-bit.

---

## 📄 License

Distributed under the [MIT License](LICENSE).
