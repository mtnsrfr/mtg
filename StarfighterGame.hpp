#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

// =============================================================================
// STARFIGHTER: WELTRAUM-SCHLACHT (Vertical Retro Space Shooter)
// =============================================================================
// Features:
// - Partikel-Sternenfeld: Sterne bewegen sich in 3 Ebenen (Parallaxe-Effekt).
// - Eigenes Raumschiff: 16x16 Delta-Wing / X-Wing Jäger mit Doppellaser.
// - Angreifer (TIE-Fighter): Schwingt hin und her und feuert Plasmageschosse.
// - Explosions-Partikel: Funkelnde Trümmer bei Treffern.
// - Highscore-System & Soundeffekte.
//
// Steuerung:
// - W / A / S / D oder Pfeiltasten: Raumschiff fliegen
// - LEERTASTE (Space): Laser abfeuern
// =============================================================================

struct StarfighterGame : Game {
  StarfighterGame() {
    initStars();
    resetGame();
  }

  void update(Engine &e) override {
    // 1. Weltraumhintergrund (Tiefschwarz)
    e.cls(Colors::Black);

    // 2. Sternenfeld-Partikel bewegen und zeichnen (Parallaxe-Hintergrund)
    updateAndDrawStars(e);

    // 3. Je nach Zustand Spielablauf steuern
    switch (state) {
    case State::Title:
      updateTitle(e);
      break;
    case State::Playing:
      updatePlaying(e);
      break;
    case State::GameOver:
      updateGameOver(e);
      break;
    }

    // 4. Laser, Explosionen und Schiffe zeichnen
    updateAndDrawExplosions(e);
    drawLasers(e);

    if (state != State::GameOver || playerAlive) {
      drawPlayer(e);
    }
    if (enemyAlive) {
      drawEnemy(e);
    }

    // 5. Spielstand oben anzeigen
    drawUI(e);
  }

private:
  enum class State {
    Title,
    Playing,
    GameOver
  };

  State state = State::Title;

  // --- Sternen-Partikel ---
  struct Star {
    float x;
    float y;
    float speed;
    uint8_t color;
  };

  std::vector<Star> stars;

