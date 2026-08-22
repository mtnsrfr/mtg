#pragma once

#include "Engine.hpp"
#include "PongGame.hpp"
#include "SnakeGame.hpp"
#include "StarfighterGame.hpp"
#include "UnicornGame.hpp"
#include <memory>

// =============================================================================
// RETRO-SPIELE-KONSOLE (Hauptmenü / Spielauswahl)
// =============================================================================
// Hier können die Kids zwischen allen Spielen wählen:
// - Taste 1: Retro 2-Spieler Pong
// - Taste 2: 2-Spieler Schlangen-Duell (Snake Battle)
// - Taste 3: Starfighter Attack (Weltraum-Shooter)
// - Taste 4: The Unicorn (Magischer Retro-Runner)
// - ESCAPE: Jederzeit zurück ins Hauptmenü!
// =============================================================================
struct MenuGame : Game {
  // ===========================================================================
  // 1. SPIEL-VARIABLEN & AUSWAHL
  // ===========================================================================
  enum class CurrentGame {
    Menu,
    Pong,
    Snake,
    Starfighter,
    Unicorn
  };

  CurrentGame currentGame = CurrentGame::Menu;
  std::unique_ptr<PongGame> pong;
  std::unique_ptr<SnakeGame> snake;
  std::unique_ptr<StarfighterGame> starfighter;
  std::unique_ptr<UnicornGame> unicorn;

  // ===========================================================================
  // 2. KONSTRUKTOR (Wird beim Start einmal ausgeführt)
  // ===========================================================================
  MenuGame() {
    pong = std::make_unique<PongGame>();
    snake = std::make_unique<SnakeGame>();
    starfighter = std::make_unique<StarfighterGame>();
    unicorn = std::make_unique<UnicornGame>();
  }

  // ===========================================================================
  // 3. HAUPTSCHLEIFE (Wird 60-mal pro Sekunde aufgerufen)
  // ===========================================================================
  void update(Engine &e) override {
    // Wenn Escape gedrückt wird: Immer zurück ins Menü!
    if (e.pressed(Key::Escape) && currentGame != CurrentGame::Menu) {
      currentGame = CurrentGame::Menu;
      e.stop_bgm();
      e.play_tone(Notes::A4, 0.08f);
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
    case CurrentGame::Unicorn:
      unicorn->update(e);
      break;
    }
  }

  void drawMenu(Engine &e) {
    e.cls(Colors::DarkGreen);

    // Deko-Rahmen
    e.rect(8, 8, 304, 224, Colors::White);
    e.rect(10, 10, 300, 220, Colors::LightGreen);

    // Titel
    e.draw_text(48, 16, "MTG RETRO-SPIELEBOX", Colors::Yellow, 2);
    e.line(26, 36, 294, 36, Colors::White);

    // Spiele-Auswahl
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    // Spiel 1: Pong
    e.rectfill(32, 44, 256, 26, Colors::DarkGray);
    e.rect(32, 44, 256, 26, Colors::Red);
    e.draw_text(46, 52, "DRUECKE 1 : 2-SPIELER PONG", Colors::White, 1);

    // Spiel 2: Snake
    e.rectfill(32, 74, 256, 26, Colors::DarkGray);
    e.rect(32, 74, 256, 26, Colors::Yellow);
    e.draw_text(46, 82, "DRUECKE 2 : SCHLANGEN-DUELL", Colors::White, 1);

    // Spiel 3: Starfighter
    e.rectfill(32, 104, 256, 26, Colors::DarkGray);
    e.rect(32, 104, 256, 26, Colors::Blue);
    e.draw_text(46, 112, "DRUECKE 3 : STARFIGHTER ATTACK", Colors::White, 1);

    // Spiel 4: The Unicorn
    e.rectfill(32, 134, 256, 26, Colors::DarkGray);
    e.rect(32, 134, 256, 26, Colors::Pink);
    e.draw_text(46, 142, "DRUECKE 4 : THE UNICORN", Colors::Gold, 1);

    // Menü-Hinweis
    if (blink) {
      e.draw_text(58, 172, "WAEHLE DEIN SPIEL (1, 2, 3 ODER 4)", Colors::Yellow, 1);
    }
    e.draw_text(44, 196, "TIPP: MIT ESCAPE ZURUECK INS MENUE", Colors::LightGray, 1);

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
    } else if (e.pressed(Key::Num4)) {
      e.play_tone(Notes::C6, 0.12f);
      unicorn = std::make_unique<UnicornGame>(); // Frisch starten
      currentGame = CurrentGame::Unicorn;
    }
  }
};
