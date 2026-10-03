#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <array>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// =============================================================================
// 1. TASTEN-DEFINITIONEN (Einfach zu tippen)
// =============================================================================
// Mit diesen Tasten kannst du deine Spiele steuern:
// z.B.: e.key(Key::W) oder e.pressed(Key::Space)
// =============================================================================
enum class Key {
  W = static_cast<int>(sf::Keyboard::Key::W),
  S = static_cast<int>(sf::Keyboard::Key::S),
  A = static_cast<int>(sf::Keyboard::Key::A),
  D = static_cast<int>(sf::Keyboard::Key::D),
  Q = static_cast<int>(sf::Keyboard::Key::Q),
  E = static_cast<int>(sf::Keyboard::Key::E),
  F = static_cast<int>(sf::Keyboard::Key::F),
  B = static_cast<int>(sf::Keyboard::Key::B),
  M = static_cast<int>(sf::Keyboard::Key::M),
  Tab = static_cast<int>(sf::Keyboard::Key::Tab),
  Up = static_cast<int>(sf::Keyboard::Key::Up),
  Down = static_cast<int>(sf::Keyboard::Key::Down),
  Left = static_cast<int>(sf::Keyboard::Key::Left),
  Right = static_cast<int>(sf::Keyboard::Key::Right),
  Space = static_cast<int>(sf::Keyboard::Key::Space),
  Enter = static_cast<int>(sf::Keyboard::Key::Enter),
  Escape = static_cast<int>(sf::Keyboard::Key::Escape),
  Num1 = static_cast<int>(sf::Keyboard::Key::Num1),
  Num2 = static_cast<int>(sf::Keyboard::Key::Num2),
  Num3 = static_cast<int>(sf::Keyboard::Key::Num3),
  Num4 = static_cast<int>(sf::Keyboard::Key::Num4),
  Num5 = static_cast<int>(sf::Keyboard::Key::Num5),
  Num6 = static_cast<int>(sf::Keyboard::Key::Num6),
  Num7 = static_cast<int>(sf::Keyboard::Key::Num7),
  Num8 = static_cast<int>(sf::Keyboard::Key::Num8),
  Num9 = static_cast<int>(sf::Keyboard::Key::Num9),
  R = static_cast<int>(sf::Keyboard::Key::R),
  LShift = static_cast<int>(sf::Keyboard::Key::LShift),
  RShift = static_cast<int>(sf::Keyboard::Key::RShift),
};

// =============================================================================
// 2. FARBPALETTE & COLOR RAMPS (Mode 13h Retro-Palettensystem)
// =============================================================================
// Diese Farbnummern und Farbverläufe kannst du für alle Zeichenbefehle verwenden:
// - Einzelne Farben: e.cls(Colors::DarkGreen) oder e.rectfill(..., Colors::Red)
// - Farbverläufe: Ramps::Fire[15] (Weißglut) bis Ramps::Fire[0] (Rauch)
// - Sanftes Sampling: Ramps::Fire.sample(lifePct) oder Ramps::Red.sample(hpPct)
// =============================================================================
namespace Colors {
constexpr uint8_t Black = 0;      // Schwarz
constexpr uint8_t DarkGreen = 1;  // Dunkelgrün (Spielfeldrange / Rasen)
constexpr uint8_t Red = 2;        // Leuchtendes Rot (Spieler 1)
constexpr uint8_t Blue = 3;       // Helles Blau (Spieler 2)
constexpr uint8_t Yellow = 4;     // Sonnengelb (Ball)
constexpr uint8_t White = 5;      // Weiß (Linien & Schrift)
constexpr uint8_t LightGreen = 6; // Hellgrün (Netz & Mittelkreis)
constexpr uint8_t Orange = 7;     // Orange
constexpr uint8_t DarkGray = 8;   // Dunkelgrau
constexpr uint8_t LightGray = 9;  // Hellgrau
constexpr uint8_t Pink = 10;      // Rosa / Pink
constexpr uint8_t Purple = 11;    // Lila / Violett
constexpr uint8_t Cyan = 12;      // Türkis / Cyan
constexpr uint8_t SkyBlue = 13;   // Himmelblau
constexpr uint8_t DarkPurple = 14;// Nacht-Lila
constexpr uint8_t Gold = 15;      // Gold / Sternengelb
constexpr uint8_t Magenta = 16;   // Magenta
} // namespace Colors

// --- Farbbänder & Helligkeitsverläufe (Ramps) ---
template <size_t N>
struct ColorRamp {
  std::array<uint8_t, N> indices;

  // Direkter Indexzugriff: ramp[step] (0 = dunkelster Schatten, N-1 = hellster Glanz)
  constexpr uint8_t operator[](size_t step) const {
    return indices[step < N ? step : N - 1];
  }

  // Fließendes Sampling von 0.0f bis 1.0f: ramp.sample(0.75f)
  constexpr uint8_t sample(float t) const {
    float clamped = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t);
    size_t idx = static_cast<size_t>(clamped * (N - 1) + 0.5f);
    return indices[idx < N ? idx : N - 1];
  }

  constexpr size_t size() const { return N; }
  constexpr uint8_t front() const { return indices[0]; }
  constexpr uint8_t back() const { return indices[N - 1]; }
};

using ColorRamp8 = ColorRamp<8>;
using ColorRamp16 = ColorRamp<16>;

constexpr ColorRamp16 makeRamp16(uint8_t start) {
  return ColorRamp16{{
      static_cast<uint8_t>(start + 0),  static_cast<uint8_t>(start + 1),
      static_cast<uint8_t>(start + 2),  static_cast<uint8_t>(start + 3),
      static_cast<uint8_t>(start + 4),  static_cast<uint8_t>(start + 5),
      static_cast<uint8_t>(start + 6),  static_cast<uint8_t>(start + 7),
      static_cast<uint8_t>(start + 8),  static_cast<uint8_t>(start + 9),
      static_cast<uint8_t>(start + 10), static_cast<uint8_t>(start + 11),
      static_cast<uint8_t>(start + 12), static_cast<uint8_t>(start + 13),
      static_cast<uint8_t>(start + 14), static_cast<uint8_t>(start + 15)
  }};
}

constexpr ColorRamp8 makeRamp8(uint8_t start) {
  return ColorRamp8{{
      static_cast<uint8_t>(start + 0), static_cast<uint8_t>(start + 1),
      static_cast<uint8_t>(start + 2), static_cast<uint8_t>(start + 3),
      static_cast<uint8_t>(start + 4), static_cast<uint8_t>(start + 5),
      static_cast<uint8_t>(start + 6), static_cast<uint8_t>(start + 7)
  }};
}