  // --- Explosions-Partikel ---
  struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    float life;    // Lebensdauer in Sekunden
    float maxLife;
    uint8_t color;
  };

  std::vector<Particle> particles;

  // --- Laser-Geschosse ---
  struct Laser {
    float x;
    float y;
    float vy;
    bool fromPlayer;
  };

  std::vector<Laser> lasers;

  // --- Spieler (Starfighter) ---
  float playerX = 152.0f; // X-Position (Mitte)
  float playerY = 190.0f; // Y-Position (unten)
  const float playerSpeed = 150.0f;
  bool playerAlive = true;
  float shootCooldown = 0.0f;

  // --- Gegner (Attacker) ---
  float enemyX = 152.0f;
  float enemyY = 35.0f;
  float enemyVx = 90.0f;
  float enemyVy = 0.0f;
  float enemyShootTimer = 1.0f;
  bool enemyAlive = true;
  float enemyRespawnTimer = 0.0f;
  float enemyMovePhase = 0.0f;

  // --- Punkte & Highscore ---
  int score = 0;
  int highscore = 0;

  // ===========================================================================
  // Sound-Effekte
  // ===========================================================================
  void playPlayerShoot(Engine &e) {
    // Hoher, schneller Laser-Pew (Note As5)
    e.play_tone(Notes::As5, 0.04f);
  }

  void playEnemyShoot(Engine &e) {
    // Dunkler Alien-Plasma-Pew (Note Gs4)
    e.play_tone(Notes::Gs4, 0.05f);
  }

  void playExplosion(Engine &e) {
    // Tiefer, satter Wumm bei Explosion (Tiefer Bass C3)
    e.play_tone(Notes::C3, 0.22f);
  }

  void playStartSound(Engine &e) {
    e.play_melody({
        {Notes::A4, 0.08f},
        {Notes::Cs5, 0.08f},
        {Notes::E5, 0.08f},
        {Notes::A5, 0.25f}
    });
  }

  void playGameOverSound(Engine &e) {
    e.play_melody({
        {Notes::A4, 0.15f},
        {Notes::Fs4, 0.15f},
        {Notes::Ds4, 0.18f},
        {Notes::A3, 0.40f}
    });
  }

  // ===========================================================================
  // Sternenfeld (Parallaxe-Partikel)
  // ===========================================================================
  void initStars() {
    stars.clear();
    // 70 Sterne in 3 verschiedenen Geschwindigkeits- und Helligkeitsstufen
    for (int i = 0; i < 70; ++i) {
      Star s;
      s.x = static_cast<float>(rand() % 320);
      s.y = static_cast<float>(rand() % 240);
      int layer = rand() % 3;
      if (layer == 0) {
        s.speed = 25.0f;  // Weit entfernte, dunkle Sterne (langsam)
        s.color = Colors::DarkGray;
      } else if (layer == 1) {
        s.speed = 65.0f;  // Mittlere Sterne
        s.color = Colors::LightGray;
      } else {
        s.speed = 130.0f; // Nahe, hell leuchtende Sterne (schnell)
        s.color = Colors::White;
      }
      stars.push_back(s);
    }
  }

  void updateAndDrawStars(Engine &e) {
    float dt = e.dt();
    for (auto &s : stars) {
      s.y += s.speed * dt;
      // Wenn der Stern unten herausfliegt, oben mit neuer X-Position wieder einfliegen
      if (s.y >= 240.0f) {
        s.y = 0.0f;
        s.x = static_cast<float>(rand() % 320);
      }
      e.pset(s.x, s.y, s.color);
    }
  }

  // ===========================================================================
  // ===========================================================================
  // Explosions-Effekt (Trägheits-Vektor des Schiffs + radiale Streuung + Farbverlauf)
  // ===========================================================================
  void spawnExplosion(float x, float y, float shipVx = 0.0f, float shipVy = 0.0f, int count = 50) {
    for (int i = 0; i < count; ++i) {
      Particle p;
      p.x = x;
      p.y = y;

      // 1. Grundimpuls: Voller Geschwindigkeitsvektor des Schiffs (starke Richtungsdynamik!)
      // 2. Überlagerte radiale Explosions-Sprengung in alle Richtungen
      float angle = (rand() % 360) * (M_PI / 180.0f);
      float burstSpeed = 15.0f + (rand() % 85);

      p.vx = shipVx + burstSpeed * std::cos(angle);
      p.vy = shipVy + burstSpeed * std::sin(angle);

      p.maxLife = 0.45f + (rand() % 35) / 100.0f; // 0.45s bis 0.80s Lebensdauer
      p.life = p.maxLife;

      particles.push_back(p);
    }
  }

  void updateAndDrawExplosions(Engine &e) {
    float dt = e.dt();
    for (size_t i = 0; i < particles.size();) {
      auto &p = particles[i];
      p.life -= dt;
      if (p.life <= 0.0f) {
        // Partikel löschen
        particles[i] = particles.back();
        particles.pop_back();
      } else {
        p.x += p.vx * dt;
        p.y += p.vy * dt;

        // Leichte Trägheits-Verlangsamung
        p.vx *= 0.975f;
        p.vy *= 0.975f;

        // Dynamischer Farbverlauf über die Lebensdauer:
        // Frisch (100% - 75%): Weiß (Glühend heißer Blitz)
        // Heiß   (75% - 50%): Gelb
        // Warm   (50% - 25%): Orange
        // Kalt   (25% - 10%): Rot
        // Rauch  (10% - 0%):  Dunkelgrau
        float progress = p.life / p.maxLife; // 1.0 (frisch) bis 0.0 (erloschen)
        uint8_t color = Colors::DarkGray;

        if (progress > 0.75f) {
          color = Colors::White;
        } else if (progress > 0.50f) {
          color = Colors::Yellow;
        } else if (progress > 0.25f) {
          color = Colors::Orange;
        } else if (progress > 0.10f) {
          color = Colors::Red;
        }

        e.pset(p.x, p.y, color);
        ++i;
      }
    }
  }

  // ===========================================================================
  // Spiel-Logik & Neustart
  // ===========================================================================
  void resetGame() {
    score = 0;
    playerX = 152.0f;
    playerY = 190.0f;
    playerAlive = true;
    shootCooldown = 0.0f;

    enemyX = 152.0f;
    enemyY = 35.0f;
    enemyVx = 90.0f;
    enemyAlive = true;
    enemyRespawnTimer = 0.0f;
    enemyShootTimer = 0.8f;
    enemyMovePhase = 0.0f;

    lasers.clear();
    particles.clear();
  }

  void updateTitle(Engine &e) {
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    e.draw_text(60, 50, "STARFIGHTER ATTACK", Colors::Yellow, 2);
    e.draw_text(74, 85, "VERTEIDIGE DIE GALAXIE!", Colors::White, 1);

    e.draw_text(64, 150, "STEUERUNG: W / A / S / D", Colors::Blue, 1);
    e.draw_text(64, 168, "FEUER    : LEERTASTE", Colors::Red, 1);

    if (blink) {
      e.draw_text(84, 120, "PRESS SPACE TO START", Colors::White, 1);
    }

    if (e.pressed(Key::Space)) {
      playStartSound(e);
      resetGame();
      state = State::Playing;
    }
  }

  void updatePlaying(Engine &e) {
    float dt = e.dt();

    // 1. Spieler-Bewegung (W/A/S/D oder Pfeiltasten)
    float curPlayerVx = 0.0f;
    float curPlayerVy = 0.0f;

    if (e.key(Key::A) || e.key(Key::Left)) {
      playerX -= playerSpeed * dt;
      curPlayerVx = -playerSpeed;
    }
    if (e.key(Key::D) || e.key(Key::Right)) {
      playerX += playerSpeed * dt;
      curPlayerVx = playerSpeed;
    }
    if (e.key(Key::W) || e.key(Key::Up)) {
      playerY -= playerSpeed * dt;
      curPlayerVy = -playerSpeed;
    }
    if (e.key(Key::S) || e.key(Key::Down)) {
      playerY += playerSpeed * dt;
      curPlayerVy = playerSpeed;
    }

    // Spieler im Bildschirm halten (16x16 Raumschiff)
    playerX = std::clamp(playerX, 8.0f, 320.0f - 24.0f);
    playerY = std::clamp(playerY, 30.0f, 240.0f - 24.0f);

    // 2. Spieler-Schuss (Doppellaser)
    shootCooldown -= dt;
    if (e.key(Key::Space) && shootCooldown <= 0.0f) {
      shootCooldown = 0.18f; // Schussrate
      playPlayerShoot(e);
      // Linker und rechter Flügellaser
      lasers.push_back({playerX + 2.0f, playerY, -340.0f, true});
      lasers.push_back({playerX + 13.0f, playerY, -340.0f, true});
    }

    // 3. Gegner-KI & Bewegung (Attacker fliegt sanfte Kurven & taucht ab)
    float curEnemyVy = 0.0f;
    if (enemyAlive) {
      enemyMovePhase += dt * 2.5f;
      enemyX += enemyVx * dt;
      curEnemyVy = std::cos(enemyMovePhase) * 18.0f * 2.5f + 10.0f;
      enemyY = 35.0f + std::sin(enemyMovePhase) * 18.0f;

      // Wand-Umkehr für den Gegner
      if (enemyX <= 16.0f) {
        enemyX = 16.0f;
        enemyVx = std::abs(enemyVx);
      } else if (enemyX >= 320.0f - 32.0f) {
        enemyX = 320.0f - 32.0f;
        enemyVx = -std::abs(enemyVx);
      }

      // Gegner schießt periodisch Laser nach unten
      enemyShootTimer -= dt;
      if (enemyShootTimer <= 0.0f) {
        enemyShootTimer = 0.75f + (rand() % 40) / 100.0f;
        playEnemyShoot(e);
        // Schüsse aus den beiden langen Frontkanonen
        lasers.push_back({enemyX + 2.0f, enemyY + 14.0f, 220.0f, false});
        lasers.push_back({enemyX + 13.0f, enemyY + 14.0f, 220.0f, false});
      }
    } else {
      // Wenn der Gegner zerstört wurde: Nach 1 Sekunde taucht ein neuer auf!
      enemyRespawnTimer -= dt;
      if (enemyRespawnTimer <= 0.0f) {
        enemyAlive = true;
        enemyX = 20.0f + (rand() % 260);
        enemyY = 25.0f;
        // Wird mit höherem Score etwas schneller
        enemyVx = (rand() % 2 == 0 ? 90.0f : -90.0f) * (1.0f + score * 0.03f);
        enemyShootTimer = 0.6f;
      }
    }

    // 4. Laser-Geschosse bewegen & Kollisionen prüfen
    for (size_t i = 0; i < lasers.size();) {
      auto &l = lasers[i];
      l.y += l.vy * dt;

      bool hit = false;

      // Spieler-Laser trifft Gegner?
      if (l.fromPlayer && enemyAlive) {
        if (l.x >= enemyX && l.x <= enemyX + 16.0f &&
            l.y >= enemyY && l.y <= enemyY + 16.0f) {
          hit = true;
          enemyAlive = false;
          enemyRespawnTimer = 0.8f;
          score++;
          if (score > highscore)
            highscore = score;
          playExplosion(e);
          spawnExplosion(enemyX + 8.0f, enemyY + 8.0f, enemyVx, curEnemyVy, 50);
        }
      }

      // Gegner-Laser trifft Spieler?
      if (!l.fromPlayer && playerAlive) {
        if (l.x >= playerX + 2.0f && l.x <= playerX + 14.0f &&
            l.y >= playerY + 2.0f && l.y <= playerY + 14.0f) {
          hit = true;
          playerAlive = false;
          playExplosion(e);
          playGameOverSound(e);
          float effPlayerVy = (curPlayerVy != 0.0f) ? curPlayerVy : -70.0f;
          spawnExplosion(playerX + 8.0f, playerY + 8.0f, curPlayerVx, effPlayerVy, 55);
          state = State::GameOver;
        }
      }

      // Schiff-zu-Schiff Crash?
      if (playerAlive && enemyAlive) {
        if (std::abs(playerX - enemyX) < 14.0f && std::abs(playerY - enemyY) < 14.0f) {
          playerAlive = false;
          enemyAlive = false;
          playExplosion(e);
          playGameOverSound(e);
          float effPlayerVy = (curPlayerVy != 0.0f) ? curPlayerVy : -70.0f;
          spawnExplosion(playerX + 8.0f, playerY + 8.0f, curPlayerVx, effPlayerVy, 50);
          spawnExplosion(enemyX + 8.0f, enemyY + 8.0f, enemyVx, curEnemyVy, 50);
          state = State::GameOver;
        }
      }

      // Laser löschen wenn außerhalb oder getroffen
      if (hit || l.y < -10.0f || l.y > 250.0f) {
        lasers[i] = lasers.back();
        lasers.pop_back();
      } else {
        ++i;
      }
    }
  }

  void updateGameOver(Engine &e) {
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    e.draw_text(110, 75, "GAME OVER", Colors::Red, 2);

    e.draw_text(90, 110, "DEIN SCORE:", Colors::White, 1);
    e.draw_digit(190, 108, score / 10, Colors::Yellow, 2);
    e.draw_digit(200, 108, score % 10, Colors::Yellow, 2);

    if (blink) {
      e.draw_text(76, 150, "PRESS SPACE TO RESTART", Colors::White, 1);
    }

    if (e.pressed(Key::Space)) {
      playStartSound(e);
      resetGame();
      state = State::Playing;
    }
  }

  // ===========================================================================
  // Zeichnen: Schiffe & Laser (Pixel-Art)
  // ===========================================================================

  // Zeichnet das 16x16 Spieler-Schiff (Delta X-Wing)
  void drawPlayer(Engine &e) {
    // 16x16 Pixelmuster:
    // clang-format off
    static const uint16_t playerSprite[16] = {
      0b0000000110000000, // Spitze
      0b0000000110000000,
      0b0000001111000000, // Cockpit
      0b0000001111000000,
      0b0100001111000010, // Flügelkanonen vorn
      0b0100011111100010,
      0b0100011111100010,
      0b0100111111110010, // Delta-Flügel
      0b1100111111110011,
      0b1101111111111011,
      0b1111111111111111, // Rumpf
      0b1111111111111111,
      0b0111011001101110, // Triebwerke
      0b0010001001000100,
      0b0000001001000000, // Schubflammen
      0b0000000000000000,
    };
    // clang-format on

    float px = playerX;
    float py = playerY;

    for (int r = 0; r < 16; ++r) {
      uint16_t row = playerSprite[r];
      for (int c = 0; c < 16; ++c) {
        if (row & (1 << (15 - c))) {
          uint8_t col = Colors::White;
          // Cockpit in Cyan/Blau
          if (r >= 2 && r <= 4 && c >= 6 && c <= 9)
            col = Colors::Blue;
          // Flügelkanonen in Rot
          else if ((c <= 1 || c >= 14) && r >= 4 && r <= 10)
            col = Colors::Red;
          // Triebwerksflamme in Orange/Gelb
          else if (r >= 13)
            col = (rand() % 2 == 0 ? Colors::Orange : Colors::Yellow);

          e.pset(px + c, py + r, col);
        }
      }
    }
  }

  // Zeichnet das 16x16 Gegner-Schiff (TIE Attacker mit langen Frontkanonen)
  void drawEnemy(Engine &e) {
    // 16x16 Pixelmuster:
    // clang-format off
    static const uint16_t enemySprite[16] = {
      0b1111111111111111, // Flügel-Panzerung oben
      0b1100000000000011,
      0b1100001111000011, // Zentrales Cockpit
      0b1100011111100011, // Rotes Auge
      0b1100011111100011,
      0b1100001111000011,
      0b1100000000000011, // Cockpit endet, lange Kanonen ragen vor!
      0b1100000000000011,
      0b1100000000000011, // Front-Kanonenrohre
      0b1100000000000011,
      0b1100000000000011,
      0b0100000000000010, // Mündungen
      0b0000000000000000,
      0b0000000000000000,
      0b0000000000000000,
      0b0000000000000000,
    };
    // clang-format on

    float ex = enemyX;
    float ey = enemyY;

    for (int r = 0; r < 16; ++r) {
      uint16_t row = enemySprite[r];
      for (int c = 0; c < 16; ++c) {
        if (row & (1 << (15 - c))) {
          uint8_t col = Colors::DarkGray;
          // Feindliches Auge im Zentrum leuchtet Rot
          if (r >= 3 && r <= 4 && c >= 7 && c <= 8)
            col = Colors::Red;
          // Kanonenspitzen glühen Grün
          else if (r >= 10 && (c == 0 || c == 1 || c == 14 || c == 15))
            col = Colors::LightGreen;

          e.pset(ex + c, ey + r, col);
        }
      }
    }
  }

  void drawLasers(Engine &e) {
    for (const auto &l : lasers) {
      if (l.fromPlayer) {
        // Spieler-Laser: Leuchtend Blau & Weißer Kern
        e.line(l.x, l.y, l.x, l.y + 5.0f, Colors::Blue);
        e.pset(l.x, l.y, Colors::White);
      } else {
        // Feind-Laser: Leuchtend Hellgrün & Gelber Kern
        e.line(l.x, l.y, l.x, l.y + 5.0f, Colors::LightGreen);
        e.pset(l.x, l.y + 5.0f, Colors::Yellow);
      }
    }
  }

  void drawUI(Engine &e) {
    // Obere Infoleiste
    e.draw_text(16, 8, "SCORE:", Colors::White, 1);
    e.draw_digit(68, 6, score / 10, Colors::Yellow, 2);
    e.draw_digit(78, 6, score % 10, Colors::Yellow, 2);

    e.draw_text(200, 8, "HIGH:", Colors::White, 1);
    e.draw_digit(244, 6, highscore / 10, Colors::Red, 2);
    e.draw_digit(254, 6, highscore % 10, Colors::Red, 2);

    // Dünne Trennlinie unter dem Scoreboard
    e.line(0, 24, 320, 24, Colors::DarkGray);
  }
};
