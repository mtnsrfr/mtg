#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <string>
#include <vector>

// =============================================================================
// DAS 2-SPIELER SCHLANGEN-DUELL (Snake Battle - Modell B: "Apfel-Klau")
// =============================================================================
// Regeln:
// - Beide Spieler jagen gleichzeitig nach roten Äpfeln auf dem Feld.
// - Jeder Apfel macht die Schlange 1 Segment länger und gibt 1 Punkt!
// - Wenn eine Schlange anstößt (Wand, eigener Schwanz oder Gegnerschlange):
//   -> Sie zerplatzt und hinterlässt alle ihre Äpfel auf dem Spielfeld!
//   -> Der Gegner kann die Äpfel schnell fressen und riesig werden!
//   -> Die Schlange startet sofort wieder klein in ihrer Ecke.
// - Wer zuerst 12 Punkte erreicht, gewinnt das Spiel!
//
// Steuerung:
// - Spieler 1 (Gelb):  W (Hoch), A (Links), S (Runter), D (Rechts)
// - Spieler 2 (Blau):  Pfeiltasten (Hoch, Links, Runter, Rechts)
// =============================================================================

struct SnakeGame : Game {
  // ===========================================================================
  // 1. SPIEL-VARIABLEN & EINSTELLUNGEN
  // ===========================================================================
  // --- Spiel-Phasen ---
  enum class State {
    Title,   // Startbildschirm ("Leertaste drücken")
    Playing, // Match läuft
    Victory  // Ein Spieler hat 12 Punkte erreicht
  };

  // Richtungen für die Schlangenbewegung
  enum class Direction {
    Up,
    Down,
    Left,
    Right
  };

  // Eine Position auf dem Kachel-Gitter (Grid)
  struct Cell {
    int x;
    int y;

    bool operator==(const Cell &other) const {
      return x == other.x && y == other.y;
    }
  };

  // Zustand einer Schlange
  struct Snake {
    std::deque<Cell> body;
    Direction dir = Direction::Right;
    Direction nextDir = Direction::Right;
    int score = 0;
    float respawnTimer = 0.0f;
    uint8_t bodyColor;
    uint8_t headColor;
  };

  State state = State::Title;

  // --- Gitter-Maße (320x240 Pixel / Kachelgröße 8x8) ---
  static constexpr int CELL_SIZE = 8;
  static constexpr int GRID_WIDTH = 40;  // 40 Kacheln breit (0 bis 39)
  static constexpr int GRID_HEIGHT = 30; // 30 Kacheln hoch (0 bis 29)
  static constexpr int TOP_OFFSET = 3;   // Oberste 3 Reihen (0-2) sind für den Spielstand reserviert

  // Spielfeldbereich: X = 1..38, Y = 3..28
  static constexpr int MIN_X = 1;
  static constexpr int MAX_X = 38;
  static constexpr int MIN_Y = 3;
  static constexpr int MAX_Y = 28;

  // Schlangen für beide Spieler
  Snake p1; // Spieler 1 (Gelb)
  Snake p2; // Spieler 2 (Blau)

  // Liste aller Äpfel auf dem Feld
  std::vector<Cell> apples;
  const int targetScore = 12; // Wer zuerst 12 Punkte hat, gewinnt!

  // Takt-Timer: Schlangen bewegen sich alle 0.11 Sekunden um 1 Feld
  float moveTimer = 0.0f;
  const float stepDelay = 0.11f;

  // ===========================================================================
  // 2. KONSTRUKTOR (Wird beim Start einmal ausgeführt)
  // ===========================================================================
  SnakeGame() {
    resetGame();
  }