namespace Ramps {
  // 16-Stufen Graustufen (0 = Schwarz -> 15 = Reines Weiß)
  constexpr ColorRamp16 Grays = makeRamp16(32);

  // 16-Stufen Thermische Explosion & Feuer (0 = Rauch -> 7 = Rot -> 11 = Gelb -> 15 = Weißglut)
  constexpr ColorRamp16 Fire = makeRamp16(48);
  constexpr ColorRamp16 Explosion = Fire;

  // 8-Stufen Natur- & Erdtöne (Perfekt für Gelände, Felsen, Sand & Höhlen)
  namespace Earth {
    constexpr ColorRamp8 Soil = makeRamp8(64); // Dunkle Erde & Humus
    constexpr ColorRamp8 Clay = makeRamp8(72); // Lehm, Schlamm & Terracotta
    constexpr ColorRamp8 Sand = makeRamp8(80); // Sandstein & Wüstensand
    constexpr ColorRamp8 Moss = makeRamp8(88); // Waldmoos, Laub & Flechten
  }

  // 16-Stufen Regenbogen-Farben (0-3 Schatten -> 4-11 Gesättigte Farbe -> 12-15 Weißglanz)
  constexpr ColorRamp16 Red     = makeRamp16(96);
  constexpr ColorRamp16 Orange  = makeRamp16(112);
  constexpr ColorRamp16 Gold    = makeRamp16(128);
  constexpr ColorRamp16 Yellow  = Gold;
  constexpr ColorRamp16 Lime    = makeRamp16(144);
  constexpr ColorRamp16 Green   = makeRamp16(160);
  constexpr ColorRamp16 Cyan    = makeRamp16(176);
  constexpr ColorRamp16 SkyBlue = makeRamp16(192);
  constexpr ColorRamp16 Blue    = makeRamp16(208);
  constexpr ColorRamp16 Purple  = makeRamp16(224);
  constexpr ColorRamp16 Pink    = makeRamp16(240);
  constexpr ColorRamp16 Magenta = Pink;
}

constexpr int PALETTE_SIZE = 256;

// =============================================================================
// 3. MUSIKNOTEN & FREQUENZEN (in Hertz)
// =============================================================================
// Hier findest du alle Notennamen, damit du keine Hertz-Zahlen eintippen musst:
// z.B. Notes::C4 (Mittleres C), Notes::A4 (Kammerton A), Notes::G5, etc.
// 's' steht für Kreuz (#) z.B. Fs4 = Fis (F#) / 'b' für 'b' z.B. Bb4 = B (B♭)
// =============================================================================
namespace Notes {
// Oktave 2 (Tiefe Basstöne)
constexpr double C2  = 65.41;
constexpr double Cs2 = 69.30; constexpr double Db2 = Cs2;
constexpr double D2  = 73.42;
constexpr double Ds2 = 77.78; constexpr double Eb2 = Ds2;
constexpr double E2  = 82.41;
constexpr double F2  = 87.31;
constexpr double Fs2 = 92.50; constexpr double Gb2 = Fs2;
constexpr double G2  = 98.00;
constexpr double Gs2 = 103.83; constexpr double Ab2 = Gs2;
constexpr double A2  = 110.00;
constexpr double As2 = 116.54; constexpr double Bb2 = As2;
constexpr double B2  = 123.47;

// Oktave 3 (Bass / Bariton)
constexpr double C3  = 130.81;
constexpr double Cs3 = 138.59; constexpr double Db3 = Cs3;
constexpr double D3  = 146.83;
constexpr double Ds3 = 155.56; constexpr double Eb3 = Ds3;
constexpr double E3  = 164.81;
constexpr double F3  = 174.61;
constexpr double Fs3 = 185.00; constexpr double Gb3 = Fs3;
constexpr double G3  = 196.00;
constexpr double Gs3 = 207.65; constexpr double Ab3 = Gs3;
constexpr double A3  = 220.00;
constexpr double As3 = 233.08; constexpr double Bb3 = As3;
constexpr double B3  = 246.94;

// Oktave 4 (Mittlere Oktave - C4 ist das mittlere C am Klavier, A4 = 440 Hz)
constexpr double C4  = 261.63;
constexpr double Cs4 = 277.18; constexpr double Db4 = Cs4;
constexpr double D4  = 293.66;
constexpr double Ds4 = 311.13; constexpr double Eb4 = Ds4;
constexpr double E4  = 329.63;
constexpr double F4  = 349.23;
constexpr double Fs4 = 369.99; constexpr double Gb4 = Fs4;
constexpr double G4  = 392.00;
constexpr double Gs4 = 415.30; constexpr double Ab4 = Gs4;
constexpr double A4  = 440.00;
constexpr double As4 = 466.16; constexpr double Bb4 = As4;
constexpr double B4  = 493.88;

// Oktave 5 (Hohe Töne / Melodien)
constexpr double C5  = 523.25;
constexpr double Cs5 = 554.37; constexpr double Db5 = Cs5;
constexpr double D5  = 587.33;
constexpr double Ds5 = 622.25; constexpr double Eb5 = Ds5;
constexpr double E5  = 659.25;
constexpr double F5  = 698.46;
constexpr double Fs5 = 739.99; constexpr double Gb5 = Fs5;
constexpr double G5  = 783.99;
constexpr double Gs5 = 830.61; constexpr double Ab5 = Gs5;
constexpr double A5  = 880.00;
constexpr double As5 = 932.33; constexpr double Bb5 = As5;
constexpr double B5  = 987.77;

// Oktave 6 (Sehr hohe Töne / Piepser / Glocken)
constexpr double C6  = 1046.50;
constexpr double Cs6 = 1108.73; constexpr double Db6 = Cs6;
constexpr double D6  = 1174.66;
constexpr double Ds6 = 1244.51; constexpr double Eb6 = Ds6;
constexpr double E6  = 1318.51;
constexpr double F6  = 1396.91;
constexpr double Fs6 = 1479.98; constexpr double Gb6 = Fs6;
constexpr double G6  = 1567.98;
constexpr double Gs6 = 1661.22; constexpr double Ab6 = Gs6;
constexpr double A6  = 1760.00;
constexpr double As6 = 1864.66; constexpr double Bb6 = As6;
constexpr double B6  = 1975.53;
} // namespace Notes

