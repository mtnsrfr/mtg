#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

// =============================================================================
// 🦄 THE UNICORN: DAS MAGISCHE EINHORN-ABENTEUER
// =============================================================================
// Ein rasanter Retro-Runner im Stil von Mario mit Sternenkraft & Geometry Dash!
//
// Features:
// - 🎨 Einhorn-Farbauswahl: Wähle zwischen BLAU, WEISS und PINK!
// - ⚡ Dynamischer Highspeed: Das Spiel wird stetig schneller und herausfordernder!
// - 🌈 Regenbogen-Trampoline: Normale & RIESIGE Doppel-Regenbögen für Mega-Sprünge!
//      (Von oben = Trampolin-Katapult! Von vorne = Crash!)
// - 🌠 Sternschnuppen (Meteore): Weiche ihnen aus ODER springe von oben auf sie,
//      um sie mit einer Starfighter-Explosion zu zerschmettern!
// - 🦄 Magische Einhorn-Effekte: Pink, Cyan, Gold & Lila Sternenstaub beim Crash.
// - 🌙 Wunderschöner Parallaxe-Nachthimmel mit Mond, Sternen & funkelnden Wolken.
//
// Steuerung:
// - LEERTASTE / W / PFEIL HOCH : Springen / Doppelsprung
// - 1 / 2 / 3 oder LINKS / RECHTS: Einhorn-Farbe wählen (im Menü & Game Over)
// - ESCAPE                    : Zurück zum Hauptmenü
// =============================================================================

struct UnicornGame : Game {
  // ===========================================================================
  // 1. TYPEN & STRUKTUREN
  // ===========================================================================
  enum class State {
    ColorSelect,
    Playing,
    GameOver,
    Victory
  };

  enum class UnicornColor {
    Blue = 0,
    White = 1,
    Pink = 2
  };