  // ===========================================================================
  // 3. HAUPTSCHLEIFE (Wird 60-mal pro Sekunde aufgerufen)
  // ===========================================================================
  void update(Engine &e) override {
    // 1. Dunkelgrünen Rasen zeichnen
    e.cls(Colors::DarkGreen);

    // 2. Spielfeldrand und Punktestand oben zeichnen
    drawGrid(e);
    drawScores(e);

    // 3. Je nach Spielphase Logik ausführen
    switch (state) {
    case State::Title:
      updateTitle(e);
      break;
    case State::Playing:
      updatePlaying(e);
      break;
    case State::Victory:
      updateVictory(e);
      break;
    }

    // 4. Äpfel und Schlangen zeichnen
    drawApples(e);
    drawSnakes(e);
  }

  // ===========================================================================
  // Sound-Effekte
  // ===========================================================================
  void playNomSound(Engine &e) {
    // Heller Blip beim Apfelessen (G5)
    e.play_tone(Notes::G5, 0.06f);
  }

  void playCrashSound(Engine &e) {
    // Tiefer Knall beim Zusammenstoß (Fs3)
    e.play_tone(Notes::Fs3, 0.20f);
  }

  void playStartSound(Engine &e) {
    // Fröhliche 4-Ton-Aufstiegs-Fanfare
    e.play_melody({
        {Notes::C5, 0.10f},
        {Notes::E5, 0.10f},
        {Notes::G5, 0.10f},
        {Notes::C6, 0.25f}
    });
  }

  void playVictorySound(Engine &e) {
    // Sieges-Melodie
    e.play_melody({
        {Notes::C5, 0.12f},
        {Notes::E5, 0.12f},
        {Notes::G5, 0.12f},
        {Notes::C6, 0.15f},
        {Notes::A5, 0.15f},
        {Notes::C6, 0.35f}
    });
  }

  // ===========================================================================
  // Spiel-Verwaltung & Initialisierung
  // ===========================================================================
  void resetGame() {
    state = State::Title;
    initSnake(p1, 1);
    initSnake(p2, 2);
    apples.clear();
    spawnInitialApples(5);
  }

  void initSnake(Snake &s, int playerNum) {
    s.body.clear();
    s.score = 0;
    s.respawnTimer = 0.0f;

    if (playerNum == 1) {
      // Spieler 1 startet oben links und blickt nach rechts
      s.dir = Direction::Right;
      s.nextDir = Direction::Right;
      s.bodyColor = Colors::Yellow;
      s.headColor = Colors::Orange;
      s.body.push_back({6, 8});
      s.body.push_back({5, 8});
      s.body.push_back({4, 8});
    } else {
      // Spieler 2 startet unten rechts und blickt nach links
      s.dir = Direction::Left;
      s.nextDir = Direction::Left;
      s.bodyColor = Colors::Blue;
      s.headColor = Colors::White;
      s.body.push_back({33, 23});
      s.body.push_back({34, 23});
      s.body.push_back({35, 23});
    }
  }

  // Lässt eine Schlange an ihrer Startposition neu starten (nach einem Crash)
  void respawnSnake(Snake &s, int playerNum) {
    s.body.clear();
    s.score = 0;
    s.respawnTimer = 0.5f; // Kurze Pause vor dem Weiterkriechen

    if (playerNum == 1) {
      s.dir = Direction::Right;
      s.nextDir = Direction::Right;
      s.body.push_back({6, 8});
      s.body.push_back({5, 8});
      s.body.push_back({4, 8});
    } else {
      s.dir = Direction::Left;
      s.nextDir = Direction::Left;
      s.body.push_back({33, 23});
      s.body.push_back({34, 23});
      s.body.push_back({35, 23});
    }
  }

  // Platziert Äpfel an freien Stellen auf dem Spielfeld
  void spawnInitialApples(int count) {
    for (int i = 0; i < count; ++i) {
      spawnApple();
    }
  }

