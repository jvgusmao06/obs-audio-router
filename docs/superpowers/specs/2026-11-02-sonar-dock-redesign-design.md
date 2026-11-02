# Design Doc: Redesign do Dock Audio Router (Estilo SteelSeries Sonar)

**Data**: 2026-11-02  
**Status**: Aprovado para Planejamento  

---

## 1. Contexto e Motivação (Intent & Why)

O plugin **OBS Audio Router** foi injetado com sucesso no OBS Studio, porém a versão inicial da interface dock apresentou problemas de usabilidade, defeitos visuais e falhas de integração com o OBS:
1. **Dock não desmarca ao fechar**: ao fechar o painel, a marcação no menu *Painéis (Docks)* do OBS não se atualizava, e havia uma barra de título dupla interna (duplo close button).
2. **Layout poluído e contra-intuitivo**: uma barra superior separada listava aplicativos "soltos" com botões de arrastar, enquanto os canais tinham caixas pretas vazias e sliders verticais esmagados (renderizados como meros pontos coloridos).
3. **Ausência de controle funcional de volume**: os sliders não controlavam efetivamente o volume do fone (Normal) no Windows nem os barramentos no OBS (Stream).
4. **Falta de recursos consagrados do Sonar**: ausência de faders duplos (Fone vs Stream) e do recurso **ChatMix** (balanço rápido entre Áudio do Jogo e Áudio do Chat).

O objetivo deste redesign é alinhar a experiência visual e funcional diretamente ao padrão do **SteelSeries Sonar**: um mixer vertical moderno e intuitivo, onde aplicativos são auto-alocados nos seus canais correspondentes na base de cada coluna, com faders verticais duplos estilizados, controle ChatMix integrado e controle de volume funcional tanto para fone de ouvido quanto para a live.

---

## 2. Metas, Não-Metas e Anti-Metas

### Metas (Goals)
- **Correção da Janela/Dock**: Mudar `AudioRouterDock` de `QDockWidget` para `QWidget`, permitindo que o OBS gerencie a janela de dock nativamente, sem barras duplicadas e com sincronização correta do menu *Docks*.
- **Auto-Alocação Direta de Apps**: Eliminar a barra de apps soltos do topo. Qualquer aplicativo de áudio ativo detectado pelo watcher WASAPI vai direto para a caixa "Apps" da coluna correspondente (GAME, CHAT, MEDIA, SYSTEM).
- **Faders Duplos Funcionais por Coluna (Abordagem 2)**:
  - Fader 🎧 **Normal (Fone/Headphones)**: atua diretamente na sessão de áudio WASAPI do Windows (`ISimpleAudioVolume`) de cada aplicativo atribuído ao canal.
  - Fader 📡 **Stream (Live)**: ajusta o volume do barramento de áudio correspondente no OBS (`obs_source_set_volume`).
  - Botão de **Mudo independente** para Fone e Stream em cada canal.
- **Slider ChatMix**: Um slider horizontal no topo do dock permitindo balanço contínuo entre **🎮 Game** e **💬 Chat**. Quando puxado para o lado do Game, atenua o Chat; quando puxado para o Chat, atenua o Game; no centro (50%), ambos permanecem no volume padrão. O ChatMix é estritamente pessoal (afeta o Fone/Headphones, preservando a mixagem de transmissão da live intacta).
- **Ciclo de Vida de Sessões**:
  - Novos processos que abrirem herdam imediatamente o volume, mute e ChatMix do seu canal, evitando picos de volume a 100%.
  - Múltiplos processos com o mesmo executável são agrupados no mesmo chip.
  - Ao fechar o processo, o chip é removido do dock; as regras personalizadas permanecem salvas em JSON.
- **Visual Sonar Moderno**:
  - Tema escuro consistente (`#18191c`, `#2f3136`, `#202225`).
  - Cabeçalhos de canal com ícone temático, cor de destaque (accent color) e título.
  - Manípulos retangulares estilo mesa de som com notch central, trilhos com cor de canal e indicador numérico (%).
  - Pílulas compactas de aplicativos na base da coluna com botão/menu para troca rápida de canal caso o usuário queira reatribuir.

### Não-Metas (Non-Goals)
- Criar drivers virtuais de kernel no Windows (usamos os recursos nativos do WASAPI loopback e captura de processo do OBS).
- Efeitos avançados de equalizador paramétrico de 10 bandas ou compressores de estúdio (fora do escopo inicial).

### Anti-Metas (Anti-Goals)
- Não adicionar bibliotecas pesadas de terceiros ou frameworks web (Chromium/CEF) para desenhar a interface; manter **100% Qt nativo** e ultra-leve (< 5 MB de RAM).
- Não exigir reinicialização do OBS para detecção de novos aplicativos ou troca de canais.

---

## 3. Abordagens Consideradas (Approaches)

