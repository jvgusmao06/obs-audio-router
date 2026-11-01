# Plano de Implementação - OBS Audio Router Plugin

> **Objetivo**: Criar a fundação e a infraestrutura de compilação do plugin para OBS Studio em C++20 com CMake, gerando o esqueleto do plugin com registro de módulo na `libobs` e um Dock Qt básico inicializável.

---

## Estrutura de Arquivos Planejada

```
c:/Users/Jvgso/Projetos/apps/plugin-obs/
├── CMakeLists.txt                       # Script de build principal com libobs e Qt6
├── cmake/
│   └── FindLibObs.cmake                 # Helper para localizar libobs no Windows
├── src/
│   ├── plugin-main.cpp                  # Entrypoint: obs_module_load, obs_module_unload
│   ├── plugin-macros.hpp                # Definições de logging (blog) e constantes
│   ├── core/
│   │   ├── routing-rules.hpp            # Estrutura de dados das regras de roteamento
│   │   └── routing-rules.cpp            # Parser/salvamento de regras em JSON
│   └── ui/
│       ├── audio-router-dock.hpp        # Classe QDockWidget
│       └── audio-router-dock.cpp        # Interface do Dock no OBS
└── tests/
    └── test_routing_rules.cpp           # Testes unitários do motor de regras
```

---

## Tarefas de Implementação

### Tarefa 1: Esqueleto de Configuração do CMake e Definições de Módulo
- **Arquivos**:
  - `CMakeLists.txt`
  - `src/plugin-macros.hpp`
  - `src/plugin-main.cpp`
- **Passos**:
  1. Configurar `CMakeLists.txt` com C++20, busca de `libobs` e `Qt6` (Widgets, Core, Gui).
  2. Definir macros de log e metadados do módulo OBS (`OBS_DECLARE_MODULE()`, `obs_module_load()`).
  3. Criar função de verificação do carregamento do módulo.

### Tarefa 2: Motor de Regras e Persistência (Core Rules Engine)
- **Arquivos**:
  - `src/core/routing-rules.hpp`
  - `src/core/routing-rules.cpp`
  - `tests/test_routing_rules.cpp`
- **Passos**:
  1. Definir enum de canais (`GAME`, `CHAT`, `MEDIA`, `SYSTEM`).
  2. Implementar tabela padrão de regras de associação de executáveis (ex: `discord.exe` -> `CHAT`).
  3. Implementar serialização/desserialização JSON para persistência em disco.
  4. Criar teste unitário garantindo que novas regras e fallbacks funcionam como esperado.

### Tarefa 3: Dock Qt Básico Integrado ao OBS
- **Arquivos**:
  - `src/ui/audio-router-dock.hpp`
  - `src/ui/audio-router-dock.cpp`
  - `src/plugin-main.cpp` (integração do dock via `obs_frontend_add_dock`)
- **Passos**:
  1. Criar classe `AudioRouterDock : public QDockWidget`.
  2. Adicionar placeholder visual inicial para os 4 canais.
  3. Conectar a inicialização do Dock ao evento de carregamento da interface do OBS (`obs_frontend_add_event_callback`).

---

## Critérios de Sucesso
- [ ] Estrutura do CMake configurada pronta para compilação com MSVC/Clang e Ninja.
- [ ] Módulo do plugin compilável gerando a DLL (`obs-audio-router.dll`).
- [ ] Testes unitários do motor de regras passando.