  void spawnApple() {
    for (int tries = 0; tries < 100; ++tries) {
      int rx = MIN_X + (rand() % (MAX_X - MIN_X + 1));
      int ry = MIN_Y + (rand() % (MAX_Y - MIN_Y + 1));
      Cell candidate{rx, ry};

      // Prüfen, ob die Zelle schon von einem Apfel oder einer Schlange belegt ist
      bool occupied = false;
      for (const auto &a : apples) {
        if (a == candidate) {
          occupied = true;
          break;
        }
      }
      for (const auto &c : p1.body) {
        if (c == candidate) {
          occupied = true;
          break;
        }
      }
      for (const auto &c : p2.body) {
        if (c == candidate) {
          occupied = true;
          break;
        }
      }

      if (!occupied) {
        apples.push_back(candidate);
        return;
      }
    }
  }

  // ===========================================================================
  // Tastatursteuerung
  // ===========================================================================
  void handleInput(Engine &e) {
    // Spieler 1 (Gelb): W, A, S, D
    if (e.key(Key::W) && p1.dir != Direction::Down)
      p1.nextDir = Direction::Up;
    else if (e.key(Key::S) && p1.dir != Direction::Up)
      p1.nextDir = Direction::Down;
    else if (e.key(Key::A) && p1.dir != Direction::Right)
      p1.nextDir = Direction::Left;
    else if (e.key(Key::D) && p1.dir != Direction::Left)
      p1.nextDir = Direction::Right;

    // Spieler 2 (Blau): Pfeiltasten
    if (e.key(Key::Up) && p2.dir != Direction::Down)
      p2.nextDir = Direction::Up;
    else if (e.key(Key::Down) && p2.dir != Direction::Up)
      p2.nextDir = Direction::Down;
    else if (e.key(Key::Left) && p2.dir != Direction::Right)
      p2.nextDir = Direction::Left;
    else if (e.key(Key::Right) && p2.dir != Direction::Left)
      p2.nextDir = Direction::Right;
  }

  // ===========================================================================
  // Spiel-Zustände (Title, Playing, Victory)
  // ===========================================================================
  void updateTitle(Engine &e) {
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    e.draw_text(68, 60, "SNAKE BATTLE 2-PLAYER", Colors::Yellow, 2);
    e.draw_text(60, 95, "SAMMLE 12 AEPFEL ZUM SIEG!", Colors::White, 1);

    e.draw_text(26, 160, "P1 (GELB): W / A / S / D", Colors::Yellow, 1);
    e.draw_text(184, 160, "P2 (BLAU): PFEILTASTEN", Colors::Blue, 1);

    if (blink) {
      e.draw_text(84, 125, "PRESS SPACE TO START", Colors::White, 1);
    }

    if (e.pressed(Key::Space)) {
      playStartSound(e);
      p1.score = 0;
      p2.score = 0;
      initSnake(p1, 1);
      initSnake(p2, 2);
      apples.clear();
      spawnInitialApples(5);
      state = State::Playing;
    }
  }

  void updatePlaying(Engine &e) {
    handleInput(e);

    // Timer herunterzählen (für Respawns)
    if (p1.respawnTimer > 0.0f)
      p1.respawnTimer -= e.dt();
    if (p2.respawnTimer > 0.0f)
      p2.respawnTimer -= e.dt();

    moveTimer += e.dt();
    if (moveTimer >= stepDelay) {
      moveTimer = 0.0f;
      stepSnakes(e);
    }

    // Siegprüfung
    if (p1.score >= targetScore || p2.score >= targetScore) {
      playVictorySound(e);
      state = State::Victory;
    }
  }

