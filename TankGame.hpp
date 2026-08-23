#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
// 💥 TANK DUEL: RETRO ARTILLERY (2-Spieler Runden-Panzerduell)
// =============================================================================
// Features:
// - 🏔️ Prozedurales zerstörbares Gelände mit visueller Schnee-/Sediment-Simulation:
//      1. Phase: Zufallspunkte linear verbunden + "Grey Noise" Felsuntergrund.
//      2. Phase: Erde schneit von oben herab (~320*5 Partikel) & füllt Täler.
//      3. Phase: Gras schneit herab (~320*2 Partikel) & glättet sanft die Kuppen.
// - 📐 Präzise Winkelsteuerung mit Anzeige in GRAD (Degrees) & BOGENMASS (Radians)!
// - ⚡ Stärke-Einstellung von 0 bis 99 (Startwert 50).
// - 💣 Ballistische Geschossphysik mit Schwerkraft & Partikel-Rauchschweif.
// - 💥 Zerstörbare Krater-Explosionen: Sprengt Löcher ins Gelände & lässt Panzer nachrutschen!
// =============================================================================

struct TankGame : Game {
  // ===========================================================================
  // 1. TYPEN & STRUKTUREN
  // ===========================================================================
  enum class State {
    GeneratingTerrain, // Visuelle Geländegenerierung & Sediment-Simulation
    Aiming,            // Aktiver Spieler stellt Winkel & Kraft ein
    BulletInFlight,    // Ballistisches Geschoss fliegt
    Exploding,         // Krater-Explosion & Schadensberechnung
    TankFalling,       // Panzer rutschen nach, falls Boden weggesprengt wurde
    GameOver           // Siegbildschirm
  };

  enum class GenPhase {
    JaggedLinesAndRockNoise,
    DirtSnowing,
    GrassSnowing,
    Completed
  };

