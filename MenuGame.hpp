#pragma once

#include "Engine.hpp"
#include "PongGame.hpp"
#include "SnakeGame.hpp"
#include "StarfighterGame.hpp"
#include <memory>

// =============================================================================
// RETRO-SPIELE-KONSOLE (Hauptmenü / Spielauswahl)
// =============================================================================
// Hier können die Kids zwischen allen Spielen wählen:
// - Taste 1: Retro 2-Spieler Pong
// - Taste 2: 2-Spieler Schlangen-Duell (Snake Battle)
// - Taste 3: Starfighter Attack (Weltraum-Shooter)
// - ESCAPE: Jederzeit zurück ins Hauptmenü!
// =============================================================================
class MenuGame : public Game {
public:
  MenuGame() {
    pong = std::make_unique<PongGame>();
    snake = std::make_unique<SnakeGame>();
    starfighter = std::make_unique<StarfighterGame>();
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
    case CurrentGame::Starfighter:
      starfighter->update(e);
      break;
    }
  }

private:
  enum class CurrentGame {
    Menu,
    Pong,
    Snake,
    Starfighter
  };

  CurrentGame currentGame = CurrentGame::Menu;
  std::unique_ptr<PongGame> pong;
  std::unique_ptr<SnakeGame> snake;
  std::unique_ptr<StarfighterGame> starfighter;

  void drawMenu(Engine &e) {
    e.cls(Colors::DarkGreen);

    // Deko-Rahmen
    e.rect(10, 10, 300, 220, Colors::White);
    e.rect(12, 12, 296, 216, Colors::LightGreen);

    // Titel
    e.draw_text(48, 24, "MTG RETRO-SPIELEBOX", Colors::Yellow, 2);
    e.line(30, 48, 290, 48, Colors::White);

    // Spiele-Auswahl
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    // Spiel 1: Pong
    e.rectfill(40, 58, 240, 30, Colors::DarkGray);
    e.rect(40, 58, 240, 30, Colors::Red);
    e.draw_text(54, 68, "DRUECKE 1 : 2-SPIELER PONG", Colors::White, 1);

    // Spiel 2: Snake
    e.rectfill(40, 96, 240, 30, Colors::DarkGray);
    e.rect(40, 96, 240, 30, Colors::Yellow);
    e.draw_text(54, 106, "DRUECKE 2 : SCHLANGEN-DUELL", Colors::White, 1);

    // Spiel 3: Starfighter
    e.rectfill(40, 134, 240, 30, Colors::DarkGray);
    e.rect(40, 134, 240, 30, Colors::Blue);
    e.draw_text(54, 144, "DRUECKE 3 : STARFIGHTER ATTACK", Colors::White, 1);

    // Menü-Hinweis
    if (blink) {
      e.draw_text(66, 178, "WAEHLE DEIN SPIEL (1, 2 ODER 3)", Colors::Yellow, 1);
    }
    e.draw_text(50, 204, "TIPP: MIT ESCAPE ZURUECK INS MENUE", Colors::LightGray, 1);

    // Tasteneingabe zur Spielauswahl
    if (e.pressed(Key::Num1)) {
      e.play_tone(Notes::C5, 0.10f);
      pong = std::make_unique<PongGame>(); // Frisch starten
      currentGame = CurrentGame::Pong;
    } else if (e.pressed(Key::Num2)) {
      e.play_tone(Notes::E5, 0.10f);
      snake = std::make_unique<SnakeGame>(); // Frisch starten
      currentGame = CurrentGame::Snake;
    } else if (e.pressed(Key::Num3)) {
      e.play_tone(Notes::G5, 0.10f);
      starfighter = std::make_unique<StarfighterGame>(); // Frisch starten
      currentGame = CurrentGame::Starfighter;
    }
  }
};