  // ===========================================================================
  // Der Bewegungsschritt beider Schlangen (Kern der Spiellogik)
  // ===========================================================================
  void stepSnakes(Engine &e) {
    p1.dir = p1.nextDir;
    p2.dir = p2.nextDir;

    // Neue Kopf-Positionen berechnen
    Cell nextHead1 = getNextCell(p1.body.front(), p1.dir);
    Cell nextHead2 = getNextCell(p2.body.front(), p2.dir);

    bool p1Crashed = false;
    bool p2Crashed = false;

    // 1. Kollision mit Spielfeldrand prüfen
    if (nextHead1.x < MIN_X || nextHead1.x > MAX_X || nextHead1.y < MIN_Y || nextHead1.y > MAX_Y) {
      p1Crashed = true;
    }
    if (nextHead2.x < MIN_X || nextHead2.x > MAX_X || nextHead2.y < MIN_Y || nextHead2.y > MAX_Y) {
      p2Crashed = true;
    }

    // 2. Kollision mit eigenem Körper prüfen
    for (size_t i = 0; i < p1.body.size() - 1; ++i) {
      if (p1.body[i] == nextHead1)
        p1Crashed = true;
    }
    for (size_t i = 0; i < p2.body.size() - 1; ++i) {
      if (p2.body[i] == nextHead2)
        p2Crashed = true;
    }

    // 3. Kollision mit gegnerischer Schlange prüfen
    for (const auto &c : p2.body) {
      if (c == nextHead1)
        p1Crashed = true;
    }
    for (const auto &c : p1.body) {
      if (c == nextHead2)
        p2Crashed = true;
    }

    // Beide stoßen frontal zusammen
    if (nextHead1 == nextHead2) {
      p1Crashed = true;
      p2Crashed = true;
    }

    // --- MODELL B: DER APFEL-KLAU ---
    // Wenn eine Schlange stirbt, platzt ihr ganzer Körper in Äpfel auf das Feld!
    if (p1Crashed) {
      playCrashSound(e);
      // Jeden Körperteil (außer Kopf) in einen Apfel auf dem Feld verwandeln!
      for (size_t i = 1; i < p1.body.size(); ++i) {
        apples.push_back(p1.body[i]);
      }
      respawnSnake(p1, 1);
    } else {
      // Schlange 1 zieht vorwärts
      p1.body.push_front(nextHead1);
      // Prüfen, ob ein Apfel gefressen wurde
      auto it = std::find(apples.begin(), apples.end(), nextHead1);
      if (it != apples.end()) {
        apples.erase(it);
        p1.score++;
        playNomSound(e);
        spawnApple(); // Neuen Apfel auf dem Feld erzeugen
      } else {
        p1.body.pop_back(); // Schwanz nachziehen
      }
    }

    if (p2Crashed) {
      playCrashSound(e);
      // Schlange 2 platzt in Äpfel
      for (size_t i = 1; i < p2.body.size(); ++i) {
        apples.push_back(p2.body[i]);
      }
      respawnSnake(p2, 2);
    } else {
      // Schlange 2 zieht vorwärts
      p2.body.push_front(nextHead2);
      auto it = std::find(apples.begin(), apples.end(), nextHead2);
      if (it != apples.end()) {
        apples.erase(it);
        p2.score++;
        playNomSound(e);
        spawnApple();
      } else {
        p2.body.pop_back();
      }
    }

    // Immer mindestens 4 Äpfel auf dem Feld halten
    while (apples.size() < 4) {
      spawnApple();
    }
  }

  Cell getNextCell(const Cell &current, Direction d) const {
    Cell next = current;
    switch (d) {
    case Direction::Up:
      next.y -= 1;
      break;
    case Direction::Down:
      next.y += 1;
      break;
    case Direction::Left:
      next.x -= 1;
      break;
    case Direction::Right:
      next.x += 1;
      break;
    }
    return next;
  }

  void updateVictory(Engine &e) {
    bool blink = (int(e.time() * 3.0f) % 2) == 0;

    if (p1.score >= targetScore) {
      e.draw_text(74, 80, "SPIELER 1 (GELB) GEWINNT!", Colors::Yellow, 1);
    } else {
      e.draw_text(74, 80, "SPIELER 2 (BLAU) GEWINNT!", Colors::Blue, 1);
    }

    if (blink) {
      e.draw_text(76, 130, "PRESS SPACE TO RESTART", Colors::White, 1);
    }

    if (e.pressed(Key::Space)) {
      playStartSound(e);
      resetGame();
      state = State::Playing;
    }
  }