Durante a fase de levantamento de requisitos, foram avaliadas duas abordagens de controle de volume:

1. **Abordagem 1: Alternância de Mix no Topo (`[🎧 Fone]` / `[📡 Stream]`)**:
   - Um fader largo por coluna, com um botão seletor no topo do mixer para alternar o que os faders estão regulando.
   - *Trade-off*: Visualmente mais idêntico à aba padrão do Sonar, mas exige um clique adicional para ajustar fone vs stream.
2. **Abordagem 2: Faders Duplos Lado a Lado em Cada Canal (Escolhida pelo Usuário)**:
   - Dois faders paralelos em cada coluna (🎧 Fone e 📡 Stream), com mutes independentes.
   - Adição do slider de **ChatMix** no topo.
   - *Trade-off*: As colunas precisam de uma largura mínima um pouco maior, mas oferecem controle imediato simultâneo sobre ambos os barramentos, idêntico ao *Sonar Streamer Mode* e ao *Elgato Wave Link*.

---

## 4. Arquitetura Detalhada

### 4.1. Camada de UI (`src/ui/`)
- `AudioRouterDock` (subclasse de `QWidget`):
  - **Barra Superior**:
    - Logo / Título sutil do Audio Router.
    - Componente **ChatMix**: `[🎮 Game]` ----`[|]`---- `[💬 Chat]`, com reset de duplo clique no centro.
  - **Área Central (Mixer Grid)**:
    - 4 colunas temáticas:
      1. 🎮 **GAME** (Destaque Verde/Ciano `#00f0aa`)
      2. 💬 **CHAT** (Destaque Azul `#5865f2`)
      3. 🎵 **MEDIA** (Destaque Rosa/Magenta `#eb459e`)
      4. ⚙️ **SYSTEM / AUX** (Destaque Laranja/Amarelo `#faa61a`)
  - **Componente `ChannelStripWidget` (cada coluna)**:
    - Cabeçalho: Ícone do canal + Nome + Indicador de cor.
    - Seção de Faders:
      - Coluna Fone: Ícone 🎧, slider vertical estilizado, rótulo de porcentagem (ex: "85%"), botão Mute.
      - Coluna Stream: Ícone 📡, slider vertical estilizado, rótulo de porcentagem (ex: "100%"), botão Mute.
    - Seção inferior **Apps**:
      - Rótulo "Apps" discreto.
      - Container escuro com rolagem suave (`QScrollArea` compacta).
      - Chips/pílulas de apps detectados (ícone genérico ou do app, nome abreviado `discord.exe`, botão 'X' ou dropdown para mover para outro canal).

### 4.2. Camada de Áudio (`src/core/`)
- **Controle WASAPI (`ISimpleAudioVolume`) para Volume Normal**:
  - `AudioSessionWatcher` mantém referências às sessões de áudio (`IAudioSessionControl` / `ISimpleAudioVolume`) por processo.
  - Quando o fader Fone de um canal é movido:
    - Calcula o ganho combinado: `ganho = volumeCanal * chatMixFator`.
    - Itera sobre todos os processos vinculados a esse canal e chama `ISimpleAudioVolume::SetMasterVolume(ganho, nullptr)`.
    - Ao mutar o fader Fone: `ISimpleAudioVolume::SetMute(TRUE, nullptr)`.
- **Controle OBS (`libobs`) para Volume Stream**:
  - `ChannelAudioEngine` gerencia as fontes de áudio de cada barramento (`obs_source_t*`).
  - Quando o fader Stream de um canal é movido:
    - Chama `obs_source_set_volume(source, pow(streamVolume, 3.0f))`.
    - Ao mutar: `obs_source_set_muted(source, isMuted)`.
- **Cálculo do ChatMix (Específico para Headphones)**:
  - Valor normalizado de -1.0 (100% Game) a +1.0 (100% Chat), com centro em 0.0.
  - Se `chatMix <= 0.0`: Game = 1.0, Chat = `1.0 + chatMix` (atenua chat no fone).
  - Se `chatMix > 0.0`: Chat = 1.0, Game = `1.0 - chatMix` (atenua game no fone).

---

## 5. Plano de Entrega e Verificação
1. **Passo 1**: Refatoração do `AudioRouterDock` para herdar de `QWidget` e validação do registro em `plugin-main.cpp`.
2. **Passo 2**: Implementação do `ChannelStripWidget` com sliders verticais estilizados, faders duplos (Fone/Stream) e badges de apps na base.
3. **Passo 3**: Implementação do componente horizontal `ChatMixWidget` na barra superior.
4. **Passo 4**: Conexão das ações de volume ao `ChannelAudioEngine` e `AudioSessionWatcher` (`ISimpleAudioVolume`).
5. **Passo 5**: Compilação e verificação no ambiente Windows/OBS Studio.
