# Plano de Implementação - Fase 3 & 4: Interface Dock Interativa, Drag & Drop e Internacionalização (EN / PT)

> **Objetivo**: Conectar o subsistema de áudio WASAPI à interface Qt, implementar a barra de aplicativos ativos com suporte a Drag & Drop entre as colunas dos canais, adicionar persistência automática de regras e habilitar suporte bilíngue completo (Inglês padrão + Português pt-BR) via sistema de localização nativo do OBS e Qt.

---

## 1. Arquitetura de UI e Fluxo Drag & Drop

```
+-----------------------------------------------------------------------------------------+
| [ Dock: Audio Router ]                                                                  |
+-----------------------------------------------------------------------------------------+
| Active Applications (Drag to assign):                                                   |
|   ┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐                      |
|   │ 💬 discord.exe   │  │ 🎵 spotify.exe   │  │ 🎮 cs2.exe       │                      |
|   └─────────┬────────┘  └──────────────────┘  └──────────────────┘                      |
|             │ (Drag & Drop)                                                             |
+─────────────┼───────────────────────────────────────────────────────────────────────────+
| CANAIS:     ▼                                                                           |
|  [ 🎮 GAME ]            [ 💬 CHAT ]             [ 🎵 MEDIA ]            [ 🌐 SYSTEM ]   |
|  ┌───────────────────┐  ┌───────────────────┐  ┌───────────────────┐  ┌───────────────┐ |
|  │ (Drop Target)     │  │ • discord.exe  ✖  │  │ • spotify.exe  ✖  │  │ (Drop Target) │ |
|  └───────────────────┘  └───────────────────┘  └───────────────────┘  └───────────────┘ |
|                                                                                         |
|  Stream: [==|===] 85%   Stream: [===|==] 100%  Stream: [=|====] 50%   Stream: [==|==]   |
|  Phone:  [===|==] 100%  Phone:  [===|==] 100%  Phone:  [==|===] 80%   Phone:  [==|==]   |
|  [Mute]                 [Mute]                 [Mute]                 [Mute]            |
+-----------------------------------------------------------------------------------------+
```

---

## 2. Internacionalização (i18n): EN-US e PT-BR

O OBS Studio carrega arquivos de tradução em `.ini` ou strings de módulo (`obs_module_text`), e o Qt utiliza `QTranslator` / `tr()`.
- Criaremos a pasta `data/locale/`:
  - `en-US.ini` (Chaves padrão em inglês)
  - `pt-BR.ini` (Tradução completa em português do Brasil)
- Todas as mensagens e textos de UI usarão funções de tradução (`obs_module_text()` e `tr()`).

---

## 3. Tarefas de Implementação

### Tarefa 1: Arquivos de Idioma (Locales)
- **Arquivos**:
  - `data/locale/en-US.ini`
  - `data/locale/pt-BR.ini`
  - `src/ui/localization-helper.hpp`
- **Chaves**:
  - Títulos dos canais (`Game`, `Chat`, `Media`, `System` / `Jogo`, `Chat`, `Mídia`, `Sistema`)
  - Sliders (`Stream Volume`, `Monitor Volume` / `Volume da Live`, `Volume do Fone`)
  - Tooltips e instruções de arrastar e soltar

### Tarefa 2: Widgets Customizados para Drag & Drop
- **Arquivos**:
  - `src/ui/app-badge-widget.hpp` / `.cpp` (`QLabel` customizado com `mousePressEvent` e início de `QDrag` contendo o nome do `.exe` via `QMimeData`)
  - `src/ui/channel-drop-zone.hpp` / `.cpp` (`QFrame` receptor com `dragEnterEvent` e `dropEvent` que invoca `RoutingRules::setCustomRule`)

### Tarefa 3: Conexão Reativa do WASAPI com o Dock
- **Arquivos**:
  - `src/ui/audio-router-dock.hpp` / `.cpp`
- **Ações**:
  - Inicializar `AudioSessionWatcher` dentro do dock.
  - Conectar callbacks WASAPI com Qt via `QMetaObject::invokeMethod` ou signals para segurança de threads.
  - Atualizar a barra superior automaticamente quando um novo app abrir ou fechar.

### Tarefa 4: Teste de Integridade & Persistência
- **Arquivos**:
  - `tests/test_i18n_and_rules.cpp`
- **Validação**:
  - Testar carregamento dos arquivos de tradução.
  - Testar fluxo de reatribuição e conferência da serialização JSON.

