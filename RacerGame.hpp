#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
// RETRO GRAND PRIX - 2-PLAYER TOP-DOWN RACER (RacerGame.hpp)
// =============================================================================
// - 2 Spieler lokales PvP Rennspiel auf einem spannenden GP-Rundkurs!
// - Steuerung:
//   * Spieler 1: W (Gas), S (Bremse/Rückwärts), A / D (Lenken)
//                W doppelt tippen = TURBO BOOST für 2s (5s Cooldown)
//                SHIFT LINKS = Zielsuchende Rakete (bremst Gegner für 2s auf 50% Speed!)
//   * Spieler 2: UP (Gas), DOWN (Bremse/Rückwärts), LEFT / RIGHT (Lenken)
//                UP doppelt tippen = TURBO BOOST für 2s (5s Cooldown)
//                SHIFT RECHTS = Zielsuchende Rakete (bremst Gegner für 2s auf 50% Speed!)
// - 3 Runden bis zum Champion-Sieg!
// =============================================================================

struct RacerGame : Game {
public:
  enum class State {
    Countdown,
    Racing,
    Finished
  };

  struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    uint8_t color;
    float size;
  };

  struct SkidMark {
    float x, y;
    float life;
  };

  struct Rocket {
    int owner; // 0 = P1, 1 = P2
    float x, y;
    float vx, vy;
    float angle;
    float life;
    bool active;
  };

  struct Car {
    int id; // 0 = P1 (Rot), 1 = P2 (Blau)
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float angle = 0.0f; // Blickrichtung in Radiant (-PI bis +PI, 0 = Rechts, -PI/2 = Oben)
    float speed = 0.0f; // Vorwärts-Geschwindigkeit in Pixel/s

    // Boost Mechanik (Doppel-Tap Vorwärts -> 2s Boost, 5s Cooldown)
    bool boostActive = false;
    float boostTimer = 0.0f;      // Läuft während Boost von 2.0s auf 0.0s
    float boostCooldown = 0.0f;   // Läuft nach Boost von 5.0s auf 0.0s
    float forwardTapTimer = 0.0f; // Fenster für Doppel-Tap Erkennung (0.28s)
    int forwardTapCount = 0;
    bool prevForward = false;

    // Raketen-Mechanik (Shift Links / Shift Rechts)
    float rocketCooldown = 0.0f; // 3.0s zwischen Raketenschüssen
    float slowTimer = 0.0f;      // Verlangsamung auf 50% Geschwindigkeit bei Treffer (2s)

    // Runden & Checkpoints
    int currentLap = 1;
    int nextCheckpoint = 1; // Startet bei 1 (CP 0 ist die Start/Ziel-Linie)
    bool finished = false;
    float finishTime = 0.0f;
    bool onGrass = false;
  };

  // ===========================================================================
  // 1. SPIEL-VARIABLEN & RENNSTRECKE
  // ===========================================================================
  State state = State::Countdown;
  Car cars[2];
  std::vector<Particle> particles;
  std::vector<SkidMark> skidMarks;
  std::vector<Rocket> rockets;

  float raceTimer = 0.0f;
  float countdownTimer = 3.5f;
  int winner = -1; // 0 = P1, 1 = P2
  const int TOTAL_LAPS = 3;

  // Rennstrecken-Geometrie (Closed Loop Spline Waypoints)
  struct TrackPoint {
    float x, y;
  };

  static const int NUM_TRACK_NODES = 16;
  std::array<TrackPoint, NUM_TRACK_NODES> trackNodes;
  const float TRACK_HALF_WIDTH = 18.0f; // Streckenbreite = 36 Pixel (genug Platz für 2 Autos)

  // 8 Checkpoint-Tore entlang der Strecke
  static const int NUM_CHECKPOINTS = 8;
  struct Checkpoint {
    float x, y;
    float radius;
  };
  std::array<Checkpoint, NUM_CHECKPOINTS> checkpoints;

  // ===========================================================================
  // 2. KONSTRUKTOR & RESET
  // ===========================================================================
  RacerGame() {
    initTrack();
    resetRace();
  }

  void initTrack() {
    // 16 Wegpunkte für einen abwechslungsreichen, flüssigen Rundkurs
    // Rechts: High-Speed Start-Ziel Gerade -> Oben: Schneller Schwung -> Links: Haarnadel -> Mitte: S-Schikane
    trackNodes[0]  = {268.0f, 175.0f}; // Start / Finish Line
    trackNodes[1]  = {268.0f, 120.0f}; // Gerade 1
    trackNodes[2]  = {268.0f,  75.0f}; // Ende Gerade
    trackNodes[3]  = {250.0f,  40.0f}; // Kurve 1 Rechts-Oben
    trackNodes[4]  = {200.0f,  30.0f}; // Obere High-Speed Passage
    trackNodes[5]  = {145.0f,  32.0f};
    trackNodes[6]  = { 90.0f,  42.0f}; // Kurve 2 Links-Oben
    trackNodes[7]  = { 46.0f,  70.0f}; // Bergab-Passage Links
    trackNodes[8]  = { 38.0f, 120.0f};
    trackNodes[9]  = { 48.0f, 175.0f}; // Haarnadel-Eingang
    trackNodes[10] = { 85.0f, 208.0f}; // Haarnadel-Scheitelpunkt
    trackNodes[11] = {135.0f, 205.0f}; // Ausgang Haarnadel
    trackNodes[12] = {165.0f, 168.0f}; // Schikane Einlenkpunkt (Zentrum)
    trackNodes[13] = {198.0f, 136.0f}; // S-Kurve Scheitel
    trackNodes[14] = {230.0f, 158.0f}; // Schikane Ausgang
    trackNodes[15] = {252.0f, 205.0f}; // Letzte Kurve vor Start/Ziel

    // 8 Checkpoints an Schlüsselstellen
    checkpoints[0] = {268.0f, 175.0f, 26.0f}; // Start / Ziel Tor
    checkpoints[1] = {268.0f,  90.0f, 26.0f}; // Ende Gerade 1
    checkpoints[2] = {210.0f,  31.0f, 26.0f}; // Obere Passage
    checkpoints[3] = { 70.0f,  52.0f, 26.0f}; // Links Oben
    checkpoints[4] = { 40.0f, 140.0f, 26.0f}; // Haarnadel Vorbereitung
    checkpoints[5] = {105.0f, 208.0f, 26.0f}; // Haarnadel Scheitel
    checkpoints[6] = {180.0f, 150.0f, 26.0f}; // Schikane Mitte
    checkpoints[7] = {245.0f, 195.0f, 26.0f}; // Zielkurve
  }

  void resetRace() {
    state = State::Countdown;
    countdownTimer = 3.5f;
    raceTimer = 0.0f;
    winner = -1;

    // Spieler 1 (Rot / Gold): Startplatz Links
    cars[0].id = 0;
    cars[0].x = 260.0f;
    cars[0].y = 188.0f;
    cars[0].vx = 0.0f;
    cars[0].vy = 0.0f;
    cars[0].angle = -3.14159265f * 0.5f; // Schaut nach oben (-90 Grad)
    cars[0].speed = 0.0f;
    cars[0].boostActive = false;
    cars[0].boostTimer = 0.0f;
    cars[0].boostCooldown = 0.0f;
    cars[0].forwardTapTimer = 0.0f;
    cars[0].forwardTapCount = 0;
    cars[0].prevForward = false;
    cars[0].rocketCooldown = 0.0f;
    cars[0].slowTimer = 0.0f;
    cars[0].currentLap = 1;
    cars[0].nextCheckpoint = 1;
    cars[0].finished = false;
    cars[0].finishTime = 0.0f;
    cars[0].onGrass = false;

    // Spieler 2 (Blau / Cyan): Startplatz Rechts (leicht versetzt)
    cars[1].id = 1;
    cars[1].x = 276.0f;
    cars[1].y = 196.0f;
    cars[1].vx = 0.0f;
    cars[1].vy = 0.0f;
    cars[1].angle = -3.14159265f * 0.5f;
    cars[1].speed = 0.0f;
    cars[1].boostActive = false;
    cars[1].boostTimer = 0.0f;
    cars[1].boostCooldown = 0.0f;
    cars[1].forwardTapTimer = 0.0f;
    cars[1].forwardTapCount = 0;
    cars[1].prevForward = false;
    cars[1].rocketCooldown = 0.0f;
    cars[1].slowTimer = 0.0f;
    cars[1].currentLap = 1;
    cars[1].nextCheckpoint = 1;
    cars[1].finished = false;
    cars[1].finishTime = 0.0f;
    cars[1].onGrass = false;

    particles.clear();
    skidMarks.clear();
    rockets.clear();
  }

  // ===========================================================================
  // 3. STRECKEN-DISTANZ & UNTERGRUND-PRÜFUNG
  // ===========================================================================
  // Berechnet den kürzesten Abstand eines Punktes (px, py) zur Strecken-Mittellinie
  float getDistanceToTrack(float px, float py) const {
    float minDistSq = 1e9f;

    for (int i = 0; i < NUM_TRACK_NODES; ++i) {
      int next = (i + 1) % NUM_TRACK_NODES;
      float x1 = trackNodes[i].x;
      float y1 = trackNodes[i].y;
      float x2 = trackNodes[next].x;
      float y2 = trackNodes[next].y;

      float dx = x2 - x1;
      float dy = y2 - y1;
      float segLenSq = dx * dx + dy * dy;

      if (segLenSq > 0.001f) {
        float t = std::clamp(((px - x1) * dx + (py - y1) * dy) / segLenSq, 0.0f, 1.0f);
        float projX = x1 + t * dx;
        float projY = y1 + t * dy;
        float distSq = (px - projX) * (px - projX) + (py - projY) * (py - projY);
        if (distSq < minDistSq) {
          minDistSq = distSq;
        }
      }
    }

    return std::sqrt(minDistSq);
  }

  // ===========================================================================
  // 4. HAUPTSCHLEIFE (Update)
  // ===========================================================================
  void update(Engine &e) override {
    float dt = e.dt();

    // Szene aktualisieren
    updateCountdown(e, dt);
    updateCars(e, dt);
    updateRockets(e, dt);
    updateParticles(dt);
    updateSkidMarks(dt);

    // Zeichnen
    render(e);
  }

  // ===========================================================================
  // 5. COUNTDOWN & START-AMPEL
  // ===========================================================================
  void updateCountdown(Engine &e, float dt) {
    if (state == State::Countdown) {
      float prev = countdownTimer;
      countdownTimer -= dt;

      // Sound-Signale für 3, 2, 1, GO!
      if (prev > 3.0f && countdownTimer <= 3.0f) {
        e.play_tone(Notes::G4, 0.15f); // Beep 3
      } else if (prev > 2.0f && countdownTimer <= 2.0f) {
        e.play_tone(Notes::G4, 0.15f); // Beep 2
      } else if (prev > 1.0f && countdownTimer <= 1.0f) {
        e.play_tone(Notes::G4, 0.15f); // Beep 1
      } else if (prev > 0.0f && countdownTimer <= 0.0f) {
        e.play_tone(Notes::C6, 0.40f); // High GO!
        state = State::Racing;
      }
    } else if (state == State::Racing) {
      raceTimer += dt;
    }
  }

  // ===========================================================================
  // 6. FAHRZEUG-PHYSIK & STEUERUNG
  // ===========================================================================
  void updateCars(Engine &e, float dt) {
    for (int i = 0; i < 2; ++i) {
      Car &c = cars[i];
      if (c.finished) continue;

      // 1. Tasten-Eingaben abfragen
      bool forward = false;
      bool backward = false;
      bool steerLeft = false;
      bool steerRight = false;
      bool shootRocket = false;

      if (state == State::Racing) {
        if (i == 0) {
          // Spieler 1: W, S, A, D + Shift Links
          forward = e.key(Key::W);
          backward = e.key(Key::S);
          steerLeft = e.key(Key::A);
          steerRight = e.key(Key::D);
          shootRocket = e.pressed(Key::LShift);
        } else {
          // Spieler 2: Pfeiltasten + Shift Rechts (oder Enter)
          forward = e.key(Key::Up);
          backward = e.key(Key::Down);
          steerLeft = e.key(Key::Left);
          steerRight = e.key(Key::Right);
          shootRocket = e.pressed(Key::RShift) || e.pressed(Key::Enter);
        }
      }

      // --- DOPPEL-TAP VORWÄRTS -> TURBO BOOST (2 Sek. aktiv, 5 Sek. Cooldown) ---
      if (c.boostActive) {
        c.boostTimer -= dt;
        if (c.boostTimer <= 0.0f) {
          c.boostActive = false;
          c.boostTimer = 0.0f;
          c.boostCooldown = 5.0f; // 5 Sekunden Erholungszeit nach Boost!
        }
      } else if (c.boostCooldown > 0.0f) {
        c.boostCooldown -= dt;
        if (c.boostCooldown < 0.0f) c.boostCooldown = 0.0f;
      }

      // Erkennung des schnellen Doppel-Klicks nach vorne
      if (forward && !c.prevForward) {
        if (c.forwardTapTimer > 0.0f && c.forwardTapCount >= 1 && c.boostCooldown <= 0.0f && !c.boostActive) {
          // 🚀 TURBO BOOST ZÜNDEN!
          c.boostActive = true;
          c.boostTimer = 2.0f; // 2 Sekunden Boost!
          c.boostCooldown = 5.0f;
          c.forwardTapCount = 0;
          c.forwardTapTimer = 0.0f;
          playBoostSound(e);
          spawnBoostFlames(c.x, c.y, c.angle);
        } else {
          c.forwardTapTimer = 0.28f; // 0.28 Sekunden Zeitfenster für den 2. Tap
          c.forwardTapCount = 1;
        }
      }
      c.prevForward = forward;

      if (c.forwardTapTimer > 0.0f) {
        c.forwardTapTimer -= dt;
        if (c.forwardTapTimer <= 0.0f) {
          c.forwardTapCount = 0;
        }
      }

      // --- RAKETEN-ABSCHUSS (Shift) ---
      if (c.rocketCooldown > 0.0f) {
        c.rocketCooldown -= dt;
        if (c.rocketCooldown < 0.0f) c.rocketCooldown = 0.0f;
      }

      if (shootRocket && c.rocketCooldown <= 0.0f && state == State::Racing) {
        c.rocketCooldown = 3.2f;
        fireRocket(i, e);
      }

      // --- SLOWDOWN-EFFEKT BEI RAKETEN-TREFFER ---
      if (c.slowTimer > 0.0f) {
        c.slowTimer -= dt;
        if (c.slowTimer < 0.0f) c.slowTimer = 0.0f;
        // Rauch & Funken vom getroffenen Motor
        if (rand() % 2 == 0) {
          spawnSlowSparks(c.x, c.y);
        }
      }

      // --- 2. UNTERGRUND-ERKENNUNG (Asphalt vs. Rasen/Gras) ---
      float distToTrack = getDistanceToTrack(c.x, c.y);
      c.onGrass = (distToTrack > TRACK_HALF_WIDTH);

      // Maximale Höchstgeschwindigkeit & Beschleunigung berechnen
      float topSpeed = 138.0f; // Normaler Topspeed auf Asphalt
      float accelRate = 120.0f;
      float frictionRate = 50.0f;

      if (c.boostActive) {
        topSpeed = 225.0f; // 🚀 MEGA BOOST (+63% Speed!)
        accelRate = 260.0f;
        // Nitro-Flammen aus dem Auspuff stoßen
        spawnBoostFlames(c.x, c.y, c.angle);
      }

      if (c.onGrass) {
        topSpeed *= 0.35f; // Starker Tempoverlust auf der Wiese
        frictionRate = 110.0f;
        if (std::abs(c.speed) > 15.0f && rand() % 2 == 0) {
          spawnGrassDirt(c.x, c.y);
        }
      }

      // Bei Raketentreffer: Geschwindigkeit auf 50% halbiert!
      if (c.slowTimer > 0.0f) {
        topSpeed *= 0.50f;
      }

      // 3. Beschleunigen & Bremsen
      if (forward) {
        if (c.speed < topSpeed) {
          c.speed += accelRate * dt;
          if (c.speed > topSpeed) c.speed = topSpeed;
        }
      } else if (backward) {
        if (c.speed > -50.0f) { // Rückwärtsgang
          c.speed -= 130.0f * dt;
        }
      } else {
        // Natürliches Ausrollen / Reibung
        if (c.speed > 0.0f) {
          c.speed -= frictionRate * dt;
          if (c.speed < 0.0f) c.speed = 0.0f;
        } else if (c.speed < 0.0f) {
          c.speed += frictionRate * dt;
          if (c.speed > 0.0f) c.speed = 0.0f;
        }
      }

      // 4. Lenkung (Präzise Arcade-Lenkphysik)
      if (std::abs(c.speed) > 2.0f) {
        float steerSpeed = 3.6f;
        if (c.boostActive) steerSpeed = 2.8f; // Bei Boost etwas stabiler
        float dir = (c.speed >= 0.0f) ? 1.0f : -1.0f;

        if (steerLeft) {
          c.angle -= steerSpeed * dt * dir;
          if (std::abs(c.speed) > 90.0f && !c.onGrass) {
            spawnSkidMark(c.x, c.y);
          }
        }
        if (steerRight) {
          c.angle += steerSpeed * dt * dir;
          if (std::abs(c.speed) > 90.0f && !c.onGrass) {
            spawnSkidMark(c.x, c.y);
          }
        }
      }

      // 5. Positions-Aktualisierung
      c.vx = std::cos(c.angle) * c.speed;
      c.vy = std::sin(c.angle) * c.speed;
      c.x += c.vx * dt;
      c.y += c.vy * dt;

      // Bildschirm-Randbegrenzung
      c.x = std::clamp(c.x, 10.0f, 310.0f);
      c.y = std::clamp(c.y, 10.0f, 230.0f);

      // 6. Checkpoint & Runden-Prüfung
      checkCheckpoint(c, e);
    }

    // 7. Auto-Auto Kollision (Gegenseitiges Wegrammen)
    checkCarCollision(e);
  }

  // ===========================================================================
  // 7. CHECKPOINT- & RUNDEN-SYSTEM
  // ===========================================================================
  void checkCheckpoint(Car &c, Engine &e) {
    const auto &targetCp = checkpoints[c.nextCheckpoint];
    float dx = c.x - targetCp.x;
    float dy = c.y - targetCp.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= targetCp.radius) {
      // Checkpoint erfolgreich passiert!
      c.nextCheckpoint = (c.nextCheckpoint + 1) % NUM_CHECKPOINTS;

      // Wenn CP 0 erreicht wurde -> 1 Runde abgeschlossen!
      if (c.nextCheckpoint == 1) {
        c.currentLap++;
        playLapSound(e);

        if (c.currentLap > TOTAL_LAPS) {
          // 🏁 RENNEN GEWONNEN!
          c.finished = true;
          c.finishTime = raceTimer;
          if (winner == -1) {
            winner = c.id;
            state = State::Finished;
            playVictoryFanfare(e);
          }
        }
      }
    }
  }

  // ===========================================================================
  // 8. AUTO-KOLLISION (Bodycheck)
  // ===========================================================================
  void checkCarCollision(Engine &e) {
    float dx = cars[1].x - cars[0].x;
    float dy = cars[1].y - cars[0].y;
    float distSq = dx * dx + dy * dy;
    float minDist = 12.0f; // Kollisionsradius

    if (distSq < minDist * minDist && distSq > 0.001f) {
      float dist = std::sqrt(distSq);
      float nx = dx / dist;
      float ny = dy / dist;

      // Auseinanderdrücken
      float overlap = (minDist - dist) * 0.5f;
      cars[0].x -= nx * overlap;
      cars[0].y -= ny * overlap;
      cars[1].x += nx * overlap;
      cars[1].y += ny * overlap;

      // Elastischer Impulsaustausch
      float avgSpeed = (cars[0].speed + cars[1].speed) * 0.5f;
      cars[0].speed = avgSpeed * 0.85f;
      cars[1].speed = avgSpeed * 0.85f;

      // Karambolage-Funken & Geräusch
      spawnCrashSparks((cars[0].x + cars[1].x) * 0.5f, (cars[0].y + cars[1].y) * 0.5f);
      e.play_tone(Notes::D3, 0.05f);
    }
  }

  // ===========================================================================
  // 9. ZIELSUCHENDE RAKETEN (Homing Missiles)
  // ===========================================================================
  void fireRocket(int shooterId, Engine &e) {
    const Car &shooter = cars[shooterId];

    Rocket r;
    r.owner = shooterId;
    r.x = shooter.x + std::cos(shooter.angle) * 10.0f;
    r.y = shooter.y + std::sin(shooter.angle) * 10.0f;
    r.angle = shooter.angle;
    r.vx = std::cos(shooter.angle) * 180.0f;
    r.vy = std::sin(shooter.angle) * 180.0f;
    r.life = 2.8f; // Verfolgt den Gegner für max 2.8 Sekunden
    r.active = true;

    rockets.push_back(r);
    playRocketLaunchSound(e);
  }

  void updateRockets(Engine &e, float dt) {
    for (size_t i = 0; i < rockets.size();) {
      auto &r = rockets[i];
      r.life -= dt;

      if (r.life <= 0.0f || !r.active) {
        rockets[i] = rockets.back();
        rockets.pop_back();
        continue;
      }

      // Zielpeilung zum gegnerischen Auto
      int targetId = 1 - r.owner;
      const Car &target = cars[targetId];

      float dx = target.x - r.x;
      float dy = target.y - r.y;
      float dist = std::sqrt(dx * dx + dy * dy);

      if (dist > 1.0f) {
        float desiredAngle = std::atan2(dy, dx);
        float angleDiff = desiredAngle - r.angle;

        // Winkel normalisieren (-PI bis +PI)
        while (angleDiff > 3.14159265f) angleDiff -= 2.0f * 3.14159265f;
        while (angleDiff < -3.14159265f) angleDiff += 2.0f * 3.14159265f;

        // Raketen-Wendigkeit
        float turnSpeed = 4.2f * dt;
        r.angle += std::clamp(angleDiff, -turnSpeed, turnSpeed);

        float rocketSpeed = 195.0f;
        r.vx = std::cos(r.angle) * rocketSpeed;
        r.vy = std::sin(r.angle) * rocketSpeed;
      }

      r.x += r.vx * dt;
      r.y += r.vy * dt;

      // Rauchschweif hinter der Rakete
      spawnRocketTrail(r.x, r.y, r.vx, r.vy);

      // Treffer-Prüfung gegen das Ziel-Auto
      if (dist < 10.0f) {
        r.active = false;
        // 💥 VOLLTREFFER: Gegner wird für 2 Sekunden auf 50% Speed verlangsamt!
        cars[targetId].slowTimer = 2.0f;
        cars[targetId].speed *= 0.50f;

        spawnExplosion(r.x, r.y);
        playExplosionSound(e);

        rockets[i] = rockets.back();
        rockets.pop_back();
        continue;
      }

      ++i;
    }
  }

  // ===========================================================================
  // 10. PARTIKEL & EFFEKTE
  // ===========================================================================
  void updateParticles(float dt) {
    for (size_t i = 0; i < particles.size();) {
      auto &p = particles[i];
      p.life -= dt;
      if (p.life <= 0.0f) {
        particles[i] = particles.back();
        particles.pop_back();
      } else {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        ++i;
      }
    }
  }

  void updateSkidMarks(float dt) {
    for (size_t i = 0; i < skidMarks.size();) {
      skidMarks[i].life -= dt;
      if (skidMarks[i].life <= 0.0f) {
        skidMarks[i] = skidMarks.back();
        skidMarks.pop_back();
      } else {
        ++i;
      }
    }
  }

  void spawnBoostFlames(float x, float y, float angle) {
    // Auspuff-Flammen entgegen der Fahrtrichtung
    float rearX = x - std::cos(angle) * 7.0f;
    float rearY = y - std::sin(angle) * 7.0f;

    for (int i = 0; i < 3; ++i) {
      Particle p;
      p.x = rearX + (rand() % 4 - 2);
      p.y = rearY + (rand() % 4 - 2);
      p.vx = -std::cos(angle) * (60.0f + rand() % 50) + (rand() % 20 - 10);
      p.vy = -std::sin(angle) * (60.0f + rand() % 50) + (rand() % 20 - 10);
      p.maxLife = 0.18f + (rand() % 10) / 100.0f;
      p.life = p.maxLife;
      p.color = (i == 0) ? Ramps::Cyan[14] : ((i == 1) ? Colors::Yellow : Ramps::Fire[12]);
      p.size = 1.5f;
      particles.push_back(p);
    }
  }

  void spawnRocketTrail(float x, float y, float vx, float vy) {
    Particle p;
    p.x = x + (rand() % 3 - 1);
    p.y = y + (rand() % 3 - 1);
    p.vx = -vx * 0.15f + (rand() % 16 - 8);
    p.vy = -vy * 0.15f + (rand() % 16 - 8);
    p.maxLife = 0.22f;
    p.life = p.maxLife;
    p.color = (rand() % 2 == 0) ? Colors::DarkGray : Ramps::Fire[10];
    p.size = 1.0f;
    particles.push_back(p);
  }

  void spawnExplosion(float x, float y) {
    for (int i = 0; i < 35; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      float ang = (rand() % 360) * (3.14159265f / 180.0f);
      float spd = 20.0f + (rand() % 85);
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd;
      p.maxLife = 0.35f + (rand() % 20) / 100.0f;
      p.life = p.maxLife;
      p.color = (i < 10) ? Colors::White : ((i < 24) ? Ramps::Fire[12] : Colors::DarkGray);
      p.size = 2.0f;
      particles.push_back(p);
    }
  }

  void spawnSlowSparks(float x, float y) {
    Particle p;
    p.x = x + (rand() % 8 - 4);
    p.y = y + (rand() % 8 - 4);
    p.vx = (rand() % 30 - 15);
    p.vy = -20.0f - (rand() % 25);
    p.maxLife = 0.25f;
    p.life = p.maxLife;
    p.color = (rand() % 2 == 0) ? Ramps::Yellow[14] : Colors::DarkGray;
    p.size = 1.0f;
    particles.push_back(p);
  }

  void spawnSkidMark(float x, float y) {
    SkidMark sm;
    sm.x = x;
    sm.y = y;
    sm.life = 2.5f;
    skidMarks.push_back(sm);
  }

  void spawnGrassDirt(float x, float y) {
    Particle p;
    p.x = x + (rand() % 6 - 3);
    p.y = y + (rand() % 6 - 3);
    p.vx = (rand() % 30 - 15);
    p.vy = (rand() % 30 - 15);
    p.maxLife = 0.20f;
    p.life = p.maxLife;
    p.color = (rand() % 2 == 0) ? Colors::DarkGreen : Ramps::Earth::Soil[4];
    p.size = 1.0f;
    particles.push_back(p);
  }

  void spawnCrashSparks(float x, float y) {
    for (int i = 0; i < 8; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      p.vx = (rand() % 60 - 30);
      p.vy = (rand() % 60 - 30);
      p.maxLife = 0.15f;
      p.life = p.maxLife;
      p.color = (i % 2 == 0) ? Colors::White : Colors::Gold;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  // ===========================================================================
  // 11. RENDERING & ZEICHENFUNKTIONEN
  // ===========================================================================
  void render(Engine &e) {
    // 1. Rasen / Landschaft (Schöner satter Farbton mit Gras-Muster)
    e.cls(Colors::DarkGreen);

    // Deko-Rasenmuster
    for (int y = 0; y < 240; y += 32) {
      for (int x = 0; x < 320; x += 32) {
        if (((x / 32) + (y / 32)) % 2 == 0) {
          e.rectfill(x, y, 32, 32, Ramps::Green[6]);
        }
      }
    }

    // 2. Asphalt-Rennstrecke & Curbs zeichnen
    renderTrack(e);

    // 3. Reifenspuren auf dem Asphalt
    for (const auto &sm : skidMarks) {
      e.pset(sm.x, sm.y, Colors::DarkGray);
    }

    // 4. Raketen rendern
    renderRockets(e);

    // 5. Rennwagen beider Spieler rendern
    renderCar(e, cars[0]);
    renderCar(e, cars[1]);

    // 6. Partikel (Feuer, Rauch, Funken)
    for (const auto &p : particles) {
      e.pset(p.x, p.y, p.color);
    }

    // 7. HUD & Renn-Informationen
    renderHUD(e);
  }

  // --- RENNSTRECKE MIT ASPHALT, CURBS & START/ZIEL LINIE ---
  void renderTrack(Engine &e) {
    // Strecke als verbundene dicke Asphalt-Segmente zeichnen
    for (int i = 0; i < NUM_TRACK_NODES; ++i) {
      int next = (i + 1) % NUM_TRACK_NODES;
      float x1 = trackNodes[i].x;
      float y1 = trackNodes[i].y;
      float x2 = trackNodes[next].x;
      float y2 = trackNodes[next].y;

      // Normalen-Vektor zur Strecken-Richtung
      float dx = x2 - x1;
      float dy = y2 - y1;
      float len = std::sqrt(dx * dx + dy * dy);
      if (len < 0.001f) continue;
      float nx = (-dy / len);
      float ny = (dx / len);

      // Asphalt-Streifen
      float hw = TRACK_HALF_WIDTH;
      for (float t = 0.0f; t <= 1.0f; t += 0.04f) {
        float cx = x1 + t * dx;
        float cy = y1 + t * dy;

        // 1. Rand-Curbs (Rot/Weiß gestreift)
        bool kerbWhite = (static_cast<int>(t * 14.0f + i * 4) % 2 == 0);
        uint8_t kerbCol = kerbWhite ? Colors::White : Colors::Red;
        e.circlefill(cx + nx * (hw + 2.0f), cy + ny * (hw + 2.0f), 3.0f, kerbCol);
        e.circlefill(cx - nx * (hw + 2.0f), cy - ny * (hw + 2.0f), 3.0f, kerbCol);

        // 2. Asphalt-Fahrbahn (Dunkelgrau)
        e.circlefill(cx, cy, hw, Colors::DarkGray);

        // 3. Weiße Mittellinie (gestrichelt)
        if (static_cast<int>(t * 8.0f) % 2 == 0) {
          e.pset(cx, cy, Colors::White);
        }
      }
    }

    // Start-Ziel Linie (Kariertes Schachbrettmuster quer über die Strecke)
    float sX = trackNodes[0].x;
    float sY = trackNodes[0].y;
    for (int k = -16; k <= 16; k += 4) {
      uint8_t c1 = ((k / 4) % 2 == 0) ? Colors::White : Colors::Black;
      uint8_t c2 = ((k / 4) % 2 == 0) ? Colors::Black : Colors::White;
      e.rectfill(sX + k, sY - 2, 4, 2, c1);
      e.rectfill(sX + k, sY, 4, 2, c2);
    }
  }

  // --- RENNWAGEN-SPRITE (Pixel-Art mit Drehung) ---
  void renderCar(Engine &e, const Car &c) {
    float px = c.x;
    float py = c.y;
    float cosA = std::cos(c.angle);
    float sinA = std::sin(c.angle);

    // Basis-Karosserie Vektoren (Länge 12px, Breite 7px)
    auto drawRotatedPixel = [&](float rx, float ry, uint8_t color) {
      int screenX = static_cast<int>(px + rx * cosA - ry * sinA);
      int screenY = static_cast<int>(py + rx * sinA + ry * cosA);
      e.pset(screenX, screenY, color);
    };

    uint8_t bodyColor = (c.id == 0) ? Colors::Red : Colors::Blue;
    uint8_t roofColor = (c.id == 0) ? Colors::Gold : Colors::Cyan;
    uint8_t stripeColor = (c.id == 0) ? Colors::White : Colors::Yellow;

    // 1. Schwarze Breitreifen (4 Räder)
    drawRotatedPixel( 4.0f, -3.5f, Colors::Black);
    drawRotatedPixel( 4.0f,  3.5f, Colors::Black);
    drawRotatedPixel(-4.0f, -3.5f, Colors::Black);
    drawRotatedPixel(-4.0f,  3.5f, Colors::Black);

    // 2. Karosserie / Chassis
    for (float lx = -5.0f; lx <= 5.0f; lx += 1.0f) {
      for (float ly = -2.5f; ly <= 2.5f; ly += 1.0f) {
        drawRotatedPixel(lx, ly, bodyColor);
      }
    }

    // 3. Rennstreifen auf der Motorhaube
    drawRotatedPixel( 3.0f, 0.0f, stripeColor);
    drawRotatedPixel( 4.0f, 0.0f, stripeColor);

    // 4. Cockpit / Windschutzscheibe
    for (float lx = -1.0f; lx <= 1.0f; lx += 1.0f) {
      drawRotatedPixel(lx, -1.0f, roofColor);
      drawRotatedPixel(lx,  0.0f, roofColor);
      drawRotatedPixel(lx,  1.0f, roofColor);
    }
    drawRotatedPixel(1.0f, 0.0f, Colors::White); // Glanzpunkt Frontscheibe

    // 5. Frontscheinwerfer (Gelb/Weiß vorne)
    drawRotatedPixel(5.5f, -2.0f, Colors::Yellow);
    drawRotatedPixel(5.5f,  2.0f, Colors::Yellow);

    // 6. Rückleuchten / Bremslicht
    uint8_t rearLight = (c.speed < 0.0f || (c.boostActive)) ? Colors::White : Colors::Red;
    drawRotatedPixel(-5.5f, -2.0f, rearLight);
    drawRotatedPixel(-5.5f,  2.0f, rearLight);

    // Status-Anzeige über dem Auto (SLOWED 50% / BOOST)
    if (c.slowTimer > 0.0f) {
      bool blink = (static_cast<int>(raceTimer * 8.0f) % 2 == 0);
      if (blink) {
        e.draw_text(px - 14, py - 12, "SLOW 50%", Colors::Red, 1);
      }
    } else if (c.boostActive) {
      e.draw_text(px - 12, py - 12, "BOOST!", Colors::Cyan, 1);
    }
  }

  // --- RAKETEN RENDERN ---
  void renderRockets(Engine &e) {
    for (const auto &r : rockets) {
      int rx = static_cast<int>(r.x);
      int ry = static_cast<int>(r.y);
      e.circlefill(rx, ry, 2.5f, Colors::Red);
      e.pset(rx, ry, Colors::White);
      e.pset(rx - static_cast<int>(r.vx * 0.02f), ry - static_cast<int>(r.vy * 0.02f), Colors::Yellow);
    }
  }

  // --- HUD: TACHO, BOOST-METER & RUNDENZEITEN ---
  void renderHUD(Engine &e) {
    // 1. SPIELER 1 (LINKS - ROT)
    e.rectfill(8, 6, 95, 26, Colors::Black);
    e.rect(8, 6, 95, 26, Colors::Red);
    e.draw_text(12, 9, "P1: ROT", Colors::Red, 1);
    std::string lap1 = "L:" + std::to_string(std::min(TOTAL_LAPS, cars[0].currentLap)) + "/" + std::to_string(TOTAL_LAPS);
    e.draw_text(58, 9, lap1, Colors::White, 1);

    // Boost-Balken P1
    if (cars[0].boostActive) {
      e.draw_text(12, 19, "BOOSTING!", Colors::Cyan, 1);
    } else if (cars[0].boostCooldown > 0.0f) {
      int cdW = static_cast<int>((1.0f - (cars[0].boostCooldown / 5.0f)) * 40.0f);
      e.draw_text(12, 19, "BOOST:", Colors::DarkGray, 1);
      e.rectfill(48, 20, cdW, 4, Colors::Yellow);
    } else {
      e.draw_text(12, 19, "BOOST READY!", Colors::LightGreen, 1);
    }

    // Raketen-Status P1
    if (cars[0].rocketCooldown <= 0.0f) {
      e.draw_text(68, 19, "[L-SHIFT]", Colors::Gold, 1);
    }

    // 2. SPIELER 2 (RECHTS - BLAU)
    e.rectfill(217, 6, 95, 26, Colors::Black);
    e.rect(217, 6, 95, 26, Colors::Blue);
    e.draw_text(221, 9, "P2: BLAU", Colors::Blue, 1);
    std::string lap2 = "L:" + std::to_string(std::min(TOTAL_LAPS, cars[1].currentLap)) + "/" + std::to_string(TOTAL_LAPS);
    e.draw_text(268, 9, lap2, Colors::White, 1);

    // Boost-Balken P2
    if (cars[1].boostActive) {
      e.draw_text(221, 19, "BOOSTING!", Colors::Cyan, 1);
    } else if (cars[1].boostCooldown > 0.0f) {
      int cdW = static_cast<int>((1.0f - (cars[1].boostCooldown / 5.0f)) * 40.0f);
      e.draw_text(221, 19, "BOOST:", Colors::DarkGray, 1);
      e.rectfill(257, 20, cdW, 4, Colors::Yellow);
    } else {
      e.draw_text(221, 19, "BOOST READY!", Colors::LightGreen, 1);
    }

    // Raketen-Status P2
    if (cars[1].rocketCooldown <= 0.0f) {
      e.draw_text(278, 19, "[R-SHIFT]", Colors::Gold, 1);
    }

    // 3. START-AMPEL / COUNTDOWN BANNER
    if (state == State::Countdown) {
      e.rectfill(100, 90, 120, 45, Colors::Black);
      e.rect(100, 90, 120, 45, Colors::Gold);

      if (countdownTimer > 3.0f) {
        e.draw_text(115, 102, "BEREIT MACHEN!", Colors::White, 1);
      } else if (countdownTimer > 2.0f) {
        e.draw_text(152, 98, "3", Colors::Red, 3);
      } else if (countdownTimer > 1.0f) {
        e.draw_text(152, 98, "2", Colors::Yellow, 3);
      } else if (countdownTimer > 0.0f) {
        e.draw_text(152, 98, "1", Colors::LightGreen, 3);
      }
    }

    // 4. SIEGER-PODIUM
    if (state == State::Finished) {
      e.rectfill(60, 70, 200, 80, Colors::Black);
      e.rect(60, 70, 200, 80, Colors::Gold);

      std::string winTxt = (winner == 0) ? "SPIELER 1 (ROT) GEWINNT!" : "SPIELER 2 (BLAU) GEWINNT!";
      uint8_t winCol = (winner == 0) ? Colors::Red : Colors::Blue;
      e.draw_text(72, 85, "ZIEL-EINLAUF!", Colors::Gold, 2);
      e.draw_text(74, 110, winTxt, winCol, 1);

      bool blink = (static_cast<int>(raceTimer * 2.5f) % 2 == 0);
      if (blink) {
        e.draw_text(78, 130, "LEERTASTE: NEUSTART", Colors::Yellow, 1);
      }

      if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
        resetRace();
      }
    }

    // Tastatur-Hilfe am unteren Rand
    e.draw_text(8, 230, "P1: W/A/S/D [2xW:BOOST] [L-SHIFT:RAKETE] | P2: PFEILE [2xUP:BOOST] [R-SHIFT:RAKETE]", Ramps::Grays[8], 1);
  }

  // ===========================================================================
  // 12. SYNTHESIZER SOUNDS
  // ===========================================================================
  void playBoostSound(Engine &e) {
    // Aufsteigender Turbo-Pfeifton
    e.play_tone(Notes::D5, 0.06f);
    e.play_tone(Notes::G5, 0.08f);
    e.play_tone(Notes::C6, 0.15f);
  }

  void playRocketLaunchSound(Engine &e) {
    // Bedrohlicher Raketenstart-Whoosh
    e.play_tone(Notes::C4, 0.05f);
    e.play_tone(Notes::G4, 0.07f);
    e.play_tone(Notes::B4, 0.10f);
  }

  void playExplosionSound(Engine &e) {
    // Tiefes Wummern bei Einschlag
    e.play_tone(Notes::C2, 0.28f);
    e.play_tone(Notes::F2, 0.18f);
  }

  void playLapSound(Engine &e) {
    // Heller Runden-Gong
    e.play_tone(Notes::E5, 0.08f);
    e.play_tone(Notes::A5, 0.16f);
  }

  void playVictoryFanfare(Engine &e) {
    // Grand Prix Siegerfanfare
    e.play_tone(Notes::C4, 0.12f);
    e.play_tone(Notes::E4, 0.12f);
    e.play_tone(Notes::G4, 0.12f);
    e.play_tone(Notes::C5, 0.35f);
  }
};
