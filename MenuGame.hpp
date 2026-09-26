#pragma once

#include "Engine.hpp"
#include "PongGame.hpp"
#include "SnakeGame.hpp"
#include "StarfighterGame.hpp"
#include "UnicornGame.hpp"
#include "TankGame.hpp"
#include "DarioGame.hpp"
#include "RacerGame.hpp"
#include <memory>

// =============================================================================
// RETRO-SPIELE-KONSOLE (Hauptmenü / Spielauswahl)
// =============================================================================
// Hier können die Kids zwischen allen Spielen wählen:
// - Taste 1: Retro 2-Spieler Pong
// - Taste 2: 2-Spieler Schlangen-Duell (Snake Battle)
// - Taste 3: Starfighter Attack (Weltraum-Shooter)
// - Taste 4: The Unicorn (Magischer Retro-Runner)
// - Taste 5: Tank Duel (Artillerie-Panzerduell)
// - Taste 6: Darios Spiel (Elementar-Kampfarena)
// - Taste 7: Retro Grand Prix (2-Player Top-Down Racer)
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
    Unicorn,
    Tank,
    Dario,
    Racer
  };

  CurrentGame currentGame = CurrentGame::Menu;
  std::unique_ptr<PongGame> pong;
  std::unique_ptr<SnakeGame> snake;
  std::unique_ptr<StarfighterGame> starfighter;
  std::unique_ptr<UnicornGame> unicorn;
  std::unique_ptr<TankGame> tank;
  std::unique_ptr<DarioGame> dario;
  std::unique_ptr<RacerGame> racer;

  // ===========================================================================
  // 2. KONSTRUKTOR (Wird beim Start einmal ausgeführt)
  // ===========================================================================
  MenuGame() {
    pong = std::make_unique<PongGame>();
    snake = std::make_unique<SnakeGame>();
    starfighter = std::make_unique<StarfighterGame>();
    unicorn = std::make_unique<UnicornGame>();
    tank = std::make_unique<TankGame>();
    dario = std::make_unique<DarioGame>();
    racer = std::make_unique<RacerGame>();
  }

  // ===========================================================================
  // 3. HAUPTSCHLEIFE (Wird 60-mal pro Sekunde aufgerufen)
  // ===========================================================================
  void update(Engine &e) override {
    // Wenn Escape gedrückt wird: Immer zurück ins Menü!
    if (e.pressed(Key::Escape) && currentGame != CurrentGame::Menu) {
      currentGame = CurrentGame::Menu;
      e.stop_all_audio();
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
    case CurrentGame::Tank:
      tank->update(e);
      break;
    case CurrentGame::Dario:
      dario->update(e);
      break;
    case CurrentGame::Racer:
      racer->update(e);
      break;
    }
  }

  void drawMenu(Engine &e) {
    e.cls(Colors::DarkGreen);

    // Deko-Rahmen
    e.rect(8, 6, 304, 228, Colors::White);
    e.rect(10, 8, 300, 224, Colors::LightGreen);

    // Titel
    e.draw_text(48, 10, "MTG RETRO-SPIELEBOX", Colors::Yellow, 2);
    e.line(26, 26, 294, 26, Colors::White);

    // Spiele-Auswahl
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    // Spiel 1: Pong
    e.rectfill(28, 29, 264, 16, Colors::DarkGray);
    e.rect(28, 29, 264, 16, Colors::Red);
    e.draw_text(38, 33, "DRUECKE 1 : 2-SPIELER PONG", Colors::White, 1);

    // Spiel 2: Snake
    e.rectfill(28, 48, 264, 16, Colors::DarkGray);
    e.rect(28, 48, 264, 16, Colors::Yellow);
    e.draw_text(38, 52, "DRUECKE 2 : SCHLANGEN-DUELL", Colors::White, 1);

    // Spiel 3: Starfighter
    e.rectfill(28, 67, 264, 16, Colors::DarkGray);
    e.rect(28, 67, 264, 16, Colors::Blue);
    e.draw_text(38, 71, "DRUECKE 3 : STARFIGHTER ATTACK", Colors::White, 1);

    // Spiel 4: The Unicorn
    e.rectfill(28, 86, 264, 16, Colors::DarkGray);
    e.rect(28, 86, 264, 16, Colors::Pink);
    e.draw_text(38, 90, "DRUECKE 4 : THE UNICORN", Colors::Gold, 1);

    // Spiel 5: Tank Duel
    e.rectfill(28, 105, 264, 16, Colors::DarkGray);
    e.rect(28, 105, 264, 16, Colors::Cyan);
    e.draw_text(38, 109, "DRUECKE 5 : TANK DUEL (ARTILLERY)", Colors::Cyan, 1);

    // Spiel 6: Darios Spiel (Elemental Brawl)
    e.rectfill(28, 124, 264, 16, Colors::DarkGray);
    e.rect(28, 124, 264, 16, Colors::Gold);
    e.draw_text(38, 128, "DRUECKE 6 : DARIOS SPIEL (ELEMENTAL BRAWL)", Colors::Gold, 1);

    // Spiel 7: Retro Grand Prix (Top-Down Racer)
    e.rectfill(28, 143, 264, 16, Colors::DarkGray);
    e.rect(28, 143, 264, 16, Colors::Red);
    e.draw_text(38, 147, "DRUECKE 7 : RETRO GRAND PRIX (RACER)", Ramps::Fire[12], 1);

    // Menü-Hinweis
    if (blink) {
      e.draw_text(36, 168, "WAEHLE DEIN SPIEL (1, 2, 3, 4, 5, 6 ODER 7)", Colors::Yellow, 1);
    }
    e.draw_text(44, 186, "TIPP: MIT ESCAPE ZURUECK INS MENUE", Colors::LightGray, 1);

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
    } else if (e.pressed(Key::Num5)) {
      e.play_tone(Notes::E6, 0.12f);
      tank = std::make_unique<TankGame>(); // Frisch starten
      currentGame = CurrentGame::Tank;
    } else if (e.pressed(Key::Num6)) {
      e.play_tone(Notes::G6, 0.12f);
      dario = std::make_unique<DarioGame>(); // Frisch starten
      currentGame = CurrentGame::Dario;
    } else if (e.pressed(Key::Num7)) {
      e.play_tone(Notes::B6, 0.12f);
      racer = std::make_unique<RacerGame>(); // Frisch starten
      currentGame = CurrentGame::Racer;
    }
  }
};