// =============================================================================
// 4. SOUND-SYNTHESIZER (Erzeugt Töne und Melodien)
// =============================================================================
// Ein einzelner Ton mit Frequenz (in Hertz) und Dauer (in Sekunden)
struct Tone {
  double freq;    // Tonhöhe in Hertz (z.B. Notes::A4 oder 440.0)
  float duration; // Dauer in Sekunden (z.B. 0.15s)
};

class SoundEngine {
public:
  static constexpr size_t SFX_POOL_SIZE = 4;

  SoundEngine() {
    for (size_t i = 0; i < SFX_POOL_SIZE; ++i) {
      sounds[i] = std::make_unique<sf::Sound>(soundBuffers[i]);
    }
    bgmSound = std::make_unique<sf::Sound>(bgmBuffer);
  }

  // Spielt einen einzelnen Ton mit Frequenz und Dauer ab
  void playTone(double freq, float duration = 0.1f) {
    const unsigned sampleRate = 44100;
    unsigned totalSamples = static_cast<unsigned>(sampleRate * duration);
    if (totalSamples == 0)
      return;

    std::vector<int16_t> samples(totalSamples);
    const double twoPi = 2.0 * M_PI;

    for (unsigned i = 0; i < totalSamples; ++i) {
      double t = static_cast<double>(i) / sampleRate;
      double progress = static_cast<double>(i) / totalSamples;
      // Sanftes Ein- und Ausblenden gegen Knackgeräusche
      double env = (progress < 0.1) ? (progress / 0.1) : (1.0 - progress);
      // Sinuswelle mit leichter Oberton-Wärme für Retro-Klang
      double wave = std::sin(twoPi * freq * t) + 0.25 * std::sin(twoPi * freq * 2.0 * t);
      samples[i] = static_cast<int16_t>(26000.0 * env * wave);
    }

    size_t idx = poolIndex;
    poolIndex = (poolIndex + 1) % SFX_POOL_SIZE;
    if (soundBuffers[idx].loadFromSamples(samples.data(), samples.size(), 1, sampleRate,
                                         {sf::SoundChannel::Mono})) {
      sounds[idx]->setBuffer(soundBuffers[idx]);
      sounds[idx]->setPitch(1.0f);
      sounds[idx]->setVolume(100.0f);
      sounds[idx]->play();
    }
  }

  // Spielt eine MIDI-Note ab (z.B. 60 = Mittleres C, 69 = A 440Hz)
  void playNote(int midiNote, float duration = 0.15f) {
    double freq = 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
    playTone(freq, duration);
  }

  // Spielt eine Liste von Noten nacheinander als Melodie ab
  void playMelody(std::initializer_list<Tone> tones) {
    melodyQueue.assign(tones.begin(), tones.end());
    queueIndex = 0;
    noteTimer = 0.0f;
  }

  // --- MOTOR-SOUND (Echtzeit-Drehzahl und Lautstärke) ---
  void initEngineSound() {
    if (engineInitialized) return;
    const unsigned sampleRate = 44100;
    const unsigned totalSamples = static_cast<unsigned>(sampleRate * 0.30f); // 0.30s nahtloser Loop
    std::vector<int16_t> samples(totalSamples);
    const double twoPi = 2.0 * M_PI;
    const double baseFreq = 55.0; // 55 Hz weicher Bass-Puls

    for (unsigned i = 0; i < totalSamples; ++i) {
      double t = static_cast<double>(i) / sampleRate;
      // Weiche Sinus- und Dreiecks-Schwingung
      double pulse = std::sin(twoPi * baseFreq * t) 
                   + 0.35 * std::sin(twoPi * baseFreq * 2.0 * t) 
                   + 0.15 * std::sin(twoPi * baseFreq * 3.0 * t);
      double tri = 2.0 * std::abs(2.0 * (std::fmod(t * baseFreq, 1.0) - 0.5)) - 1.0;
      double raw = 0.75 * pulse + 0.25 * tri;

      // Loop-Kanten sanft ausblenden
      double win = 1.0;
      if (i < 200) win = static_cast<double>(i) / 200.0;
      else if (i > totalSamples - 200) win = static_cast<double>(totalSamples - i) / 200.0;

      samples[i] = static_cast<int16_t>(14000.0 * raw * win);
    }

    if (engineBuffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate,
                                    {sf::SoundChannel::Mono})) {
      engineSound = std::make_unique<sf::Sound>(engineBuffer);
      engineSound->setLooping(true);
      engineInitialized = true;
    }
  }

  void setEngineAudio(float pitch, float volume) {
    if (!engineInitialized) {
      initEngineSound();
    }
    if (!engineSound) return;

    if (volume <= 0.001f) {
      if (engineSound->getStatus() == sf::Sound::Status::Playing) {
        engineSound->stop();
      }
      return;
    }

    engineSound->setPitch(std::clamp(pitch, 0.45f, 3.5f));
    engineSound->setVolume(std::clamp(volume * 100.0f, 0.0f, 100.0f));
    if (engineSound->getStatus() != sf::Sound::Status::Playing) {
      engineSound->play();
    }
  }

  void stopEngineAudio() {
    if (engineSound) {
      engineSound->stop();
    }
  }

  // Hintergrundmusik (Loop) abspielen
  void playBgm(const std::vector<int16_t> &samples, unsigned sampleRate = 44100, bool loop = true) {
    if (samples.empty())
      return;
    if (bgmBuffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate,
                                 {sf::SoundChannel::Mono})) {
      bgmSound->setBuffer(bgmBuffer);
      bgmSound->setLooping(loop);
      bgmSound->play();
    }
  }

  void stopBgm() {
    if (bgmSound) {
      bgmSound->stop();
    }
  }

  void pauseBgm() {
    if (bgmSound) {
      bgmSound->pause();
    }
  }

  void resumeBgm() {
    if (bgmSound && bgmSound->getStatus() == sf::Sound::Status::Paused) {
      bgmSound->play();
    }
  }

  // Aktualisiert die Melodie-Warteschlange (wird vom Motor gesteuert)
  void update(float dt) {
    if (queueIndex < melodyQueue.size()) {
      noteTimer -= dt;
      if (noteTimer <= 0.0f) {
        const auto &t = melodyQueue[queueIndex++];
        playTone(t.freq, t.duration);
        noteTimer = t.duration * 0.95f;
      }
    }
  }

