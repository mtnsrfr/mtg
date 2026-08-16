#pragma once

#include "Engine.hpp"
#include "PongGame.hpp"
#include "SnakeGame.hpp"
#include <memory>

// =============================================================================
// RETRO-SPIELE-KONSOLE (Hauptmenü / Spielauswahl)
// =============================================================================
// Hier können die Kids zwischen allen Spielen wählen:
// - Taste 1: Retro 2-Spieler Pong
// - Taste 2: 2-Spieler Schlangen-Duell (Snake Battle)
// - ESCAPE: Jederzeit zurück ins Hauptmenü!
// =============================================================================
class MenuGame : public Game {
public:
  MenuGame() {
    pong = std::make_unique<PongGame>();
    snake = std::make_unique<SnakeGame>();
  }

  void update(Engine &e) override {
    // Wenn Escape gedrückt wird: Immer zurück ins Menü!
    if (e.pressed(Key::Escape) && currentGame != CurrentGame::Menu) {
      currentGame = CurrentGame::Menu;
      e.play_tone(440.0, 0.08f);
    }

    // Welches Spiel ist gerade aktiv?
    switch (currentGame) {
    case CurrentGame::Menu:
      drawMenu(e);
      break;
    case CurrentGame::Pong:
      pong->update(e);
      break;
    case CurrentGame::Snake:
      snake->update(e);
      break;
    }
  }

private:
  enum class CurrentGame {
    Menu,
    Pong,
    Snake
  };

  CurrentGame currentGame = CurrentGame::Menu;
  std::unique_ptr<PongGame> pong;
  std::unique_ptr<SnakeGame> snake;

  void drawMenu(Engine &e) {
    e.cls(Colors::DarkGreen);

    // Deko-Rahmen
    e.rect(10, 10, 300, 220, Colors::White);
    e.rect(12, 12, 296, 216, Colors::LightGreen);

    // Titel
    e.draw_text(48, 30, "MTG RETRO-SPIELEBOX", Colors::Yellow, 2);
    e.line(30, 56, 290, 56, Colors::White);

    // Spiele-Auswahl
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    // Spiel 1: Pong
    e.rectfill(40, 75, 240, 36, Colors::DarkGray);
    e.rect(40, 75, 240, 36, Colors::Red);
    e.draw_text(54, 86, "DRUECKE 1 : 2-SPIELER PONG", Colors::White, 1);

    // Spiel 2: Snake
    e.rectfill(40, 125, 240, 36, Colors::DarkGray);
    e.rect(40, 125, 240, 36, Colors::Yellow);
    e.draw_text(54, 136, "DRUECKE 2 : SCHLANGEN-DUELL", Colors::White, 1);

    // Menü-Hinweis
    if (blink) {
      e.draw_text(70, 180, "WAEHLE DEIN SPIEL (1 ODER 2)", Colors::Yellow, 1);
    }
    e.draw_text(50, 205, "TIPP: MIT ESCAPE ZURUECK INS MENUE", Colors::LightGray, 1);

    // Tasteneingabe zur Spielauswahl
    if (e.pressed(Key::Num1)) {
      e.play_tone(523.25, 0.10f); // C5
      pong = std::make_unique<PongGame>(); // Frisch starten
      currentGame = CurrentGame::Pong;
    } else if (e.pressed(Key::Num2)) {
      e.play_tone(659.25, 0.10f); // E5
      snake = std::make_unique<SnakeGame>(); // Frisch starten
      currentGame = CurrentGame::Snake;
    }
  }
};