  // ===========================================================================
  // Zeichnen (Grafik)
  // ===========================================================================
  void drawGrid(Engine &e) {
    // Weißer Spielfeldrahmen um den Spielbereich (Kacheln 1..38, 3..28)
    e.rect(MIN_X * CELL_SIZE - 1, MIN_Y * CELL_SIZE - 1,
           (MAX_X - MIN_X + 1) * CELL_SIZE + 2,
           (MAX_Y - MIN_Y + 1) * CELL_SIZE + 2,
           Colors::White);

    // Trennlinie unter dem Spielstand oben
    e.line(0, 22, 320, 22, Colors::White);
  }

  void drawScores(Engine &e) {
    // Spieler 1 Spielstand (Gelb, links oben)
    e.draw_text(24, 6, "P1 (GELB):", Colors::Yellow, 1);
    e.draw_digit(100, 4, p1.score / 10, Colors::Yellow, 2);
    e.draw_digit(110, 4, p1.score % 10, Colors::Yellow, 2);

    // Zielpunktzahl in der Mitte
    e.draw_text(138, 6, "/ 12", Colors::White, 1);

    // Spieler 2 Spielstand (Blau, rechts oben)
    e.draw_text(210, 6, "P2 (BLAU):", Colors::Blue, 1);
    e.draw_digit(286, 4, p2.score / 10, Colors::Blue, 2);
    e.draw_digit(296, 4, p2.score % 10, Colors::Blue, 2);
  }

  void drawApples(Engine &e) {
    for (const auto &a : apples) {
      float cx = a.x * CELL_SIZE + 4.0f;
      float cy = a.y * CELL_SIZE + 4.0f;
      // Roter saftiger Apfel
      e.circlefill(cx, cy, 3.2f, Colors::Red);
      // Kleines gelbes Glanzlicht
      e.pset(cx - 1, cy - 1, Colors::Yellow);
    }
  }

  void drawSnakes(Engine &e) {
    // 1. Schlange 1 (Gelb)
    for (size_t i = 0; i < p1.body.size(); ++i) {
      const auto &c = p1.body[i];
      uint8_t col = (i == 0) ? p1.headColor : p1.bodyColor;
      e.rectfill(c.x * CELL_SIZE + 1, c.y * CELL_SIZE + 1,
                 CELL_SIZE - 2, CELL_SIZE - 2, col);
      // Augen auf den Kopf zeichnen
      if (i == 0) {
        drawEyes(e, c, p1.dir);
      }
    }

    // 2. Schlange 2 (Blau)
    for (size_t i = 0; i < p2.body.size(); ++i) {
      const auto &c = p2.body[i];
      uint8_t col = (i == 0) ? p2.headColor : p2.bodyColor;
      e.rectfill(c.x * CELL_SIZE + 1, c.y * CELL_SIZE + 1,
                 CELL_SIZE - 2, CELL_SIZE - 2, col);
      if (i == 0) {
        drawEyes(e, c, p2.dir);
      }
    }
  }

  // Zeichnet zwei kleine weiße Schlangen-Augen
  void drawEyes(Engine &e, const Cell &head, Direction d) {
    float x = head.x * CELL_SIZE;
    float y = head.y * CELL_SIZE;

    if (d == Direction::Right) {
      e.pset(x + 6, y + 2, Colors::Black);
      e.pset(x + 6, y + 5, Colors::Black);
    } else if (d == Direction::Left) {
      e.pset(x + 1, y + 2, Colors::Black);
      e.pset(x + 1, y + 5, Colors::Black);
    } else if (d == Direction::Up) {
      e.pset(x + 2, y + 1, Colors::Black);
      e.pset(x + 5, y + 1, Colors::Black);
    } else if (d == Direction::Down) {
      e.pset(x + 2, y + 6, Colors::Black);
      e.pset(x + 5, y + 6, Colors::Black);
    }
  }
};