private:
  std::vector<Tone> melodyQueue;
  size_t queueIndex = 0;
  float noteTimer = 0.0f;

  std::array<sf::SoundBuffer, SFX_POOL_SIZE> soundBuffers;
  std::array<std::unique_ptr<sf::Sound>, SFX_POOL_SIZE> sounds;
  size_t poolIndex = 0;

  sf::SoundBuffer engineBuffer;
  std::unique_ptr<sf::Sound> engineSound;
  bool engineInitialized = false;

  sf::SoundBuffer bgmBuffer;
  std::unique_ptr<sf::Sound> bgmSound;
};

// =============================================================================
// 4. DIE SPIEL-ENGINE (Deine Werkzeugkiste zum Zeichnen, Töne & Steuern)
// =============================================================================
class Engine {
public:
  Engine(int w, int h)
      : screenWidth(w), screenHeight(h), framebuffer(w * h, Colors::DarkGreen) {
    initPalette();
  }

  // --- Zufallsgenerator & Rausch-Funktionen (RNG & Spatial Noise) ---
  // Liefert eine Zufallszahl von 0 bis max - 1 (oder [min, max])
  int rnd(int max) {
    if (max <= 0) return 0;
    rngState = (rngState * 1664525u + 1013904223u);
    return static_cast<int>(rngState % static_cast<uint32_t>(max));
  }

  int rnd(int min, int max) {
    if (min >= max) return min;
    return min + rnd(max - min + 1);
  }

  // Liefert eine Zufallskommazahl von 0.0f bis 1.0f (oder [min, max])
  float rndf() {
    rngState = (rngState * 1664525u + 1013904223u);
    return static_cast<float>(rngState & 0x00FFFFFF) / 16777216.0f;
  }

  float rndf(float min, float max) {
    return min + rndf() * (max - min);
  }

