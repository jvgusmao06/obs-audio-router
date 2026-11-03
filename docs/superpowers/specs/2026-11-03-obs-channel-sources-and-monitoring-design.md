# Design Doc: Fontes de Canal Nativas no OBS e Mixagem Dupla (Fone vs Live)

**Data**: 2026-11-03  
**Status**: Proposto / Em Avaliação  
**Autor**: Antigravity & User  

---

## 1. Contexto e Motivação (Intent & Why)

O plugin **OBS Audio Router** possui uma interface moderna de mesa de som com faders duplos (**🎧 Fone** e **📡 Live**) para cada canal (Jogo, Chat, Mídia, Sistema).

No entanto, atualmente:
1. O fader **Fone** atua diretamente na sessão de áudio WASAPI do Windows (`ISimpleAudioVolume`), atenuando o volume do processo no sistema operacional.
2. Quando o usuário define **Fone em 0%** e **Live em 100%** (ex: para tocar música na live sem escutar nos próprios fones), o processo é silenciado na origem pelo Windows.
3. Como o OBS tradicionalmente captura o dispositivo padrão ("Áudio do Desktop"), a gravação/live recebe silêncio absoluto.
4. Além disso, o Audio Router não instanciava fontes de áudio dedicadas dentro do motor do OBS Studio (`libobs`), impedindo que a transmissão recebesse os canais de forma isolada.

O objetivo deste projeto é fazer com que o Audio Router instancie e gerencie automaticamente os barramentos de áudio dentro do OBS Studio para cada canal (**Jogo, Chat, Mídia, Sistema**), permitindo que a gravação/stream receba o áudio configurado no fader **Live**, enquanto a escuta do usuário nos fones é controlada de forma independente pelo fader **Fone** através do motor de monitoramento do OBS, sem a necessidade de drivers de áudio virtuais no Windows.

---

## 2. Metas, Não-Metas e Anti-Metas

### Metas (Goals)
- **Criação Automática dos 4 Barramentos no OBS**:
  - Instanciar fontes de áudio globais no OBS:
    - `Audio Router - Jogo`
    - `Audio Router - Chat`
    - `Audio Router - Mídia`
    - `Audio Router - Sistema`
  - As fontes devem aparecer no mixer do OBS e estar ativas para envio às faixas de gravação e live.
- **Controle Efetivo do Fader Live (Transmissão/Gravação)**:
  - O fader **Live** e o botão **Mudo Live** controlam o ganho e o estado da fonte OBS (`obs_source_set_volume` e `obs_source_set_muted`).
  - O áudio dessas fontes é direcionado para a faixa de saída da transmissão (Trilha 1/Tracks do OBS).
- **Controle Efetivo do Fader Fone (Headphones / Monitor)**:
  - O fader **Fone** controla o envio de áudio para os fones de ouvido através do sistema de **Audio Monitoring** do OBS (`obs_monitoring_type`).
  - Quando **Fone = 0%** ou **Mudo Fone = Ativado**: o monitoramento daquele canal é desativado (`OBS_MONITORING_TYPE_NONE`), garantindo que **zero som chegue aos ouvidos do streamer**, enquanto a Live continua recebendo a 100% (`OBS_MONITORING_TYPE_NONE` envia apenas para a saída da live).
  - Quando **Fone > 0%**: o canal transmite tanto para a Live quanto para os fones (`OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT`).
- **Botão "Ouvir Stream" no Topo**:
  - Quando ativado, sobrepõe temporariamente o mix dos fones para refletir exatamente o que está saindo para a Live (Stream Monitoring Mode).
- **Sem Drivers Extras**:
  - Manter o plugin 100% autocontido no único arquivo `obs-audio-router.dll`.

### Não-Metas (Non-Goals)
- Instalar drivers de kernel de placas de som virtuais no Windows (ex: tipo Virtual Audio Cable).
- Substituir o painel de configurações gerais de áudio do OBS.