  // --- Partikel (Rauch, Explosionen, Schmutz) ---
  struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    float maxLife;
    uint8_t color;
  };

  // --- Ballistisches Projektil ---
  struct Bullet {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    bool active = false;
    int shooter = 0; // 0 = Spieler 1, 1 = Spieler 2
  };

  // --- Panzer-Daten ---
  struct Tank {
    float x = 0.0f;
    float y = 0.0f; // Y-Position des Panzerbodens
    float angleDeg = 45.0f; // 0 = rechts, 90 = oben, 180 = links
    float power = 50.0f;    // [0, 100)
    int hp = 100;
    const int maxHp = 100;
    uint8_t primaryColor = Colors::Blue;
    uint8_t secondaryColor = Colors::Cyan;
    bool alive = true;
    float barrelLen = 12.0f;
  };

  // ===========================================================================
  // 2. SPIEL-VARIABLEN
  // ===========================================================================
  State state = State::GeneratingTerrain;
  GenPhase genPhase = GenPhase::JaggedLinesAndRockNoise;

  // 2D Geländegitter (0 = Luft, sonst Paletten-Farbwert)
  uint8_t terrain[320][240];

  // Geländegenerierungs-Zustand
  float genTimer = 0.0f;
  int dirtParticlesSpawned = 0;
  const int totalDirtParticles = 320 * 10; // 3200 Erdpartikel (2x mehr)
  int grassParticlesSpawned = 0;
  const int totalGrassParticles = 320 * 4;  // 1280 Graspartikel (2x mehr)

  // Spieler
  Tank tanks[2];
  int activePlayer = 0; // 0 = Spieler 1 (Links), 1 = Spieler 2 (Rechts)
  int winner = -1;

  // Geschoss & Explosion
  Bullet bullet;
  float explosionX = 0.0f;
  float explosionY = 0.0f;
  float explosionRadius = 0.0f;
  const float maxExplosionRadius = 14.0f;
  float explosionTimer = 0.0f;

  std::vector<Particle> particles;
  float animTimer = 0.0f;
  float screenShake = 0.0f;

  // ===========================================================================
  // 3. KONSTRUKTOR & RESET
  // ===========================================================================
  TankGame() {
    startNewGame();
  }

  void startNewGame() {
    // Panzer initialisieren
    tanks[0].hp = 100;
    tanks[0].angleDeg = 45.0f; // Zeigt nach rechts oben zum Gegner
    tanks[0].power = 50.0f;
    tanks[0].primaryColor = Ramps::Blue[9];
    tanks[0].secondaryColor = Ramps::Blue[13];
    tanks[0].alive = true;

    tanks[1].hp = 100;
    tanks[1].angleDeg = 135.0f; // Zeigt nach links oben zum Gegner
    tanks[1].power = 50.0f;
    tanks[1].primaryColor = Ramps::Red[9];
    tanks[1].secondaryColor = Ramps::Red[13];
    tanks[1].alive = true;

    activePlayer = 0;
    winner = -1;
    particles.clear();
    bullet.active = false;

    // Starte visuelle Geländegenerierung
    initTerrainGeneration();
  }

  uint32_t terrainSeed = 1337;

  // ===========================================================================
  // 4. PROZEDURALE GELÄNDEGENERIERUNG (Visuell & Physikalisch)
  // ===========================================================================
  void initTerrainGeneration() {
    state = State::GeneratingTerrain;
    genPhase = GenPhase::JaggedLinesAndRockNoise;
    genTimer = 0.0f;
    dirtParticlesSpawned = 0;
    grassParticlesSpawned = 0;
    terrainSeed = static_cast<uint32_t>(rand() * 65537 + rand());

    // 1. Gesamtes Gitter leeren
    for (int x = 0; x < 320; ++x) {
      for (int y = 0; y < 240; ++y) {
        terrain[x][y] = 0;
      }
    }

    // 2. Phase 1: Makro-Stützpunkte mit starker Höhenvarianz (Hindernisberge in der Mitte!)
    const int numPoints = 11;
    struct Point { int x; int y; };
    Point pts[numPoints];

    // Start-Täler für beide Panzer (stabiler Boden bei Y ≈ 195 - 215)
    pts[0] = {0, rand() % 15 + 195};
    pts[1] = {35, rand() % 15 + 195};
    pts[2] = {70, rand() % 25 + 175};

    // Zentrale Hindernisse & Bergmassive (Höhenvarianz zwischen 105 und 155 zum Drüberschießen!)
    int midTheme = rand() % 3;
    if (midTheme == 0) {
      // Großer zentraler Bergkegel
      pts[3] = {105, rand() % 25 + 145};
      pts[4] = {135, rand() % 30 + 110}; // Zentraler Hauptgipfel
      pts[5] = {160, rand() % 25 + 105}; // Höchste Bergspitze (Y ≈ 105-130)
      pts[6] = {185, rand() % 30 + 115};
      pts[7] = {215, rand() % 25 + 145};
    } else if (midTheme == 1) {
      // Zwillings-Gipfel mit Bergpass in der Mitte
      pts[3] = {105, rand() % 25 + 115}; // Gipfel 1
      pts[4] = {130, rand() % 20 + 120};
      pts[5] = {160, rand() % 20 + 165}; // Schlucht / Pass
      pts[6] = {190, rand() % 20 + 120};
      pts[7] = {215, rand() % 25 + 115}; // Gipfel 2
    } else {
      // Weite Hochebene / Mesa mit steilen Felskanten
      pts[3] = {105, rand() % 20 + 140};
      pts[4] = {135, rand() % 15 + 125};
      pts[5] = {160, rand() % 15 + 125};
      pts[6] = {185, rand() % 15 + 125};
      pts[7] = {215, rand() % 20 + 140};
    }

    pts[8] = {250, rand() % 25 + 175};
    pts[9] = {285, rand() % 15 + 195};
    pts[10] = {319, rand() % 15 + 195};

    // Lineare Interpolation zwischen Makropunkten (ohne Hochfrequenz-Zacken)
    int curPt = 0;
    for (int x = 0; x < 320; ++x) {
      while (curPt < numPoints - 2 && x > pts[curPt + 1].x) {
        curPt++;
      }
      float t = static_cast<float>(x - pts[curPt].x) / static_cast<float>(std::max(1, pts[curPt + 1].x - pts[curPt].x));
      t = std::clamp(t, 0.0f, 1.0f);
      int surfaceY = static_cast<int>(pts[curPt].y + t * (pts[curPt + 1].y - pts[curPt].y));
      surfaceY = std::clamp(surfaceY, 80, 230);

      // Fülle Felsuntergrund mit hellem Granit & Kalkstein über Spatial Noise
      for (int y = surfaceY; y < 240; ++y) {
        float n = Engine::noise(x, y, terrainSeed + 101);
        if (n < 0.55f) {
          terrain[x][y] = Ramps::Grays[8 + static_cast<int>(n * 5.0f)]; // Heller Granit
        } else if (n < 0.88f) {
          terrain[x][y] = Ramps::Grays[11 + static_cast<int>((n - 0.55f) * 6.0f)]; // Sehr heller Kalkstein
        } else {
          terrain[x][y] = Ramps::Grays[6]; // Weicher Schattenfels
        }
      }
    }
  }

  // Simulationsschritt für fallende Partikel (Erde & Gras mit individuellem Per-Partikel-Rauschen)
  void simulateFallingParticles(bool isGrass, int count = 70) {
    // Lass Partikel von oben herabrieseln
    for (int n = 0; n < count; ++n) {
      int spawnX = rand() % 320;
      int curX = spawnX;
      int curY = 0;

      // Jedes einzelne Partikel wählt eine eigene Farbe (beseitigt Farbstreifen!)
      uint8_t particleColor;
      if (isGrass) {
        particleColor = (rand() % 3 == 0) ? Ramps::Green[6 + (rand() % 5)] : Ramps::Earth::Moss[4 + (rand() % 4)];
      } else {
        particleColor = (rand() % 3 == 0) ? Ramps::Earth::Clay[2 + (rand() % 5)] : Ramps::Earth::Soil[2 + (rand() % 5)];
      }

      // Partikel fällt nach unten
      while (curY < 239) {
        // Freier Fall nach unten
        if (terrain[curX][curY + 1] == 0) {
          curY++;
        } else {
          // Auftreffen auf Untergrund: Prüfe linke / rechte Diagonale zum Auffüllen von Tälern
          bool canSlideLeft = (curX > 0 && terrain[curX - 1][curY + 1] == 0);
          bool canSlideRight = (curX < 319 && terrain[curX + 1][curY + 1] == 0);

          if (isGrass) {
            // Gras-Glättungsregel: Gras bleibt liegen, wenn es NICHT nach links/rechts wegrutschen kann
            if (canSlideLeft && canSlideRight) {
              curX += (rand() % 2 == 0) ? -1 : 1;
              curY++;
            } else if (canSlideLeft) {
              curX--;
              curY++;
            } else if (canSlideRight) {
              curX++;
              curY++;
            } else {
              // Bleibt stabil liegen & glättet die Oberfläche
              terrain[curX][curY] = particleColor;
              break;
            }
          } else {
            // Erde-Regel: Rutscht in Lücken und Vertiefungen
            if (canSlideLeft && canSlideRight) {
              curX += (rand() % 2 == 0) ? -1 : 1;
              curY++;
            } else if (canSlideLeft) {
              curX--;
              curY++;
            } else if (canSlideRight) {
              curX++;
              curY++;
            } else {
              terrain[curX][curY] = particleColor;
              break;
            }
          }
        }
      }
    }
  }

  void updateTerrainGeneration(Engine &e) {
    float dt = e.dt();
    genTimer += dt;

    if (genPhase == GenPhase::JaggedLinesAndRockNoise) {
      // 0.3s Pause zum Betrachten des Felsuntergrunds
      if (genTimer >= 0.35f) {
        genPhase = GenPhase::DirtSnowing;
      }
    } else if (genPhase == GenPhase::DirtSnowing) {
      // Erde schneit herab (Per-Partikel Farbmischung)
      simulateFallingParticles(false, 70);
      dirtParticlesSpawned += 70;

      if (dirtParticlesSpawned >= totalDirtParticles) {
        genPhase = GenPhase::GrassSnowing;
      }
    } else if (genPhase == GenPhase::GrassSnowing) {
      // Gras rieselt herab & glättet die Hänge (Per-Partikel Farbmischung)
      simulateFallingParticles(true, 70);
      grassParticlesSpawned += 70;

      if (grassParticlesSpawned >= totalGrassParticles) {
        genPhase = GenPhase::Completed;
        finalizeTankPlacement();
      }
    }

    // Leertaste überspringt die Animation sofort
    if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
      while (dirtParticlesSpawned < totalDirtParticles) {
        simulateFallingParticles(false, 100);
        dirtParticlesSpawned += 100;
      }
      while (grassParticlesSpawned < totalGrassParticles) {
        simulateFallingParticles(true, 100);
        grassParticlesSpawned += 100;
      }
      genPhase = GenPhase::Completed;
      finalizeTankPlacement();
    }
  }

  // Finde die genaue Bodenhöhe für einen Panzer an gegebener X-Position
  int findGroundHeight(int x) {
    x = std::clamp(x, 0, 319);
    for (int y = 0; y < 240; ++y) {
      if (terrain[x][y] != 0) {
        return y;
      }
    }
    return 235;
  }

  void finalizeTankPlacement() {
    // Spieler 1 links (X ≈ 48), Spieler 2 rechts (X ≈ 272)
    tanks[0].x = 48.0f;
    tanks[0].y = static_cast<float>(findGroundHeight(48));

    tanks[1].x = 272.0f;
    tanks[1].y = static_cast<float>(findGroundHeight(272));

    state = State::Aiming;
  }

  // ===========================================================================
  // 5. HAUPTSCHLEIFE (Wird 60-mal pro Sekunde aufgerufen)
  // ===========================================================================
  void update(Engine &e) override {
    float dt = e.dt();
    animTimer += dt;

    if (screenShake > 0.0f) {
      screenShake = std::max(0.0f, screenShake - dt * 2.5f);
    }

    // 1. Himmel & Szenerie zeichnen
    drawSkyAndEnvironment(e);

    // 2. Gelände aus dem Gitter rendern
    drawTerrainGrid(e);

    // 3. Je nach Zustand Steuerlogik ausführen
    switch (state) {
    case State::GeneratingTerrain:
      updateTerrainGeneration(e);
      drawGenerationUI(e);
      break;

    case State::Aiming:
      updateAiming(e);
      drawTanks(e);
      drawAimingUI(e);
      break;

    case State::BulletInFlight:
      updateBullet(e);
      drawTanks(e);
      drawBullet(e);
      drawAimingUI(e);
      break;

    case State::Exploding:
      updateExplosion(e);
      drawTanks(e);
      drawExplosion(e);
      drawAimingUI(e);
      break;

    case State::TankFalling:
      updateTankFalling(e);
      drawTanks(e);
      drawAimingUI(e);
      break;

    case State::GameOver:
      drawTanks(e);
      drawGameOverUI(e);
      if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
        startNewGame();
      }
      break;
    }

    // 4. Partikel rendern
    updateAndDrawParticles(e);

    // Taste R: Neues Gelände generieren
    if (e.pressed(Key::R)) {
      e.play_tone(Notes::C4, 0.08f);
      startNewGame();
    }
  }

  // ===========================================================================
  // 6. STEUERUNG (Zielen & Feuern)
  // ===========================================================================
  void updateAiming(Engine &e) {
    float dt = e.dt();
    Tank &t = tanks[activePlayer];

    // Winkel mit Links / Rechts anpassen (0° = rechts, 90° = oben, 180° = links)
    float angleSpeed = 40.0f; // Grad pro Sekunde
    if (e.key(Key::Left) || e.key(Key::A)) {
      t.angleDeg += angleSpeed * dt;
    }
    if (e.key(Key::Right) || e.key(Key::D)) {
      t.angleDeg -= angleSpeed * dt;
    }
    t.angleDeg = std::clamp(t.angleDeg, 0.0f, 180.0f);

    // Stärke / Kraft mit Oben / Unten anpassen [0, 100)
    float powerSpeed = 35.0f;
    if (e.key(Key::Up) || e.key(Key::W)) {
      t.power += powerSpeed * dt;
    }
    if (e.key(Key::Down) || e.key(Key::S)) {
      t.power -= powerSpeed * dt;
    }
    t.power = std::clamp(t.power, 0.0f, 99.0f);

    // Schuss abfeuern mit Leertaste oder Enter
    if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
      fireBullet(e);
    }
  }

  void fireBullet(Engine &e) {
    Tank &t = tanks[activePlayer];
    float rad = t.angleDeg * (3.14159265f / 180.0f);

    // Mündungs-Position an der Spitze des Rohrs
    bullet.x = t.x + std::cos(rad) * t.barrelLen;
    bullet.y = (t.y - 6.0f) - std::sin(rad) * t.barrelLen;

    // Ballistische Anfangsgeschwindigkeit
    float speedScale = 3.8f;
    bullet.vx = std::cos(rad) * t.power * speedScale;
    bullet.vy = -std::sin(rad) * t.power * speedScale;
    bullet.active = true;
    bullet.shooter = activePlayer;

    playCannonFire(e);
    spawnMuzzleFlash(bullet.x, bullet.y, t.angleDeg);

    state = State::BulletInFlight;
  }

  // ===========================================================================
  // 7. BALLISTISCHE GESCHOSSPHYSIK & KOLLISION
  // ===========================================================================
  void updateBullet(Engine &e) {
    float dt = e.dt();
    const float gravity = 150.0f; // Schwerkraft-Beschleunigung nach unten

    // Mehrere Sub-Steps pro Frame für präzise Kollisionserkennung
    const int subSteps = 6;
    float subDt = dt / subSteps;

    for (int step = 0; step < subSteps && bullet.active; ++step) {
      bullet.vy += gravity * subDt;
      bullet.x += bullet.vx * subDt;
      bullet.y += bullet.vy * subDt;

      // Rauchspur erzeugen
      if (rand() % 2 == 0) {
        Particle p;
        p.x = bullet.x;
        p.y = bullet.y;
        p.vx = (rand() % 20 - 10) * 0.4f;
        p.vy = (rand() % 20 - 10) * 0.4f;
        p.maxLife = 0.35f + (rand() % 20) / 100.0f;
        p.life = p.maxLife;
        p.color = (rand() % 2 == 0) ? Colors::LightGray : Colors::White;
        particles.push_back(p);
      }

      int bx = static_cast<int>(bullet.x);
      int by = static_cast<int>(bullet.y);

      // 1. Bildschirm-Randprüfung (Aus dem Bild geflogen)
      if (bx < -20 || bx > 340 || by > 240) {
        bullet.active = false;
        endTurn();
        return;
      }

      // 2. Trefferprüfung gegen Gelände
      if (bx >= 0 && bx < 320 && by >= 0 && by < 240) {
        if (terrain[bx][by] != 0) {
          triggerExplosion(bullet.x, bullet.y, e);
          return;
        }
      }

      // 3. Trefferprüfung gegen Panzer
      for (int i = 0; i < 2; ++i) {
        if (!tanks[i].alive) continue;
        float dx = bullet.x - tanks[i].x;
        float dy = bullet.y - (tanks[i].y - 5.0f);
        if (std::abs(dx) <= 9.0f && std::abs(dy) <= 7.0f) {
          triggerExplosion(bullet.x, bullet.y, e);
          return;
        }
      }
    }
  }

  // ===========================================================================
  // 8. EXPLOSION, KRATERBILDUNG & SCHADEN
  // ===========================================================================
  void triggerExplosion(float x, float y, Engine &e) {
    bullet.active = false;
    explosionX = x;
    explosionY = y;
    explosionRadius = 2.0f;
    explosionTimer = 0.0f;
    screenShake = 1.0f;

    playExplosionSound(e);

    // Zerstöre Gelände im Explosionskreis (Krater bohren)
    int cx = static_cast<int>(x);
    int cy = static_cast<int>(y);
    int r = static_cast<int>(maxExplosionRadius);

    for (int dy = -r; dy <= r; ++dy) {
      for (int dx = -r; dx <= r; ++dx) {
        if (dx * dx + dy * dy <= r * r) {
          int px = cx + dx;
          int py = cy + dy;
          if (px >= 0 && px < 320 && py >= 0 && py < 240) {
            terrain[px][py] = 0; // Gelände pulverisieren
          }
        }
      }
    }

    // Schadensberechnung für beide Panzer
    for (int i = 0; i < 2; ++i) {
      if (!tanks[i].alive) continue;
      float dist = std::sqrt((tanks[i].x - x) * (tanks[i].x - x) + (tanks[i].y - 5.0f - y) * (tanks[i].y - 5.0f - y));
      if (dist <= maxExplosionRadius + 14.0f) {
        // Direkttreffer = 45-55 Schaden, Streiftreffer fällt linear ab
        float damageFactor = 1.0f - std::min(1.0f, dist / (maxExplosionRadius + 14.0f));
        int damage = static_cast<int>(15 + damageFactor * 40);
        tanks[i].hp = std::max(0, tanks[i].hp - damage);

        if (tanks[i].hp <= 0) {
          tanks[i].alive = false;
          spawnTankDeathDebris(tanks[i].x, tanks[i].y, tanks[i].primaryColor);
        }
      }
    }

    // Glühende Trümmer & Explosionspartikel
    spawnExplosionParticles(x, y);

    state = State::Exploding;
  }

  void updateExplosion(Engine &e) {
    float dt = e.dt();
    explosionTimer += dt;
    explosionRadius = maxExplosionRadius * std::min(1.0f, explosionTimer * 4.0f);

    if (explosionTimer >= 0.45f) {
      // Prüfe, ob Panzer durch weggesprengten Boden nach unten fallen müssen
      state = State::TankFalling;
    }
  }

  void updateTankFalling(Engine &e) {
    float dt = e.dt();
    bool anyFalling = false;

    for (int i = 0; i < 2; ++i) {
      if (!tanks[i].alive) continue;
      int ground = findGroundHeight(static_cast<int>(tanks[i].x));
      if (tanks[i].y < ground) {
        tanks[i].y += 90.0f * dt;
        if (tanks[i].y >= ground) {
          tanks[i].y = static_cast<float>(ground);
        }
        anyFalling = true;
      }
    }

    if (!anyFalling) {
      // Prüfe Sieg-Bedingung
      if (!tanks[0].alive || !tanks[1].alive) {
        if (!tanks[0].alive && !tanks[1].alive) {
          winner = 2; // Unentschieden
        } else if (!tanks[1].alive) {
          winner = 0; // Spieler 1 gewinnt
        } else {
          winner = 1; // Spieler 2 gewinnt
        }
        playVictorySound(e);
        state = State::GameOver;
      } else {
        endTurn();
      }
    }
  }

  void endTurn() {
    activePlayer = 1 - activePlayer;
    state = State::Aiming;
  }

  // ===========================================================================
  // 9. RENDERING & ZEICHENFUNKTIONEN
  // ===========================================================================
  void drawSkyAndEnvironment(Engine &e) {
    // Option 1: Goldene Bernstein-Dämmerung (Maximaler Kontrast für Rot & Blau!)
    for (int y = 0; y < 240; ++y) {
      float baseT = y / 240.0f;
      for (int x = 0; x < 320; ++x) {
        // Räumliches Rauschen für weiches kachelfreies Blending
        float noiseVal = (Engine::noise(x, y, 7919) - 0.5f) * 0.045f;
        float t = std::clamp(baseT + noiseVal, 0.0f, 1.0f);

        uint8_t skyCol;
        if (t < 0.28f) {
          // 1. Oberer Nachthimmel: Tiefes Schiefer-Anthrazit
          float subT = t / 0.28f;
          skyCol = Ramps::Grays[2 + static_cast<int>(subT * 3.0f)];
        } else if (t < 0.52f) {
          // 2. Übergang: Dunkler Ocker / Bronze-Schiefer
          float subT = (t - 0.28f) / 0.24f;
          skyCol = (subT < 0.5f) ? Ramps::Grays[5] : Ramps::Earth::Sand[2 + static_cast<int>((subT - 0.5f) * 4.0f)];
        } else if (t < 0.76f) {
          // 3. Unterer Himmel: Warmes Bernstein & sattes Gold
          float subT = (t - 0.52f) / 0.24f;
          skyCol = Ramps::Gold.sample(0.35f + subT * 0.35f);
        } else {
          // 4. Horizont: Strahlendes Sonnengold / Amber-Glühen
          float subT = (t - 0.76f) / 0.24f;
          skyCol = Ramps::Gold.sample(0.70f + subT * 0.28f);
        }
        e.pset(x, y, skyCol);
      }
    }

    // Ferne funkelnde Sterne am oberen dunklen Nachthimmel
    for (int i = 0; i < 22; ++i) {
      int sx = (i * 37 + 13) % 320;
      int sy = (i * 23 + 7) % 55;
      uint8_t sc = (static_cast<int>(animTimer * 2.0f + i) % 2 == 0) ? Colors::White : Colors::Gold;
      e.pset(sx, sy, sc);
    }
  }

  void drawTerrainGrid(Engine &e) {
    for (int x = 0; x < 320; ++x) {
      for (int y = 0; y < 240; ++y) {
        uint8_t col = terrain[x][y];
        if (col != 0) {
          e.pset(x, y, col);
        }
      }
    }
  }

  void drawTanks(Engine &e) {
    for (int i = 0; i < 2; ++i) {
      if (!tanks[i].alive) continue;
      const Tank &t = tanks[i];
      int tx = static_cast<int>(t.x);
      int ty = static_cast<int>(t.y);

      // Wähle Farbraupe für Spieler 1 (Blau) oder Spieler 2 (Rot)
      const auto &ramp = (i == 0) ? Ramps::Blue : Ramps::Red;

      // 1. Panzerketten (Abgestufte Schattierung im jeweiligen Farbton & Schatten)
      e.rectfill(tx - 8, ty - 3, 17, 3, ramp[2]); // Dunkler Kettenschatten
      e.rect(tx - 8, ty - 3, 17, 3, ramp[5]);     // Kettengehäuse
      for (int k = -6; k <= 6; k += 4) {
        e.pset(tx + k, ty - 2, ramp[9]);          // Kettenräder / Nieten
      }

      // 2. Panzer-Rumpf (Hauptfarbe Vollton mit Licht- & Schattenkanten)
      e.rectfill(tx - 6, ty - 6, 13, 3, ramp[9]);       // Hauptfarbe (Full Blue / Full Red)
      e.line(tx - 6, ty - 6, tx + 6, ty - 6, ramp[13]); // Obere Glanzkante
      e.line(tx - 6, ty - 4, tx + 6, ty - 4, ramp[5]);  // Untere Schattenkante

      // 3. Panzerturm (Kuppel mit Lichtreflex)
      e.circlefill(tx, ty - 6, 3.5f, ramp[10]);     // Kuppel
      e.circle(tx, ty - 6, 3.5f, ramp[13]);        // Turmglanz / Umriss
      e.pset(tx - 1, ty - 7, ramp[15]);            // Lichtreflex

      // 4. Kanonenrohr (Sattes Blau/Rot mit dunkler Schattenlinie, OHNE Weiß)
      float rad = t.angleDeg * (3.14159265f / 180.0f);
      int barrelEndX = static_cast<int>(tx + std::cos(rad) * t.barrelLen);
      int barrelEndY = static_cast<int>((ty - 6) - std::sin(rad) * t.barrelLen);

      e.line(tx, ty - 6, barrelEndX, barrelEndY, ramp[9]);      // Satter Vollton (Full Blue / Full Red)
      e.line(tx, ty - 5, barrelEndX, barrelEndY + 1, ramp[3]);  // Dunkler Tiefschatten für starken Kontrast
      e.pset(barrelEndX, barrelEndY, ramp[11]);                 // Mündungsöffnung (ohne Weiß)

      // 5. HP-Balken weiter oben platziert (ty - 22, damit 90°-Rohr nicht blockiert wird)
      int barW = 20;
      int curHpW = static_cast<int>((static_cast<float>(t.hp) / t.maxHp) * barW);
      int barY = ty - 22;
      e.rectfill(tx - 10, barY, barW, 4, Colors::Black);
      e.rect(tx - 10, barY, barW, 4, ramp[6]);
      uint8_t hpCol = (t.hp > 50) ? Colors::LightGreen : (t.hp > 25 ? Colors::Yellow : Colors::Red);
      e.rectfill(tx - 9, barY + 1, curHpW, 2, hpCol);
    }
  }

  void drawBullet(Engine &e) {
    if (!bullet.active) return;
    int bx = static_cast<int>(bullet.x);
    int by = static_cast<int>(bullet.y);

    e.circlefill(bx, by, 2.0f, Colors::Yellow);
    e.pset(bx, by, Colors::White);
  }

  void drawExplosion(Engine &e) {
    int ex = static_cast<int>(explosionX);
    int ey = static_cast<int>(explosionY);
    float r = explosionRadius;

    e.circlefill(ex, ey, r, Ramps::Fire[6]);
    e.circlefill(ex, ey, r * 0.72f, Ramps::Fire[9]);
    e.circlefill(ex, ey, r * 0.45f, Ramps::Fire[12]);
    e.circlefill(ex, ey, r * 0.22f, Ramps::Fire[15]);
  }

  // ===========================================================================
  // 10. UI & ANZEIGEN (Grad & Bogenmaß!)
  // ===========================================================================
  void drawAimingUI(Engine &e) {
    // Oberer Statusbalken
    e.rectfill(0, 0, 320, 26, Colors::Black);
    e.line(0, 26, 320, 26, Colors::White);

    const Tank &t = tanks[activePlayer];
    float rad = t.angleDeg * (3.14159265f / 180.0f);

    // Aktiver Spieler
    std::string playerTitle = (activePlayer == 0) ? "SPIELER 1 (BLAU)" : "SPIELER 2 (ROT)";
    uint8_t pCol = (activePlayer == 0) ? Colors::Cyan : Colors::Pink;
    e.draw_text(6, 4, playerTitle, pCol, 1);

    // WINKEL IN GRAD & BOGENMASS (Radians)
    std::ostringstream angleStr;
    angleStr << std::fixed << std::setprecision(1) << t.angleDeg << " DEG  ("
             << std::setprecision(2) << rad << " RAD)";
    e.draw_text(118, 4, angleStr.str(), Colors::Yellow, 1);

    // KRAFT / POWER [0, 100)
    std::string powerStr = "KRAFT:" + std::to_string(static_cast<int>(t.power));
    e.draw_text(6, 15, powerStr, Colors::Gold, 1);

    // Visueller Kraftbalken
    int pBarW = 50;
    int curPW = static_cast<int>((t.power / 99.0f) * pBarW);
    e.rect(58, 15, pBarW, 7, Colors::DarkGray);
    e.rectfill(59, 16, curPW, 5, Colors::Gold);

    // Tastatur-Hilfe
    e.draw_text(118, 15, "L/R:WINKEL  U/D:KRAFT  SPACE:FEUER", Colors::LightGray, 1);
  }

  void drawGenerationUI(Engine &e) {
    e.rectfill(40, 15, 240, 48, Colors::Black);
    e.rect(40, 15, 240, 48, Colors::White);

    e.draw_text(52, 21, "GELAENDE-GENERIERUNG...", Colors::Yellow, 1);

    if (genPhase == GenPhase::JaggedLinesAndRockNoise) {
      e.draw_text(52, 33, "PHASE 1: FELS & GRAUES RAUSCHEN", Colors::LightGray, 1);
    } else if (genPhase == GenPhase::DirtSnowing) {
      int pct = (dirtParticlesSpawned * 100) / totalDirtParticles;
      e.draw_text(52, 33, "PHASE 2: ERDE SCHNEIT (" + std::to_string(pct) + "%)", Colors::Orange, 1);
    } else if (genPhase == GenPhase::GrassSnowing) {
      int pct = (grassParticlesSpawned * 100) / totalGrassParticles;
      e.draw_text(52, 33, "PHASE 3: GRAS GLAETTUNG (" + std::to_string(pct) + "%)", Colors::LightGreen, 1);
    }

    e.draw_text(52, 47, "LEERTASTE: SOFORT UEBERSPRINGEN", Colors::Cyan, 1);
  }

  void drawGameOverUI(Engine &e) {
    e.rectfill(40, 60, 240, 110, Colors::Black);
    e.rect(40, 60, 240, 110, Colors::Gold);
    e.rect(42, 62, 236, 106, Colors::DarkPurple);

    if (winner == 0) {
      e.draw_text(60, 72, "SPIELER 1 GEWINNT!", Colors::Cyan, 2);
      e.draw_text(52, 105, "BLAUER PANZER TRIUMPHIERT!", Colors::White, 1);
    } else if (winner == 1) {
      e.draw_text(60, 72, "SPIELER 2 GEWINNT!", Colors::Pink, 2);
      e.draw_text(56, 105, "ROTER PANZER TRIUMPHIERT!", Colors::White, 1);
    } else {
      e.draw_text(80, 72, "UNENTSCHIEDEN!", Colors::Yellow, 2);
      e.draw_text(56, 105, "BEIDE PANZER ZERSTOERT!", Colors::White, 1);
    }

    bool blink = (static_cast<int>(animTimer * 2.5f) % 2) == 0;
    if (blink) {
      e.draw_text(54, 132, "LEERTASTE: NOCHMAL SPIELEN", Colors::Yellow, 1);
    }
    e.draw_text(64, 148, "R: NEUES GELAENDE  ESC: MENUE", Colors::LightGray, 1);
  }

  // ===========================================================================
  // 11. PARTIKEL-EFFEKTE
  // ===========================================================================
  void spawnMuzzleFlash(float x, float y, float angleDeg) {
    float rad = angleDeg * (3.14159265f / 180.0f);
    for (int i = 0; i < 18; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      float spread = (rand() % 40 - 20) * (3.14159265f / 180.0f);
      float spd = 25.0f + (rand() % 75);
      p.vx = std::cos(rad + spread) * spd;
      p.vy = -std::sin(rad + spread) * spd;
      p.maxLife = 0.20f + (rand() % 15) / 100.0f;
      p.life = p.maxLife;
      p.color = Ramps::Fire[10 + (rand() % 6)];
      particles.push_back(p);
    }
  }

  void spawnExplosionParticles(float x, float y) {
    for (int i = 0; i < 55; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      float ang = (rand() % 360) * (3.14159265f / 180.0f);
      float spd = 15.0f + (rand() % 110);
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd - 30.0f;
      p.maxLife = 0.45f + (rand() % 35) / 100.0f;
      p.life = p.maxLife;
      p.color = Ramps::Fire.sample(0.2f + (rand() % 80) / 100.0f);
      particles.push_back(p);
    }
  }

  void spawnTankDeathDebris(float x, float y, uint8_t col) {
    for (int i = 0; i < 65; ++i) {
      Particle p;
      p.x = x + (rand() % 14 - 7);
      p.y = y + (rand() % 10 - 5);
      float ang = (rand() % 360) * (3.14159265f / 180.0f);
      float spd = 20.0f + (rand() % 120);
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd - 45.0f;
      p.maxLife = 0.65f + (rand() % 40) / 100.0f;
      p.life = p.maxLife;
      p.color = (i % 2 == 0) ? col : Ramps::Fire[8 + (rand() % 7)];
      particles.push_back(p);
    }
  }

  void updateAndDrawParticles(Engine &e) {
    float dt = e.dt();
    for (size_t i = 0; i < particles.size();) {
      auto &p = particles[i];
      p.life -= dt;
      if (p.life <= 0.0f) {
        particles[i] = particles.back();
        particles.pop_back();
      } else {
        p.vy += 85.0f * dt; // Schwerkraft auf Trümmer
        p.x += p.vx * dt;
        p.y += p.vy * dt;

        int px = static_cast<int>(p.x);
        int py = static_cast<int>(p.y);
        if (px >= 0 && px < 320 && py >= 0 && py < 240) {
          e.pset(px, py, p.color);
        }
        ++i;
      }
    }
  }

  // ===========================================================================
  // 12. SYNTHESIZER SOUND-EFFEKTE
  // ===========================================================================
  void playCannonFire(Engine &e) {
    e.play_melody({
        {Notes::G3, 0.04f},
        {Notes::C3, 0.12f}
    });
  }

  void playExplosionSound(Engine &e) {
    e.play_melody({
        {Notes::C3, 0.18f},
        {Notes::G2, 0.22f}
    });
  }

  void playVictorySound(Engine &e) {
    e.play_melody({
        {Notes::C4, 0.08f},
        {Notes::E4, 0.08f},
        {Notes::G4, 0.08f},
        {Notes::C5, 0.14f},
        {Notes::G4, 0.08f},
        {Notes::C5, 0.35f}
    });
  }
};