  // --- Partikel (Explosionen, Schweif, Glitzer & Regenbogenstaub) ---
  struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    float life;
    float maxLife;
    uint8_t color;
    bool isThermal;      // Weiß -> Gelb -> Orange -> Rot -> Dunkelgrau (Sternschnuppen)
    bool isRainbow;      // Funkelnder Regenbogen-Farbwechsel
    bool isUnicornMagic; // Magischer Einhorn-Sternenstaub (Pink, Cyan, Gold, Lila, Weiß)
  };

  // --- Hintergrund-Sterne (Parallaxe) ---
  struct Star {
    float x;
    float y;
    float speed;
    uint8_t color;
    float twinkleTimer;
  };

  // --- Wolken am Himmel ---
  struct Cloud {
    float x;
    float y;
    float speed;
    float width;
    uint8_t color;
  };

  // --- Boden-Hindernisse: Regenbögen ---
  struct RainbowArch {
    float x;
    float y;
    float width;
    float height;
    bool isGiant = false; // Doppelt so großer Riesen-Regenbogen!
    bool touched = false;
    float fadeTimer = 1.0f; // 1.0 = voll sichtbar, 0.0 = verblasst
  };

  // --- Fallende Hindernisse: Sternschnuppen / Meteore ---
  struct ShootingStar {
    float x;
    float y;
    float vx;
    float vy;
    float size;
    float trailTimer = 0.0f;
    bool active = true;
  };

  // ===========================================================================
  // 2. SPIEL-VARIABLEN & EINSTELLUNGEN
  // ===========================================================================
  State state = State::ColorSelect;
  UnicornColor chosenColor = UnicornColor::White;

  // --- Level-System (Level 1: Blumenwiese, Level 2: Nachthimmel) ---
  int currentLevel = 1;
  float levelTimer = 0.0f;
  const float levelDuration = 20.0f; // 20 Sekunden pro Level bis zum Sieg!
  float levelBannerTimer = 0.0f;

  // --- Einhorn-Physik & Boden ---
  const float playerX = 48.0f;
  const float groundY = 200.0f; // Exakte Y-Linie der Bodenoberfläche
  const float unicornWidth = 24.0f;
  const float unicornHeight = 22.0f;

  float playerY = 178.0f; // groundY - unicornHeight
  float playerVy = 0.0f;
  const float gravity = 960.0f;
  const float jumpVelocity = -365.0f;
  const float doubleJumpVelocity = -325.0f;
  const float trampolineVelocity = -460.0f;     // Normaler Regenbogen
  const float megaTrampolineVelocity = -540.0f; // Riesen-Regenbogen Super-Katapult!
  const float stompBounceVelocity = -330.0f;    // Sternschnuppen-Sprung

  bool isOnGround = true;
  bool canDoubleJump = true;
  float animTimer = 0.0f;
  float hoofSparkleTimer = 0.0f;

  // --- Welt & Kontinuierliche Beschleunigung ---
  const float baseScrollSpeed = 215.0f;
  float currentScrollSpeed = 215.0f;
  float distanceTraveled = 0.0f;

  // --- Spawner-Timer ---
  float rainbowSpawnTimer = 1.6f;
  float meteorSpawnTimer = 2.4f;

  // --- Spielstand ---
  int score = 0;
  int highscore = 0;
  int rainbowsCollected = 0;
  int meteorsStomped = 0;

  // --- Listen für Spielobjekte & Partikel ---
  std::vector<Star> stars;
  std::vector<Cloud> clouds;
  std::vector<RainbowArch> rainbows;
  std::vector<ShootingStar> meteors;
  std::vector<Particle> particles;

  // --- Chiptune-Hintergrundmusik (Gamer-Hymne) ---
  std::vector<int16_t> bgmSamples;
  bool isBgmPlaying = false;

  // ===========================================================================
  // 3. KONSTRUKTOR & DESTRUKTOR
  // ===========================================================================
  UnicornGame() {
    initBackground();
    generateBgm();
    resetGame();
  }

  virtual ~UnicornGame() = default;

  // ===========================================================================
  // 4. HAUPTSCHLEIFE (Wird 60-mal pro Sekunde aufgerufen)
  // ===========================================================================
  void update(Engine &e) override {
    // 1. Hintergrund zeichnen (Level 1: Blumenwiese & Sonne, Level 2: Nachthimmel & Mond)
    if (currentLevel == 1) {
      drawMeadowBackground(e);
    } else {
      drawNightBackground(e);
    }

    // 2. Je nach Zustand ausführen
    switch (state) {
    case State::ColorSelect:
      updateColorSelect(e);
      break;
    case State::Playing:
      updatePlaying(e);
      break;
    case State::GameOver:
      updateGameOver(e);
      break;
    case State::Victory:
      updateVictory(e);
      break;
    }

    // 3. Partikel zeichnen & aktualisieren
    updateAndDrawParticles(e);

    // 4. Benutzeroberfläche (Score, Tempo, Zähler)
    if (state == State::Playing) {
      drawPlayingUI(e);
    }
  }

  // ===========================================================================
  // 5. INITIALISIERUNG & NEUSTART
  // ===========================================================================
  void initBackground() {
    stars.clear();
    for (int i = 0; i < 60; ++i) {
      Star s;
      s.x = static_cast<float>(rand() % 320);
      s.y = static_cast<float>(rand() % 160);
      int layer = rand() % 3;
      if (layer == 0) {
        s.speed = 15.0f;
        s.color = Colors::DarkGray;
      } else if (layer == 1) {
        s.speed = 35.0f;
        s.color = Colors::LightGray;
      } else {
        s.speed = 70.0f;
        s.color = Colors::White;
      }
      s.twinkleTimer = (rand() % 100) / 100.0f;
      stars.push_back(s);
    }

    clouds.clear();
    for (int i = 0; i < 5; ++i) {
      Cloud c;
      c.x = static_cast<float>(i * 75 + rand() % 30);
      c.y = static_cast<float>(18 + rand() % 65);
      c.speed = 22.0f + (rand() % 20);
      c.width = 30.0f + (rand() % 25);
      c.color = (i % 2 == 0) ? Colors::Purple : Colors::DarkPurple;
      clouds.push_back(c);
    }
  }

  void resetGame() {
    playerY = groundY - unicornHeight;
    playerVy = 0.0f;
    isOnGround = true;
    canDoubleJump = true;
    animTimer = 0.0f;
    hoofSparkleTimer = 0.0f;

    currentScrollSpeed = baseScrollSpeed;
    distanceTraveled = 0.0f;
    rainbowSpawnTimer = 1.4f;
    meteorSpawnTimer = 2.2f;

    currentLevel = 1;
    levelTimer = 0.0f;
    levelBannerTimer = 2.5f;

    score = 0;
    rainbowsCollected = 0;
    meteorsStomped = 0;

    rainbows.clear();
    meteors.clear();
    particles.clear();
  }

  // ===========================================================================
  // 6. CHIPTUNE-MUSIK-KOMPOSITION (Zeitlose Retro-Gamer-Hymne)
  // ===========================================================================
  void generateBgm() {
    const unsigned sampleRate = 44100;
    const double bpm = 150.0;
    const double stepDur = 60.0 / (bpm * 4.0); // 0.100s pro 16tel-Schritt
    const int totalSteps = 128;                // 8 Takte à 16 Steps = 12.8 Sekunden
    const double totalDuration = totalSteps * stepDur;
    const size_t totalSamples = static_cast<size_t>(sampleRate * totalDuration);

    bgmSamples.assign(totalSamples, 0);

    auto pulseWave = [](double phase, double duty = 0.5) -> double {
      double p = phase - std::floor(phase);
      return (p < duty) ? 1.0 : -1.0;
    };

    auto triangleWave = [](double phase) -> double {
      double p = phase - std::floor(phase);
      return 2.0 * std::abs(2.0 * (p - std::floor(p + 0.5))) - 1.0;
    };

    // --- SPUR 1: DRUMS (Kick, Snare, Hi-Hats) ---
    for (int step = 0; step < totalSteps; ++step) {
      size_t startSample = static_cast<size_t>(step * stepDur * sampleRate);
      bool isKick = (step % 4 == 0);
      bool isSnare = (step % 8 == 4);
      bool isHat = (step % 2 == 0);

      // Kick: Pitch-Drop
      if (isKick) {
        size_t kickLen = static_cast<size_t>(sampleRate * 0.11);
        for (size_t i = 0; i < kickLen && (startSample + i) < totalSamples; ++i) {
          double t = static_cast<double>(i) / sampleRate;
          double env = std::exp(-t * 30.0);
          double freq = 44.0 + 88.0 * std::exp(-t * 38.0);
          double wave = std::sin(2.0 * M_PI * freq * t);
          bgmSamples[startSample + i] += static_cast<int16_t>(8000.0 * env * wave);
        }
      }

      // Snare: Rauschen + Ton
      if (isSnare) {
        size_t snareLen = static_cast<size_t>(sampleRate * 0.13);
        for (size_t i = 0; i < snareLen && (startSample + i) < totalSamples; ++i) {
          double t = static_cast<double>(i) / sampleRate;
          double env = std::exp(-t * 22.0);
          double noise = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
          double tone = std::sin(2.0 * M_PI * 190.0 * t);
          bgmSamples[startSample + i] += static_cast<int16_t>(5800.0 * env * (noise * 0.75 + tone * 0.25));
        }
      }

      // Hi-Hat: Crisp Noise-Click
      if (isHat) {
        size_t hatLen = static_cast<size_t>(sampleRate * (step % 4 == 2 ? 0.065 : 0.035));
        for (size_t i = 0; i < hatLen && (startSample + i) < totalSamples; ++i) {
          double t = static_cast<double>(i) / sampleRate;
          double env = std::exp(-t * 65.0);
          double noise = (static_cast<double>(rand()) / RAND_MAX) * 2.0 - 1.0;
          bgmSamples[startSample + i] += static_cast<int16_t>(2400.0 * env * noise);
        }
      }
    }

    // --- SPUR 2: BASSLINE (Galloppierender Synth-Bass: Am -> F -> C -> G -> Am -> F -> Dm -> E7) ---
    static const double bassNotes[8][8] = {
      {Notes::A2, Notes::A2, Notes::A3, Notes::A2, Notes::C3, Notes::A2, Notes::E3, Notes::G2}, // Am
      {Notes::F2, Notes::F2, Notes::F3, Notes::F2, Notes::A2, Notes::F2, Notes::C3, Notes::E2}, // F
      {Notes::C3, Notes::C3, Notes::C4, Notes::C3, Notes::E3, Notes::C3, Notes::G3, Notes::B2}, // C
      {Notes::G2, Notes::G2, Notes::G3, Notes::G2, Notes::B2, Notes::G2, Notes::D3, Notes::F2}, // G
      {Notes::A2, Notes::A2, Notes::A3, Notes::A2, Notes::C3, Notes::A2, Notes::E3, Notes::G2}, // Am
      {Notes::F2, Notes::F2, Notes::F3, Notes::F2, Notes::A2, Notes::F2, Notes::C3, Notes::E2}, // F
      {Notes::D3, Notes::D3, Notes::D4, Notes::D3, Notes::F3, Notes::D3, Notes::A3, Notes::C3}, // Dm
      {Notes::E3, Notes::E3, Notes::Gs3, Notes::E3, Notes::B3, Notes::E3, Notes::D4, Notes::B3} // E7
    };

    for (int bar = 0; bar < 8; ++bar) {
      for (int step = 0; step < 16; ++step) {
        int noteIdx = step % 8;
        double freq = bassNotes[bar][noteIdx];
        int globalStep = bar * 16 + step;
        size_t startSample = static_cast<size_t>(globalStep * stepDur * sampleRate);
        size_t noteLen = static_cast<size_t>(sampleRate * stepDur * 0.92);

        for (size_t i = 0; i < noteLen && (startSample + i) < totalSamples; ++i) {
          double t = static_cast<double>(i) / sampleRate;
          double env = (t < 0.008) ? (t / 0.008) : std::exp(-t * 15.0);
          double wave = triangleWave(freq * t) * 0.70 + pulseWave(freq * t, 0.40) * 0.30;
          bgmSamples[startSample + i] += static_cast<int16_t>(5400.0 * env * wave);
        }
      }
    }

    // --- SPUR 3: ARPEGGIOS (Funkelnde 16tel Glocken-Akkorde) ---
    static const double arpChords[8][4] = {
      {Notes::A4, Notes::C5, Notes::E5, Notes::A5}, // Am
      {Notes::F4, Notes::A4, Notes::C5, Notes::F5}, // F
      {Notes::C4, Notes::E4, Notes::G4, Notes::C5}, // C
      {Notes::G4, Notes::B4, Notes::D5, Notes::G5}, // G
      {Notes::A4, Notes::C5, Notes::E5, Notes::A5}, // Am
      {Notes::F4, Notes::A4, Notes::C5, Notes::F5}, // F
      {Notes::D4, Notes::F4, Notes::A4, Notes::D5}, // Dm
      {Notes::E4, Notes::Gs4, Notes::B4, Notes::E5} // E7
    };

    for (int bar = 0; bar < 8; ++bar) {
      for (int step = 0; step < 16; ++step) {
        int noteIdx = (step % 4 == 3) ? 1 : (step % 4);
        double freq = arpChords[bar][noteIdx];
        int globalStep = bar * 16 + step;
        size_t startSample = static_cast<size_t>(globalStep * stepDur * sampleRate);
        size_t noteLen = static_cast<size_t>(sampleRate * stepDur * 0.85);

        for (size_t i = 0; i < noteLen && (startSample + i) < totalSamples; ++i) {
          double t = static_cast<double>(i) / sampleRate;
          double env = std::exp(-t * 24.0);
          double wave = pulseWave(freq * t, 0.25);
          bgmSamples[startSample + i] += static_cast<int16_t>(2500.0 * env * wave);
        }
      }
    }

    // --- SPUR 4: LEAD-MELODIE (Heroische, mitreißende Gamer-Hymne mit Vibrato) ---
    struct LeadNote {
      int startStep;
      int durationSteps;
      double freq;
    };

    static const LeadNote melody[] = {
      // Takt 1
      {0, 3, Notes::E5}, {3, 1, Notes::G5}, {4, 3, Notes::A5}, {7, 3, Notes::C6}, {10, 3, Notes::B5}, {13, 3, Notes::A5},
      // Takt 2
      {16, 4, Notes::G5}, {20, 2, Notes::E5}, {22, 2, Notes::D5}, {24, 4, Notes::E5}, {28, 4, Notes::G5},
      // Takt 3
      {32, 3, Notes::A5}, {35, 1, Notes::B5}, {36, 3, Notes::C6}, {39, 3, Notes::D6}, {42, 4, Notes::E6}, {46, 2, Notes::D6},
      // Takt 4
      {48, 4, Notes::C6}, {52, 4, Notes::B5}, {56, 2, Notes::G5}, {58, 2, Notes::B5}, {60, 4, Notes::D6},
      // Takt 5
      {64, 3, Notes::E6}, {67, 1, Notes::D6}, {68, 3, Notes::C6}, {71, 3, Notes::B5}, {74, 4, Notes::A5}, {78, 4, Notes::C6},
      // Takt 6
      {82, 4, Notes::G5}, {86, 3, Notes::A5}, {89, 3, Notes::G5}, {92, 4, Notes::E5},
      // Takt 7
      {96, 3, Notes::F5}, {99, 3, Notes::A5}, {102, 3, Notes::C6}, {105, 3, Notes::D6}, {108, 3, Notes::F6}, {111, 2, Notes::E6},
      // Takt 8
      {113, 3, Notes::D6}, {116, 3, Notes::E6}, {119, 3, Notes::B5}, {122, 3, Notes::C6}, {125, 3, Notes::B5}
    };

    for (const auto &note : melody) {
      size_t startSample = static_cast<size_t>(note.startStep * stepDur * sampleRate);
      double noteSecs = note.durationSteps * stepDur;
      size_t noteLen = static_cast<size_t>(sampleRate * noteSecs * 0.94);

      for (size_t i = 0; i < noteLen && (startSample + i) < totalSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double progress = t / noteSecs;

        double env = (progress < 0.08) ? (progress / 0.08) : (1.0 - (progress - 0.08) * 0.32);

        // Sanftes 5.5Hz Vibrato
        double vibrato = 1.0 + 0.007 * std::sin(2.0 * M_PI * 5.5 * t);
        double f = note.freq * vibrato;

        // 25% Duty Pulse Wave + sanfte Sinus-Wärme
        double wave = pulseWave(f * t, 0.25) * 0.35 + std::sin(2.0 * M_PI * f * t) * 0.65;

        // Leise abgemischt (auf Begleitungs-Lautstärke)
        bgmSamples[startSample + i] += static_cast<int16_t>(1300.0 * env * wave);
      }
    }

    // Klickfreier Loop-Übergang
    for (size_t i = 0; i < 120 && i < totalSamples; ++i) {
      double f = static_cast<double>(i) / 120.0;
      bgmSamples[i] = static_cast<int16_t>(bgmSamples[i] * f);
      bgmSamples[totalSamples - 1 - i] = static_cast<int16_t>(bgmSamples[totalSamples - 1 - i] * f);
    }
  }

  // ===========================================================================
  // 7. SOUND-EFFEKTE (Synthesizer)
  // ===========================================================================
  void playJumpSound(Engine &e) {
    e.play_melody({
        {Notes::E4, 0.04f},
        {Notes::B4, 0.08f}
    });
  }

  void playDoubleJumpSound(Engine &e) {
    e.play_melody({
        {Notes::G5, 0.04f},
        {Notes::E6, 0.09f}
    });
  }

  void playTrampolineSound(Engine &e, bool isGiant = false) {
    if (isGiant) {
      // Epische Mega-Sprungfeder-Fanfare
      e.play_melody({
          {Notes::C4, 0.03f},
          {Notes::E4, 0.03f},
          {Notes::G4, 0.03f},
          {Notes::C5, 0.04f},
          {Notes::E5, 0.04f},
          {Notes::G5, 0.05f},
          {Notes::C6, 0.16f}
      });
    } else {
      // Fröhlicher Trampolin-Sprungfedersound
      e.play_melody({
          {Notes::C4, 0.03f},
          {Notes::G4, 0.04f},
          {Notes::C5, 0.05f},
          {Notes::G5, 0.07f},
          {Notes::C6, 0.12f}
      });
    }
  }

  void playStompExplosion(Engine &e) {
    e.play_melody({
        {Notes::C3, 0.15f},
        {Notes::A5, 0.10f}
    });
  }

  void playGameOverSound(Engine &e) {
    e.play_melody({
        {Notes::G4, 0.12f},
        {Notes::E4, 0.12f},
        {Notes::C4, 0.15f},
        {Notes::A3, 0.35f}
    });
  }

  void playStartFanfare(Engine &e) {
    e.play_melody({
        {Notes::C5, 0.06f},
        {Notes::E5, 0.06f},
        {Notes::G5, 0.06f},
        {Notes::C6, 0.22f}
    });
  }

  void playLevelUpSound(Engine &e) {
    e.play_melody({
        {Notes::C5, 0.06f},
        {Notes::E5, 0.06f},
        {Notes::G5, 0.06f},
        {Notes::B5, 0.06f},
        {Notes::C6, 0.22f}
    });
  }

  void playVictorySound(Engine &e) {
    e.play_melody({
        {Notes::C5, 0.08f},
        {Notes::E5, 0.08f},
        {Notes::G5, 0.08f},
        {Notes::C6, 0.12f},
        {Notes::G5, 0.08f},
        {Notes::C6, 0.35f}
    });
  }

  // ===========================================================================
  // 7. PARTIKEL-SYSTEME (Sternenstaub, Explosionen, Einhorn-Magie)
  // ===========================================================================
  // Magischer Einhorn-Crash: Pink, Cyan, Magenta, Lila & Goldener Sternenstaub
  void spawnUnicornDeathMagic(float x, float y, int count = 75) {
    static const uint8_t magicColors[] = {
      Colors::Pink, Colors::Magenta, Colors::Cyan,
      Colors::SkyBlue, Colors::Purple, Colors::Gold, Colors::White
    };
    for (int i = 0; i < count; ++i) {
      Particle p;
      p.x = x + ((rand() % 12) - 6);
      p.y = y + ((rand() % 12) - 6);

      float angle = (rand() % 360) * (M_PI / 180.0f);
      float speed = 30.0f + (rand() % 130);
      p.vx = speed * std::cos(angle);
      p.vy = speed * std::sin(angle) - 45.0f; // Leichter sanfter Auftrieb

      p.maxLife = 0.60f + (rand() % 45) / 100.0f;
      p.life = p.maxLife;
      p.isThermal = false;
      p.isRainbow = false;
      p.isUnicornMagic = true;
      p.color = magicColors[rand() % 7];
      particles.push_back(p);
    }
  }

  // Starfighter-Explosion für zerschmetterte Sternschnuppen
  void spawnMeteorExplosion(float x, float y, float meteorVx, float meteorVy, int count = 45) {
    for (int i = 0; i < count; ++i) {
      Particle p;
      p.x = x;
      p.y = y;

      float angle = (rand() % 360) * (M_PI / 180.0f);
      float burstSpeed = 25.0f + (rand() % 90);

      p.vx = meteorVx * 0.4f + burstSpeed * std::cos(angle);
      p.vy = meteorVy * 0.4f + burstSpeed * std::sin(angle);

      p.maxLife = 0.45f + (rand() % 35) / 100.0f;
      p.life = p.maxLife;
      p.isThermal = true;
      p.isRainbow = false;
      p.isUnicornMagic = false;
      p.color = Colors::White;

      particles.push_back(p);
    }
  }

  void spawnRainbowSparkles(float x, float y, float w, int count = 35) {
    for (int i = 0; i < count; ++i) {
      Particle p;
      p.x = x + (rand() % std::max(1, static_cast<int>(w)));
      p.y = y + (rand() % 16);
      p.vx = ((rand() % 120) - 60) - 40.0f;
      p.vy = -70.0f - (rand() % 110); // Schwebt kräftig nach oben
      p.maxLife = 0.60f + (rand() % 40) / 100.0f;
      p.life = p.maxLife;
      p.isThermal = false;
      p.isRainbow = true;
      p.isUnicornMagic = false;

      static const uint8_t rainbowColors[] = {
        Colors::Red, Colors::Orange, Colors::Yellow,
        Colors::LightGreen, Colors::Cyan, Colors::Pink, Colors::Purple
      };
      p.color = rainbowColors[rand() % 7];

      particles.push_back(p);
    }
  }

  void spawnUnicornTrail(float x, float y, UnicornColor col) {
    Particle p;
    p.x = x - 2.0f;
    p.y = y + 8.0f + (rand() % 10);
    p.vx = -currentScrollSpeed * 0.35f + ((rand() % 20) - 10);
    p.vy = ((rand() % 30) - 15);
    p.maxLife = 0.30f + (rand() % 20) / 100.0f;
    p.life = p.maxLife;
    p.isThermal = false;
    p.isRainbow = false;
    p.isUnicornMagic = false;

    if (col == UnicornColor::Blue) {
      p.color = (rand() % 2 == 0) ? Colors::Cyan : Colors::SkyBlue;
    } else if (col == UnicornColor::Pink) {
      p.color = (rand() % 2 == 0) ? Colors::Pink : Colors::Magenta;
    } else {
      static const uint8_t colors[] = {Colors::White, Colors::Gold, Colors::Cyan, Colors::Pink};
      p.color = colors[rand() % 4];
    }

    particles.push_back(p);
  }

  void spawnMeteorTrail(float x, float y, float vx, float vy) {
    Particle p;
    p.x = x + ((rand() % 6) - 3);
    p.y = y + ((rand() % 6) - 3);
    p.vx = -vx * 0.20f + ((rand() % 20) - 10);
    p.vy = -vy * 0.20f + ((rand() % 20) - 10);
    p.maxLife = 0.22f + (rand() % 15) / 100.0f;
    p.life = p.maxLife;
    p.isThermal = true;
    p.isRainbow = false;
    p.isUnicornMagic = false;
    p.color = (rand() % 2 == 0) ? Colors::Yellow : Colors::Gold;
    particles.push_back(p);
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
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.vx *= 0.98f;
        p.vy *= 0.98f;

        float progress = p.life / p.maxLife;

        if (p.isUnicornMagic) {
          // Traumhafter Einhorn-Glitzerstaub
          p.vy += 65.0f * dt; // Sanftes Absinken
          if (rand() % 4 == 0) {
            static const uint8_t magicColors[] = {
              Colors::Pink, Colors::Magenta, Colors::Cyan,
              Colors::SkyBlue, Colors::Purple, Colors::Gold, Colors::White
            };
            p.color = magicColors[rand() % 7];
          }
        } else if (p.isThermal) {
          if (progress > 0.75f) {
            p.color = Colors::White;
          } else if (progress > 0.50f) {
            p.color = Colors::Yellow;
          } else if (progress > 0.25f) {
            p.color = Colors::Orange;
          } else if (progress > 0.10f) {
            p.color = Colors::Red;
          } else {
            p.color = Colors::DarkGray;
          }
        } else if (p.isRainbow) {
          if (rand() % 5 == 0) {
            static const uint8_t rColors[] = {
              Colors::Red, Colors::Orange, Colors::Yellow,
              Colors::LightGreen, Colors::Cyan, Colors::Pink, Colors::Purple
            };
            p.color = rColors[rand() % 7];
          }
        }

        e.pset(p.x, p.y, p.color);
        ++i;
      }
    }
  }

  // ===========================================================================
  // 8. HINTERGRUND-RENDERING (Level 1: Blumenwiese, Level 2: Nachthimmel)
  // ===========================================================================
  // --- LEVEL 1: STRAHLENDER SONNENTAG & BLAUE BLUMENWIESE ---
  void drawMeadowBackground(Engine &e) {
    float dt = e.dt();

    // 1. Himmel (Himmelblau mit sanftem Cyan-Übergang zum Horizont)
    e.rectfill(0, 0, 320, 130, Colors::SkyBlue);
    e.rectfill(0, 130, 320, static_cast<int>(groundY) - 130, Colors::Cyan);

    // 2. Sanfte grüne Hügel im Hintergrund (Parallaxe-Bewegung)
    float hillOffset = distanceTraveled * 0.45f;
    for (int x = 0; x < 320; ++x) {
      float h1 = 168.0f + 14.0f * std::sin((x + hillOffset) * 0.014f);
      e.line(x, static_cast<int>(h1), x, static_cast<int>(groundY), Colors::DarkGreen);
      float h2 = 180.0f + 10.0f * std::sin((x + hillOffset * 1.5f + 75.0f) * 0.022f);
      e.line(x, static_cast<int>(h2), x, static_cast<int>(groundY), Colors::LightGreen);
    }

    // 3. Strahlende Sonne mit rotierenden/pulsierenden Sonnenstrahlen & Gesicht
    float sunX = 265.0f;
    float sunY = 38.0f;

    // Sonnenstrahlen
    for (int a = 0; a < 8; ++a) {
      float angle = a * (3.14159265f / 4.0f) + animTimer * 0.75f;
      float r1 = 17.0f;
      float r2 = 23.0f + 2.5f * std::sin(animTimer * 4.0f + a * 1.5f);
      e.line(static_cast<int>(sunX + std::cos(angle) * r1),
             static_cast<int>(sunY + std::sin(angle) * r1),
             static_cast<int>(sunX + std::cos(angle) * r2),
             static_cast<int>(sunY + std::sin(angle) * r2),
             (a % 2 == 0) ? Colors::Gold : Colors::Yellow);
    }

    // Sonnenkörper & Glanz
    e.circlefill(sunX, sunY, 18.0f, Colors::Gold);
    e.circlefill(sunX, sunY, 15.0f, Colors::Yellow);
    e.circlefill(sunX - 3.0f, sunY - 3.0f, 4.0f, Colors::White);

    // Retro-Gesicht der Sonne
    e.pset(static_cast<int>(sunX - 4), static_cast<int>(sunY - 1), Colors::Black);
    e.pset(static_cast<int>(sunX + 4), static_cast<int>(sunY - 1), Colors::Black);
    e.line(static_cast<int>(sunX - 3), static_cast<int>(sunY + 4),
           static_cast<int>(sunX + 3), static_cast<int>(sunY + 4), Colors::Orange);

    // 4. Weisse Schönwetter-Wolken
    for (auto &c : clouds) {
      if (state == State::Playing) {
        c.x -= c.speed * (currentScrollSpeed / baseScrollSpeed) * dt;
        if (c.x + c.width < 0.0f) {
          c.x = 320.0f + rand() % 50;
          c.y = static_cast<float>(18 + rand() % 65);
        }
      }
      e.circlefill(c.x + 8.0f, c.y, 8.0f, Colors::White);
      e.circlefill(c.x + 18.0f, c.y - 3.0f, 10.0f, Colors::White);
      e.circlefill(c.x + 28.0f, c.y, 7.0f, Colors::White);
      e.circlefill(c.x + 18.0f, c.y + 2.0f, 6.0f, Colors::LightGray);
      e.rectfill(c.x + 4.0f, c.y, c.width - 8.0f, 6.0f, Colors::White);
    }

    // 5. Saftig grüne Wiese (Boden ab groundY = 200)
    e.rectfill(0, groundY, 320, 240 - groundY, Colors::DarkGreen);
    e.rectfill(0, groundY, 320, 3, Colors::LightGreen);
    e.rectfill(0, groundY + 3, 320, 2, Colors::DarkGreen);

    // Bunte Blumen & Blüten auf der Wiese
    int flowerOffset = static_cast<int>(distanceTraveled * 1.5f) % 24;
    for (int fx = -flowerOffset; fx < 320; fx += 24) {
      int flowerType = std::abs((fx + static_cast<int>(distanceTraveled * 1.5f)) / 24) % 4;
      // Stängel & Blatt
      e.rectfill(fx + 5, groundY - 6, 2, 7, Colors::DarkGreen);
      e.pset(fx + 4, groundY - 3, Colors::LightGreen);
      // Blüte je nach Typ
      if (flowerType == 0) {
        // Rote Mohnblume
        e.rectfill(fx + 4, groundY - 9, 4, 4, Colors::Red);
        e.pset(fx + 5, groundY - 8, Colors::Pink);
      } else if (flowerType == 1) {
        // Gelbe Butterblume
        e.circlefill(fx + 6, groundY - 8, 2.5f, Colors::Yellow);
        e.pset(fx + 6, groundY - 8, Colors::White);
      } else if (flowerType == 2) {
        // Rosa Glockenblume
        e.rectfill(fx + 4, groundY - 9, 4, 4, Colors::Pink);
        e.pset(fx + 5, groundY - 8, Colors::Gold);
      } else {
        // Weisse Margerite
        e.circlefill(fx + 6, groundY - 8, 2.5f, Colors::White);
        e.pset(fx + 6, groundY - 8, Colors::Gold);
      }
    }
  }

  // --- LEVEL 2: ZAUBERHAFTER NACHTHIMMEL & MOND ---
  void drawNightBackground(Engine &e) {
    float dt = e.dt();

    // 1. Himmel
    e.rectfill(0, 0, 320, 110, Colors::DarkPurple);
    e.rectfill(0, 110, 320, groundY - 110, Colors::Black);

    // 2. Mond
    float moonX = 265.0f;
    float moonY = 36.0f;
    e.circlefill(moonX, moonY, 18.0f, Colors::DarkPurple);
    e.circlefill(moonX, moonY, 16.0f, Colors::LightGray);
    e.circlefill(moonX, moonY, 14.0f, Colors::White);
    e.circlefill(moonX - 4.0f, moonY - 3.0f, 3.0f, Colors::LightGray);
    e.circlefill(moonX + 5.0f, moonY + 2.0f, 2.5f, Colors::LightGray);

    // 3. Sterne
    for (auto &s : stars) {
      if (state == State::Playing) {
        s.x -= s.speed * (currentScrollSpeed / baseScrollSpeed) * dt;
        if (s.x < 0.0f) {
          s.x = 320.0f;
          s.y = static_cast<float>(rand() % 160);
        }
      }
      s.twinkleTimer += dt * 3.0f;
      uint8_t starColor = s.color;
      if (std::sin(s.twinkleTimer) > 0.6f && s.color == Colors::White) {
        starColor = Colors::Gold;
      }
      e.pset(s.x, s.y, starColor);
    }

    // 4. Wolken
    for (auto &c : clouds) {
      if (state == State::Playing) {
        c.x -= c.speed * (currentScrollSpeed / baseScrollSpeed) * dt;
        if (c.x + c.width < 0.0f) {
          c.x = 320.0f + rand() % 50;
          c.y = static_cast<float>(18 + rand() % 65);
        }
      }
      e.circlefill(c.x + 8.0f, c.y, 8.0f, c.color);
      e.circlefill(c.x + 18.0f, c.y - 3.0f, 10.0f, c.color);
      e.circlefill(c.x + 28.0f, c.y, 7.0f, c.color);
      e.rectfill(c.x + 4.0f, c.y, c.width - 8.0f, 6.0f, c.color);
    }

    // 5. Boden (Exakt ab groundY = 200)
    e.rectfill(0, groundY, 320, 240 - groundY, Colors::DarkGreen);
    e.rectfill(0, groundY, 320, 3, Colors::LightGreen);
    e.rectfill(0, groundY + 3, 320, 2, Colors::DarkGreen);

    // Deko-Gras & Kristalle
    int scrollOffset = static_cast<int>(distanceTraveled * 1.5f) % 32;
    for (int gx = -scrollOffset; gx < 320; gx += 32) {
      e.pset(gx + 6, groundY + 1, Colors::Cyan);
      e.pset(gx + 18, groundY + 1, Colors::Pink);
      e.pset(gx + 26, groundY + 2, Colors::Gold);
    }
  }

  // ===========================================================================
  // 9. EINHORN-PIXEL-ART & ANIMATION
  // ===========================================================================
  void drawUnicorn(Engine &e, float x, float y, UnicornColor col, bool running, float animPhase) {
    uint8_t bodyColor = Colors::White;
    uint8_t shadeColor = Colors::LightGray;
    uint8_t maneColor1 = Colors::Pink;
    uint8_t maneColor2 = Colors::Purple;

    if (col == UnicornColor::Blue) {
      bodyColor = Colors::Cyan;
      shadeColor = Colors::SkyBlue;
      maneColor1 = Colors::Blue;
      maneColor2 = Colors::Pink;
    } else if (col == UnicornColor::Pink) {
      bodyColor = Colors::Pink;
      shadeColor = Colors::Magenta;
      maneColor1 = Colors::Purple;
      maneColor2 = Colors::Cyan;
    } else {
      bodyColor = Colors::White;
      shadeColor = Colors::LightGray;
      maneColor1 = Colors::Pink;
      maneColor2 = Colors::Cyan;
    }

    float speedFactor = (currentScrollSpeed / baseScrollSpeed);
    float bob = 0.0f;
    int legFrame = 0;
    if (running && isOnGround) {
      bob = std::abs(std::sin(animPhase * 13.0f * speedFactor)) * 2.0f;
      legFrame = static_cast<int>(animPhase * 16.0f * speedFactor) % 4;
    }

    float py = y - bob;

    // 1. Schweif
    float tailWave = std::sin(animPhase * 10.0f * speedFactor) * 2.0f;
    e.rectfill(x - 6, py + 7 + tailWave, 5, 2, maneColor1);
    e.rectfill(x - 9, py + 8 + tailWave, 4, 3, maneColor2);
    e.rectfill(x - 12, py + 10 + tailWave, 4, 2, Colors::Gold);

    // 2. Körper & Rumpf
    e.rectfill(x + 2, py + 6, 14, 8, bodyColor);
    e.rectfill(x + 3, py + 12, 12, 2, shadeColor);

    // 3. Hals & Kopf
    e.rectfill(x + 11, py + 2, 5, 6, bodyColor);
    e.rectfill(x + 13, py, 7, 5, bodyColor);
    e.rectfill(x + 18, py + 2, 3, 3, shadeColor);

    // Auge
    e.pset(x + 16, py + 1, Colors::DarkPurple);
    e.pset(x + 16, py, Colors::White);

    // Ohr
    e.rectfill(x + 12, py - 2, 2, 2, bodyColor);
    e.pset(x + 12, py - 1, Colors::Pink);

    // 4. Mähne
    float maneWave = std::sin(animPhase * 8.0f * speedFactor) * 1.5f;
    e.rectfill(x + 8 + maneWave, py + 1, 4, 3, maneColor1);
    e.rectfill(x + 6 + maneWave, py + 4, 5, 3, maneColor2);
    e.rectfill(x + 5, py + 7, 3, 2, Colors::Gold);

    // 5. Goldenes Horn
    e.line(x + 17, py - 1, x + 23, py - 6, Colors::Gold);
    e.line(x + 18, py, x + 23, py - 6, Colors::Yellow);
    e.pset(x + 23, py - 6, Colors::White);

    if (static_cast<int>(animPhase * 6.0f) % 2 == 0) {
      e.pset(x + 24, py - 6, Colors::Gold);
      e.pset(x + 23, py - 7, Colors::Yellow);
    }

    // 6. Beine & Hufe
    if (!isOnGround) {
      e.rectfill(x + 13, py + 14, 2, 5, bodyColor);
      e.rectfill(x + 15, py + 17, 3, 2, bodyColor);
      e.rectfill(x + 17, py + 18, 2, 2, Colors::Gold);

      e.rectfill(x + 3, py + 14, 2, 5, bodyColor);
      e.rectfill(x, py + 17, 3, 2, bodyColor);
      e.rectfill(x - 2, py + 18, 2, 2, Colors::Gold);
    } else {
      switch (legFrame) {
      case 0:
        e.rectfill(x + 14, py + 14, 2, 7, bodyColor);
        e.rectfill(x + 14, py + 20, 2, 2, Colors::Gold);
        e.rectfill(x + 3, py + 14, 2, 7, bodyColor);
        e.rectfill(x + 3, py + 20, 2, 2, Colors::Gold);
        break;
      case 1:
        e.rectfill(x + 12, py + 14, 2, 6, bodyColor);
        e.rectfill(x + 12, py + 19, 2, 2, Colors::Gold);
        e.rectfill(x + 5, py + 14, 2, 6, bodyColor);
        e.rectfill(x + 5, py + 19, 2, 2, Colors::Gold);
        break;
      case 2:
        e.rectfill(x + 15, py + 14, 2, 7, bodyColor);
        e.rectfill(x + 15, py + 20, 2, 2, Colors::Gold);
        e.rectfill(x + 2, py + 14, 2, 7, bodyColor);
        e.rectfill(x + 2, py + 20, 2, 2, Colors::Gold);
        break;
      case 3:
        e.rectfill(x + 13, py + 14, 2, 6, bodyColor);
        e.rectfill(x + 13, py + 19, 2, 2, Colors::Gold);
        e.rectfill(x + 4, py + 14, 2, 6, bodyColor);
        e.rectfill(x + 4, py + 19, 2, 2, Colors::Gold);
        break;
      }
    }
  }

  // ===========================================================================
  // 10. REGENBOGEN-HINDERNISSE (Boden) & VERBLASSEN (Fade-Out)
  // ===========================================================================
  void drawRainbowArch(Engine &e, const RainbowArch &r) {
    if (r.fadeTimer <= 0.0f)
      return;

    static const uint8_t archColors[6] = {
      Colors::Red,
      Colors::Orange,
      Colors::Yellow,
      Colors::LightGreen,
      Colors::Cyan,
      Colors::Purple
    };

    bool isFading = r.touched;
    int fadeThreshold = static_cast<int>(r.fadeTimer * 10.0f);

    float cx = r.x + r.width * 0.5f;
    float cy = groundY;
    float rx = r.width * 0.5f;
    float ry = r.height;

    int minX = std::max(0, static_cast<int>(r.x));
    int maxX = std::min(319, static_cast<int>(r.x + r.width));
    int minY = std::max(0, static_cast<int>(r.y));
    int maxY = std::min(239, static_cast<int>(groundY));

    // Mindest-Innenaussparung für den Bogen
    float innerCutoff = r.isGiant ? 0.28f : 0.20f;

    for (int py = minY; py <= maxY; ++py) {
      float dy = (cy - py) / ry;
      if (dy < 0.0f)
        continue;

      for (int px = minX; px <= maxX; ++px) {
        float dx = (px - cx) / rx;
        float d2 = dx * dx + dy * dy;

        if (d2 >= innerCutoff && d2 <= 1.0f) {
          float d = std::sqrt(d2);

          int band = static_cast<int>((1.0f - d) / (1.0f - std::sqrt(innerCutoff)) * 6.0f);
          band = std::clamp(band, 0, 5);

          if (isFading) {
            int hash = (std::abs(px * 17 ^ py * 31)) % 10;
            if (hash >= fadeThreshold)
              continue;
          }

          e.pset(px, py, archColors[band]);
        }
      }
    }
  }

  // ===========================================================================
  // 11. STERNSCHNUPPEN / METEORE (Fallende Hindernisse)
  // ===========================================================================
  void drawShootingStar(Engine &e, const ShootingStar &m) {
    if (!m.active)
      return;

    float x = m.x;
    float y = m.y;

    e.rectfill(x - 2, y - 2, 5, 5, Colors::Yellow);
    e.line(x - 5, y, x + 5, y, Colors::Gold);
    e.line(x, y - 5, x, y + 5, Colors::Gold);
    e.pset(x, y, Colors::White);

    e.line(x, y, x - m.vx * 0.05f, y - m.vy * 0.05f, Colors::Orange);
    e.line(x + 1, y, x + 1 - m.vx * 0.04f, y - m.vy * 0.04f, Colors::Red);
  }

  // ===========================================================================
  // 12. ZUSTAND: COLOR SELECT / TITELBILDSCHIRM
  // ===========================================================================
  void updateColorSelect(Engine &e) {
    float dt = e.dt();
    animTimer += dt;

    e.draw_text(68, 18, "THE UNICORN", Colors::Gold, 2);
    e.draw_text(54, 42, "DAS MAGISCHE RETRO-ABENTEUER", Colors::White, 1);

    e.rect(20, 56, 280, 66, Colors::DarkGray);
    e.draw_text(26, 62, "- 2 LEVEL    : 1. BLUMENWIESE -> 2. NACHT (JE 20s)", Colors::LightGreen, 1);
    e.draw_text(26, 74, "- REGENBOGEN : VON OBEN DRAUF = TRAMPOLIN!", Colors::Yellow, 1);
    e.draw_text(26, 86, "- METEORE    : AUSWEICHEN ODER MARIO-STOMPEN!", Colors::Pink, 1);
    e.draw_text(26, 98, "- ZIEL       : BEIDE LEVEL SCHAFFEN ZUM SIEG!", Colors::Cyan, 1);
    e.draw_text(26, 110,"  (TRAMPOLIN-SPRUNG! VON VORNE = CRASH)", Colors::LightGray, 1);

    e.draw_text(70, 128, "WAEHLE DEIN EINHORN:", Colors::White, 1);

    // 1. Blau
    bool isBlue = (chosenColor == UnicornColor::Blue);
    e.rectfill(30, 140, 75, 45, isBlue ? Colors::DarkGray : Colors::Black);
    e.rect(30, 140, 75, 45, isBlue ? Colors::Cyan : Colors::DarkGray);
    drawUnicorn(e, 54, 150, UnicornColor::Blue, true, animTimer);
    e.draw_text(42, 172, "1: BLAU", isBlue ? Colors::Cyan : Colors::LightGray, 1);

    // 2. Weiß
    bool isWhite = (chosenColor == UnicornColor::White);
    e.rectfill(122, 140, 75, 45, isWhite ? Colors::DarkGray : Colors::Black);
    e.rect(122, 140, 75, 45, isWhite ? Colors::White : Colors::DarkGray);
    drawUnicorn(e, 146, 150, UnicornColor::White, true, animTimer);
    e.draw_text(132, 172, "2: WEISS", isWhite ? Colors::White : Colors::LightGray, 1);

    // 3. Pink
    bool isPink = (chosenColor == UnicornColor::Pink);
    e.rectfill(214, 140, 75, 45, isPink ? Colors::DarkGray : Colors::Black);
    e.rect(214, 140, 75, 45, isPink ? Colors::Pink : Colors::DarkGray);
    drawUnicorn(e, 238, 150, UnicornColor::Pink, true, animTimer);
    e.draw_text(226, 172, "3: PINK", isPink ? Colors::Pink : Colors::LightGray, 1);

    bool blink = (static_cast<int>(animTimer * 2.5f) % 2) == 0;
    if (blink) {
      e.draw_text(60, 202, "DRUECKE LEERTASTE ZUM START!", Colors::Gold, 1);
    }
    e.draw_text(48, 220, "STEUERUNG: LEERTASTE / W = SPRINGEN", Colors::LightGray, 1);

    if (e.pressed(Key::Num1) || (e.pressed(Key::Left) && chosenColor == UnicornColor::White)) {
      chosenColor = UnicornColor::Blue;
      e.play_tone(Notes::C5, 0.08f);
    } else if (e.pressed(Key::Num2)) {
      chosenColor = UnicornColor::White;
      e.play_tone(Notes::E5, 0.08f);
    } else if (e.pressed(Key::Num3) || (e.pressed(Key::Right) && chosenColor == UnicornColor::White)) {
      chosenColor = UnicornColor::Pink;
      e.play_tone(Notes::G5, 0.08f);
    }

    if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
      playStartFanfare(e);
      resetGame();
      e.play_bgm(bgmSamples, 44100, true);
      isBgmPlaying = true;
      state = State::Playing;
    }
  }

  // ===========================================================================
  // 13. ZUSTAND: PLAYING (Aktives Spiel & Level-Fortschritt)
  // ===========================================================================
  void updatePlaying(Engine &e) {
    float dt = e.dt();
    animTimer += dt;

    if (!isBgmPlaying) {
      e.play_bgm(bgmSamples, 44100, true);
      isBgmPlaying = true;
    }

    // --- LEVEL-TIMER & FORTSCHRITT ---
    levelTimer += dt;
    if (levelBannerTimer > 0.0f) {
      levelBannerTimer -= dt;
    }

    // Level 1 -> Level 2 Übergang nach 20 Sekunden
    if (currentLevel == 1 && levelTimer >= levelDuration) {
      currentLevel = 2;
      levelTimer = 0.0f;
      levelBannerTimer = 3.0f;
      playLevelUpSound(e);
      spawnRainbowSparkles(playerX, playerY, 120.0f, 65);
    }
    // Level 2 -> SIEG / YOU WIN nach weiteren 20 Sekunden
    else if (currentLevel == 2 && levelTimer >= levelDuration) {
      e.stop_bgm();
      isBgmPlaying = false;
      playVictorySound(e);
      score += 2000; // Glorreicher Sieg-Bonus!
      if (score > highscore) {
        highscore = score;
      }
      state = State::Victory;
      return;
    }

    // KONTINUIERLICHE BESCHLEUNIGUNG (Immer schneller & herausfordernder!)
    distanceTraveled += currentScrollSpeed * dt * 0.1f;
    currentScrollSpeed = baseScrollSpeed + std::min(320.0f, distanceTraveled * 0.20f);

    score = static_cast<int>(distanceTraveled) + rainbowsCollected * 200 + meteorsStomped * 250;
    if (score > highscore) {
      highscore = score;
    }

    float speedScale = currentScrollSpeed / baseScrollSpeed;

    // --- 1. Einhorn-Steuerung & Physik ---
    bool jumpPressed = e.pressed(Key::Space) || e.pressed(Key::W) || e.pressed(Key::Up);

    if (jumpPressed) {
      if (isOnGround) {
        playerVy = jumpVelocity;
        isOnGround = false;
        canDoubleJump = true;
        playJumpSound(e);
        spawnRainbowSparkles(playerX, playerY + unicornHeight, unicornWidth, 8);
      } else if (canDoubleJump) {
        playerVy = doubleJumpVelocity;
        canDoubleJump = false;
        playDoubleJumpSound(e);
        spawnRainbowSparkles(playerX, playerY + unicornHeight, unicornWidth, 14);
      }
    }

    playerVy += gravity * dt;
    playerY += playerVy * dt;

    // Bodenkollision
    if (playerY >= groundY - unicornHeight) {
      playerY = groundY - unicornHeight;
      playerVy = 0.0f;
      isOnGround = true;
      canDoubleJump = true;
    }

    // Glitzerspur
    hoofSparkleTimer -= dt;
    if (hoofSparkleTimer <= 0.0f) {
      hoofSparkleTimer = 0.06f / speedScale;
      spawnUnicornTrail(playerX, playerY, chosenColor);
    }

    // --- 2. Regenbögen spawnen, bewegen & Kollision prüfen ---
    rainbowSpawnTimer -= dt;
    if (rainbowSpawnTimer <= 0.0f) {
      RainbowArch r;

      // 30% Chance auf einen RIESIGEN Doppel-Regenbogen!
      bool isGiant = (rand() % 100 < 30);
      r.isGiant = isGiant;

      if (isGiant) {
        r.width = 62.0f + (rand() % 14);  // Doppelt so breit!
        r.height = 36.0f + (rand() % 8);  // Doppelt so hoch!
      } else {
        r.width = 34.0f + (rand() % 12);  // Normal
        r.height = 20.0f + (rand() % 8);
      }

      r.x = 320.0f + 10.0f;
      r.y = groundY - r.height;
      r.touched = false;
      r.fadeTimer = 1.0f;
      rainbows.push_back(r);

      // Spawner skaliert dynamisch mit der Spielgeschwindigkeit
      float baseInterval = isGiant ? 2.4f : 1.7f;
      rainbowSpawnTimer = (baseInterval + (rand() % 120) / 100.0f) / speedScale;
    }

    // Hitbox des Einhorns
    float pLeft = playerX + 4.0f;
    float pRight = playerX + unicornWidth - 2.0f;
    float pTop = playerY;
    float pBottom = playerY + unicornHeight;

    for (size_t i = 0; i < rainbows.size();) {
      auto &r = rainbows[i];
      r.x -= currentScrollSpeed * dt;

      drawRainbowArch(e, r);

      if (!r.touched) {
        float rLeft = r.x + 3.0f;
        float rRight = r.x + r.width - 3.0f;
        float rTop = r.y;
        float rBottom = groundY;

        bool overlap = (pRight >= rLeft && pLeft <= rRight && pBottom >= rTop && pTop <= rBottom);

        if (overlap) {
          // TRAMPOLIN-PRÜFUNG:
          // Fällt das Einhorn von oben herab auf das obere Drittel des Regenbogens?
          float stompTolerance = r.isGiant ? 18.0f : 14.0f;
          if (playerVy > 0.0f && pBottom <= rTop + stompTolerance) {
            // TRAMPOLIN-KATAPULT!
            r.touched = true;
            rainbowsCollected += (r.isGiant ? 2 : 1);
            playerVy = r.isGiant ? megaTrampolineVelocity : trampolineVelocity;
            isOnGround = false;
            canDoubleJump = true;
            playTrampolineSound(e, r.isGiant);
            spawnRainbowSparkles(r.x, r.y, r.width, r.isGiant ? 60 : 35);
          } else {
            // CRASH! Magischer Einhorn-Crash-Effekt
            e.stop_bgm();
            isBgmPlaying = false;
            playGameOverSound(e);
            spawnUnicornDeathMagic(playerX + 12.0f, playerY + 10.0f, 80);
            state = State::GameOver;
          }
        }
      } else {
        r.fadeTimer -= dt * 2.8f;
      }

      if (r.x + r.width < -20.0f || (r.touched && r.fadeTimer <= 0.0f)) {
        rainbows[i] = rainbows.back();
        rainbows.pop_back();
      } else {
        ++i;
      }
    }

    // --- 3. Sternschnuppen (Meteore) spawnen & bewegen ---
    meteorSpawnTimer -= dt;
    if (meteorSpawnTimer <= 0.0f) {
      ShootingStar m;
      m.x = 240.0f + (rand() % 90);
      m.y = -10.0f;
      m.vx = -currentScrollSpeed * (0.85f + (rand() % 35) / 100.0f);
      m.vy = (120.0f + (rand() % 60)) * speedScale;
      m.size = 12.0f;
      m.active = true;
      meteors.push_back(m);

      meteorSpawnTimer = (2.2f + (rand() % 130) / 100.0f) / speedScale;
    }

    for (size_t i = 0; i < meteors.size();) {
      auto &m = meteors[i];
      m.x += m.vx * dt;
      m.y += m.vy * dt;

      m.trailTimer -= dt;
      if (m.trailTimer <= 0.0f) {
        m.trailTimer = 0.04f / speedScale;
        spawnMeteorTrail(m.x, m.y, m.vx, m.vy);
      }

      drawShootingStar(e, m);

      float mLeft = m.x - 5.0f;
      float mRight = m.x + 5.0f;
      float mTop = m.y - 5.0f;
      float mBottom = m.y + 5.0f;

      bool overlap = (pRight >= mLeft && pLeft <= mRight && pBottom >= mTop && pTop <= mBottom);

      if (overlap && m.active) {
        if (playerVy > 0.0f && pBottom <= mTop + 12.0f) {
          // ERFOLGREICHER MARIO STOMP!
          m.active = false;
          meteorsStomped++;
          playerVy = stompBounceVelocity;
          isOnGround = false;
          canDoubleJump = true;
          playStompExplosion(e);
          spawnMeteorExplosion(m.x, m.y, m.vx, m.vy, 55);
        } else {
          // CRASH! Magischer Einhorn-Crash-Effekt
          m.active = false;
          e.stop_bgm();
          isBgmPlaying = false;
          playGameOverSound(e);
          spawnUnicornDeathMagic(playerX + 12.0f, playerY + 10.0f, 80);
          state = State::GameOver;
        }
      }

      if (m.y >= groundY || m.x < -30.0f || !m.active) {
        if (m.active && m.y >= groundY) {
          spawnMeteorExplosion(m.x, groundY, m.vx * 0.3f, -40.0f, 15);
        }
        meteors[i] = meteors.back();
        meteors.pop_back();
      } else {
        ++i;
      }
    }

    // --- 4. Einhorn zeichnen ---
    drawUnicorn(e, playerX, playerY, chosenColor, true, animTimer);
  }

  // ===========================================================================
  // 14. ZUSTAND: GAME OVER
  // ===========================================================================
  void updateGameOver(Engine &e) {
    float dt = e.dt();
    animTimer += dt;

    e.rectfill(40, 30, 240, 175, Colors::Black);
    e.rect(40, 30, 240, 175, Colors::Pink);
    e.rect(42, 32, 236, 171, Colors::DarkPurple);

    e.draw_text(94, 42, "GAME OVER", Colors::Red, 2);

    std::string lvlStr = (currentLevel == 1) ? "LEVEL 1 (WIESE)" : "LEVEL 2 (NACHT)";
    e.draw_text(60, 72, "ERREICHT     : " + lvlStr, Colors::Yellow, 1);
    e.draw_text(60, 88, "PUNKTE       : " + std::to_string(score), Colors::Gold, 1);
    e.draw_text(60, 104, "HIGHSCORE    : " + std::to_string(highscore), Colors::White, 1);
    e.draw_text(60, 120, "REGENBOGEN   : " + std::to_string(rainbowsCollected), Colors::Cyan, 1);
    e.draw_text(60, 136, "STERNE STOMP : " + std::to_string(meteorsStomped), Colors::Pink, 1);

    e.draw_text(54, 154, "FARBE WECHSELN: 1, 2 ODER 3", Colors::LightGray, 1);

    bool blink = (static_cast<int>(animTimer * 2.5f) % 2) == 0;
    if (blink) {
      e.draw_text(58, 172, "LEERTASTE: NOCHMAL SPIELEN", Colors::Yellow, 1);
    }
    e.draw_text(62, 188, "ESCAPE   : HAUPTMENUE", Colors::LightGray, 1);

    if (e.pressed(Key::Num1)) {
      chosenColor = UnicornColor::Blue;
      e.play_tone(Notes::C5, 0.08f);
    } else if (e.pressed(Key::Num2)) {
      chosenColor = UnicornColor::White;
      e.play_tone(Notes::E5, 0.08f);
    } else if (e.pressed(Key::Num3)) {
      chosenColor = UnicornColor::Pink;
      e.play_tone(Notes::G5, 0.08f);
    }

    if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
      playStartFanfare(e);
      resetGame();
      e.play_bgm(bgmSamples, 44100, true);
      isBgmPlaying = true;
      state = State::Playing;
    }
  }

  // ===========================================================================
  // 15. ZUSTAND: VICTORY / SIEG (BEIDE LEVEL MEISTERHAFT GESCHAFFT!)
  // ===========================================================================
  void updateVictory(Engine &e) {
    float dt = e.dt();
    animTimer += dt;

    // Feierlicher Konfetti- & Sternenstaubregen
    if (rand() % 3 == 0) {
      spawnRainbowSparkles(static_cast<float>(rand() % 320), 0.0f, 30.0f, 4);
    }

    // Sieg-Fenster
    e.rectfill(30, 22, 260, 195, Colors::Black);
    e.rect(30, 22, 260, 195, Colors::Gold);
    e.rect(32, 24, 256, 191, Colors::DarkPurple);

    e.draw_text(68, 32, "YOU WIN! SIEG!", Colors::Gold, 2);
    e.draw_text(42, 58, "EINHORN-CHAMPION DER GALAXIE!", Colors::Yellow, 1);

    e.draw_text(46, 76, "LEVEL 1 (WIESE)  : GESCHAFFT!", Colors::LightGreen, 1);
    e.draw_text(46, 90, "LEVEL 2 (NACHT)  : GESCHAFFT!", Colors::Cyan, 1);
    e.draw_text(46, 106, "SIEG-BONUS       : +2000 PUNKTE", Colors::Gold, 1);
    e.draw_text(46, 120, "GESAMT-SCORE     : " + std::to_string(score), Colors::White, 1);
    e.draw_text(46, 134, "HIGHSCORE        : " + std::to_string(highscore), Colors::Gold, 1);
    e.draw_text(46, 148, "REGENBOGEN       : " + std::to_string(rainbowsCollected), Colors::Cyan, 1);
    e.draw_text(46, 162, "STERNE STOMP     : " + std::to_string(meteorsStomped), Colors::Pink, 1);

    // Feierndes tanzendes Einhorn
    float danceY = 176.0f - std::abs(std::sin(animTimer * 6.0f)) * 14.0f;
    drawUnicorn(e, 246, danceY, chosenColor, false, animTimer);

    bool blink = (static_cast<int>(animTimer * 2.5f) % 2) == 0;
    if (blink) {
      e.draw_text(48, 184, "LEERTASTE: NOCHMAL SPIELEN", Colors::Yellow, 1);
    }
    e.draw_text(52, 198, "ESCAPE   : HAUPTMENUE", Colors::LightGray, 1);

    if (e.pressed(Key::Num1)) {
      chosenColor = UnicornColor::Blue;
      e.play_tone(Notes::C5, 0.08f);
    } else if (e.pressed(Key::Num2)) {
      chosenColor = UnicornColor::White;
      e.play_tone(Notes::E5, 0.08f);
    } else if (e.pressed(Key::Num3)) {
      chosenColor = UnicornColor::Pink;
      e.play_tone(Notes::G5, 0.08f);
    }

    if (e.pressed(Key::Space) || e.pressed(Key::Enter)) {
      playStartFanfare(e);
      resetGame();
      e.play_bgm(bgmSamples, 44100, true);
      isBgmPlaying = true;
      state = State::Playing;
    }
  }

  // ===========================================================================
  // 16. UI-ANZEIGE (WÄHREND DES SPIELENS)
  // ===========================================================================
  void drawPlayingUI(Engine &e) {
    e.rectfill(0, 0, 320, 16, Colors::DarkPurple);
    e.line(0, 16, 320, 16, Colors::Purple);

    // Score
    e.draw_text(6, 4, "SCORE:" + std::to_string(score), Colors::Gold, 1);

    // Level-Anzeige
    if (currentLevel == 1) {
      e.draw_text(86, 4, "LV1:WIESE", Colors::LightGreen, 1);
    } else {
      e.draw_text(86, 4, "LV2:NACHT", Colors::Cyan, 1);
    }

    // Mini Level-Fortschrittsbalken (20 Sekunden Ziel)
    const int barW = 32;
    int progressW = std::min(barW, static_cast<int>((levelTimer / levelDuration) * barW));
    e.rect(144, 4, barW, 8, Colors::LightGray);
    e.rectfill(145, 5, progressW, 6, currentLevel == 1 ? Colors::LightGreen : Colors::Cyan);

    // Dynamische Tempo-Anzeige (z.B. 1.2x)
    int speedInt = static_cast<int>((currentScrollSpeed / baseScrollSpeed) * 10.0f);
    std::string speedStr = std::to_string(speedInt / 10) + "." + std::to_string(speedInt % 10) + "X";
    e.draw_text(182, 4, speedStr, Colors::Yellow, 1);

    // Regenbogen & Stomps
    e.draw_text(216, 4, "R:" + std::to_string(rainbowsCollected), Colors::Cyan, 1);
    e.draw_text(268, 4, "S:" + std::to_string(meteorsStomped), Colors::Pink, 1);

    // Level-Übergangs- / Start-Banner
    if (levelBannerTimer > 0.0f) {
      if (currentLevel == 1) {
        e.rectfill(45, 65, 230, 42, Colors::Black);
        e.rect(45, 65, 230, 42, Colors::LightGreen);
        e.draw_text(68, 72, "LEVEL 1: BLUMENWIESE!", Colors::Yellow, 1);
        e.draw_text(56, 88, "UEBERLEBE 20 SEKUNDEN!", Colors::White, 1);
      } else if (currentLevel == 2) {
        e.rectfill(45, 65, 230, 42, Colors::Black);
        e.rect(45, 65, 230, 42, Colors::Cyan);
        e.draw_text(64, 72, "LEVEL 2: NACHTHIMMEL!", Colors::Cyan, 1);
        e.draw_text(58, 88, "FINALER 20s HIGH-SPEED!", Colors::Gold, 1);
      }
    }
  }
};
