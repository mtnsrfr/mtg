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
};

// =============================================================================
// 2. FARBPALETTE (Retro-Farben)
// =============================================================================
// Diese Farbnummern kannst du für alle Zeichenbefehle verwenden:
// z.B.: e.cls(Colors::DarkGreen) oder e.rectfill(..., Colors::Red)
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
} // namespace Colors

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
  SoundEngine() { sound = std::make_unique<sf::Sound>(soundBuffer); }

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

    if (soundBuffer.loadFromSamples(samples.data(), samples.size(), 1, sampleRate,
                                   {sf::SoundChannel::Mono})) {
      sound->setBuffer(soundBuffer);
      sound->play();
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

  sf::SoundBuffer soundBuffer;
  std::unique_ptr<sf::Sound> sound;
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

  void clearKeyTransitions() {
    keyPressedState.clear();
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

  void initPalette() {
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

    for (int i = 10; i < PALETTE_SIZE; ++i) {
      palette[i] = sf::Color(i, i, i);
    }
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
};

// =============================================================================
// 6. ENGINE APP (Erstellt das Fenster und startet den 60 FPS Spiel-Takt)
// =============================================================================
class EngineApp {
public:
  EngineApp(int width = 320, int height = 240, int windowScale = 4,
            const std::string &title = "Retro Game Engine")
      : engineWidth(width), engineHeight(height), engine(width, height),
        window(sf::VideoMode({static_cast<unsigned>(width * windowScale),
                              static_cast<unsigned>(height * windowScale)}),
               title),
        image({static_cast<unsigned>(width), static_cast<unsigned>(height)},
              sf::Color::Black) {
    window.setFramerateLimit(60);
    (void)texture.resize({static_cast<unsigned>(width), static_cast<unsigned>(height)});
    sprite.emplace(texture);
    sprite->setScale({static_cast<float>(windowScale), static_cast<float>(windowScale)});
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

      // Fenster-Ereignisse (Schließen, Tastendrücke) verarbeiten
      while (std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
          window.close();
        } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
          engine.setKeyState(keyPressed->code, true);
        } else if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>()) {
          engine.setKeyState(keyReleased->code, false);
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

      for (int y = 0; y < engineHeight; ++y) {
        int offset = y * engineWidth;
        for (int x = 0; x < engineWidth; ++x) {
          uint8_t palIdx = buffer[offset + x];
          image.setPixel({static_cast<unsigned>(x), static_cast<unsigned>(y)},
                         palette[palIdx]);
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
  Engine engine;
  sf::RenderWindow window;
  sf::Image image;
  sf::Texture texture;
  std::optional<sf::Sprite> sprite;
  std::unique_ptr<Game> game;
};