  // 2D Spatial Hash Noise (Kachelfreies, hochqualitatives Weißrauschen von 0.0f bis 1.0f)
  static float noise(int x, int y, uint32_t seed = 1337) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + seed * 374761u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<float>((h ^ (h >> 16)) & 0x00FFFFFF) / 16777216.0f;
  }

  void srand_seed(uint32_t s) { rngState = s ? s : 123456789u; }

  // --- Bildschirm-Maße & Zeit ---
  int width() const { return screenWidth; }   // Bildschirmbreite (320 Pixel)
  int height() const { return screenHeight; } // Bildschirmhöhe (240 Pixel)
  float dt() const { return deltaTime; }      // Zeit seit dem letzten Frame (z.B. 0.016s bei 60 FPS)
  float time() const { return totalTime; }    // Gesamtspielzeit in Sekunden

  // --- Tastatur & Maus Abfragen ---
  // Prüft, ob eine Taste gerade gedrückt gehalten wird
  bool key(Key k) const {
    auto it = keyState.find(static_cast<sf::Keyboard::Key>(k));
    return (it != keyState.end() && it->second);
  }

  // Prüft, ob eine Taste genau in diesem Moment neu angetippt wurde
  bool pressed(Key k) const {
    auto it = keyPressedState.find(static_cast<sf::Keyboard::Key>(k));
    return (it != keyPressedState.end() && it->second);
  }

  int mouse_x() const { return mousePos.x; } // Maus X-Position auf dem Bildschirm
  int mouse_y() const { return mousePos.y; } // Maus Y-Position auf dem Bildschirm
  bool mouse_down(sf::Mouse::Button btn = sf::Mouse::Button::Left) const {
    return sf::Mouse::isButtonPressed(btn);
  }
  bool mouse_pressed(sf::Mouse::Button btn = sf::Mouse::Button::Left) const {
    auto it = mousePressedState.find(btn);
    return (it != mousePressedState.end() && it->second);
  }
  bool mouse_released(sf::Mouse::Button btn = sf::Mouse::Button::Left) const {
    auto it = mouseReleasedState.find(btn);
    return (it != mouseReleasedState.end() && it->second);
  }

  // --- ZEICHEN-FUNKTIONEN (Unterstützt Kommazahlen und ganze Zahlen) ---

  // Bildschirm komplett mit einer Farbe übermalen
  void cls(uint8_t color = Colors::DarkGreen) {
    std::fill(framebuffer.begin(), framebuffer.end(), color);
  }

  // Einen einzelnen Pixel an Position (x, y) setzen
  void pset(float x, float y, uint8_t color) {
    int ix = static_cast<int>(std::round(x));
    int iy = static_cast<int>(std::round(y));
    if (ix >= 0 && ix < screenWidth && iy >= 0 && iy < screenHeight) {
      framebuffer[iy * screenWidth + ix] = color;
    }
  }

  // Farbe des Pixels an Position (x, y) auslesen
  uint8_t pget(float x, float y) const {
    int ix = static_cast<int>(std::round(x));
    int iy = static_cast<int>(std::round(y));
    if (ix >= 0 && ix < screenWidth && iy >= 0 && iy < screenHeight) {
      return framebuffer[iy * screenWidth + ix];
    }
    return 0;
  }

  // Eine gerade Linie von (x0, y0) nach (x1, y1) zeichnen
  void line(float x0, float y0, float x1, float y1, uint8_t color) {
    int ix0 = static_cast<int>(std::round(x0));
    int iy0 = static_cast<int>(std::round(y0));
    int ix1 = static_cast<int>(std::round(x1));
    int iy1 = static_cast<int>(std::round(y1));

    int dx = std::abs(ix1 - ix0);
    int dy = -std::abs(iy1 - iy0);
    int sx = ix0 < ix1 ? 1 : -1;
    int sy = iy0 < iy1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
      if (ix0 >= 0 && ix0 < screenWidth && iy0 >= 0 && iy0 < screenHeight) {
        framebuffer[iy0 * screenWidth + ix0] = color;
      }
      if (ix0 == ix1 && iy0 == iy1)
        break;
      int e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        ix0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        iy0 += sy;
      }
    }
  }

  // Den Umriss eines Rechtecks zeichnen
  void rect(float x, float y, float w, float h, uint8_t color) {
    line(x, y, x + w - 1.0f, y, color);
    line(x, y + h - 1.0f, x + w - 1.0f, y + h - 1.0f, color);
    line(x, y, x, y + h - 1.0f, color);
    line(x + w - 1.0f, y, x + w - 1.0f, y + h - 1.0f, color);
  }

  // Ein ausgefülltes Rechteck zeichnen (w = Breite, h = Höhe)
  void rectfill(float x, float y, float w, float h, uint8_t color) {
    int ix = static_cast<int>(std::round(x));
    int iy = static_cast<int>(std::round(y));
    int iw = static_cast<int>(std::round(w));
    int ih = static_cast<int>(std::round(h));

    int xStart = std::max(0, ix);
    int xEnd = std::min(screenWidth, ix + iw);
    int yStart = std::max(0, iy);
    int yEnd = std::min(screenHeight, iy + ih);

    for (int cy = yStart; cy < yEnd; ++cy) {
      int rowOffset = cy * screenWidth;
      for (int cx = xStart; cx < xEnd; ++cx) {
        framebuffer[rowOffset + cx] = color;
      }
    }
  }

  // Den Umriss eines Kreises zeichnen (cx, cy = Mittelpunkt, r = Radius)
  void circle(float cx, float cy, float r, uint8_t color) {
    int icx = static_cast<int>(std::round(cx));
    int icy = static_cast<int>(std::round(cy));
    int ir = static_cast<int>(std::round(r));

    int x = ir;
    int y = 0;
    int err = 0;

    auto plot = [this, icx, icy, color](int px, int py) {
      int rx = icx + px;
      int ry = icy + py;
      if (rx >= 0 && rx < screenWidth && ry >= 0 && ry < screenHeight) {
        framebuffer[ry * screenWidth + rx] = color;
      }
    };

    while (x >= y) {
      plot(x, y);
      plot(y, x);
      plot(-y, x);
      plot(-x, y);
      plot(-x, -y);
      plot(-y, -x);
      plot(y, -x);
      plot(x, -y);

      y += 1;
      err += 1 + 2 * y;
      if (2 * (err - x) + 1 > 0) {
        x -= 1;
        err += 1 - 2 * x;
      }
    }
  }

  // Einen ausgefüllten Kreis zeichnen (cx, cy = Mittelpunkt, r = Radius)
  void circlefill(float cx, float cy, float r, uint8_t color) {
    int icx = static_cast<int>(std::round(cx));
    int icy = static_cast<int>(std::round(cy));
    int ir = static_cast<int>(std::round(r));

    for (int y = -ir; y <= ir; ++y) {
      for (int x = -ir; x <= ir; ++x) {
        if (x * x + y * y <= ir * ir) {
          int rx = icx + x;
          int ry = icy + y;
          if (rx >= 0 && rx < screenWidth && ry >= 0 && ry < screenHeight) {
            framebuffer[ry * screenWidth + rx] = color;
          }
        }
      }
    }
  }

  // Alias-Methoden (Retro-Kurzschreibweise)
  void circ(float cx, float cy, float r, uint8_t color) { circle(cx, cy, r, color); }
  void circfill(float cx, float cy, float r, uint8_t color) { circlefill(cx, cy, r, color); }

  // Eine Ziffer (0-9) zeichnen (scale = Größe)
  void draw_digit(float x, float y, int digit, uint8_t color, int scale = 2) {
    // Hier ist das 3x5 Pixel-Muster für jede Ziffer:
    // clang-format off
    static const uint8_t digits[10][5][3] = {
      // 0
      {
        {1, 1, 1},
        {1, 0, 1},
        {1, 0, 1},
        {1, 0, 1},
        {1, 1, 1}
      },
      // 1
      {
        {0, 1, 0},
        {1, 1, 0},
        {0, 1, 0},
        {0, 1, 0},
        {1, 1, 1}
      },
      // 2
      {
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 1},
        {1, 0, 0},
        {1, 1, 1}
      },
      // 3
      {
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 1}
      },
      // 4
      {
        {1, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 0, 1},
        {0, 0, 1}
      },
      // 5
      {
        {1, 1, 1},
        {1, 0, 0},
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 1}
      },
      // 6
      {
        {1, 1, 1},
        {1, 0, 0},
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1}
      },
      // 7
      {
        {1, 1, 1},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 0},
        {0, 1, 0}
      },
      // 8
      {
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1}
      },
      // 9
      {
        {1, 1, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 0, 1},
        {1, 1, 1}
      }
    };
    // clang-format on

    if (digit < 0 || digit > 9)
      return;
    for (int r = 0; r < 5; ++r) {
      for (int c = 0; c < 3; ++c) {
        if (digits[digit][r][c]) {
          rectfill(x + c * scale, y + r * scale, scale, scale, color);
        }
      }
    }
  }

  // Text auf den Bildschirm schreiben (scale = Textgröße)
  void draw_text(float x, float y, const std::string &text, uint8_t color, int scale = 1) {
    float curX = x;
    for (char ch : text) {
      if (ch >= '0' && ch <= '9') {
        draw_digit(curX, y, ch - '0', color, scale);
        curX += 4.0f * scale;
      } else if (ch == ' ') {
        curX += 3.0f * scale;
      } else if (ch == ':') {
        rectfill(curX + scale, y + scale, scale, scale, color);
        rectfill(curX + scale, y + 3 * scale, scale, scale, color);
        curX += 3.0f * scale;
      } else if (ch == '-') {
        rectfill(curX, y + 2 * scale, 3 * scale, scale, color);
        curX += 4.0f * scale;
      } else if (ch == '!') {
        rectfill(curX, y, scale, 3 * scale, color);
        rectfill(curX, y + 4 * scale, scale, scale, color);
        curX += 2.0f * scale;
      } else if (ch == '(') {
        rectfill(curX + scale, y, scale, 5 * scale, color);
        rectfill(curX + scale * 2, y, scale, scale, color);
        rectfill(curX + scale * 2, y + 4 * scale, scale, scale, color);
        curX += 3.0f * scale;
      } else if (ch == ')') {
        rectfill(curX + scale, y, scale, 5 * scale, color);
        rectfill(curX, y, scale, scale, color);
        rectfill(curX, y + 4 * scale, scale, scale, color);
        curX += 3.0f * scale;
      } else if (ch == '/') {
        line(curX, y + 4 * scale, curX + 2 * scale, y, color);
        curX += 4.0f * scale;
      } else {
        draw_char(curX, y, ch, color, scale);
        curX += 4.0f * scale;
      }
    }
  }

  // --- AUDIO-FUNKTIONEN ---
  // Spielt einen Ton mit Frequenz (Hz) und Dauer (Sekunden) ab
  void play_tone(double freq, float duration = 0.1f) { sound.playTone(freq, duration); }

  // Spielt eine Note über ihre MIDI-Notennummer ab
  void play_note(int midiNote, float duration = 0.15f) { sound.playNote(midiNote, duration); }

  // Spielt eine Notenabfolge (Melodie) ab
  void play_melody(std::initializer_list<Tone> tones) { sound.playMelody(tones); }

  // Hintergrundmusik (Loop)
  void play_bgm(const std::vector<int16_t> &samples, unsigned sampleRate = 44100, bool loop = true) {
    sound.playBgm(samples, sampleRate, loop);
  }
  void stop_bgm() { sound.stopBgm(); }
  void pause_bgm() { sound.pauseBgm(); }
  void resume_bgm() { sound.resumeBgm(); }

  // Motor-Sound (RPM Pitch & Lautstärke)
  void set_engine_sound(float pitch, float volume) { sound.setEngineAudio(pitch, volume); }
  void stop_engine_sound() { sound.stopEngineAudio(); }

  // Alle Audio-Kanäle stoppen
  void stop_all_audio() {
    sound.stopBgm();
    sound.stopEngineAudio();
  }

  // --- Interne Engine-Methoden ---
  const std::vector<uint8_t> &getBuffer() const { return framebuffer; }
  const std::array<sf::Color, PALETTE_SIZE> &getPalette() const { return palette; }

  void internalUpdate(float dt, const sf::Vector2i &mPos) {
    deltaTime = dt;
    totalTime += dt;
    mousePos = mPos;
    sound.update(dt);
  }

  void setKeyState(sf::Keyboard::Key key, bool isDown) {
    bool wasDown = keyState[key];
    keyState[key] = isDown;
    if (isDown && !wasDown) {
      keyPressedState[key] = true;
    }
  }

  void setMouseButtonState(sf::Mouse::Button btn, bool isDown) {
    bool wasDown = mouseButtonState[btn];
    mouseButtonState[btn] = isDown;
    if (isDown && !wasDown) {
      mousePressedState[btn] = true;
    } else if (!isDown && wasDown) {
      mouseReleasedState[btn] = true;
    }
  }

  void clearKeyTransitions() {
    keyPressedState.clear();
    mousePressedState.clear();
    mouseReleasedState.clear();
  }

