/**
 * @file test_routing_rules.cpp
 * @brief Unit tests verifying process classification rules and JSON serialization logic.
 */

#include "core/routing-rules.hpp"
#include <cassert>
#include <iostream>

void runTests() {
    AudioRouter::RoutingRules rules;

    std::cout << "[TEST] 1. Auto-roteamento padrão out-of-the-box..." << std::endl;
    assert(rules.resolveChannel("discord.exe") == AudioRouter::ChannelType::CHAT);
    assert(rules.resolveChannel("DISCORD.EXE") == AudioRouter::ChannelType::CHAT); // Case insensitive
    assert(rules.resolveChannel("Spotify.exe") == AudioRouter::ChannelType::MEDIA);
    assert(rules.resolveChannel("cs2.exe") == AudioRouter::ChannelType::GAME);
    assert(rules.resolveChannel("notepad.exe") == AudioRouter::ChannelType::SYSTEM); // Fallback

    std::cout << "[TEST] 2. Regra customizada do usuário..." << std::endl;
    // Usuário joga chrome para o canal de mídia
    rules.setCustomRule("chrome.exe", AudioRouter::ChannelType::MEDIA);
    assert(rules.resolveChannel("chrome.exe") == AudioRouter::ChannelType::MEDIA);

    std::cout << "[TEST] 3. Serialização e Desserialização JSON..." << std::endl;
    std::string json = rules.serializeToJson();
    assert(json.find("\"chrome.exe\": \"Mídia\"") != std::string::npos);

    AudioRouter::RoutingRules newRules;
    newRules.deserializeFromJson(json);
    assert(newRules.resolveChannel("chrome.exe") == AudioRouter::ChannelType::MEDIA);

    std::cout << "[SUCCESS] Todos os testes do motor de regras passaram com sucesso!" << std::endl;
}

int main() {
    try {
        runTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Erro no teste: " << e.what() << std::endl;
        return 1;
    }
}

