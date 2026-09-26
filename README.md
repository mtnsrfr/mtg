# 🎮 MTG Retro Gaming Engine

A retro 2D game engine built in modern C++ (SFML 3) designed for kids and beginners to learn game programming from **Zero to Hero**.

---

## 🕹️ Included Games

1. **🏓 2-Player Pong (`PongGame.hpp`)**:
   * Dynamic paddle bounce angles, increasing ball speed, score tracker, and retro sound effects.
   * *Controls*: Player 1 (`W`/`S`), Player 2 (`Up`/`Down`), Start (`Space`).

2. **🐍 2-Player Snake Battle (`SnakeGame.hpp`)**:
   * 40x30 tile grid with collectable apples (Goal: 100 apples to win!).
   * **Model B: Apple Theft**: When a snake crashes, its body bursts into collectable apples for the opponent to steal!
   * *Controls*: Player 1 (`W`/`A`/`S`/`D`), Player 2 (`Arrow keys`).

3. **🚀 Starfighter Attack (`StarfighterGame.hpp`)**:
   * 3-layer parallax starfield, 16x16 pixel-art delta fighters and TIE attackers.
   * Player laser overheating mechanic, charged mega-laser shield breaker, and enemy homing missiles.
   * Directional momentum particle explosions with thermal color decay (White $\rightarrow$ Yellow $\rightarrow$ Orange $\rightarrow$ Red $\rightarrow$ Dark Gray).
   * *Controls*: Movement (`W`/`A`/`S`/`D` or `Arrow keys`), Shoot (`Space`), Charge Mega-Laser (Hold & Release `Space`).

4. **🦄 The Unicorn (`UnicornGame.hpp`)**:
   * Multi-level retro runner (Level 1 Sunny Meadow $\rightarrow$ Level 2 Magic Night Sky $\rightarrow$ Victory!).
   * Choose your unicorn color: **Blue**, **White**, or **Pink**.
   * Touch ground rainbow arches for trampoline boosts and stomp falling meteors.
   * *Controls*: Jump / Double Jump (`Space` / `W` / `Up`), Color Choice (`1`/`2`/`3`).

5. **💥 Tank Duel: Retro Artillery (`TankGame.hpp`)**:
   * Turn-based 2-player tank artillery combat with destructible procedural terrain.
   * Cellular automata terrain generation simulation (Rock lines $\rightarrow$ Grey Noise $\rightarrow$ Dirt avalanche $\rightarrow$ Grass smoothing).
   * Ballistic physics with angle control shown in both **Degrees** and **Radians** ($0^\circ \sim 180^\circ$, starting at $45^\circ$ and $135^\circ$), power adjustments (`0` to `99`, starting at `50`), and crater-forming explosions.
   * *Controls*: Angle (`Left`/`Right`), Power (`Up`/`Down`), Fire (`Space`/`Enter`), New Terrain (`R`).

6. **⚡ Darios Spiel: Elemental Pixel Brawl (`DarioGame.hpp`)**:
   * Dedicated 2-player local PvP arena fighting game with 6 unique champions:
     * 🎸 **Musik-Kämpfer (Dario)**: E-Gitarre, Drachen-Klangwelle, rote Haare, schwarze Augenringe.
     * ❄️ **Eis-Kämpfer (Frost)**: Blau-Weiß, Eiskristalle am Körper und Arm.
     * 🔥 **Feuer-Kämpfer (Ignis)**: Lodernde Feuerfrisur, Feuer am Arm, Inferno-Feuerbälle & brennender Boden.
     * ⚡ **Blitz-Kämpfer (Volt)**: Strahlende Blitz-Augen, elektrisierende Funken, Donnerkeil.
     * 💧 **Wasser-Kämpfer (Aqua)**: Blaue Haare/Augen, Ninja-Maske, Tsunami-Hydro-Welle & Wasserpfützen.
     * 💻 **Hacker-Kämpfer (Byte)**: Cyber-Hoodie, Visor, tippt auf dem Laptop und feuert fliegende ASCII-Code Buchstaben & Matrix-Salven mit extrem hohem Schaden!
   * Rolling 2D heightmap terrain with one-way floating battle platforms.
   * Best of 3 rounds format.
   * *Controls (4 keys per player)*:
     * Player 1: `W` (Jump), `A`/`D` (Move), `Away+S` (Block Shield), `Towards+S` (Superpower Attack).
     * Player 2: `Up` (Jump), `Left`/`Right` (Move), `Away+Down` (Block Shield), `Towards+Down` (Superpower Attack).

7. **🎯 Multi-Game Launcher (`MenuGame.hpp`)**:
   * Switch between games instantly with keys `1`, `2`, `3`, `4`, `5`, and `6`.
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
./run_the_games

# Build static zero-dependency binary
make static
./run_the_games_static

# Build standalone macOS Application bundle (MTG.app)
make bundle
open MTG.app
```

---

## 📜 License
GPL-3.0 (see [LICENSE](LICENSE)).