private:
  int screenWidth;
  int screenHeight;
  float deltaTime = 0.016f;
  float totalTime = 0.0f;
  sf::Vector2i mousePos{0, 0};

  std::vector<uint8_t> framebuffer;
  std::array<sf::Color, PALETTE_SIZE> palette;
  SoundEngine sound;

  std::unordered_map<sf::Keyboard::Key, bool> keyState;
  std::unordered_map<sf::Keyboard::Key, bool> keyPressedState;
  std::unordered_map<sf::Mouse::Button, bool> mouseButtonState;
  std::unordered_map<sf::Mouse::Button, bool> mousePressedState;
  std::unordered_map<sf::Mouse::Button, bool> mouseReleasedState;
  uint32_t rngState = 123456789u;

  void initPalette() {
    // --- 1. Klassische 17 Standard-Farben (0 - 16, 100% abwärtskompatibel) ---
    palette[Colors::Black] = sf::Color(0, 0, 0);
    palette[Colors::DarkGreen] = sf::Color(18, 90, 36);
    palette[Colors::Red] = sf::Color(240, 60, 60);
    palette[Colors::Blue] = sf::Color(60, 130, 255);
    palette[Colors::Yellow] = sf::Color(255, 230, 40);
    palette[Colors::White] = sf::Color(255, 255, 255);
    palette[Colors::LightGreen] = sf::Color(45, 145, 65);
    palette[Colors::Orange] = sf::Color(255, 140, 30);
    palette[Colors::DarkGray] = sf::Color(40, 40, 40);
    palette[Colors::LightGray] = sf::Color(180, 180, 180);
    palette[Colors::Pink] = sf::Color(255, 105, 180);
    palette[Colors::Purple] = sf::Color(160, 50, 240);
    palette[Colors::Cyan] = sf::Color(0, 235, 255);
    palette[Colors::SkyBlue] = sf::Color(90, 180, 255);
    palette[Colors::DarkPurple] = sf::Color(25, 10, 45);
    palette[Colors::Gold] = sf::Color(255, 215, 0);
    palette[Colors::Magenta] = sf::Color(240, 40, 160);

    // --- 2. Retro UI & Akzent-Farben (17 - 31) ---
    static const sf::Color uiAccents[15] = {
      sf::Color(12, 18, 32),    // 17: Mitternachtsblau
      sf::Color(45, 55, 68),    // 18: Dunkelstahl
      sf::Color(120, 135, 150), // 19: Hellstahl
      sf::Color(255, 245, 220), // 20: Warmes Cremeweiß
      sf::Color(115, 18, 28),   // 21: Dunkles Karmin
      sf::Color(190, 75, 25),   // 22: Rostorange
      sf::Color(130, 240, 175), // 23: Minzgrün
      sf::Color(70, 15, 95),    // 24: Nachtviolett
      sf::Color(170, 255, 40),  // 25: Neon-Limette
      sf::Color(200, 175, 245), // 26: Pastell-Lavendel
      sf::Color(185, 115, 65),  // 27: Kupfer
      sf::Color(140, 90, 45),   // 28: Bronze
      sf::Color(215, 220, 230), // 29: Silber
      sf::Color(5, 5, 10),      // 30: Tiefschwarz
      sf::Color(255, 255, 255)  // 31: Reines Weiß
    };
    for (int i = 0; i < 15; ++i) {
      palette[17 + i] = uiAccents[i];
    }

    // --- 3. 16-Stufen Graustufen-Ramp (32 - 47: Schwarz bis Reines Weiß) ---
    for (int i = 0; i < 16; ++i) {
      uint8_t v = static_cast<uint8_t>((i * 255) / 15);
      palette[32 + i] = sf::Color(v, v, v);
    }

    // --- 4. 16-Stufen Thermische Explosion & Feuer (48 - 63) ---
    static const sf::Color fireColors[16] = {
      sf::Color(20, 16, 18),    // 48: Rauch / Holzkohle
      sf::Color(48, 14, 20),    // 49: Tiefes Glutrot
      sf::Color(82, 12, 18),    // 50: Dunkles Karminrot
      sf::Color(128, 16, 16),   // 51: Weinrot
      sf::Color(172, 22, 14),   // 52: Sattes Dunkelrot
      sf::Color(215, 30, 12),   // 53: Rubinrot
      sf::Color(245, 48, 10),   // 54: Reines Feuerrot
      sf::Color(255, 82, 10),   // 55: Rotorange
      sf::Color(255, 118, 12),  // 56: Kräftiges Orange
      sf::Color(255, 152, 16),  // 57: Helles Orange
      sf::Color(255, 188, 20),  // 58: Flammengold
      sf::Color(255, 218, 28),  // 59: Sonnengelb
      sf::Color(255, 238, 64),  // 60: Helles Zitronengelb
      sf::Color(255, 248, 130), // 61: Gelbweiß
      sf::Color(255, 252, 200), // 62: Weißgelbes Glühen
      sf::Color(255, 255, 255)  // 63: Reines weißes Blendlicht
    };
    for (int i = 0; i < 16; ++i) {
      palette[48 + i] = fireColors[i];
    }

    // --- 5. 32 Natur- & Erdtöne (64 - 95: 4 Ramps à 8 Stufen) ---
    // A. Erde & dunkler Humus (64 - 71)
    static const sf::Color soilColors[8] = {
      sf::Color(28, 16, 10), sf::Color(46, 26, 16), sf::Color(68, 40, 24), sf::Color(90, 54, 32),
      sf::Color(112, 70, 42), sf::Color(135, 86, 52), sf::Color(158, 104, 64), sf::Color(182, 124, 78)
    };
    for (int i = 0; i < 8; ++i) palette[64 + i] = soilColors[i];

    // B. Lehm, Schlamm & Terracotta (72 - 79)
    static const sf::Color clayColors[8] = {
      sf::Color(65, 22, 14), sf::Color(92, 34, 22), sf::Color(120, 46, 30), sf::Color(148, 60, 40),
      sf::Color(175, 76, 52), sf::Color(200, 96, 66), sf::Color(222, 122, 88), sf::Color(242, 152, 118)
    };
    for (int i = 0; i < 8; ++i) palette[72 + i] = clayColors[i];

    // C. Sandstein & Wüstensand (80 - 87)
    static const sf::Color sandColors[8] = {
      sf::Color(86, 72, 42), sf::Color(112, 94, 56), sf::Color(140, 118, 72), sf::Color(170, 144, 90),
      sf::Color(198, 170, 110), sf::Color(222, 194, 132), sf::Color(242, 218, 158), sf::Color(255, 240, 188)
    };
    for (int i = 0; i < 8; ++i) palette[80 + i] = sandColors[i];

    // D. Waldmoos, Laub & Flechten (88 - 95)
    static const sf::Color mossColors[8] = {
      sf::Color(16, 34, 18), sf::Color(28, 54, 30), sf::Color(42, 78, 42), sf::Color(60, 104, 54),
      sf::Color(82, 130, 68), sf::Color(108, 158, 82), sf::Color(138, 185, 102), sf::Color(178, 218, 132)
    };
    for (int i = 0; i < 8; ++i) palette[88 + i] = mossColors[i];

    // --- 6. 10 Regenbogen-Farbbänder (96 - 255: 10 Hues à 16 Stufen) ---
    // Erzeugt sanfte 3-Punkt-Kurven: [Schatten (0-3) -> Farbe (4-11) -> Weißglanz (12-15)]
    auto buildRamp16 = [this](uint8_t startIdx, sf::Color baseRgb) {
      for (int step = 0; step < 16; ++step) {
        float t = step / 15.0f;
        uint8_t r, g, b;
        if (t <= 0.55f) {
          // Schatten (0.12f) bis Grundfarbe
          float subT = t / 0.55f;
          float factor = 0.12f + 0.88f * subT;
          r = static_cast<uint8_t>(baseRgb.r * factor);
          g = static_cast<uint8_t>(baseRgb.g * factor);
          b = static_cast<uint8_t>(baseRgb.b * factor);
        } else {
          // Grundfarbe bis Weißglut
          float subT = (t - 0.55f) / 0.45f;
          r = static_cast<uint8_t>(baseRgb.r + (255 - baseRgb.r) * subT);
          g = static_cast<uint8_t>(baseRgb.g + (255 - baseRgb.g) * subT);
          b = static_cast<uint8_t>(baseRgb.b + (255 - baseRgb.b) * subT);
        }
        palette[startIdx + step] = sf::Color(r, g, b);
      }
    };

    buildRamp16(96,  sf::Color(240, 35, 35));   // 1. Red
    buildRamp16(112, sf::Color(255, 120, 15));  // 2. Orange
    buildRamp16(128, sf::Color(255, 210, 20));  // 3. Gold / Yellow
    buildRamp16(144, sf::Color(140, 230, 25));  // 4. Lime
    buildRamp16(160, sf::Color(30, 190, 60));   // 5. Green
    buildRamp16(176, sf::Color(0, 225, 245));   // 6. Cyan / Aqua
    buildRamp16(192, sf::Color(55, 150, 255));  // 7. Sky Blue
    buildRamp16(208, sf::Color(35, 65, 240));   // 8. Deep Blue
    buildRamp16(224, sf::Color(150, 45, 235));  // 9. Purple / Violet
    buildRamp16(240, sf::Color(245, 50, 165));  // 10. Pink / Magenta
  }

  void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    palette[index] = sf::Color(r, g, b);
  }

  sf::Color get_palette_color(uint8_t index) const {
    return palette[index];
  }

  // 3x5 Pixel-Muster für alle Buchstaben von A bis Z
  void draw_char(float x, float y, char ch, uint8_t color, int scale) {
    ch = std::toupper(ch);

    // clang-format off
    static const std::unordered_map<char, std::array<uint8_t, 5>> fontMap = {
      {'A', {
        0b111,
        0b101,
        0b111,
        0b101,
        0b101
      }},
      {'B', {
        0b110,
        0b101,
        0b110,
        0b101,
        0b110
      }},
      {'C', {
        0b111,
        0b100,
        0b100,
        0b100,
        0b111
      }},
      {'D', {
        0b110,
        0b101,
        0b101,
        0b101,
        0b110
      }},
      {'E', {
        0b111,
        0b100,
        0b111,
        0b100,
        0b111
      }},
      {'F', {
        0b111,
        0b100,
        0b110,
        0b100,
        0b100
      }},
      {'G', {
        0b111,
        0b100,
        0b101,
        0b101,
        0b111
      }},
      {'H', {
        0b101,
        0b101,
        0b111,
        0b101,
        0b101
      }},
      {'I', {
        0b111,
        0b010,
        0b010,
        0b010,
        0b111
      }},
      {'J', {
        0b001,
        0b001,
        0b001,
        0b101,
        0b111
      }},
      {'K', {
        0b101,
        0b110,
        0b100,
        0b110,
        0b101
      }},
      {'L', {
        0b100,
        0b100,
        0b100,
        0b100,
        0b111
      }},
      {'M', {
        0b101,
        0b111,
        0b101,
        0b101,
        0b101
      }},
      {'N', {
        0b111,
        0b101,
        0b101,
        0b101,
        0b101
      }},
      {'O', {
        0b111,
        0b101,
        0b101,
        0b101,
        0b111
      }},
      {'P', {
        0b111,
        0b101,
        0b111,
        0b100,
        0b100
      }},
      {'Q', {
        0b111,
        0b101,
        0b111,
        0b001,
        0b001
      }},
      {'R', {
        0b110,
        0b101,
        0b110,
        0b101,
        0b101
      }},
      {'S', {
        0b111,
        0b100,
        0b111,
        0b001,
        0b111
      }},
      {'T', {
        0b111,
        0b010,
        0b010,
        0b010,
        0b010
      }},
      {'U', {
        0b101,
        0b101,
        0b101,
        0b101,
        0b111
      }},
      {'V', {
        0b101,
        0b101,
        0b101,
        0b101,
        0b010
      }},
      {'W', {
        0b101,
        0b101,
        0b101,
        0b111,
        0b101
      }},
      {'X', {
        0b101,
        0b101,
        0b010,
        0b101,
        0b101
      }},
      {'Y', {
        0b101,
        0b101,
        0b111,
        0b010,
        0b010
      }},
      {'Z', {
        0b111,
        0b001,
        0b010,
        0b100,
        0b111
      }},
    };
    // clang-format on

    auto it = fontMap.find(ch);
    if (it != fontMap.end()) {
      for (int r = 0; r < 5; ++r) {
        uint8_t rowBits = it->second[r];
        for (int c = 0; c < 3; ++c) {
          if (rowBits & (1 << (2 - c))) {
            rectfill(x + c * scale, y + r * scale, scale, scale, color);
          }
        }
      }
    }
  }
};