### Anti-Metas (Anti-Goals)
- Não exigir que o usuário crie fontes manuais nas cenas do OBS a cada transmissão.
- Não introduzir duplicação de áudio (eco) na live.

---

## 3. Abordagens Consideradas (Approaches)

1. **Abordagem 1: Fontes Internas no OBS + Monitoramento de Áudio do OBS (Escolhida)**:
   - O plugin cria os barramentos no OBS (`obs_source_create`).
   - O fader Live alimenta a saída de gravação/live (`obs_source_set_volume`).
   - O fader Fone ajusta a escuta pessoal do streamer via `obs_source_set_monitoring_type` e volume de monitoramento do OBS.
   - *Vantagens*: 100% nativo no OBS 28+/30+, zero drivers externos, funciona diretamente na DLL do plugin.
2. **Abordagem 2: Integração com Dispositivo Virtual (VB-Cable)**:
   - Exigiria que o streamer baixasse e instalasse softwares/drivers externos de terceiros no Windows.
   - *Descartada*: Adiciona atrito de configuração e viola a meta de entrega em arquivo único (`obs-audio-router.dll`).
3. **Abordagem 3: Manipulação Exclusiva de Volume WASAPI no Windows (Estado Anterior)**:
   - Atenua o volume do processo no Windows.
   - *Descartada*: Se o fone for 0%, zera as amostras na origem do Windows, silenciando a live e a gravação.

---

## 4. Arquitetura Detalhada

### 4.1. Camada de Áudio do OBS (`ChannelAudioEngine`)
- **Ciclo de Vida das Fontes**:
  - Em `OBS_FRONTEND_EVENT_FINISHED_LOADING`, com o subsistema de áudio do OBS pronto, o `ChannelAudioEngine` inicializa a infraestrutura de barramentos.
  - Para cada aplicativo atribuído a um canal (ex: `spotify.exe` em `MEDIA`), o engine instancia uma fonte nativa do OBS (`wasapi_process_output_capture`) dedicada a capturar aquele executável.
  - O canal gerencia a lista de fontes dos seus processos ativos.
  - Quando um aplicativo fecha, o `unassignProcess` libera a fonte correspondente via `obs_source_release`.
- **Aplicação de Volume Live (`setStreamVolume`)**:
  - Volume linear do fader (0.0 a 1.0) é convertido na curva cúbica do OBS:
    $$\text{ganho} = \text{volumeLive}^3$$
  - Itera sobre as fontes ativas do canal e aplica `obs_source_set_volume(source, ganho)`.
  - Mute Live aplica `obs_source_set_muted(source, isStreamMuted)`.
- **Aplicação de Volume Fone (`setMonitorVolume`)**:
  - Se `monitorVolume <= 0.001f` ou `isMonitorMuted`:
    Aplica `obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_NONE)` (áudio vai exclusivamente para a gravação/transmissão, zero som nos fones).
  - Se `monitorVolume > 0.001f` e `!isMonitorMuted`:
    Aplica `obs_source_set_monitoring_type(source, OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT)` (áudio vai para a gravação e é monitorado nos fones).

### 4.2. Camada de UI (`AudioRouterDock` & `ChannelStripWidget`)
- Os faders da interface continuam os mesmos visualmente:
  - Fader 🎧 **Fone**: ajusta o monitoramento pessoal nos fones.
  - Fader 📡 **Live**: ajusta o volume do barramento gravado/transmitido.
- Os VU meters capturam os picos reais do barramento do OBS, refletindo o sinal que vai para a live.

---

## 5. O que fica a cargo do construtor (Implementation Tasks)
- Implementar o ciclo de vida de criação e liberação de `obs_source_t` no `ChannelAudioEngine`.
- Configurar os `obs_data_t` de cada fonte para o tipo de captura adequado.
- Gerenciar as flags de `obs_monitoring_type` de acordo com os estados dos faders de Fone.
- Testes unitários com stubs de mock para libobs.
