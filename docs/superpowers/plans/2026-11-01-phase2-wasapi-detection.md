# Plano de Implementação - Fase 2: Detecção de Sessões de Áudio e Captura WASAPI

> **Objetivo**: Implementar o subsistema Windows WASAPI para enumerar dispositivos e sessões ativas de áudio, monitorar notificações de criação/término de sessões em tempo real (`IAudioSessionNotification`) e obter metadados do processo (PID, nome do executável e título da janela) para alimentar o roteamento automático.

---

## Arquitetura da Fase 2

```
Windows Audio Subsystem (WASAPI)
       │
       ▼ IMMDeviceEnumerator (Default Render Device)
       │
       ▼ IAudioSessionManager2
       │
       ├──► IAudioSessionNotification (Callback de novas sessões criadas)
       │
       └──► IAudioSessionEnumerator (Iteração de sessões ativas existentes)
                 │
                 ▼ IAudioSessionControl2
                 │   ├── GetProcessId() -> PID
                 │   ├── IsSystemSoundsSession()
                 │   └── GetState() (Active / Inactive)
                 │
                 ▼ Process Helper (Win32 API)
                     ├── QueryFullProcessImageName() -> "Discord.exe"
                     └── EnumWindows() / GetWindowText() -> Window Title
```

---

## Tarefas de Implementação

### Tarefa 1: Process Helper Win32
- **Arquivos**:
  - `src/core/process-utils.hpp`
  - `src/core/process-utils.cpp`
- **Responsabilidade**:
  - Dado um PID, obter o nome do executável de forma segura (ex: `spotify.exe`).
  - Lidar com privilégios de processo e caminhos do sistema.

### Tarefa 2: Monitor de Sessões WASAPI (`AudioSessionWatcher`)
- **Arquivos**:
  - `src/core/audio-session-watcher.hpp`
  - `src/core/audio-session-watcher.cpp`
- **Responsabilidade**:
  - Inicializar COM e obter `IAudioSessionManager2` no dispositivo de saída padrão.
  - Implementar `IAudioSessionNotification` para receber eventos de novas aplicações tocando som.
  - Notificar callbacks quando uma aplicação começa ou para de emitir áudio.

### Tarefa 3: Teste Unitário / Spike de Verificação WASAPI
- **Arquivos**:
  - `tests/test_wasapi_sessions.cpp`
- **Responsabilidade**:
  - Enumerar as sessões ativas rodando na máquina do usuário.
  - Verificar se processos como Discord, Spotify ou navegadores são identificados corretamente com seus canais alvo via `RoutingRules`.

---

## Critérios de Sucesso
- [ ] Enumeração correta de processos tocando áudio no Windows sem travar o thread principal.
- [ ] Conversão precisa de PID para o nome do `.exe`.
- [ ] Integração com `RoutingRules` demonstrando que o processo é mapeado para o canal correto (ex: `spotify.exe` -> `MÍDIA`).