// =============================================================================
// 5. BASIS-STRUKTUR FÜR EIN SPIEL
// =============================================================================
// Jedes Spiel basiert auf 'Game' und muss nur eine Funktion schreiben: update(Engine& e)
struct Game {
  virtual ~Game() = default;
  virtual void update(Engine &e) = 0;

  // Optional: Ermöglicht Sub-Pixel-Effekte wie Fluid-Refraktion / Blur auf 2x-Auflösung (640x480)
  virtual bool customPresent(sf::Image &image, unsigned outW, unsigned outH,
                             const std::vector<uint8_t> &buffer, int srcW, int srcH,
                             const std::array<sf::Color, 256> &palette) {
    return false;
  }
};

// =============================================================================
// 6. ENGINE APP (Erstellt das Fenster und startet den 60 FPS Spiel-Takt)
// =============================================================================
class EngineApp {
public:
  EngineApp(int width = 320, int height = 240, int windowScale = 4,
            const std::string &title = "Retro Game Engine")
      : engineWidth(width), engineHeight(height),
        outWidth(width * 2), outHeight(height * 2),
        engine(width, height),
        window(sf::VideoMode({static_cast<unsigned>(width * windowScale),
                              static_cast<unsigned>(height * windowScale)}),
               title),
        image({static_cast<unsigned>(width * 2), static_cast<unsigned>(height * 2)},
              sf::Color::Black) {
    window.setFramerateLimit(60);
    (void)texture.resize({static_cast<unsigned>(outWidth), static_cast<unsigned>(outHeight)});
    sprite.emplace(texture);
    float finalScale = static_cast<float>(width * windowScale) / static_cast<float>(outWidth);
    sprite->setScale({finalScale, finalScale});
  }

