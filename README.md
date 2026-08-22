# 🎮 MTG Retro Gaming Engine

A retro 2D game engine built in modern C++ (SFML 3) designed for kids and beginners to learn game programming from **Zero to Hero**.

---

## 🕹️ Included Games

1. **🏓 2-Player Pong (`PongGame.hpp`)**:
   * Dynamic paddle bounce angles, increasing ball speed, score tracker, and retro sound effects.
   * *Controls*: Player 1 (`W`/`S`), Player 2 (`Up`/`Down`), Start (`Space`).

2. **🐍 2-Player Snake Battle (`SnakeGame.hpp`)**:
   * 40x30 tile grid with collectable apples.
   * **Model B: Apple Theft**: When a snake crashes, its body bursts into collectable apples for the opponent to steal!
   * *Controls*: Player 1 (`W`/`A`/`S`/`D`), Player 2 (`Arrow keys`).

3. **🚀 Starfighter Attack (`StarfighterGame.hpp`)**:
   * 3-layer parallax starfield, 16x16 pixel-art delta fighters and TIE attackers.
   * Directional momentum particle explosions with thermal color decay (White $\rightarrow$ Yellow $\rightarrow$ Orange $\rightarrow$ Red $\rightarrow$ Dark Gray).
   * *Controls*: Movement (`W`/`A`/`S`/`D` or `Arrow keys`), Shoot (`Space`).

4. **🦄 The Unicorn (`UnicornGame.hpp`)**:
   * Fast-paced retro runner (Mario Starman / Geometry Dash inspired pace).
   * Choose your unicorn color: **Blue**, **White**, or **Pink**.
   * Touch ground rainbow arches to collect magic points and watch them fade out in rainbow sparkles.
   * Dodge falling shooting stars (*Sternschnuppen*) or jump on top of them (*Mario stomp*) to trigger Starfighter-style thermal particle explosions and bounce higher!
   * *Controls*: Jump / Double Jump (`Space` / `W` / `Up`), Color Choice (`1`/`2`/`3` or `Arrow keys`).

5. **🎯 Multi-Game Launcher (`MenuGame.hpp`)**:
   * Switch between games instantly with keys `1`, `2`, `3`, `4`.
   * Return to the menu at any time by pressing `Escape`.

---

## 📖 Tutorial & Guide

* 📄 **[ANLEITUNG.md](ANLEITUNG.md)**: Comprehensive, kid-friendly step-by-step game programming tutorial in German (Levels 0–9, Spickzettel, and Appendix).
* 📑 **[ANLEITUNG.pdf](ANLEITUNG.pdf)**: Formatted, printable DIN A4 PDF version of the complete tutorial.
* 🌐 **[ANLEITUNG.html](ANLEITUNG.html)**: Interactive web version with print support.

---

## 🚀 Quick Start & Building

### Prerequisites (macOS)
```bash
brew install sfml
```

### Build & Run
```bash
# Build dynamic binary (quick development)
make
./test

# Build static zero-dependency binary
make static
./test_static

# Build standalone macOS Application bundle (MTG.app)
make bundle
open MTG.app
```

---

## 📜 License
GPL-3.0 (see [LICENSE](LICENSE)).
