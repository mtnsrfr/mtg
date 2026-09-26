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
// - Kollisionen:
//   * Echtes Auto-zu-Auto Rammen & Abprallen mit Funken und Trägheitsübertrag!
//   * Reifenstapel, Leitplanken & Streckenbarrieren mit Aufprall-Physik!
// - Sound & Musik:
//   * Prozedural generierte rockige E-Gitarren BGM mit Drums, Bass & Powerchords!
//   * Dynamischer Motor-Sound (Drehzahl & Nitro-Pfeifen) in Echtzeit!
// - 3 Runden bis zum Champion-Sieg!
// =============================================================================

// Prozeduraler Rock-Soundtrack Generator (Gitarren-Powerchords, Bass, Drums & Lead-Riffs)
inline std::vector<int16_t> generateRockBgm() {
  const unsigned sampleRate = 44100;
  const float tempoBpm = 142.0f;
  const float beatDuration = 60.0f / tempoBpm; // ~0.4225 s
  const float barDuration = beatDuration * 4.0f; // ~1.690 s
  const int numBars = 8;
  const float totalDuration = barDuration * numBars; // ~13.52 s
  const unsigned totalSamples = static_cast<unsigned>(sampleRate * totalDuration);

  std::vector<int16_t> buffer(totalSamples, 0);
  const double twoPi = 2.0 * M_PI;

  // Powerchord Harmonien: E5 (Bar 0-1), G5 (Bar 2-3), A5 (Bar 4-5), C5 (Bar 6), D5 (Bar 7)
  struct PowerChord {
    double root;
    double fifth;
    double octave;
  };
  PowerChord chordList[8] = {
    {Notes::E3, Notes::B3, Notes::E4}, // Bar 0: E5
    {Notes::E3, Notes::B3, Notes::E4}, // Bar 1: E5
    {Notes::G3, Notes::D4, Notes::G4}, // Bar 2: G5
    {Notes::G3, Notes::D4, Notes::G4}, // Bar 3: G5
    {Notes::A3, Notes::E4, Notes::A4}, // Bar 4: A5
    {Notes::A3, Notes::E4, Notes::A4}, // Bar 5: A5
    {Notes::C4, Notes::G4, Notes::C5}, // Bar 6: C5
    {Notes::D4, Notes::A4, Notes::D5}  // Bar 7: D5
  };

  // Lead-Gitarren Melodie (64 Achtelnoten)
  double leadMelody[64] = {
    // Bars 0-1: E-Moll Rock Riff
    Notes::E5, Notes::E5, Notes::G5, Notes::A5,
    Notes::B5, Notes::A5, Notes::G5, Notes::E5,
    Notes::D5, Notes::E5, Notes::G5, Notes::E5,
    Notes::A5, Notes::G5, Notes::E5, Notes::D5,
    // Bars 2-3: G-Dur Blast
    Notes::G5, Notes::B5, Notes::D6, Notes::B5,
    Notes::A5, Notes::G5, Notes::E5, Notes::G5,
    Notes::B5, Notes::D6, Notes::E6, Notes::D6,
    Notes::B5, Notes::A5, Notes::G5, Notes::A5,
    // Bars 4-5: A-Power High Rise
    Notes::A5, Notes::C6, Notes::E6, Notes::C6,
    Notes::B5, Notes::A5, Notes::G5, Notes::A5,
    Notes::C6, Notes::D6, Notes::E6, Notes::G6,
    Notes::E6, Notes::D6, Notes::C6, Notes::A5,
    // Bars 6-7: Climax & Turnaround
    Notes::C6, Notes::E6, Notes::G6, Notes::E6,
    Notes::D6, Notes::Fs6, Notes::A6, Notes::Fs6,
    Notes::E6, Notes::B5, Notes::G5, Notes::Fs5,
    Notes::E5, Notes::G5, Notes::B5, Notes::D6
  };

  // Rausch-Tabelle für Becken, Snare & Gitarren-Dreck
  std::vector<float> noise(sampleRate, 0.0f);
  uint32_t rng = 123456789;
  for (size_t i = 0; i < noise.size(); ++i) {
    rng = rng * 1664525u + 1013904223u;
    noise[i] = (static_cast<float>(rng) / 4294967295.0f) * 2.0f - 1.0f;
  }

  for (unsigned i = 0; i < totalSamples; ++i) {
    double t = static_cast<double>(i) / sampleRate;
    int currentBar = static_cast<int>(t / barDuration) % numBars;
    float barTime = std::fmod(t, barDuration);
    int currentBeat = static_cast<int>(barTime / beatDuration);
    float beatTime = std::fmod(barTime, beatDuration);

    double totalMix = 0.0;

    // --- 1. DRUMS ---
    // Bass Drum (Beats 0, 2 + 16tel Akzent)
    bool isKick = (currentBeat == 0 && beatTime < 0.17f) || 
                  (currentBeat == 2 && beatTime < 0.17f) ||
                  (currentBeat == 1 && beatTime > beatDuration * 0.75f && (beatTime - beatDuration * 0.75f) < 0.13f);
    if (isKick) {
      float kTime = (currentBeat == 1) ? (beatTime - beatDuration * 0.75f) : beatTime;
      float kEnv = std::max(0.0f, 1.0f - kTime / 0.17f);
      double kFreq = 145.0 * std::exp(-kTime * 24.0) + 42.0;
      double kickWave = std::sin(twoPi * kFreq * kTime);
      totalMix += 0.36 * kickWave * kEnv;
    }

    // Snare Drum (Beats 1 und 3)
    if ((currentBeat == 1 || currentBeat == 3) && beatTime < 0.20f) {
      float sEnv = std::max(0.0f, 1.0f - beatTime / 0.20f);
      double sTone = std::sin(twoPi * (210.0 - beatTime * 450.0) * beatTime);
      float sNoise = noise[i % noise.size()];
      double snareWave = 0.4 * sTone + 0.6 * sNoise;
      totalMix += 0.30 * snareWave * sEnv;
    }

    // Hi-Hat (Achtel-Noten)
    float eighthTime = std::fmod(beatTime, beatDuration * 0.5f);
    if (eighthTime < 0.045f) {
      float hEnv = std::max(0.0f, 1.0f - eighthTime / 0.045f);
      float hNoise = noise[(i * 3) % noise.size()];
      totalMix += 0.13 * hNoise * hEnv;
    }

    // Crash Cymbal (Bar 0 & 4)
    if ((currentBar == 0 || currentBar == 4) && barTime < 1.0f) {
      float cEnv = std::max(0.0f, 1.0f - barTime / 1.0f);
      float cNoise = noise[(i * 7) % noise.size()];
      totalMix += 0.16 * cNoise * cEnv * cEnv;
    }

    // --- 2. BASS GUITAR ---
    const PowerChord &ch = chordList[currentBar];
    double bassFreq = ch.root * 0.5;
    float sixteenthTime = std::fmod(beatTime, beatDuration * 0.25f);
    float bEnv = std::max(0.0f, 1.0f - sixteenthTime / (beatDuration * 0.24f));
    double bassWave = std::sin(twoPi * bassFreq * t) + 0.5 * std::sin(twoPi * bassFreq * 2.0 * t);
    totalMix += 0.22 * std::tanh(bassWave * 1.6) * bEnv;

    // --- 3. DISTORTED POWER CHORDS (Rhythmus-Gitarre) ---
    double chordRaw = std::sin(twoPi * ch.root * t) + 
                      0.85 * std::sin(twoPi * ch.fifth * t) + 
                      0.65 * std::sin(twoPi * ch.octave * t);
    float gEnv = 0.65f + 0.35f * std::max(0.0f, 1.0f - sixteenthTime / (beatDuration * 0.22f));
    double gDist = std::tanh(chordRaw * 2.7);
    totalMix += 0.27 * gDist * gEnv;

    // --- 4. LEAD ROCK GUITAR ---
    int noteIdx = static_cast<int>(t / (beatDuration * 0.5f)) % 64;
    double leadFreq = leadMelody[noteIdx];
    float leadNoteTime = std::fmod(t, beatDuration * 0.5f);
    float lEnv = std::max(0.0f, 1.0f - leadNoteTime / (beatDuration * 0.48f));
    double vib = 1.0 + 0.012 * std::sin(twoPi * 6.0 * t);
    double lRaw = std::sin(twoPi * leadFreq * vib * t) + 0.4 * std::sin(twoPi * leadFreq * 2.0 * vib * t);
    double lDist = std::tanh(lRaw * 3.0);
    totalMix += 0.25 * lDist * lEnv;

    // Master-Limiter
    double master = std::tanh(totalMix * 1.25);
    buffer[i] = static_cast<int16_t>(24000.0 * master);
  }

  return buffer;
}

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

  struct Barrier {
    float x, y;
    float radius;
    uint8_t color1;
    uint8_t color2;
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
  std::vector<Barrier> barriers;

  float raceTimer = 0.0f;
  float countdownTimer = 3.5f;
  int winner = -1; // 0 = P1, 1 = P2
  const int TOTAL_LAPS = 3;
  bool bgmStarted = false;

  // Rennstrecken-Geometrie (Closed Loop Spline Waypoints)
  struct TrackPoint {
    float x, y;
  };

  static const int NUM_TRACK_NODES = 22;
  std::array<TrackPoint, NUM_TRACK_NODES> trackNodes;
  const float TRACK_HALF_WIDTH = 17.0f; // Streckenbreite = 34 Pixel

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
    initBarriers();
    resetRace();
  }

  void initTrack() {
    // 22 Wegpunkte für einen echten Corner-to-Corner GP Kurs:
    // Start-Ziel (Rechts) -> Scharfe 90-Grad Kurve Rechts-Oben -> Topspeed Gerade Oben ->
    // Scharfe 90-Grad Kurve Links-Oben -> Bergab-Passage Links -> Infield Haarnadel ->
    // Schnelle S-Schikane (The Snake) -> Scharfe 90-Grad Kurve Links-Unten ->
    // High-Speed Gerade Unten -> Scharfe 90-Grad Zielkurve Rechts-Unten!
    trackNodes[0]  = {284.0f, 175.0f}; // Start / Finish Line
    trackNodes[1]  = {284.0f, 110.0f}; // Hauptgerade Mitte
    trackNodes[2]  = {284.0f,  52.0f}; // Bremspunkt Gerade 1
    trackNodes[3]  = {276.0f,  30.0f}; // Scheitelpunkt K1 (Scharfe 90° Kurve Rechts-Oben)
    trackNodes[4]  = {245.0f,  24.0f}; // Ausgang K1
    trackNodes[5]  = {160.0f,  24.0f}; // High-Speed Gerade Oben
    trackNodes[6]  = { 75.0f,  24.0f}; // Bremspunkt K2
    trackNodes[7]  = { 34.0f,  28.0f}; // Scheitelpunkt K2 (Scharfe 90° Kurve Links-Oben)
    trackNodes[8]  = { 26.0f,  58.0f}; // Ausgang K2
    trackNodes[9]  = { 26.0f, 115.0f}; // Gerade Links
    trackNodes[10] = { 28.0f, 155.0f}; // Einlenkpunkt Infield-Haarnadel
    trackNodes[11] = { 65.0f, 168.0f}; // Scheitelpunkt Haarnadel K3
    trackNodes[12] = {100.0f, 138.0f}; // Ausgang Haarnadel
    trackNodes[13] = {132.0f, 102.0f}; // S-Schikane Scheitel 1 (Rechts)
    trackNodes[14] = {172.0f, 126.0f}; // S-Schikane Scheitel 2 (Links)
    trackNodes[15] = {150.0f, 175.0f}; // Ausgang Schikane
    trackNodes[16] = {105.0f, 205.0f}; // Übergang zur Südkurve
    trackNodes[17] = { 46.0f, 218.0f}; // Scheitelpunkt K4 (Scharfe Kurve Links-Unten)
    trackNodes[18] = {100.0f, 222.0f}; // Eingang Südgerade
    trackNodes[19] = {185.0f, 222.0f}; // High-Speed Südgerade
    trackNodes[20] = {250.0f, 222.0f}; // Bremspunkt Zielkurve
    trackNodes[21] = {278.0f, 212.0f}; // Scheitelpunkt K5 (Scharfe 90° Zielkurve)

    // 8 Checkpoints an Schlüsselstellen
    checkpoints[0] = {284.0f, 175.0f, 28.0f}; // Start / Ziel Tor
    checkpoints[1] = {284.0f,  75.0f, 28.0f}; // Hauptgerade
    checkpoints[2] = {160.0f,  24.0f, 28.0f}; // Gerade Oben
    checkpoints[3] = { 26.0f,  85.0f, 28.0f}; // Gerade Links
    checkpoints[4] = { 65.0f, 168.0f, 28.0f}; // Infield Haarnadel
    checkpoints[5] = {152.0f, 114.0f, 28.0f}; // S-Schikane
    checkpoints[6] = { 46.0f, 218.0f, 28.0f}; // Kurve Links-Unten
    checkpoints[7] = {185.0f, 222.0f, 28.0f}; // Südgerade
  }

  void initBarriers() {
    barriers.clear();
    // Reifenstapel & Hindernisse an den Kurvenscheiteln (verhindern unbefugtes Abkürzen!)
    barriers.push_back({256.0f,  48.0f, 9.0f, Colors::Red, Colors::White}); // Innen Kurve 1
    barriers.push_back({308.0f,  14.0f, 9.0f, Colors::Red, Colors::White}); // Außen Kurve 1
    barriers.push_back({ 54.0f,  52.0f, 9.0f, Colors::Red, Colors::White}); // Innen Kurve 2
    barriers.push_back({ 12.0f,  14.0f, 9.0f, Colors::Red, Colors::White}); // Außen Kurve 2
    barriers.push_back({ 50.0f, 134.0f, 8.0f, Colors::Red, Colors::White}); // Innen Haarnadel K3
    barriers.push_back({152.0f,  76.0f, 8.0f, Colors::Yellow, Colors::DarkGray}); // Schikane Scheitel 1
    barriers.push_back({148.0f, 148.0f, 8.0f, Colors::Yellow, Colors::DarkGray}); // Schikane Scheitel 2
    barriers.push_back({ 72.0f, 198.0f, 9.0f, Colors::Red, Colors::White}); // Innen Kurve 4
    barriers.push_back({ 14.0f, 228.0f, 9.0f, Colors::Red, Colors::White}); // Außen Kurve 4
    barriers.push_back({256.0f, 196.0f, 8.0f, Colors::Red, Colors::White}); // Innen Zielkurve
    barriers.push_back({308.0f, 228.0f, 9.0f, Colors::Red, Colors::White}); // Außen Zielkurve
    barriers.push_back({210.0f,  75.0f, 13.0f, Colors::DarkPurple, Colors::Cyan}); // Infield Monument
  }

  void resetRace() {
    state = State::Countdown;
    countdownTimer = 3.5f;
    raceTimer = 0.0f;
    winner = -1;
    bgmStarted = false;

    // Spieler 1 (Rot / Gold): Startplatz Links
    cars[0].id = 0;
    cars[0].x = 277.0f;
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
    cars[1].x = 291.0f;
    cars[1].y = 196.0f;
    cars[1].vx = 0.0f;
    cars[1].vy = 0.0f;
    cars[1].angle = -3.14159265f * 0.5f; // Schaut nach oben (-90 Grad)
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

    // Start Rock BGM beim ersten Frame
    if (!bgmStarted) {
      static std::vector<int16_t> bgmSamples = generateRockBgm();
      e.play_bgm(bgmSamples, 44100, true);
      bgmStarted = true;
    }

    // Neustart-Taste R
    if (e.pressed(Key::R)) {
      resetRace();
      e.play_tone(Notes::A5, 0.1f);
    }

    // Szene aktualisieren
    updateCountdown(e, dt);
    updateCars(e, dt);
    updateRockets(e, dt);
    updateParticles(dt);
    updateSkidMarks(dt);

    // Motor-Audio in Echtzeit steuern
    updateEngineSound(e);

    // Zeichnen
    render(e);
  }

  // ===========================================================================
  // 5. MOTOR-SOUND SYSTEM (Echtzeit Pitch & Lautstärke)
  // ===========================================================================
  void updateEngineSound(Engine &e) {
    if (state == State::Finished) {
      e.stop_engine_sound();
      return;
    }

    // Maximale Drehzahl beider Fahrzeuge ermitteln
    float maxSpd = std::max(std::abs(cars[0].speed), std::abs(cars[1].speed));
    bool anyBoost = cars[0].boostActive || cars[1].boostActive;

    float normSpeed = std::clamp(maxSpd / 140.0f, 0.0f, 1.0f);
    float pitch = 0.65f + normSpeed * 1.15f;
    float volume = 0.35f + normSpeed * 0.45f;

    if (anyBoost) {
      pitch = 2.40f; // Roaring Turbo Pitch!
      volume = 0.95f;
    }

    e.set_engine_sound(pitch, volume);
  }

  // ===========================================================================
  // 6. COUNTDOWN & START-AMPEL
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
  // 7. FAHRZEUG-PHYSIK & STEUERUNG
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
      float topSpeed = 140.0f; // Normaler Topspeed auf Asphalt
      float accelRate = 125.0f;
      float frictionRate = 50.0f;

      if (c.boostActive) {
        topSpeed = 228.0f; // 🚀 MEGA BOOST (+63% Speed!)
        accelRate = 270.0f;
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
        float steerSpeed = 3.8f;
        if (c.boostActive) steerSpeed = 2.9f;
        float dir = (c.speed >= 0.0f) ? 1.0f : -1.0f;

        if (steerLeft) {
          c.angle -= steerSpeed * dt * dir;
          if (std::abs(c.speed) > 85.0f && !c.onGrass) {
            spawnSkidMark(c.x, c.y);
          }
        }
        if (steerRight) {
          c.angle += steerSpeed * dt * dir;
          if (std::abs(c.speed) > 85.0f && !c.onGrass) {
            spawnSkidMark(c.x, c.y);
          }
        }
      }

      // 5. Positions-Aktualisierung
      c.vx = std::cos(c.angle) * c.speed;
      c.vy = std::sin(c.angle) * c.speed;
      c.x += c.vx * dt;
      c.y += c.vy * dt;

      // Leitplanken- & Streckenrand-Kollision (Screen Border Bounce)
      checkWallCollisions(c, e);

      // Checkpoint & Runden-Prüfung
      checkCheckpoint(c, e);
    }

    // 8. Hindernis- & Reifenstapel-Kollision
    checkBarrierCollisions(e);

    // 9. Auto-Auto Kollision (Echtes Physik-Rammen & Abprallen)
    checkCarCollision(e);
  }

  // ===========================================================================
  // 8. LEITPLANKEN- & WAND-KOLLISIONEN
  // ===========================================================================
  void checkWallCollisions(Car &c, Engine &e) {
    const float minX = 10.0f;
    const float maxX = 310.0f;
    const float minY = 10.0f;
    const float maxY = 230.0f;

    bool hit = false;
    if (c.x < minX) {
      c.x = minX;
      c.speed *= -0.4f;
      c.angle = 3.14159f - c.angle;
      hit = true;
    } else if (c.x > maxX) {
      c.x = maxX;
      c.speed *= -0.4f;
      c.angle = 3.14159f - c.angle;
      hit = true;
    }

    if (c.y < minY) {
      c.y = minY;
      c.speed *= -0.4f;
      c.angle = -c.angle;
      hit = true;
    } else if (c.y > maxY) {
      c.y = maxY;
      c.speed *= -0.4f;
      c.angle = -c.angle;
      hit = true;
    }

    if (hit) {
      spawnCrashSparks(c.x, c.y);
      e.play_tone(Notes::D2, 0.06f);
    }
  }

  // ===========================================================================
  // 9. REIFENSTAPEL- & HINDERNIS-KOLLISIONEN
  // ===========================================================================
  void checkBarrierCollisions(Engine &e) {
    for (int i = 0; i < 2; ++i) {
      Car &c = cars[i];
      for (const auto &b : barriers) {
        float dx = c.x - b.x;
        float dy = c.y - b.y;
        float distSq = dx * dx + dy * dy;
        float minDist = b.radius + 6.0f;

        if (distSq < minDist * minDist && distSq > 0.001f) {
          float dist = std::sqrt(distSq);
          float nx = dx / dist;
          float ny = dy / dist;

          // Aus dem Hindernis herausschieben
          c.x = b.x + nx * minDist;
          c.y = b.y + ny * minDist;

          // Geschwindigkeit dämpfen und abprallen
          c.speed = -c.speed * 0.45f;
          c.angle += nx * 0.4f;

          // Funken & Reifensplitter
          spawnCrashSparks(c.x, c.y);
          e.play_tone(Notes::E2, 0.06f);
        }
      }
    }
  }

  // ===========================================================================
  // 10. CHECKPOINT- & RUNDEN-SYSTEM
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
  // 11. AUTO-ZU-AUTO KOLLISION (Echtes Physik-Rammen & Abprallen)
  // ===========================================================================
  void checkCarCollision(Engine &e) {
    float dx = cars[1].x - cars[0].x;
    float dy = cars[1].y - cars[0].y;
    float distSq = dx * dx + dy * dy;
    float minDist = 13.0f; // Kollisionsdurchmesser

    if (distSq < minDist * minDist && distSq > 0.001f) {
      float dist = std::sqrt(distSq);
      float nx = dx / dist;
      float ny = dy / dist;

      // 1. Positions-Trennung (Kein Ineinanderfahren!)
      float overlap = (minDist - dist) * 0.52f;
      cars[0].x -= nx * overlap;
      cars[0].y -= ny * overlap;
      cars[1].x += nx * overlap;
      cars[1].y += ny * overlap;

      // 2. Elastischer Impulsaustausch (Geschwindigkeit & Drehmoment)
      float v1x = std::cos(cars[0].angle) * cars[0].speed;
      float v1y = std::sin(cars[0].angle) * cars[0].speed;
      float v2x = std::cos(cars[1].angle) * cars[1].speed;
      float v2y = std::sin(cars[1].angle) * cars[1].speed;

      float relVx = v1x - v2x;
      float relVy = v1y - v2y;
      float impulse = (relVx * nx + relVy * ny);

      if (impulse > 0.0f) {
        float bounce = 0.85f; // Starker Arcade-Crash Bounce
        float j = (1.0f + bounce) * impulse * 0.5f;

        v1x -= j * nx;
        v1y -= j * ny;
        v2x += j * nx;
        v2y += j * ny;

        cars[0].speed = std::clamp(std::sqrt(v1x * v1x + v1y * v1y), 0.0f, 220.0f);
        cars[1].speed = std::clamp(std::sqrt(v2x * v2x + v2y * v2y), 0.0f, 220.0f);

        // Drehmoment-Schock
        cars[0].angle -= ny * 0.35f;
        cars[1].angle += ny * 0.35f;
      }

      // Karambolage-Funken & Wuchtiger Crash-Sound
      spawnCrashSparks((cars[0].x + cars[1].x) * 0.5f, (cars[0].y + cars[1].y) * 0.5f);
      e.play_tone(Notes::C2, 0.08f);
    }
  }

  // ===========================================================================
  // 12. ZIELSUCHENDE RAKETEN (Homing Missiles)
  // ===========================================================================
  void fireRocket(int shooterId, Engine &e) {
    const Car &shooter = cars[shooterId];

    Rocket r;
    r.owner = shooterId;
    r.x = shooter.x + std::cos(shooter.angle) * 11.0f;
    r.y = shooter.y + std::sin(shooter.angle) * 11.0f;
    r.angle = shooter.angle;
    r.vx = std::cos(shooter.angle) * 190.0f;
    r.vy = std::sin(shooter.angle) * 190.0f;
    r.life = 3.0f; // Verfolgt den Gegner für max 3.0 Sekunden
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
        float turnSpeed = 4.4f * dt;
        r.angle += std::clamp(angleDiff, -turnSpeed, turnSpeed);

        float rocketSpeed = 200.0f;
        r.vx = std::cos(r.angle) * rocketSpeed;
        r.vy = std::sin(r.angle) * rocketSpeed;
      }

      r.x += r.vx * dt;
      r.y += r.vy * dt;

      // Rauchschweif hinter der Rakete
      spawnRocketTrail(r.x, r.y, r.vx, r.vy);

      // Barrieren-Treffer
      bool hitBarrier = false;
      for (const auto &b : barriers) {
        float bdx = r.x - b.x;
        float bdy = r.y - b.y;
        if (bdx * bdx + bdy * bdy < (b.radius + 3.0f) * (b.radius + 3.0f)) {
          hitBarrier = true;
          break;
        }
      }

      if (hitBarrier) {
        r.active = false;
        spawnExplosion(r.x, r.y);
        playExplosionSound(e);
        rockets[i] = rockets.back();
        rockets.pop_back();
        continue;
      }

      // Treffer-Prüfung gegen das Ziel-Auto
      if (dist < 11.0f) {
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
  // 13. PARTIKEL- & EFFEKT-SYSTEME
  // ===========================================================================
  void spawnBoostFlames(float cx, float cy, float angle) {
    float backX = cx - std::cos(angle) * 7.0f;
    float backY = cy - std::sin(angle) * 7.0f;

    for (int k = 0; k < 3; ++k) {
      float spread = ((rand() % 100) / 100.0f - 0.5f) * 0.6f;
      float pAngle = angle + 3.14159f + spread;
      float pSpeed = 60.0f + (rand() % 80);

      Particle p;
      p.x = backX;
      p.y = backY;
      p.vx = std::cos(pAngle) * pSpeed;
      p.vy = std::sin(pAngle) * pSpeed;
      p.life = 0.25f + ((rand() % 100) / 100.0f) * 0.15f;
      p.maxLife = p.life;
      p.color = (k == 0) ? Colors::White : ((k == 1) ? Colors::Yellow : Colors::Red);
      p.size = 2.0f;
      particles.push_back(p);
    }
  }

  void spawnCrashSparks(float x, float y) {
    for (int k = 0; k < 12; ++k) {
      float pAngle = ((rand() % 360) * 3.14159f) / 180.0f;
      float pSpeed = 40.0f + (rand() % 90);

      Particle p;
      p.x = x;
      p.y = y;
      p.vx = std::cos(pAngle) * pSpeed;
      p.vy = std::sin(pAngle) * pSpeed;
      p.life = 0.20f + ((rand() % 100) / 100.0f) * 0.25f;
      p.maxLife = p.life;
      p.color = (k % 2 == 0) ? Colors::Gold : Colors::White;
      p.size = 1.5f;
      particles.push_back(p);
    }
  }

  void spawnSlowSparks(float x, float y) {
    for (int k = 0; k < 2; ++k) {
      float pAngle = ((rand() % 360) * 3.14159f) / 180.0f;
      Particle p;
      p.x = x + (rand() % 7 - 3);
      p.y = y + (rand() % 7 - 3);
      p.vx = std::cos(pAngle) * 35.0f;
      p.vy = std::sin(pAngle) * 35.0f;
      p.life = 0.25f;
      p.maxLife = 0.25f;
      p.color = (rand() % 2 == 0) ? Colors::Cyan : Colors::LightGray;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void spawnGrassDirt(float x, float y) {
    Particle p;
    p.x = x + (rand() % 5 - 2);
    p.y = y + (rand() % 5 - 2);
    p.vx = (rand() % 20 - 10);
    p.vy = (rand() % 20 - 10);
    p.life = 0.35f;
    p.maxLife = 0.35f;
    p.color = Colors::DarkGreen;
    p.size = 1.5f;
    particles.push_back(p);
  }

  void spawnRocketTrail(float rx, float ry, float rvx, float rvy) {
    Particle p;
    p.x = rx - rvx * 0.03f;
    p.y = ry - rvy * 0.03f;
    p.vx = -rvx * 0.1f + (rand() % 20 - 10);
    p.vy = -rvy * 0.1f + (rand() % 20 - 10);
    p.life = 0.28f;
    p.maxLife = 0.28f;
    p.color = (rand() % 2 == 0) ? Colors::LightGray : Colors::White;
    p.size = 1.5f;
    particles.push_back(p);
  }

  void spawnExplosion(float x, float y) {
    for (int k = 0; k < 20; ++k) {
      float pAngle = ((rand() % 360) * 3.14159f) / 180.0f;
      float pSpeed = 30.0f + (rand() % 100);

      Particle p;
      p.x = x;
      p.y = y;
      p.vx = std::cos(pAngle) * pSpeed;
      p.vy = std::sin(pAngle) * pSpeed;
      p.life = 0.35f + ((rand() % 100) / 100.0f) * 0.3f;
      p.maxLife = p.life;
      p.color = (k % 3 == 0) ? Colors::White : ((k % 3 == 1) ? Colors::Yellow : Colors::Red);
      p.size = 2.0f;
      particles.push_back(p);
    }
  }

  void spawnSkidMark(float x, float y) {
    if (skidMarks.size() > 200) {
      skidMarks.erase(skidMarks.begin());
    }
    skidMarks.push_back({x, y, 4.0f});
  }

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
        p.vx *= 0.94f;
        p.vy *= 0.94f;
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

  // ===========================================================================
  // 14. SOUND EFFEKTE
  // ===========================================================================
  void playBoostSound(Engine &e) {
    e.play_tone(Notes::A5, 0.12f);
  }

  void playRocketLaunchSound(Engine &e) {
    e.play_tone(Notes::F5, 0.08f);
  }

  void playExplosionSound(Engine &e) {
    e.play_tone(Notes::Cs2, 0.20f);
  }

  void playLapSound(Engine &e) {
    e.play_tone(Notes::E5, 0.08f);
  }

  void playVictoryFanfare(Engine &e) {
    e.play_tone(Notes::C5, 0.12f);
  }

  // ===========================================================================
  // 15. RENDERING & GRAFIK
  // ===========================================================================
  void render(Engine &e) {
    // 1. Hintergrund (Grüne Rasenfläche)
    e.cls(Colors::DarkGreen);

    // Rasen-Textur-Muster
    for (int y = 0; y < 240; y += 12) {
      for (int x = 0; x < 320; x += 12) {
        if (((x / 12) + (y / 12)) % 2 == 0) {
          e.rectfill(x, y, 12, 12, Colors::LightGreen);
        }
      }
    }

    // 2. Asphalt-Strecke & Curbs zeichnen
    renderTrack(e);

    // 3. Reifenspuren
    for (const auto &sm : skidMarks) {
      e.rectfill(sm.x - 1, sm.y - 1, 2, 2, Colors::Black);
    }

    // 4. Start/Ziel Schachbrett-Linie
    renderStartFinish(e);

    // 5. Hindernisse & Reifenstapel
    renderBarriers(e);

    // 6. Raketen zeichnen
    for (const auto &r : rockets) {
      renderRocket(e, r);
    }

    // 7. Rennwagen zeichnen
    for (int i = 0; i < 2; ++i) {
      renderCar(e, cars[i]);
    }

    // 8. Partikel zeichnen
    for (const auto &p : particles) {
      float lifeRatio = p.life / p.maxLife;
      uint8_t c = p.color;
      if (lifeRatio < 0.4f) c = Colors::DarkGray;
      e.rectfill(p.x - p.size * 0.5f, p.y - p.size * 0.5f, p.size, p.size, c);
    }

    // 9. HUD (Tacho, Runden & Cooldowns)
    renderHUD(e);

    // 10. Countdown & Sieger-Banner
    if (state == State::Countdown) {
      renderCountdownBanner(e);
    } else if (state == State::Finished) {
      renderVictoryBanner(e);
    }
  }

  // Zeichnet die durchgehende Asphaltstrecke mit Randsteinen
  void renderTrack(Engine &e) {
    const int STEPS_PER_SEGMENT = 18;

    for (int i = 0; i < NUM_TRACK_NODES; ++i) {
      int next = (i + 1) % NUM_TRACK_NODES;
      float x1 = trackNodes[i].x;
      float y1 = trackNodes[i].y;
      float x2 = trackNodes[next].x;
      float y2 = trackNodes[next].y;

      for (int s = 0; s <= STEPS_PER_SEGMENT; ++s) {
        float t = static_cast<float>(s) / STEPS_PER_SEGMENT;
        float cx = x1 + t * (x2 - x1);
        float cy = y1 + t * (y2 - y1);

        // Randsteine (Rot/Weiß gestreift)
        uint8_t curbColor = ((i * STEPS_PER_SEGMENT + s) / 3 % 2 == 0) ? Colors::Red : Colors::White;
        e.circfill(cx, cy, TRACK_HALF_WIDTH + 3.0f, curbColor);

        // Asphalt-Fahrbahn (Dunkelgrau)
        e.circfill(cx, cy, TRACK_HALF_WIDTH, Colors::DarkGray);

        // Fahrbahn-Textur (Leichte Asphalt-Linien)
        if (s % 4 == 0) {
          e.circfill(cx, cy, TRACK_HALF_WIDTH - 2.0f, Colors::LightGray);
          e.circfill(cx, cy, TRACK_HALF_WIDTH - 3.5f, Colors::DarkGray);
        }
      }
    }
  }

  void renderStartFinish(Engine &e) {
    float x = 284.0f;
    float y = 175.0f;
    float halfW = TRACK_HALF_WIDTH;

    // Start-Ziel Checkerboard Linie quer über die Fahrbahn
    for (float dx = -halfW + 1; dx < halfW - 1; dx += 4.0f) {
      for (int dy = -3; dy <= 3; dy += 3) {
        bool black = ((int(dx) / 4 + dy / 3) % 2 == 0);
        e.rectfill(x + dx, y + dy, 4, 3, black ? Colors::Black : Colors::White);
      }
    }
    e.line(x - halfW, y - 4, x + halfW, y - 4, Colors::Yellow);
    e.line(x - halfW, y + 4, x + halfW, y + 4, Colors::Yellow);
  }

  void renderBarriers(Engine &e) {
    for (const auto &b : barriers) {
      // Äußere Reifenwulst
      e.circfill(b.x, b.y, b.radius, Colors::Black);
      e.circfill(b.x, b.y, b.radius - 2.0f, b.color1);
      e.circfill(b.x, b.y, b.radius - 4.5f, b.color2);
      e.circfill(b.x, b.y, 2.0f, Colors::Black);
    }
  }

  void renderCar(Engine &e, const Car &c) {
    // 12x7 Pixel Sportwagen mit Cockpit, Spoiler & Breitreifen
    float cosA = std::cos(c.angle);
    float sinA = std::sin(c.angle);

    // Körperfarbe & Akzent
    uint8_t bodyColor = (c.id == 0) ? Colors::Red : Colors::Blue;
    uint8_t trimColor = (c.id == 0) ? Colors::Gold : Colors::Cyan;
    if (c.boostActive) {
      trimColor = Colors::White; // Glühen im Boost
    }

    // Auto-Mittelpunkt
    float cx = c.x;
    float cy = c.y;

    auto transform = [&](float lx, float ly) -> std::pair<float, float> {
      return {cx + lx * cosA - ly * sinA, cy + lx * sinA + ly * cosA};
    };

    // 1. Vier Breitreifen (Schwarz)
    std::pair<float, float> wheels[4] = {
      transform( 4.0f, -4.5f), transform( 4.0f,  4.5f),
      transform(-4.0f, -4.5f), transform(-4.0f,  4.5f)
    };
    for (int w = 0; w < 4; ++w) {
      e.rectfill(wheels[w].first - 1.5f, wheels[w].second - 1.5f, 3.0f, 3.0f, Colors::Black);
    }

    // 2. Karosserie (Chassis)
    for (float lx = -5.0f; lx <= 5.0f; lx += 1.0f) {
      float w = (std::abs(lx) > 3.5f) ? 2.5f : 3.5f;
      for (float ly = -w; ly <= w; ly += 1.0f) {
        auto p = transform(lx, ly);
        e.pset(p.first, p.second, bodyColor);
      }
    }

    // 3. Rennstreifen / Zierleiste
    for (float lx = -4.0f; lx <= 4.0f; lx += 1.0f) {
      auto p = transform(lx, 0.0f);
      e.pset(p.first, p.second, trimColor);
    }

    // 4. Cockpit & Windschutzscheibe
    auto cp1 = transform(0.0f, -1.0f);
    auto cp2 = transform(0.0f,  1.0f);
    auto cp3 = transform(1.5f,  0.0f);
    e.pset(cp1.first, cp1.second, Colors::SkyBlue);
    e.pset(cp2.first, cp2.second, Colors::SkyBlue);
    e.pset(cp3.first, cp3.second, Colors::White);

    // 5. Heckspoiler
    auto spL = transform(-5.5f, -3.5f);
    auto spR = transform(-5.5f,  3.5f);
    e.line(spL.first, spL.second, spR.first, spR.second, trimColor);

    // 6. Scheinwerfer vorne
    auto hlL = transform(5.5f, -2.5f);
    auto hlR = transform(5.5f,  2.5f);
    e.pset(hlL.first, hlL.second, Colors::Yellow);
    e.pset(hlR.first, hlR.second, Colors::Yellow);

    // 7. Slowdown-Warnung über dem Auto bei Raketentreffer
    if (c.slowTimer > 0.0f) {
      e.draw_text(c.x - 12, c.y - 14, "SLOW!", Colors::Red, 1);
    }
  }

  void renderRocket(Engine &e, const Rocket &r) {
    float cosA = std::cos(r.angle);
    float sinA = std::sin(r.angle);

    float tipX = r.x + cosA * 4.0f;
    float tipY = r.y + sinA * 4.0f;
    float backX = r.x - cosA * 4.0f;
    float backY = r.y - sinA * 4.0f;

    e.line(backX, backY, tipX, tipY, Colors::White);
    e.pset(tipX, tipY, Colors::Red);
    e.pset(backX, backY, Colors::Yellow);
  }

  void renderHUD(Engine &e) {
    // --- SPIELER 1 HUD (Oben Links) ---
    e.rectfill(4, 4, 98, 22, Colors::DarkGray);
    e.rect(4, 4, 98, 22, Colors::Red);
    e.draw_text(6, 6, "P1: ROT", Colors::Red, 1);

    // Runde
    std::string lap1 = "RUNDE: " + std::to_string(std::min(cars[0].currentLap, TOTAL_LAPS)) + "/" + std::to_string(TOTAL_LAPS);
    e.draw_text(48, 6, lap1, Colors::White, 1);

    // Speedometer & Boost Bar
    std::string spd1 = std::to_string(static_cast<int>(std::abs(cars[0].speed))) + " KM/H";
    e.draw_text(6, 16, spd1, Colors::Gold, 1);

    // Boost Cooldown Balken
    if (cars[0].boostActive) {
      e.draw_text(52, 16, "NITRO!", Colors::Yellow, 1);
    } else if (cars[0].boostCooldown <= 0.0f) {
      e.draw_text(52, 16, "BOOST: OK", Colors::LightGreen, 1);
    } else {
      int cd = static_cast<int>(cars[0].boostCooldown + 0.99f);
      e.draw_text(52, 16, "CD: " + std::to_string(cd) + "S", Colors::LightGray, 1);
    }

    // --- SPIELER 2 HUD (Oben Rechts) ---
    e.rectfill(218, 4, 98, 22, Colors::DarkGray);
    e.rect(218, 4, 98, 22, Colors::Blue);
    e.draw_text(220, 6, "P2: BLAU", Colors::Cyan, 1);

    // Runde
    std::string lap2 = "RUNDE: " + std::to_string(std::min(cars[1].currentLap, TOTAL_LAPS)) + "/" + std::to_string(TOTAL_LAPS);
    e.draw_text(262, 6, lap2, Colors::White, 1);

    // Speedometer & Boost Bar
    std::string spd2 = std::to_string(static_cast<int>(std::abs(cars[1].speed))) + " KM/H";
    e.draw_text(220, 16, spd2, Colors::Cyan, 1);

    // Boost Cooldown Balken
    if (cars[1].boostActive) {
      e.draw_text(266, 16, "NITRO!", Colors::Yellow, 1);
    } else if (cars[1].boostCooldown <= 0.0f) {
      e.draw_text(266, 16, "BOOST: OK", Colors::LightGreen, 1);
    } else {
      int cd = static_cast<int>(cars[1].boostCooldown + 0.99f);
      e.draw_text(266, 16, "CD: " + std::to_string(cd) + "S", Colors::LightGray, 1);
    }

    // Mitte: Renndauer
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << raceTimer << "S";
    e.draw_text(145, 6, ss.str(), Colors::White, 1);
  }

  void renderCountdownBanner(Engine &e) {
    e.rectfill(90, 85, 140, 50, Colors::DarkGray);
    e.rect(90, 85, 140, 50, Colors::Yellow);

    if (countdownTimer > 3.0f) {
      e.draw_text(125, 95, "BEREIT?", Colors::White, 2);
    } else if (countdownTimer > 2.0f) {
      e.draw_text(152, 95, "3", Colors::Red, 3);
    } else if (countdownTimer > 1.0f) {
      e.draw_text(152, 95, "2", Colors::Yellow, 3);
    } else if (countdownTimer > 0.0f) {
      e.draw_text(152, 95, "1", Colors::LightGreen, 3);
    } else {
      e.draw_text(140, 95, "GO!", Colors::White, 3);
    }
    e.draw_text(100, 122, "DOPPEL-VOR = BOOST!", Colors::Gold, 1);
  }

  void renderVictoryBanner(Engine &e) {
    e.rectfill(60, 75, 200, 75, Colors::DarkGray);
    e.rect(60, 75, 200, 75, (winner == 0) ? Colors::Red : Colors::Blue);

    std::string winText = (winner == 0) ? "SPIELER 1 (ROT) GEWINNT!" : "SPIELER 2 (BLAU) GEWINNT!";
    uint8_t winColor = (winner == 0) ? Colors::Red : Colors::Cyan;
    e.draw_text(70, 85, winText, winColor, 1);

    e.draw_text(85, 100, "CHAMPION DES GRAND PRIX!", Colors::Gold, 1);

    std::stringstream ss;
    ss << "SIEGERZEIT: " << std::fixed << std::setprecision(2) << cars[winner].finishTime << " SEKUNDEN";
    e.draw_text(74, 115, ss.str(), Colors::White, 1);

    bool blink = (int(e.time() * 3.0f) % 2) == 0;
    if (blink) {
      e.draw_text(80, 134, "DRUECKE R FUER NEUSTART", Colors::Yellow, 1);
    }
  }
};