  // Wählt das Spiel aus, das gestartet werden soll
  void setGame(std::unique_ptr<Game> newGame) {
    game = std::move(newGame);
  }

  // Startet das Spiel in einer Endlosschleife (bis das Fenster geschlossen wird)
  void run() {
    sf::Clock clock;

    while (window.isOpen()) {
      float dt = clock.restart().asSeconds();
      if (dt > 0.05f)
        dt = 0.05f;

      engine.clearKeyTransitions();

      // Fenster-Ereignisse (Schließen, Tastendrücke, Mausklicks) verarbeiten
      while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          window.close();
        } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
          engine.setKeyState(keyPressed->code, true);
        } else if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>()) {
          engine.setKeyState(keyReleased->code, false);
        } else if (const auto *mbPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
          engine.setMouseButtonState(mbPressed->button, true);
        } else if (const auto *mbReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
          engine.setMouseButtonState(mbReleased->button, false);
        }
      }

      // Mauskoordinaten umrechnen
      sf::Vector2i mouseWin = sf::Mouse::getPosition(window);
      sf::Vector2u winSize = window.getSize();
      int mouseX = (winSize.x > 0) ? (mouseWin.x * engineWidth / winSize.x) : 0;
      int mouseY = (winSize.y > 0) ? (mouseWin.y * engineHeight / winSize.y) : 0;

      engine.internalUpdate(dt, {mouseX, mouseY});

      // Spiel-Logik und Zeichnen ausführen
      if (game) {
        game->update(engine);
      }

      // Bildspeicher auf die Grafikkarte übertragen und anzeigen
      const auto &buffer = engine.getBuffer();
      const auto &palette = engine.getPalette();

      if (!game || !game->customPresent(image, outWidth, outHeight, buffer,
                                        engineWidth, engineHeight, palette)) {
        // Standard 2x-Pixelverdopplung für gestochen scharfen Retro-Look
        for (int y = 0; y < engineHeight; ++y) {
          int srcRow = y * engineWidth;
          unsigned dstY0 = static_cast<unsigned>(y * 2);
          unsigned dstY1 = dstY0 + 1;
          for (int x = 0; x < engineWidth; ++x) {
            sf::Color col = palette[buffer[srcRow + x]];
            unsigned dstX0 = static_cast<unsigned>(x * 2);
            unsigned dstX1 = dstX0 + 1;
            image.setPixel({dstX0, dstY0}, col);
            image.setPixel({dstX1, dstY0}, col);
            image.setPixel({dstX0, dstY1}, col);
            image.setPixel({dstX1, dstY1}, col);
          }
        }
      }

      texture.update(image);

      window.clear();
      if (sprite.has_value()) {
        window.draw(*sprite);
      }
      window.display();
    }
  }

private:
  int engineWidth;
  int engineHeight;
  int outWidth;
  int outHeight;
  Engine engine;
  sf::RenderWindow window;
  sf::Image image;
  sf::Texture texture;
  std::optional<sf::Sprite> sprite;
  std::unique_ptr<Game> game;
};
