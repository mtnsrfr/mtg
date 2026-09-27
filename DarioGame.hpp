#pragma once

#include "Engine.hpp"
#include "FluidSolver.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
// DARIOS SPIEL - ELEMENTAL PIXEL BRAWL (2-Player Local PvP Fighting Arena)
// =============================================================================
// 6 Einzigartige Elementar- & Cyber-Kämpfer nach Darios Vision:
// 1. 🎸 Musik-Kämpfer (Dario): E-Gitarre, Drachen-Klangwelle, rote Haare,
// schwarze Augenringe
// 2. ❄️ Eis-Kämpfer: Blau-Weiß, Eisstücke & Kristalle am Körper und Arm
// 3. 🔥 Feuer-Kämpfer: Feuer am Arm, lodernde Feuerfrisur, Inferno-Feuerbälle
// 4. ⚡ Blitz-Kämpfer: Blitz-Augen, elektrisierende Funken um Körper & Arm
// 5. 💧 Wasser-Kämpfer: Blaue Haare & Augen, Ninja-Mundmaske, Hydro-Tsunami
// 6. 💻 Hacker-Kämpfer (Byte): Cyber-Laptop, Cyber-Visor, schießt fliegende
// Buchstaben & Matrix-Code!
//
// Steuerung (Genau 4 Tasten pro Spieler!):
// - Spieler 1: W (Sprung), A / D (Laufen), Weg+Runter (Block), Hin+Runter
// (Superkraft)
// - Spieler 2: Up (Sprung), Left / Right (Laufen), Weg+Runter (Block),
// Hin+Runter (Superkraft)
// =============================================================================

struct DarioGame : Game {
public:
  enum class FighterType {
    Music = 0,     // Musik-Kämpfer (Dario)
    Ice = 1,       // Eis-Kämpfer
    Fire = 2,      // Feuer-Kämpfer
    Lightning = 3, // Blitz-Kämpfer
    Water = 4,     // Wasser-Kämpfer
    Hacker = 5     // Hacker-Kämpfer (Byte)
  };

  enum class State {
    CharacterSelect,
    RoundIntro,
    Fighting,
    RoundOver,
    MatchOver
  };

  enum class ShotMode {
    Straight,  // Vorwärts, Vorwärts (0 Grad geradeaus)
    UpDiag45,  // Vorwärts, dann Oben (45 Grad schräg nach oben)
    DownGround // Vorwärts + Runter (45 Grad nach unten -> Boden-Schockwelle)
  };

  struct FighterDef {
    FighterType type;
    std::string name;
    std::string title;
    std::string powerName;
    uint8_t mainColor;
    uint8_t accentColor;
    float cooldown;
    int damage;
    float speed;
  };

  struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    uint8_t color;
    float size;
  };

  struct Projectile {
    int owner; // 0 = P1, 1 = P2
    FighterType type;
    float x, y;
    float vx, vy;
    float life;
    float radius;
    int damage;
    bool active;
    bool isAirToGround = false;
    bool isGroundWave = false;
  };

  struct FloatingPlatform {
    float x1, x2;
    float y;
  };

  struct GroundFire {
    float x;
    float y;
    float life;
    float maxLife;
    int owner;
  };

  struct WaterPuddle {
    float x;
    float y;
    float life;
    float maxLife;
    int owner;
  };

  struct Supernova {
    float x, y;
    float life;
    float maxLife;
    float maxRadius;
    uint8_t coreColor;
    uint8_t ringColor;
    uint8_t rayColor;
    bool isMega;
  };

  enum class FluidElementType : uint8_t {
    Water, // Aqua-Wasser: schwere Tropfen, fließen Berghänge hinab, verlangsamen Gegner
    Fire,  // Ignis-Feuer: lodernde Funken, steigen durch Auftrieb auf, brennen den Boden an
    Wind,  // Sturm-Wirbel: extrem schnelle Partikel & Geschwindigkeits-Vektorlinien
    Steam  // Wasserdampf: entsteht wenn Feuer und Wasser aufeinandertreffen -> Explosions-Druckwelle
  };

  struct FluidParticle {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 1.5f;
    FluidElementType type = FluidElementType::Water;
    int owner = -1; // 0 = P1, 1 = P2, -1 = Neutral
    bool active = true;
  };

  struct Player {
    int id; // 0 oder 1
    FighterType fighter;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    int facing = 1; // +1 = Rechts, -1 = Links
    bool isGrounded = false;
    bool onPlatform = false;
    bool canDoubleJump = true;
    bool isFlying = false;

    int hp = 100;
    int maxHp = 100;
    int roundsWon = 0;

    bool isBlocking = false;
    float blockTimer = 0.0f;    // Zeit wie lange Schild aktiv ist (max 1.0s)
    float blockCooldown = 0.0f; // Erholungszeit nach Schild-Ablauf
    float attackCooldown = 0.0f;
    float maxCooldown = 0.8f;
    float hurtTimer = 0.0f;
    float animFrame = 0.0f;
    float forwardTapTimer = 0.0f;
    int forwardTapCount = 0;
    bool prevMoveTowards = false;
    bool prevUp = false;
    float slowTimer = 0.0f; // Verlangsamung im Wasser (Wasser-Schockwelle)
  };

  // ===========================================================================
  // 1. SPIEL-VARIABLEN
  // ===========================================================================
  State state = State::CharacterSelect;
  Player players[2];
  std::vector<Projectile> projectiles;
  std::vector<Particle> particles;
  std::vector<FloatingPlatform> platforms;
  std::vector<GroundFire> groundFires;
  std::vector<WaterPuddle> waterPuddles;
  std::vector<Supernova> supernovas;

  // Navier-Stokes Fluid-Simulation & Elementar-Partikel
  FluidSolver2D fluidSolver;
  std::vector<FluidParticle> fluidParticles;
  float p1FluidDmgTimer = 0.0f;
  float p2FluidDmgTimer = 0.0f;

  int p1Cursor = 0;
  int p2Cursor = 1;
  bool p1Locked = false;
  bool p2Locked = false;

  int currentRound = 1;
  int matchWinner = -1; // 0 = P1, 1 = P2

  float roundStateTimer = 0.0f;
  float globalTimer = 0.0f;
  float screenShake = 0.0f;

  // Hügel-Terrain Höhenstützpunkte
  static const int NUM_TERRAIN_POINTS = 9;
  int terrainPts[NUM_TERRAIN_POINTS];

  // ===========================================================================
  // 2. KÄMPFER-DATENBANK
  // ===========================================================================
  FighterDef getFighterDef(FighterType type) const {
    switch (type) {
    case FighterType::Music:
      return {FighterType::Music,
              "DARIO",
              "DRACHEN-REITER",
              "DRACHEN-FLUG & RIFF",
              Ramps::Gold[11],
              Ramps::Red[10],
              0.60f,
              32,
              130.0f};
    case FighterType::Ice:
      return {FighterType::Ice,
              "FROST",
              "EIS-KRIEGER",
              "EISKRISTALLE",
              Ramps::SkyBlue[10],
              Colors::White,
              0.70f,
              18,
              105.0f};
    case FighterType::Fire:
      return {FighterType::Fire,
              "IGNIS",
              "FEUER-KRIEGER",
              "INFERNO-BALL",
              Ramps::Fire[8],
              Ramps::Fire[12],
              0.80f,
              25,
              120.0f};
    case FighterType::Lightning:
      return {FighterType::Lightning,
              "VOLT",
              "BLITZ-FALKE",
              "DONNER-SCHOCK",
              Ramps::Gold[11],
              Ramps::Yellow[14],
              0.60f,
              16,
              135.0f};
    case FighterType::Water:
      return {FighterType::Water,
              "AQUA",
              "WASSER-NINJA",
              "TSUNAMI-WELLE",
              Ramps::Cyan[10],
              Ramps::Blue[12],
              0.75f,
              20,
              110.0f};
    case FighterType::Hacker:
      return {FighterType::Hacker,
              "BYTE",
              "CYBER-HACKER",
              "ASCII-SALVE",
              Ramps::Green[13],
              Ramps::Cyan[13],
              0.60f,
              20,
              128.0f};
    }
    return {FighterType::Music, "DARIO", "MUSIK", "KLANG", Ramps::Red[9],
            Ramps::Gold[11],    0.8f,    20,      110.0f};
  }

  // ===========================================================================
  // 3. KONSTRUKTOR & RESET
  // ===========================================================================
  DarioGame() {
    players[0].id = 0;
    players[1].id = 1;
    initArena();
  }

  void initArena() {
    // 1. Sanfte hügelige Landschaft generieren
    for (int i = 0; i < NUM_TERRAIN_POINTS; ++i) {
      if (i == 0 || i == NUM_TERRAIN_POINTS - 1) {
        terrainPts[i] = 195;
      } else if (i == 2 || i == 6) {
        terrainPts[i] = 185;
      } else if (i == 4) {
        terrainPts[i] = 175; // Mittlerer sanfter Hügel
      } else {
        terrainPts[i] = 190;
      }
    }

    // 2. Zwei schwebende Kampf-Plattformen
    platforms.clear();
    platforms.push_back({45.0f, 115.0f, 142.0f});  // Linke Plattform
    platforms.push_back({205.0f, 275.0f, 142.0f}); // Rechte Plattform

    // Navier-Stokes Fluid-Maske initialisieren (Boden & Plattformen)
    fluidSolver.updateSolidMask([this](float x) { return getGroundHeight(x); }, platforms);
  }

  float getGroundHeight(float x) const {
    x = std::clamp(x, 0.0f, 319.0f);
    float segWidth = 320.0f / (NUM_TERRAIN_POINTS - 1);
    int seg = static_cast<int>(x / segWidth);
    seg = std::clamp(seg, 0, NUM_TERRAIN_POINTS - 2);

    float t = (x - seg * segWidth) / segWidth;
    // Weiche Kosinus-Glättung
    float smoothT = (1.0f - std::cos(t * 3.14159265f)) * 0.5f;
    return terrainPts[seg] + smoothT * (terrainPts[seg + 1] - terrainPts[seg]);
  }

  void startNewMatch() {
    players[0].roundsWon = 0;
    players[1].roundsWon = 0;
    currentRound = 1;
    matchWinner = -1;
    startNewRound();
  }

  void startNewRound() {
    players[0].fighter = static_cast<FighterType>(p1Cursor);
    players[1].fighter = static_cast<FighterType>(p2Cursor);

    FighterDef d0 = getFighterDef(players[0].fighter);
    FighterDef d1 = getFighterDef(players[1].fighter);

    players[0].maxHp = (players[0].fighter == FighterType::Music) ? 120 : 100;
    players[0].hp = players[0].maxHp;
    players[0].x = 60.0f;
    players[0].y = getGroundHeight(60.0f);
    players[0].vx = 0.0f;
    players[0].vy = 0.0f;
    players[0].facing = 1; // Schaut nach rechts
    players[0].isBlocking = false;
    players[0].blockTimer = 0.0f;
    players[0].blockCooldown = 0.0f;
    players[0].attackCooldown = 0.0f;
    players[0].maxCooldown = d0.cooldown;
    players[0].hurtTimer = 0.0f;
    players[0].forwardTapTimer = 0.0f;
    players[0].forwardTapCount = 0;
    players[0].prevMoveTowards = false;
    players[0].prevUp = false;
    players[0].slowTimer = 0.0f;

    players[1].maxHp = (players[1].fighter == FighterType::Music) ? 120 : 100;
    players[1].hp = players[1].maxHp;
    players[1].x = 260.0f;
    players[1].y = getGroundHeight(260.0f);
    players[1].vx = 0.0f;
    players[1].vy = 0.0f;
    players[1].facing = -1; // Schaut nach links
    players[1].isBlocking = false;
    players[1].blockTimer = 0.0f;
    players[1].blockCooldown = 0.0f;
    players[1].attackCooldown = 0.0f;
    players[1].maxCooldown = d1.cooldown;
    players[1].hurtTimer = 0.0f;
    players[1].forwardTapTimer = 0.0f;
    players[1].forwardTapCount = 0;
    players[1].prevMoveTowards = false;
    players[1].prevUp = false;
    players[1].slowTimer = 0.0f;

    projectiles.clear();
    particles.clear();
    groundFires.clear();
    waterPuddles.clear();
    supernovas.clear();
    fluidParticles.clear();
    fluidSolver.reset();
    fluidSolver.updateSolidMask([this](float x) { return getGroundHeight(x); }, platforms);
    p1FluidDmgTimer = 0.0f;
    p2FluidDmgTimer = 0.0f;
    roundStateTimer = 0.0f;
    state = State::RoundIntro;
  }

  // ===========================================================================
  // 4. HAUPT-UPDATE LOOP
  // ===========================================================================
  void update(Engine &e) override {
    float dt = e.dt();
    globalTimer += dt;

    if (screenShake > 0.0f) {
      screenShake = std::max(0.0f, screenShake - dt * 2.5f);
    }

    switch (state) {
    case State::CharacterSelect:
      updateCharacterSelect(e);
      break;
    case State::RoundIntro:
      updateRoundIntro(e);
      break;
    case State::Fighting:
      updateFighting(e);
      break;
    case State::RoundOver:
      updateRoundOver(e);
      break;
    case State::MatchOver:
      updateMatchOver(e);
      break;
    }

    updateParticles(dt);

    // Szene & UI rendern
    render(e);
  }

  // ===========================================================================
  // 5. CHARAKTER-AUSWAHL
  // ===========================================================================
  void updateCharacterSelect(Engine &e) {
    // Spieler 1 Steuerung: A / D zum Wählen, W zum Einloggen
    if (!p1Locked) {
      if (e.pressed(Key::A)) {
        p1Cursor = (p1Cursor + 5) % 6;
        playMenuBeep(e);
      }
      if (e.pressed(Key::D)) {
        p1Cursor = (p1Cursor + 1) % 6;
        playMenuBeep(e);
      }
      if (e.pressed(Key::W) || e.pressed(Key::Space)) {
        p1Locked = true;
        playLockSound(e);
      }
    } else {
      if (e.pressed(Key::S)) {
        p1Locked = false;
      }
    }

    // Spieler 2 Steuerung: Left / Right zum Wählen, Up zum Einloggen
    if (!p2Locked) {
      if (e.pressed(Key::Left)) {
        p2Cursor = (p2Cursor + 5) % 6;
        playMenuBeep(e);
      }
      if (e.pressed(Key::Right)) {
        p2Cursor = (p2Cursor + 1) % 6;
        playMenuBeep(e);
      }
      if (e.pressed(Key::Up) || e.pressed(Key::Enter)) {
        p2Locked = true;
        playLockSound(e);
      }
    } else {
      if (e.pressed(Key::Down)) {
        p2Locked = false;
      }
    }

    // Wenn beide Spieler eingeloggt sind: Starte den Kampf!
    if (p1Locked && p2Locked) {
      roundStateTimer += e.dt();
      if (roundStateTimer > 0.6f) {
        startNewMatch();
      }
    } else {
      roundStateTimer = 0.0f;
    }
  }

  // ===========================================================================
  // 6. RUNDEN-INTRO & KAMPF-ABLAUF
  // ===========================================================================
  void updateRoundIntro(Engine &e) {
    roundStateTimer += e.dt();
    if (roundStateTimer >= 1.4f) {
      state = State::Fighting;
      playFightGong(e);
    }
  }

  void updateFighting(Engine &e) {
    float dt = e.dt();

    p1FluidDmgTimer = std::max(0.0f, p1FluidDmgTimer - dt);
    p2FluidDmgTimer = std::max(0.0f, p2FluidDmgTimer - dt);

    // 1. Spieler 1 Eingaben (WASD)
    processPlayerInput(0, e.key(Key::A), e.key(Key::D), e.key(Key::W),
                       e.key(Key::S), e.pressed(Key::W), e);

    // 2. Spieler 2 Eingaben (Pfeiltasten)
    processPlayerInput(1, e.key(Key::Left), e.key(Key::Right), e.key(Key::Up),
                       e.key(Key::Down), e.pressed(Key::Up), e);

    // 3. Physik & Geländekollision für beide Spieler
    for (int i = 0; i < 2; ++i) {
      updatePlayerPhysics(players[i], dt);
      // Aerodynamischer Nachlauf: Spieler erzeugen Wirbelschleppen im Fluid-Feld!
      if (std::abs(players[i].vx) > 10.0f || std::abs(players[i].vy) > 10.0f) {
        fluidSolver.addVelocity(players[i].x, players[i].y - 8.0f,
                                players[i].vx * 0.35f, players[i].vy * 0.35f, 9.0f);
      }
    }

    // 4. Automatische Blickrichtung zum Gegner
    if (players[0].x < players[1].x - 4.0f) {
      players[0].facing = 1;
      players[1].facing = -1;
    } else if (players[0].x > players[1].x + 4.0f) {
      players[0].facing = -1;
      players[1].facing = 1;
    }

    // 5. Navier-Stokes Fluid, Projektile, brennenden Boden, Wasserpfützen & Supernovas aktualisieren
    fluidSolver.step(dt);
    updateFluidParticles(e, dt);
    updateProjectiles(e, dt);
    updateGroundFires(e, dt);
    updateWaterPuddles(e, dt);
    updateSupernovas(dt);

    // 6. Rundenende prüfen (K.O.)
    if (players[0].hp <= 0 || players[1].hp <= 0) {
      if (players[0].hp <= 0 && players[1].hp <= 0) {
        // Unentschieden (beide bekommen einen Punkt)
        players[0].roundsWon++;
        players[1].roundsWon++;
        spawnSupernova(players[0].x, players[0].y - 10.0f, players[0].fighter,
                       /*isMega=*/true);
        spawnSupernova(players[1].x, players[1].y - 10.0f, players[1].fighter,
                       /*isMega=*/true);
      } else if (players[0].hp <= 0) {
        players[1].roundsWon++;
        spawnSupernova(players[0].x, players[0].y - 10.0f, players[0].fighter,
                       /*isMega=*/true);
      } else {
        players[0].roundsWon++;
        spawnSupernova(players[1].x, players[1].y - 10.0f, players[1].fighter,
                       /*isMega=*/true);
      }

      screenShake = 1.0f;
      playKoSound(e);
      roundStateTimer = 0.0f;
      state = State::RoundOver;
    }
  }

  void processPlayerInput(int pid, bool left, bool right, bool up, bool down,
                          bool jumpPressed, Engine &e) {
    Player &p = players[pid];
    FighterDef def = getFighterDef(p.fighter);

    if (p.hurtTimer > 0.0f) {
      p.hurtTimer -= e.dt();
      p.isBlocking = false;
      return;
    }

    if (p.attackCooldown > 0.0f) {
      p.attackCooldown -= e.dt();
    }
    if (p.blockCooldown > 0.0f) {
      p.blockCooldown -= e.dt();
    }
    if (p.slowTimer > 0.0f) {
      p.slowTimer -= e.dt();
    }

    // Ermittle relative Richtungen bezogen auf die Blickrichtung zum Gegner
    bool moveTowards = (p.facing > 0) ? right : left;
    bool moveAway = (p.facing > 0) ? left : right;

    bool justPressedTowards = (moveTowards && !p.prevMoveTowards);
    p.prevMoveTowards = moveTowards;

    bool justPressedUp = ((up || jumpPressed) && !p.prevUp);
    p.prevUp = (up || jumpPressed);

    if (p.forwardTapTimer > 0.0f) {
      p.forwardTapTimer -= e.dt();
      if (p.forwardTapTimer <= 0.0f) {
        p.forwardTapCount = 0;
      }
    }

    bool inAir = (!p.isGrounded && !p.onPlatform);

    // --- 1. "VORNE DANN OBEN" = SPRUNG & 45 GRAD SCHUSS NACH OBEN (JUMPING
    // ANTI-AIR!) ---
    if (justPressedUp && p.forwardTapTimer > 0.0f && p.forwardTapCount >= 1 &&
        !down) {
      if (p.attackCooldown <= 0.0f) {
        // Spieler springt dynamisch in die Luft!
        if (p.isGrounded || p.onPlatform) {
          p.vy = -195.0f;
          p.vx = p.facing * 115.0f;
          p.isGrounded = false;
          p.onPlatform = false;
          p.canDoubleJump = true;
          playJumpSound(e);
          spawnJumpDust(p.x, p.y);
        } else {
          p.vy = -165.0f; // Extra-Sprungimpuls in der Luft
          p.vx = p.facing * 115.0f;
        }

        fireSuperpower(pid, e, ShotMode::UpDiag45);
        p.attackCooldown = p.maxCooldown;
        p.forwardTapCount = 0;
        p.forwardTapTimer = 0.0f;
        spawnDoubleJumpSparkle(p.x + p.facing * 10.0f, p.y - 12.0f,
                               def.accentColor);
      }
      return; // Sprung & Schuss ausgeführt
    }

    // --- 2. "FORWARD FORWARD" = GERADEAUS-SCHUSS (AM BODEN & IN DER LUFT) ---
    if (justPressedTowards && !down && !up) {
      if (p.forwardTapTimer > 0.0f && p.forwardTapCount >= 1) {
        // Doppel-Tipp Vorwärts -> Feuert schnurgerade nach vorne!
        if (p.attackCooldown <= 0.0f) {
          fireSuperpower(pid, e, ShotMode::Straight);
          p.attackCooldown = p.maxCooldown;
          p.forwardTapCount = 0;
          p.forwardTapTimer = 0.0f;
          spawnDoubleJumpSparkle(p.x + p.facing * 10.0f, p.y - 8.0f,
                                 def.accentColor);
        }
      } else {
        p.forwardTapCount = 1;
        p.forwardTapTimer =
            0.32f; // 0.32s Zeitfenster für Folgeeingaben (Vorwärts oder Oben)
      }
    }

    // --- 3. BLOCKEN: "Weg vom Gegner + Runter" (z.B. A + S wenn nach rechts
    // geblickt wird) --- Schild hält maximal 1.0 Sekunde -> erfordert präzises
    // Reaktions-Timing!
    if (moveAway && down && p.blockCooldown <= 0.0f && p.blockTimer < 1.0f) {
      p.isBlocking = true;
      p.blockTimer += e.dt();
      p.vx = 0.0f;

      // Schildpartikel spawnen & schützender Luft-Gegenstrom
      fluidSolver.addVelocity(p.x + p.facing * 10.0f, p.y - 8.0f, p.facing * 90.0f, 0.0f, 12.0f);
      if (e.rnd(3) == 0) {
        spawnShieldParticles(p.x + p.facing * 8.0f, p.y - 8.0f,
                             def.accentColor);
      }

      // Nach genau 1.0 Sekunde bricht das Schild zusammen und geht in 0.75s
      // Cooldown!
      if (p.blockTimer >= 1.0f) {
        p.isBlocking = false;
        p.blockCooldown = 0.75f;
        spawnBlockSparks(p.x + p.facing * 8.0f, p.y - 8.0f);
        playBlockSound(e);
      }
      return; // Während des Blockens kann man nicht gleichzeitig angreifen oder
              // rennen
    } else {
      p.isBlocking = false;
      // Wenn man Block loslässt, lädt sich die Schilddauer schnell wieder auf
      if (!moveAway || !down) {
        p.blockTimer = std::max(0.0f, p.blockTimer - e.dt() * 2.5f);
      }
    }

    // --- 4. BODEN-SCHOCKWELLE: "Hin zum Gegner + Runter" (z.B. D + S) ---
    if (moveTowards && down) {
      if (p.attackCooldown <= 0.0f) {
        // Wenn man am Boden ist und zusätzlich Sprung drückt:
        // Vorwärts-Sprungangriff nach unten!
        if (!inAir && (up || jumpPressed)) {
          p.vy = -195.0f;
          p.vx = p.facing * 140.0f;
          p.isGrounded = false;
          p.onPlatform = false;
          p.canDoubleJump = true;
          inAir = true;
          playJumpSound(e);
          spawnJumpDust(p.x, p.y);
        }
        fireSuperpower(pid, e, ShotMode::DownGround);
        p.attackCooldown = p.maxCooldown;
      }
      if (p.isGrounded || p.onPlatform) {
        p.vx = 0.0f;
      }
      return;
    }

    // --- C. DURCH PLATTFORM FALLEN: "Nur Runter" während man auf einer
    // Plattform steht ---
    if (down && p.onPlatform) {
      p.y += 3.0f;
      p.onPlatform = false;
      p.isGrounded = false;
      return;
    }

    // --- D. SPRINGEN & DRACHEN-FLUG (W bei P1 / Up bei P2) ---
    bool isMusicFighter = (p.fighter == FighterType::Music);
    p.isFlying = false;

    // DARIO KANN FLIEGEN: Wenn man in der Luft W / Up hält, gleitet & schwebt
    // der Drache!
    if (isMusicFighter && !p.isGrounded && !p.onPlatform && up) {
      p.isFlying = true;
      if (p.vy > 35.0f)
        p.vy = 35.0f; // Sanftes Schweben/Gleiten im Wind
      if (e.rnd(3) == 0) {
        spawnDoubleJumpSparkle(p.x - p.facing * 4.0f, p.y - 6.0f,
                               Ramps::Gold[12]);
      }
    }

    if (jumpPressed) {
      float jForce = (p.slowTimer > 0.0f) ? -165.0f : -215.0f;
      if (p.isGrounded || p.onPlatform) {
        p.vy = jForce;
        p.isGrounded = false;
        p.onPlatform = false;
        p.canDoubleJump = true;
        playJumpSound(e);
        spawnJumpDust(p.x, p.y);
        // Abwind-Impuls in die Fluid-Simulation
        fluidSolver.addVelocity(p.x, p.y - 4.0f, 0.0f, 160.0f, 12.0f);
        spawnWindParticles(p.x, p.y - 2.0f, 0.0f, 50.0f, pid, 6);
      } else if (isMusicFighter) {
        // DARIO KANN ENDLOS WEITERFLATTERN & DURCH FLÜGELSCHLÄGE AUFSTEIGEN!
        p.vy = -185.0f;
        p.isFlying = true;
        playJumpSound(e);
        spawnDoubleJumpSparkle(p.x - p.facing * 5.0f, p.y - 8.0f,
                               Ramps::Gold[14]);
        fluidSolver.addVelocity(p.x, p.y - 6.0f, p.facing * 90.0f, 160.0f, 14.0f);
        spawnWindParticles(p.x, p.y - 4.0f, p.facing * 70.0f, 90.0f, pid, 8);
      } else if (p.canDoubleJump) {
        p.vy = (p.slowTimer > 0.0f) ? -150.0f : -185.0f;
        p.canDoubleJump = false;
        playJumpSound(e);
        spawnDoubleJumpSparkle(p.x, p.y, def.mainColor);
        // Wirbelsturm-Doppelsprung
        fluidSolver.addVelocity(p.x, p.y - 6.0f, p.facing * 110.0f, 190.0f, 14.0f);
        spawnWindParticles(p.x, p.y - 4.0f, p.facing * 80.0f, 110.0f, pid, 10);
        playWindWhooshSound(e);
      }
    }

    // --- E. LAUFEN / FLIEGEN LINKS & RECHTS ---
    float moveSpd = def.speed;
    if (p.slowTimer > 0.0f) {
      moveSpd *= 0.45f; // Stark verlangsamt im Wasser!
    }
    if (left && !right) {
      p.vx = -moveSpd;
    } else if (right && !left) {
      p.vx = moveSpd;
    } else {
      p.vx = 0.0f;
    }
  }

  // ===========================================================================
  // 7. PHYSIK & 3-PUNKT BODEN-KOLLISION
  // ===========================================================================
  void updatePlayerPhysics(Player &p, float dt) {
    // Schwerkraft (für Dario im Drachenflug sanft gedämpft)
    float grav =
        (p.fighter == FighterType::Music && p.isFlying) ? 140.0f : 480.0f;
    p.vy += grav * dt;
    if (p.vy > 350.0f)
      p.vy = 350.0f;

    float prevY = p.y;
    p.x += p.vx * dt;
    p.y += p.vy * dt;

    // Obere Deckenbegrenzung (damit der Drache nicht über das HUD hinausfliegt)
    if (p.y < 30.0f) {
      p.y = 30.0f;
      if (p.vy < 0.0f)
        p.vy = 0.0f;
    }

    // Arena-Wände (Links/Rechts Begrenzung)
    p.x = std::clamp(p.x, 14.0f, 306.0f);

    // 1. Kollision mit schwebenden Einweg-Plattformen
    p.onPlatform = false;
    if (p.vy >= 0.0f) { // Nur beim Herabfallen
      for (const auto &plat : platforms) {
        if (p.x >= plat.x1 - 4.0f && p.x <= plat.x2 + 4.0f) {
          float feetPrev = prevY;
          float feetCur = p.y;
          if (feetPrev <= plat.y + 3.0f && feetCur >= plat.y) {
            p.y = plat.y;
            p.vy = 0.0f;
            p.isGrounded = true;
            p.onPlatform = true;
            p.canDoubleJump = true;
            break;
          }
        }
      }
    }

    // 2. 3-Punkt Bodenkollision mit hügeligem Terrain (Linker Fuß, Mitte,
    // Rechter Fuß)
    if (!p.onPlatform) {
      float groundLeft = getGroundHeight(p.x - 4.0f);
      float groundMid = getGroundHeight(p.x);
      float groundRight = getGroundHeight(p.x + 4.0f);
      float effectiveGround = std::min({groundLeft, groundMid, groundRight});

      if (p.y >= effectiveGround) {
        p.y = effectiveGround;
        p.vy = 0.0f;
        p.isGrounded = true;
        p.canDoubleJump = true;
      } else {
        p.isGrounded = false;
      }
    }

    // Lauf-Animation
    if (p.isGrounded && std::abs(p.vx) > 5.0f) {
      p.animFrame += dt * 10.0f;
    } else {
      p.animFrame = 0.0f;
    }
  }

  // ===========================================================================
  // 8. ELEMENTAR-SUPERKRÄFTE & PROJEKTILE
  // ===========================================================================
  void fireSuperpower(int pid, Engine &e, ShotMode mode) {
    Player &p = players[pid];
    FighterDef def = getFighterDef(p.fighter);

    Projectile proj;
    proj.owner = pid;
    proj.type = p.fighter;
    proj.damage = def.damage;
    proj.life = 2.2f;
    proj.active = true;
    proj.isAirToGround = (mode == ShotMode::DownGround);
    proj.isGroundWave = false;

    float baseSpd = 165.0f;

    if (mode == ShotMode::UpDiag45) {
      // --- 45 GRAD SCHUSS NACH OBEN (VORNE DANN OBEN: ANTI-AIR!) ---
      proj.x = p.x + p.facing * 12.0f;
      proj.y = p.y - 12.0f;
      proj.vx = p.facing * (baseSpd + 20.0f) * 0.707f;
      proj.vy = -(baseSpd + 20.0f) * 0.707f; // Exakt 45 Grad steil nach oben!
      proj.radius = 6.0f;

      switch (p.fighter) {
      case FighterType::Music: // Drachenfeuer 45° nach oben
        proj.x = p.x + p.facing * 16.0f;
        proj.vx = p.facing * (baseSpd + 45.0f) * 0.707f;
        proj.vy = -(baseSpd + 45.0f) * 0.707f;
        proj.radius = 8.5f;
        proj.damage = 32;
        playGuitarDragonSound(e);
        break;
      case FighterType::Ice:
        playIceCastSound(e);
        break;
      case FighterType::Fire:
        playFireCastSound(e);
        spawnFireStream(p.x + p.facing * 12.0f, p.y - 12.0f, p.facing * 200.0f, -200.0f, pid, 28);
        fluidSolver.addVelocity(p.x + p.facing * 14.0f, p.y - 12.0f, p.facing * 210.0f, -210.0f, 16.0f);
        fluidSolver.addBuoyancy(p.x + p.facing * 14.0f, p.y - 12.0f, 160.0f, 16.0f);
        break;
      case FighterType::Lightning:
        proj.vx = p.facing * (baseSpd + 75.0f) * 0.707f;
        proj.vy = -(baseSpd + 75.0f) * 0.707f;
        playThunderSound(e);
        break;
      case FighterType::Water:
        playWaterSplashSound(e);
        spawnWaterStream(p.x + p.facing * 12.0f, p.y - 12.0f, p.facing * 210.0f, -210.0f, pid, 36);
        fluidSolver.addVelocity(p.x + p.facing * 14.0f, p.y - 12.0f, p.facing * 230.0f, -230.0f, 18.0f);
        break;
      case FighterType::Hacker:
        proj.vx = p.facing * (baseSpd + 40.0f) * 0.707f;
        proj.vy = -(baseSpd + 40.0f) * 0.707f;
        proj.radius = 7.5f;
        proj.damage = 22; // Ausbalancierter Cyber-Schuss (22 HP)
        playHackerCastSound(e);
        break;
      }
    } else if (mode == ShotMode::DownGround) {
      // --- 45 GRAD SCHUSS NACH UNTEN (BODEN-SCHOCKWELLE) ---
      proj.x = p.x + p.facing * 12.0f;
      proj.y = p.y - 12.0f;
      proj.vx = p.facing * (baseSpd + 20.0f) * 0.707f;
      proj.vy = (baseSpd + 20.0f) * 0.707f; // Schießt schräg abwärts!
      proj.radius = 6.5f;

      switch (p.fighter) {
      case FighterType::Music: // Drachen-Odem Sturzflug-Angriff
        proj.x = p.x + p.facing * 16.0f;
        proj.vx = p.facing * (baseSpd + 45.0f) * 0.707f;
        proj.vy = (baseSpd + 45.0f) * 0.707f;
        proj.radius = 8.5f;
        proj.damage = 32; // DER STÄRKSTE!
        playGuitarDragonSound(e);
        break;
      case FighterType::Ice:
        playIceCastSound(e);
        break;
      case FighterType::Fire:
        playFireCastSound(e);
        spawnFireStream(p.x + p.facing * 12.0f, p.y - 10.0f, p.facing * 200.0f, 140.0f, pid, 28);
        fluidSolver.addVelocity(p.x + p.facing * 14.0f, p.y - 10.0f, p.facing * 210.0f, 130.0f, 16.0f);
        fluidSolver.addBuoyancy(p.x + p.facing * 14.0f, p.y - 10.0f, 140.0f, 16.0f);
        break;
      case FighterType::Lightning:
        proj.vx = p.facing * (baseSpd + 75.0f) * 0.707f;
        proj.vy = (baseSpd + 75.0f) * 0.707f;
        playThunderSound(e);
        break;
      case FighterType::Water:
        playWaterSplashSound(e);
        spawnWaterStream(p.x + p.facing * 12.0f, p.y - 10.0f, p.facing * 210.0f, 160.0f, pid, 40);
        fluidSolver.addVelocity(p.x + p.facing * 14.0f, p.y - 10.0f, p.facing * 220.0f, 150.0f, 18.0f);
        break;
      case FighterType::Hacker:
        proj.vx = p.facing * (baseSpd + 40.0f) * 0.707f;
        proj.vy = (baseSpd + 40.0f) * 0.707f;
        proj.radius = 8.0f;
        proj.damage = 24; // Matrix-Virus Boden-Welle (24 HP)
        playHackerCastSound(e);
        break;
      }
    } else {
      // --- GERADEAUS-SCHUSS (FORWARD FORWARD, 0 GRAD) ---
      proj.x = p.x + p.facing * 12.0f;
      proj.y = p.y - 11.0f; // Brusthöhe, fliegt sauber über Bodenunebenheiten
      proj.vy = 0.0f;

      switch (p.fighter) {
      case FighterType::Music: // Drachen-Feuerodem + E-Gitarren Power-Akkord
        proj.x = p.x + p.facing * 16.0f;
        proj.y = p.y - 11.0f;
        proj.vx = p.facing * (baseSpd + 45.0f);
        proj.radius = 8.5f;
        playGuitarDragonSound(e);
        break;
      case FighterType::Ice: // Eiskristalle (fliegen exakt geradeaus!)
        proj.vx = p.facing * (baseSpd + 20.0f);
        proj.radius = 5.0f;
        playIceCastSound(e);
        break;
      case FighterType::Fire: // Inferno Feuerball & thermischer Flammenstrahl
        proj.vx = p.facing * (baseSpd + 30.0f);
        proj.radius = 6.0f;
        playFireCastSound(e);
        spawnFireStream(p.x + p.facing * 12.0f, p.y - 11.0f, p.facing * 240.0f, -20.0f, pid, 30);
        fluidSolver.addVelocity(p.x + p.facing * 14.0f, p.y - 11.0f, p.facing * 250.0f, -20.0f, 16.0f);
        fluidSolver.addBuoyancy(p.x + p.facing * 14.0f, p.y - 11.0f, 150.0f, 16.0f);
        break;
      case FighterType::Lightning: // Blitzschneller Donnerkeil
        proj.vx = p.facing * (baseSpd + 85.0f);
        proj.radius = 4.0f;
        playThunderSound(e);
        break;
      case FighterType::Water: // Tsunami Hydro-Welle & Navier-Stokes Strahl
        proj.vx = p.facing * (baseSpd - 10.0f);
        proj.radius = 7.5f;
        playWaterSplashSound(e);
        spawnWaterStream(p.x + p.facing * 12.0f, p.y - 8.0f, p.facing * 260.0f, 0.0f, pid, 38);
        fluidSolver.addVelocity(p.x + p.facing * 14.0f, p.y - 8.0f, p.facing * 270.0f, 0.0f, 18.0f);
        break;
      case FighterType::Hacker: // Fliegende Matrix-Code Buchstaben
        proj.vx = p.facing * (baseSpd + 55.0f);
        proj.radius = 7.5f;
        proj.damage = 20; // Ausbalancierte Code-Salve (20 HP)
        playHackerCastSound(e);
        break;
      }
    }

    projectiles.push_back(proj);
  }

  void updateProjectiles(Engine &e, float dt) {
    for (size_t i = 0; i < projectiles.size();) {
      auto &proj = projectiles[i];
      proj.life -= dt;

      if (proj.life <= 0.0f || !proj.active) {
        projectiles[i] = projectiles.back();
        projectiles.pop_back();
        continue;
      }

      // Wenn es eine Boden-Schockwelle ist: Folgt der Kontur des hügeligen
      // Geländes!
      if (proj.isGroundWave) {
        proj.x += proj.vx * dt;
        proj.y = getGroundHeight(proj.x) - 4.0f;
        spawnGroundWaveParticles(proj);

        // 💧 WASSER-SCHOCKWELLE hinterlässt eine Spur aus verlangsamenden
        // Wasserpfützen!
        if (proj.type == FighterType::Water) {
          spawnWaterPuddle(proj.x, proj.owner);
        }
      } else {
        proj.x += proj.vx * dt;
        proj.y += proj.vy * dt;
        spawnProjectileTrail(proj);

        // Prüfung: Wenn der Abwärtsschuss den Boden berührt -> Verwandelt sich
        // in Boden-Schockwelle!
        float gY = getGroundHeight(proj.x);
        if (proj.isAirToGround && proj.y >= gY - 4.0f) {
          proj.isGroundWave = true;
          proj.y = gY - 4.0f;
          proj.vy = 0.0f;
          proj.life = 1.1f; // Rollt 1.1 Sekunden lang über den Boden
          proj.radius = 7.5f;
          if (proj.type == FighterType::Fire) {
            spawnFireMegaExplosion(proj.x, proj.y);
            playFireExplosionSound(e);
            screenShake = 0.65f;
            igniteGroundEverywhere(proj.x, proj.owner);
          } else {
            spawnHitExplosion(proj.x, proj.y, proj.type);
            playHitSound(e);
          }
        }
      }

      // 1. Bildschirm-Randprüfung
      if (proj.x < 0 || proj.x > 320) {
        proj.active = false;
        ++i;
        continue;
      }

      // 2. Trefferprüfung gegen Gegner
      int targetId = 1 - proj.owner;
      Player &target = players[targetId];

      bool hit = false;
      if (proj.isGroundWave) {
        // BODEN-SCHOCKWELLE: Trifft NUR Gegner am Boden!
        // Wenn der Gegner SPRINGT (target.y < gY - 10), springt er sauber
        // drüber!
        float targetGroundY = getGroundHeight(target.x);
        bool targetGrounded = (target.y >= targetGroundY - 8.0f);
        if (std::abs(proj.x - target.x) <= 12.0f && targetGrounded) {
          hit = true;
        }
      } else {
        // Normaler Flugtreffer (im Flug oder am Boden)
        float hitRad = proj.radius + 8.0f;
        float distSq =
            (proj.x - target.x) * (proj.x - target.x) +
            (proj.y - (target.y - 9.0f)) * (proj.y - (target.y - 9.0f));
        if (distSq <= hitRad * hitRad) {
          hit = true;
        }
      }

      if (hit) {
        proj.active = false;

        if (proj.type == FighterType::Fire) {
          // 🔥 FEUER-EXPLOSION: Gewaltige Detonation & ÜBERALL BRENNENDER
          // BODEN!
          spawnFireMegaExplosion(proj.x, proj.y);
          playFireExplosionSound(e);
          screenShake = 0.55f;
          igniteGroundEverywhere(proj.x, proj.owner);
        } else if (proj.type == FighterType::Water) {
          // 💧 WASSER-AUFPRALL: Hinterlässt glitzernde Wasserpfützen am Boden & Navier-Stokes Gischt!
          spawnWaterPuddle(proj.x, proj.owner);
          spawnWaterPuddle(proj.x - 7.0f, proj.owner);
          spawnWaterPuddle(proj.x + 7.0f, proj.owner);
          fluidSolver.addRadialImpulse(proj.x, proj.y, 140.0f, 20.0f);
          spawnWaterStream(proj.x, proj.y, 0.0f, -80.0f, proj.owner, 18);
        } else if (proj.type == FighterType::Hacker) {
          // 💻 HACKER-TREFFER: Cyber-Glitch Sound & Bildschirm-Wackeln!
          playCyberGlitchSound(e);
          screenShake = 0.40f;
        }

        if (target.isBlocking) {
          // Blockiert! 80% Schadensreduktion & Funkenbarriere
          int blockedDamage = std::max(1, proj.damage / 5);
          target.hp = std::max(0, target.hp - blockedDamage);
          target.vx += (proj.vx > 0 ? 40.0f : -40.0f);
          spawnBlockSparks(proj.x, proj.y);
          playBlockSound(e);
        } else {
          // Volltreffer!
          target.hp = std::max(0, target.hp - proj.damage);
          target.hurtTimer = 0.28f;
          target.vx += (proj.vx > 0 ? 85.0f : -85.0f);
          target.vy = proj.isGroundWave
                          ? -175.0f
                          : (proj.type == FighterType::Fire ? -130.0f : -45.0f);
          spawnSupernova(target.x, target.y - 10.0f, proj.type,
                         /*isMega=*/false);
          if (proj.type != FighterType::Fire) {
            spawnHitExplosion(proj.x, proj.y, proj.type);
            playHitSound(e);
            screenShake = 0.45f;
          }
        }
      } else if (!proj.isGroundWave && proj.y >= getGroundHeight(proj.x) - 2.0f) {
        // Einschlag ins hügelige Gelände (KEIN direkter Treffer auf den Spieler!)
        proj.active = false;
        if (proj.type == FighterType::Fire) {
          spawnFireMegaExplosion(proj.x, proj.y);
          playFireExplosionSound(e);
          screenShake = 0.35f;
          igniteGroundEverywhere(proj.x, proj.owner);
          // Spritzschaden NUR wenn der Gegner tatsächlich DIREKT in der Explosionswolke steht:
          float distSq = (proj.x - target.x) * (proj.x - target.x) +
                         (proj.y - target.y) * (proj.y - target.y);
          if (distSq <= 20.0f * 20.0f) {
            int splashDmg = target.isBlocking ? 2 : 8;
            target.hp = std::max(0, target.hp - splashDmg);
            target.hurtTimer = 0.20f;
            target.vy = -90.0f;
          }
        } else {
          spawnHitExplosion(proj.x, proj.y, proj.type);
          playHitSound(e);
        }
      }

      ++i;
    }
  }

  // --- ÜBERALL BRENNENDER BODEN NACH FEUER-EXPLOSION ---
  void igniteGroundEverywhere(float centerX, int owner) {
    // Entzündet 14 lodernde Brandherde entlang der Geländekontur
    for (int i = 0; i < 14; ++i) {
      GroundFire gf;
      gf.x = std::clamp(centerX + (i * 7.0f - 45.0f) + (rand() % 6 - 3), 16.0f,
                        304.0f);
      gf.y = getGroundHeight(gf.x);
      gf.maxLife =
          2.4f + (rand() % 130) / 100.0f; // Brennt 2.4 bis 3.7 Sekunden lang!
      gf.life = gf.maxLife;
      gf.owner = owner;
      groundFires.push_back(gf);
    }
  }

  void updateGroundFires(Engine &e, float dt) {
    for (size_t i = 0; i < groundFires.size();) {
      auto &gf = groundFires[i];
      gf.life -= dt;

      if (gf.life <= 0.0f) {
        groundFires[i] = groundFires.back();
        groundFires.pop_back();
        continue;
      }

      // Thermischer Auftrieb in die Fluid-Simulation einspeisen (heiße Kaminluft steigt auf!)
      fluidSolver.addBuoyancy(gf.x, gf.y - 4.0f, 85.0f * dt, 10.0f);

      // Flammen-Partikel & Rauch steigen auf
      if (e.rnd(3) == 0) {
        Particle p;
        p.x = gf.x + (rand() % 6 - 3);
        p.y = gf.y - 2.0f;
        p.vx = (rand() % 12 - 6);
        p.vy = -18.0f - (rand() % 25);
        p.maxLife = 0.25f + (rand() % 15) / 100.0f;
        p.life = p.maxLife;
        p.size = (rand() % 2 == 0) ? 1.5f : 1.0f;
        p.color = Ramps::Fire[8 + (rand() % 8)];
        particles.push_back(p);
      }

      // Kollision mit dem gegnerischen Spieler am Boden
      int targetId = 1 - gf.owner;
      Player &target = players[targetId];
      float dist = std::abs(target.x - gf.x);
      float targetGroundY = getGroundHeight(target.x);
      bool targetOnGround = (target.y >= targetGroundY - 7.0f);

      if (dist <= 7.0f && targetOnGround) {
        if (target.hurtTimer <= 0.0f) {
          if (target.isBlocking) {
            target.hp = std::max(0, target.hp - 1);
            spawnBlockSparks(target.x, target.y - 6.0f);
          } else {
            target.hp =
                std::max(0, target.hp - 4); // Feuerschaden pro Brandfleck
            target.hurtTimer = 0.30f;
            target.vy = -110.0f; // Hüpft bei Feuertreffer hoch
            spawnSupernova(target.x, target.y - 8.0f, FighterType::Fire,
                           /*isMega=*/false);
            spawnHitExplosion(target.x, target.y - 6.0f, FighterType::Fire);
            playHitSound(e);
          }
        }
      }

      ++i;
    }
  }

  // --- WASSER-PFÜTZEN (Hinterlassen von der Wasser-Schockwelle, verlangsamen
  // Gegner!) ---
  void spawnWaterPuddle(float x, int owner) {
    if (x < 12.0f || x > 308.0f)
      return;
    for (auto &wp : waterPuddles) {
      if (std::abs(wp.x - x) < 6.0f) {
        wp.life = wp.maxLife; // Pfütze auffrischen
        return;
      }
    }
    WaterPuddle wp;
    wp.x = x;
    wp.y = getGroundHeight(x);
    wp.maxLife = 5.0f; // Bleibt 5 Sekunden lang als Pfütze auf dem Boden!
    wp.life = wp.maxLife;
    wp.owner = owner;
    waterPuddles.push_back(wp);
  }

  void spawnWaterDroplets(float x, float y) {
    for (int i = 0; i < 4; ++i) {
      Particle p;
      p.x = x + (rand() % 6 - 3);
      p.y = y - 1.0f;
      p.vx = (rand() % 24 - 12);
      p.vy = -15.0f - (rand() % 20);
      p.maxLife = 0.18f;
      p.life = p.maxLife;
      p.size = 1.0f;
      p.color = (i % 2 == 0) ? Ramps::Cyan[12] : Colors::White;
      particles.push_back(p);
    }
  }

  void updateWaterPuddles(Engine &e, float dt) {
    for (size_t i = 0; i < waterPuddles.size();) {
      auto &wp = waterPuddles[i];
      wp.life -= dt;
      if (wp.life <= 0.0f) {
        waterPuddles[i] = waterPuddles.back();
        waterPuddles.pop_back();
        continue;
      }

      // Sanfte Wasser-Gischt & Bläschen
      if (e.rnd(8) == 0) {
        Particle p;
        p.x = wp.x + (rand() % 6 - 3);
        p.y = wp.y - 1.0f;
        p.vx = (rand() % 8 - 4);
        p.vy = -8.0f - (rand() % 14);
        p.maxLife = 0.24f;
        p.life = p.maxLife;
        p.size = 1.0f;
        p.color = (rand() % 2 == 0) ? Ramps::Cyan[12] : Colors::White;
        particles.push_back(p);
      }

      // Prüfung: Berührt der gegnerische Spieler das Wasser am Boden ->
      // VERLANGSAMUNG (Slow Down)!
      int targetId = 1 - wp.owner;
      Player &target = players[targetId];
      float dist = std::abs(target.x - wp.x);
      float targetGroundY = getGroundHeight(target.x);
      bool targetOnGround = (target.y >= targetGroundY - 7.0f);

      if (dist <= 8.0f && targetOnGround) {
        target.slowTimer =
            0.40f; // Verlangsamt den Gegner, solange er im Wasser steht/läuft!
        if (std::abs(target.vx) > 5.0f && e.rnd(3) == 0) {
          spawnWaterDroplets(target.x, target.y);
        }
      }

      ++i;
    }
  }

  void renderWaterPuddles(Engine &e) {
    for (const auto &wp : waterPuddles) {
      int wx = static_cast<int>(wp.x);
      int wy = static_cast<int>(wp.y);

      // Wasser-Schicht (glitzernde Pfütze am Boden)
      e.rectfill(wx - 4, wy - 1, 9, 2, Ramps::Blue[7]);
      e.line(wx - 3, wy, wx + 3, wy, Ramps::Cyan[11]);

      // Sanft glitzernde Wellenkräuselung an der Oberfläche
      int ripple =
          static_cast<int>(std::sin(globalTimer * 8.0f + wp.x * 0.5f) * 2.0f);
      e.pset(wx + ripple, wy - 1, Ramps::Cyan[14]);
      if (static_cast<int>(globalTimer * 6.0f + wp.x) % 3 == 0) {
        e.pset(wx - 2, wy - 1, Colors::White);
      }
    }
  }

  // ===========================================================================
  // 9. RUNDEN- & MATCHENDE
  // ===========================================================================
  void updateRoundOver(Engine &e) {
    float dt = e.dt();
    roundStateTimer += dt;
    fluidSolver.step(dt);
    updateFluidParticles(e, dt);
    updateSupernovas(dt);

    if (roundStateTimer >= 2.2f) {
      if (players[0].roundsWon >= 2) {
        matchWinner = 0;
        state = State::MatchOver;
        playVictoryFanfare(e);
      } else if (players[1].roundsWon >= 2) {
        matchWinner = 1;
        state = State::MatchOver;
        playVictoryFanfare(e);
      } else {
        currentRound++;
        startNewRound();
      }
    }
  }

  void updateMatchOver(Engine &e) {
    // Leertaste oder Enter für Revanche
    if (e.pressed(Key::Space) || e.pressed(Key::Enter) || e.pressed(Key::W) ||
        e.pressed(Key::Up)) {
      p1Locked = false;
      p2Locked = false;
      state = State::CharacterSelect;
    }
  }

  // ===========================================================================
  // 10. RENDERING & ZEICHENFUNKTIONEN
  // ===========================================================================
  void render(Engine &e) {
    // Bildschirm-Wackeln (Screen Shake)
    int shakeOffsetX = 0;
    int shakeOffsetY = 0;
    if (screenShake > 0.0f) {
      shakeOffsetX = static_cast<int>((e.rndf() - 0.5f) * screenShake * 6.0f);
      shakeOffsetY = static_cast<int>((e.rndf() - 0.5f) * screenShake * 6.0f);
    }

    if (state == State::CharacterSelect) {
      renderCharacterSelect(e);
      return;
    }

    // 1. Hintergrund (Goldene Dämmerung für maximalen Kontrast)
    renderSky(e);

    // 2. Schwebende Plattformen
    renderPlatforms(e);

    // 3. Hügeliges 2D Terrain, brennender Boden & verlangsamende Wasserpfützen
    renderTerrain(e);
    renderGroundFires(e);
    renderWaterPuddles(e);

    // 4. Beide Kämpfer zeichnen
    renderFighter(e, players[0]);
    renderFighter(e, players[1]);

    // 5. Projektile, Navier-Stokes Fluid-Dichte, Partikel & Supernovas
    renderFluidDensity(e);
    renderProjectiles(e);
    renderFluidParticles(e);
    renderParticles(e);
    renderSupernovas(e);

    // 6. Oberes HUD (Lebensbalken, Runden-Sterne, Namen)
    renderHUD(e);

    // 7. Runden-Banner & K.O.-Anzeigen
    if (state == State::RoundIntro) {
      renderRoundIntroBanner(e);
    } else if (state == State::RoundOver) {
      renderRoundOverBanner(e);
    } else if (state == State::MatchOver) {
      renderMatchOverPodium(e);
    }
  }

  // --- HIMMEL-GRADIENT (Dunkler, kontrastreicher Nachthimmel für maximalen
  // Pop) ---
  void renderSky(Engine &e) {
    for (int y = 0; y < 240; ++y) {
      float baseT = y / 240.0f;
      float noiseVal = (Engine::noise(0, y, 1337) - 0.5f) * 0.025f;
      float t = std::clamp(baseT + noiseVal, 0.0f, 1.0f);

      uint8_t skyCol;
      if (t < 0.35f) {
        // Oberer Nachthimmel: Tiefes Mitternachts-Anthrazit
        float subT = t / 0.35f;
        skyCol = Ramps::Grays[1 + static_cast<int>(subT * 2.0f)];
      } else if (t < 0.65f) {
        // Mittlerer Himmel: Dunkler Schiefer / Muted Indigo
        float subT = (t - 0.35f) / 0.30f;
        skyCol = Ramps::Grays[3 + static_cast<int>(subT * 2.0f)];
      } else if (t < 0.85f) {
        // Unterer Himmel: Dunkles Bronze-Ocker
        float subT = (t - 0.65f) / 0.20f;
        skyCol = Ramps::Earth::Sand[1 + static_cast<int>(subT * 3.0f)];
      } else {
        // Horizont: Gedämpftes, warmes Dämmerungs-Glühen (ohne zu blenden!)
        float subT = (t - 0.85f) / 0.15f;
        skyCol = Ramps::Gold.sample(0.25f + subT * 0.25f);
      }
      e.line(0, y, 319, y, skyCol);
    }

    // Funkelnde Sterne am weiten Nachthimmel
    for (int i = 0; i < 28; ++i) {
      int sx = (i * 37 + 19) % 320;
      int sy = (i * 23 + 11) % 95;
      e.pset(sx, sy,
             (static_cast<int>(globalTimer * 2.5f + i) % 2 == 0)
                 ? Colors::White
                 : Colors::Gold);
    }
  }

  // --- PLATTFORMEN ---
  void renderPlatforms(Engine &e) {
    for (const auto &plat : platforms) {
      int x1 = static_cast<int>(plat.x1);
      int x2 = static_cast<int>(plat.x2);
      int y = static_cast<int>(plat.y);

      // Stein-Plattform mit Moosbelag
      e.rectfill(x1, y, x2 - x1, 5, Ramps::Grays[8]);
      e.rectfill(x1, y + 5, x2 - x1, 3, Ramps::Grays[5]);
      e.line(x1, y, x2, y, Ramps::Green[10]);     // Moos-Kante oben
      e.rect(x1, y, x2 - x1, 8, Ramps::Grays[3]); // Dunkler Rahmen
    }
  }

  // --- TERRAIN ---
  void renderTerrain(Engine &e) {
    for (int x = 0; x < 320; ++x) {
      int gy = static_cast<int>(getGroundHeight(static_cast<float>(x)));

      // 1. Graskante (Hellgrün)
      e.line(x, gy, x, gy + 2, Ramps::Green[10]);
      e.line(x, gy + 3, x, gy + 5, Ramps::Earth::Moss[5]);

      // 2. Erdschicht (Braun)
      e.line(x, gy + 6, x, gy + 14, Ramps::Earth::Soil[5]);
      e.line(x, gy + 15, x, gy + 22, Ramps::Earth::Soil[3]);

      // 3. Felsfundament (Granit)
      e.line(x, gy + 23, x, 239, Ramps::Grays[7]);
    }
  }

  // ===========================================================================
  // 11. PIXEL-ART KÄMPFER-RENDERER (16x18 Pixel nach Darios Vorgaben)
  // ===========================================================================
  void renderFighter(Engine &e, const Player &p) {
    int px = static_cast<int>(p.x);
    int py = static_cast<int>(p.y);
    int f = p.facing;

    // Treffer-Blinken (Hurt-Flicker: Feines Durchscheinen während die Supernova
    // aufleuchtet)
    if (p.hurtTimer > 0.0f &&
        (static_cast<int>(globalTimer * 28.0f) % 2 == 0)) {
      return; // Lässt den Sprite feingliedrig aufblitzen statt mit weißem
              // Kasten zu überdecken
    }

    // --- A. SCHUTZSCHILD (beim Blocken - maximal 1.0 Sekunde aktiv!) ---
    if (p.isBlocking) {
      FighterDef def = getFighterDef(p.fighter);
      // Wenn das Schild fast abläuft (letzte 0.3s), flackert es als visuelle
      // Warnung!
      bool warningFlicker = (p.blockTimer > 0.7f &&
                             static_cast<int>(globalTimer * 20.0f) % 2 == 0);
      if (!warningFlicker) {
        float shieldRadius = 14.0f + std::sin(globalTimer * 16.0f) * 1.5f;
        e.circle(px + f * 4, py - 10, shieldRadius, def.accentColor);
        e.circle(px + f * 4, py - 10, shieldRadius + 1.0f, Colors::White);
      }
    }

    // --- B. KÄMPFER-SPEZIFISCHE PIXEL-SPRITES ---
    switch (p.fighter) {
    case FighterType::Music: {
      // 🎸 MUSIK-KÄMPFER (DARIO): Sitzt auf einem mächtigen Drachen und rockt
      // die E-Gitarre!

      // 1. DER DRACHE (py - 10 bis py)
      // Drachen-Schwanz hinten mit Stacheln
      int tailX = px - f * 8;
      int tailY = py - 6;
      e.line(px - f * 4, py - 6, tailX, tailY, Ramps::Green[9]);
      e.line(tailX, tailY, tailX - f * 3, tailY - 3, Ramps::Green[11]);
      e.pset(tailX - f * 3, tailY - 4,
             Ramps::Gold[13]); // Goldene Schwanzspitze

      // Drachen-Körper / Bauch
      e.rectfill(px - 6, py - 9, 12, 6, Ramps::Green[9]); // Schuppenpanzer
      e.rectfill(px - (f > 0 ? 3 : 5), py - 7, 8, 4,
                 Ramps::Gold[9]); // Goldener Drachenbauch
      e.rect(px - 6, py - 9, 12, 6, Ramps::Green[6]);

      // Drachen-Beine & Krallen (stehen auf dem Boden)
      int legStep = (std::abs(p.vx) > 5.0f)
                        ? (static_cast<int>(globalTimer * 12.0f) % 2)
                        : 0;
      e.rectfill(px - 5, py - 3, 3, 3, Ramps::Green[8]);
      e.rectfill(px + 2, py - 3, 3, 3, Ramps::Green[8]);
      e.pset(px - 5 + f * legStep, py, Colors::White); // Krallen
      e.pset(px + 3 - f * legStep, py, Colors::White);

      // Drachen-Kopf & Schnauze vorne
      int hx = px + f * 7;
      int hy = py - 9;
      e.rectfill(hx - 2, hy - 3, 6, 6, Ramps::Green[10]);     // Kopf
      e.rectfill(hx + f * 2, hy - 1, 3, 3, Ramps::Green[11]); // Schnauze
      e.pset(hx + f * 1, hy - 2, Ramps::Fire[12]); // Feuriges Drachenauge
      e.line(hx - 1, hy - 4, hx - f * 2, hy - 7,
             Ramps::Gold[13]);                 // Goldene Hörner
      e.pset(hx + f * 4, hy, Ramps::Fire[14]); // Kleine Flamme aus der Nase!

      // Drachen-Flügel (flattern dynamisch & majestätisch beim Fliegen!)
      float flapFreq = p.isFlying ? 28.0f : (p.isGrounded ? 10.0f : 16.0f);
      int wingFlap = static_cast<int>(std::sin(globalTimer * flapFreq) *
                                      (p.isFlying ? 5.0f : 3.0f));
      int wx = px - f * 2;
      int wy = py - 12 + (p.isGrounded ? 0 : wingFlap);
      e.line(wx, wy, wx - f * 7, wy - 6 + wingFlap, Ramps::Green[12]);
      e.line(wx - f * 7, wy - 6 + wingFlap, wx - f * 2, wy - 1,
             Ramps::Gold[11]);
      e.line(wx - f * 5, wy - 4 + wingFlap, wx, wy + 1, Ramps::Green[10]);
      if (p.isFlying) {
        e.pset(wx - f * 7, wy - 6 + wingFlap,
               Colors::White); // Glühende Flügelspitzen im Flug
      }

      // 2. DARIO (SITZT AUF DEM DRACHENSATTEL, py - 22 bis py - 10)
      // Sattel
      e.rectfill(px - 4, py - 11, 7, 2, Ramps::Earth::Soil[4]);

      // Rumpf & Rocker-Weste
      e.rectfill(px - 3, py - 16, 6, 6, Ramps::Purple[4]);
      e.line(px - 1, py - 16, px - 1, py - 11, Colors::White); // Shirt-Streifen

      // Kopf & Gesicht (nach Darios Beschreibung)
      e.rectfill(px - 3, py - 21, 6, 5, Ramps::Earth::Sand[6]); // Hautfarbe
      // Schwarze Augenringe mit hellen Augen
      e.rectfill(px + f * 1, py - 19, 3, 3, Colors::Black);
      e.pset(px + f * 2, py - 18, Colors::White);

      // Rote Haare (ordentlich, keine Rocker-Frisur)
      e.rectfill(px - 3, py - 22, 6, 2, Ramps::Red[9]);
      e.rectfill(px - (f > 0 ? 3 : 1), py - 21, 3, 2, Ramps::Red[8]);

      // 3. E-GITARRE (Dario spielt ein Gitarrensolo auf dem Drachen!)
      int gx = px + f * 4;
      int gy = py - 14;
      e.line(gx - 3, gy + 3, gx + 3, gy - 3, Ramps::Red[9]);     // Roter Korpus
      e.line(gx + 1, gy - 1, gx + 6 * f, gy - 6, Colors::White); // Gitarrenhals
      e.pset(gx, gy + 1, Colors::White);                         // Tonabnehmer
      break;
    }

    case FighterType::Ice: {
      // ❄️ EIS-KÄMPFER: Blau-Weiß, Eisstücke am Körper & am Arm
      // 1. Beine
      e.rectfill(px - 4, py - 6, 3, 6, Ramps::Blue[7]);
      e.rectfill(px + 1, py - 6, 3, 6, Ramps::Blue[7]);

      // 2. Eis-Rumpf & Rüstung
      e.rectfill(px - 5, py - 12, 10, 6, Colors::White);
      e.rect(px - 5, py - 12, 10, 6, Ramps::SkyBlue[10]);

      // 3. Kopf & Eishelm
      e.rectfill(px - 4, py - 17, 8, 5, Ramps::Earth::Sand[6]);
      e.pset(px + f * 2, py - 15, Ramps::Cyan[12]);     // Eisblaues Auge
      e.rectfill(px - 4, py - 18, 8, 2, Colors::White); // Eiskappe

      // 4. Eis-Stücke / Kristalle am Körper und am Arm
      e.line(px - 5, py - 13, px - 8, py - 16,
             Ramps::Cyan[14]); // Kristall an Schulter
      e.line(px + f * 5, py - 10, px + f * 9, py - 13,
             Colors::White); // Eisspeer am Arm
      e.pset(px + f * 8, py - 12, Ramps::Cyan[14]);
      break;
    }

    case FighterType::Fire: {
      // 🔥 FEUER-KÄMPFER: Feuer am Arm, lodernde Feuerfrisur
      // 1. Beine
      e.rectfill(px - 4, py - 6, 3, 6, Ramps::Grays[3]);
      e.rectfill(px + 1, py - 6, 3, 6, Ramps::Grays[3]);

      // 2. Rumpf
      e.rectfill(px - 5, py - 12, 10, 6, Ramps::Fire[6]);
      e.rect(px - 5, py - 12, 10, 6, Ramps::Fire[9]);

      // 3. Gesicht
      e.rectfill(px - 4, py - 17, 8, 5, Ramps::Earth::Sand[6]);
      e.pset(px + f * 2, py - 15, Ramps::Fire[12]); // Glühendes Auge

      // 4. Lodernde FEUER-FRISUR (Flammenhaare animiert)
      int flameFlicker = (static_cast<int>(globalTimer * 12.0f) % 3);
      e.rectfill(px - 4, py - 19, 8, 3, Ramps::Fire[8]);
      e.pset(px - 2, py - 20 - flameFlicker, Ramps::Fire[12]); // Flammenspitzen
      e.pset(px + 1, py - 21 + flameFlicker, Ramps::Fire[14]);
      e.pset(px + 3, py - 20, Ramps::Fire[10]);

      // 5. FEUER AM ARM
      int ax = px + f * 6;
      int ay = py - 10;
      e.circlefill(ax, ay, 2.5f, Ramps::Fire[10]);
      e.pset(ax + f * 2, ay - 1, Ramps::Fire[14]);
      break;
    }

    case FighterType::Lightning: {
      // ⚡ BLITZ-KÄMPFER: Blitz-Augen, Blitz-Funken um Körper & Arm
      // 1. Beine
      e.rectfill(px - 4, py - 6, 3, 6, Ramps::Purple[4]);
      e.rectfill(px + 1, py - 6, 3, 6, Ramps::Purple[4]);

      // 2. Rumpf
      e.rectfill(px - 5, py - 12, 10, 6, Ramps::Purple[6]);
      e.line(px - 4, py - 9, px + 4, py - 9, Ramps::Yellow[14]); // Blitz-Gürtel

      // 3. Kopf mit strahlenden BLITZ-AUGEN
      e.rectfill(px - 4, py - 17, 8, 5, Ramps::Earth::Sand[6]);
      e.rectfill(px - 4, py - 18, 8, 2, Ramps::Grays[2]); // Dunkle Haare
      // Strahlende Blitz-Augen
      e.pset(px + f * 1, py - 15, Ramps::Yellow[15]);
      e.pset(px + f * 3, py - 15, Colors::White);

      // 4. BLITZE UM KÖRPER & AM ARM
      int sparkPhase = static_cast<int>(globalTimer * 15.0f) % 4;
      if (sparkPhase == 0) {
        e.line(px - 6, py - 14, px - 3, py - 11, Ramps::Yellow[14]);
        e.line(px + f * 5, py - 10, px + f * 9, py - 8, Colors::White);
      } else if (sparkPhase == 1) {
        e.line(px + 4, py - 15, px + 7, py - 12, Ramps::Yellow[14]);
        e.line(px + f * 5, py - 11, px + f * 8, py - 13, Ramps::Yellow[15]);
      } else if (sparkPhase == 2) {
        e.line(px - 7, py - 8, px - 4, py - 6, Colors::White);
        e.line(px + f * 6, py - 9, px + f * 10, py - 11, Ramps::Yellow[14]);
      }
      break;
    }

    case FighterType::Water: {
      // 💧 WASSER-KÄMPFER: Blaue Haare & Augen, verdeckter Mund (Ninja-Maske)
      // 1. Beine
      e.rectfill(px - 4, py - 6, 3, 6, Ramps::Blue[5]);
      e.rectfill(px + 1, py - 6, 3, 6, Ramps::Blue[5]);

      // 2. Ninja-Gewand
      e.rectfill(px - 5, py - 12, 10, 6, Ramps::Blue[8]);
      e.line(px - 4, py - 8, px + 4, py - 8,
             Ramps::Cyan[12]); // Türkise Schärpe

      // 3. Gesicht mit BLAUEN AUGEN & NINJA-MASKE
      e.rectfill(px - 4, py - 17, 8, 5, Ramps::Earth::Sand[6]);
      // Mund verdeckt mit Ninja-Maske
      e.rectfill(px - 4, py - 14, 8, 3, Ramps::Blue[4]);
      // Blaue Augen
      e.pset(px + f * 2, py - 15, Ramps::Cyan[13]);

      // 4. BLAUE HAARE & Stirnband
      e.rectfill(px - 4, py - 18, 8, 2, Ramps::Blue[9]);
      e.line(px - 5, py - 16, px - 8 * f, py - 14,
             Ramps::Cyan[12]); // Wehendes Band
      break;
    }

    case FighterType::Hacker: {
      // 💻 HACKER-KÄMPFER (BYTE): Cyber-Hoodie, Cyber-Visor & tippt rasant auf
      // dem Laptop!
      // 1. Cyber-Beine & Schuhe mit leuchtender Neon-Sohle
      int legStep = (std::abs(p.vx) > 5.0f)
                        ? (static_cast<int>(globalTimer * 12.0f) % 2)
                        : 0;
      e.rectfill(px - 4, py - 6, 3, 6, Ramps::Grays[3]);
      e.rectfill(px + 1, py - 6, 3, 6, Ramps::Grays[3]);
      // Neon-grüne Schuhsohlen
      e.line(px - 4 + f * legStep, py, px - 2 + f * legStep, py,
             Ramps::Green[14]);
      e.line(px + 1 - f * legStep, py, px + 3 - f * legStep, py,
             Ramps::Green[14]);

      // 2. Rumpf / Cyber-Hoodie mit Matrix-Datenleitungen
      e.rectfill(px - 5, py - 12, 10, 6, Ramps::Grays[4]);
      e.rect(px - 5, py - 12, 10, 6, Ramps::Grays[2]);
      // Grüne Matrix-Schaltkreis-Linie auf der Brust
      e.line(px - 2, py - 12, px - 2, py - 8, Ramps::Green[13]);
      e.line(px - 2, py - 8, px + 2, py - 8, Ramps::Green[14]);

      // 3. Kopf mit Hoodie & CYBER-VISOR (leuchtende Cyber-Brille)
      e.rectfill(px - 4, py - 17, 8, 5, Ramps::Earth::Sand[6]);  // Gesicht
      e.rectfill(px - 4, py - 18, 8, 2, Ramps::Grays[3]);        // Hoodie oben
      e.line(px - 5, py - 17, px - 5, py - 13, Ramps::Grays[3]); // Hoodie Seite

      // Leuchtender Cyber-Visor / VR-Brille (Cyan + Neon-Grün)
      e.rectfill(px + f * 0, py - 16, 5, 2, Ramps::Cyan[14]);
      e.pset(px + f * 2, py - 16, Colors::White);    // Glanzpunkt
      e.pset(px + f * 4, py - 15, Ramps::Green[15]); // Status-LED

      // 4. DER HACKER-COMPUTER / LAPTOP (in den Händen gehalten!)
      int lx = px + f * 5;
      int ly = py - 9;

      // Laptop-Basis / Gehäuse
      e.rectfill(lx - 4, ly + 1, 8, 3, Ramps::Grays[6]);
      // Tastatur mit leuchtenden Tasten
      e.line(lx - 3, ly + 2, lx + 3, ly + 2, Ramps::Cyan[12]);

      // Laptop-Bildschirm (aufgeklappt, leuchtet hellgrün/weiß)
      int scrX = lx + f * 3;
      int scrY = ly - 4;
      e.rectfill(scrX - 3, scrY - 2, 6, 5, Ramps::Grays[2]); // Rahmen
      e.rectfill(scrX - 2, scrY - 1, 4, 3,
                 Ramps::Green[13]);              // Display Matrix-Grün
      e.pset(scrX - 1, scrY, Colors::White);     // Code-Pixel 1
      e.pset(scrX + 1, scrY - 1, Colors::White); // Code-Pixel 2

      // Schnelle Tipp-Animation der Hände
      int typeAnim = static_cast<int>(globalTimer * 20.0f) % 3;
      e.pset(lx - 2 + typeAnim, ly + 1, Ramps::Earth::Sand[6]);
      e.pset(lx + 1 - typeAnim, ly + 1, Ramps::Earth::Sand[6]);

      // Aufsteigende fliegende Matrix-Code Bits direkt aus dem Laptop!
      if (rand() % 3 == 0) {
        e.pset(scrX + (rand() % 5 - 2), scrY - 3 - (rand() % 3),
               Ramps::Green[15]);
      }
      break;
    }
    }

    // Verlangsamungs-Anzeige (Wasserspritzer um die Füße bei Slowdown)
    if (p.slowTimer > 0.0f) {
      e.line(px - 4, py, px + 4, py, Ramps::Cyan[12]);
      e.pset(px + (static_cast<int>(globalTimer * 14.0f) % 7 - 3), py - 2,
             Colors::White);
    }
  }

  // --- PROJEKTILE RENDERN ---
  void renderProjectiles(Engine &e) {
    for (const auto &proj : projectiles) {
      if (!proj.active)
        continue;
      int px = static_cast<int>(proj.x);
      int py = static_cast<int>(proj.y);
      int dir = (proj.vx > 0) ? 1 : -1;

      if (proj.isGroundWave) {
        // --- ROLLENDE BODEN-SCHOCKWELLE (Zwingt zum Drüberspringen!) ---
        switch (proj.type) {
        case FighterType::Music: {
          // Drachenfeuer-Teppich mit goldenen Noten-Flammen
          e.circlefill(px, py - 2, 6.0f, Ramps::Gold[11]);
          e.circlefill(px, py - 1, 4.0f, Ramps::Fire[12]);
          e.line(px - dir * 4, py, px + dir * 6, py - 6, Ramps::Fire[14]);
          e.line(px - dir * 2, py, px + dir * 3, py - 8, Colors::White);
          break;
        }
        case FighterType::Ice: {
          // Aus dem Boden schießende Eiskristall-Spitzen
          e.line(px - 5, py + 2, px, py - 8, Colors::White);
          e.line(px, py - 8, px + 5, py + 2, Ramps::Cyan[13]);
          e.line(px - 2, py + 2, px + 2, py - 5, Ramps::Cyan[14]);
          break;
        }
        case FighterType::Fire: {
          // Lodernde Bodenflamme
          e.circlefill(px, py - 2, 5.0f, Ramps::Fire[8]);
          e.circlefill(px, py - 3, 3.5f, Ramps::Fire[12]);
          e.pset(px, py - 7, Ramps::Fire[15]);
          break;
        }
        case FighterType::Lightning: {
          // Am Boden kriechender Blitzschlag
          e.line(px - dir * 6, py, px - dir * 2, py - 6, Ramps::Yellow[14]);
          e.line(px - dir * 2, py - 6, px + dir * 5, py - 1, Colors::White);
          e.line(px + dir * 1, py - 4, px + dir * 6, py - 8, Ramps::Yellow[15]);
          break;
        }
        case FighterType::Water: {
          // Überschlagende Brandungswelle
          e.circlefill(px, py - 3, 6.0f, Ramps::Cyan[10]);
          e.circle(px, py - 3, 6.5f, Colors::White);
          e.line(px - dir * 4, py, px + dir * 5, py - 7, Colors::White);
          break;
        }
        case FighterType::Hacker: {
          // 💻 Hacker-Bodenwelle: Kriechender grüner Matrix-Code &
          // Glitch-Stream am Boden!
          static const char *groundCodes[] = {"010101", "_VIRUS_", "<ERR!>",
                                              "{404}",  ">>>EXE",  "[HACK]"};
          int codeIdx =
              (static_cast<int>(globalTimer * 8.0f) + std::abs(px / 18)) % 6;

          e.rectfill(px - 14, py - 6, 28, 7, Ramps::Grays[1]);
          e.rect(px - 14, py - 6, 28, 7, Ramps::Green[12]);
          e.draw_text(px - 12, py - 5, groundCodes[codeIdx], Ramps::Green[15],
                      1);

          // Digitaler Funkenschweif
          e.line(px - dir * 10, py, px + dir * 8, py, Ramps::Cyan[14]);
          e.pset(px + (rand() % 16 - 8), py - (rand() % 8), Colors::White);
          break;
        }
        }
        continue;
      }

      // --- NORMALE FLUG-PROJEKTILE (Gerade & Schrägflug) ---
      switch (proj.type) {
      case FighterType::Music: {
        // 🎸 Musik-Drache: Mächtiger fliegender Drachenkopf aus goldenem
        // Schallfeuer & E-Gitarren-Akkorden
        e.circlefill(px, py, 7.0f, Ramps::Gold[11]);
        e.circlefill(px, py, 5.0f, Ramps::Fire[10]);
        e.circlefill(px + dir * 2, py, 3.0f, Colors::White);
        // Drachenhörner & Notenschweif
        e.line(px - dir * 3, py - 6, px + dir * 4, py - 8, Ramps::Gold[14]);
        e.line(px - dir * 3, py + 6, px + dir * 4, py + 8, Ramps::Gold[14]);
        e.line(px + dir * 5, py - 4, px + dir * 9, py, Ramps::Red[9]);
        e.pset(px + dir * 6, py - 2,
               Colors::White); // Glühendes Drachenauge im Projektil
        break;
      }
      case FighterType::Ice: {
        // ❄️ Eis: 3 rotierende spitze Eiskristalle
        e.line(px - 4, py, px + 4, py, Colors::White);
        e.line(px, py - 4, px, py + 4, Ramps::Cyan[13]);
        e.pset(px + dir * 4, py, Colors::White);
        break;
      }
      case FighterType::Fire: {
        // 🔥 Feuer: Lodernder rot-gelber Feuerball
        e.circlefill(px, py, 5.0f, Ramps::Fire[8]);
        e.circlefill(px, py, 3.0f, Ramps::Fire[12]);
        e.circlefill(px, py, 1.5f, Colors::White);
        break;
      }
      case FighterType::Lightning: {
        // ⚡ Blitz: Zick-Zack Donnerkeil
        e.line(px - dir * 6, py - 3, px, py + 2, Ramps::Yellow[14]);
        e.line(px, py + 2, px + dir * 7, py - 2, Colors::White);
        break;
      }
      case FighterType::Water: {
        // 💧 Wasser: Rotierende Hydro-Wasserwelle
        e.circlefill(px, py, 5.5f, Ramps::Cyan[10]);
        e.circle(px, py, 6.0f, Colors::White);
        e.pset(px + dir * 2, py - 2, Colors::White);
        break;
      }
      case FighterType::Hacker: {
        // 💻 Hacker: Fliegende Matrix-Buchstaben & Code-Blöcke aus dem Laptop!
        static const char *codeSnippets[] = {"<HACK>", "{CODE}", "0101", "ROOT",
                                             "404!",   "EXE",    "C++",  "[#]"};
        int codeIdx =
            (static_cast<int>(proj.life * 10.0f) + proj.owner * 3) % 8;
        const char *txt = codeSnippets[codeIdx];

        // Leuchtende Cyber-Matrix Kapsel / Hologramm-Kasten
        e.rectfill(px - 14, py - 5, 28, 10, Ramps::Grays[1]);
        e.rect(px - 14, py - 5, 28, 10, Ramps::Green[13]);
        e.rect(px - 13, py - 4, 26, 8, Ramps::Cyan[14]);

        // Matrix-Code Text (Grün mit weißem Glanz)
        e.draw_text(px - 11, py - 3, txt, Ramps::Green[15], 1);

        // Fliegende Satelliten-Buchstaben & Bits vor und hinter dem Projektil
        char bit1 =
            (static_cast<int>(globalTimer * 16.0f) % 2 == 0) ? '0' : '1';
        char bit2 =
            (static_cast<int>(globalTimer * 12.0f) % 2 == 0) ? '{' : '}';
        char bit3 =
            (static_cast<int>(globalTimer * 14.0f) % 2 == 0) ? '<' : '>';

        std::string s1(1, bit1);
        std::string s2(1, bit2);
        std::string s3(1, bit3);

        e.draw_text(px - dir * 18, py - 6, s1, Ramps::Green[13], 1);
        e.draw_text(px - dir * 16, py + 2, s2, Ramps::Cyan[13], 1);
        e.draw_text(px + dir * 16, py - 2, s3, Colors::White, 1);

        // Digitaler Glitch-Blitz
        if (rand() % 3 == 0) {
          e.line(px - 10, py + (rand() % 8 - 4), px + 10, py + (rand() % 8 - 4),
                 Colors::White);
        }
        break;
      }
      }
    }
  }

  // --- ÜBERALL BRENNENDER BODEN RENDERN (Flammen-Teppich) ---
  void renderGroundFires(Engine &e) {
    for (const auto &gf : groundFires) {
      int gx = static_cast<int>(gf.x);
      int gy = static_cast<int>(gf.y);

      // Lebendig züngelnde Flammen auf dem Boden
      float flamePhase = globalTimer * 22.0f + gf.x * 0.9f;
      int flameH = 4 + static_cast<int>(std::sin(flamePhase) * 2.5f + 2.0f);
      int flameWobble = static_cast<int>(std::cos(flamePhase * 1.4f) * 2.0f);

      // Glut-Basis im Boden
      e.rectfill(gx - 2, gy - 1, 5, 2, Ramps::Fire[7]);

      // Lodernde Flammenzungen nach oben
      e.line(gx - 1, gy, gx + flameWobble, gy - flameH, Ramps::Fire[10]);
      e.line(gx + 1, gy, gx + flameWobble + 1, gy - (flameH - 2),
             Ramps::Fire[13]);
      e.pset(gx + flameWobble, gy - flameH,
             Colors::White); // Weißglühende Flammenspitze
    }
  }

  // --- PARTIKEL RENDERN ---
  void renderParticles(Engine &e) {
    for (const auto &p : particles) {
      int px = static_cast<int>(p.x);
      int py = static_cast<int>(p.y);
      if (px >= 0 && px < 320 && py >= 0 && py < 240) {
        if (p.size > 1.5f) {
          e.rectfill(px - 1, py - 1, 2, 2, p.color);
        } else {
          e.pset(px, py, p.color);
        }
      }
    }
  }

  // --- SUPERNOVA EFFEKTE RENDERN (KLEIN BEI TREFFER, 2X RIESIG BEI K.O.) ---
  void renderSupernovas(Engine &e) {
    for (const auto &sn : supernovas) {
      float prog = 1.0f - (sn.life / sn.maxLife);
      int sx = static_cast<int>(sn.x);
      int sy = static_cast<int>(sn.y);

      // 1. Expandierender kosmischer Schockwellen-Ring
      float curRadius = sn.maxRadius * std::sin(prog * 1.5708f);
      if (curRadius > 1.0f) {
        e.circle(sx, sy, curRadius, sn.ringColor);
        if (sn.isMega && curRadius > 3.0f) {
          e.circle(sx, sy, curRadius - 1.5f, Colors::White);
          e.circle(sx, sy, curRadius + 1.0f, sn.rayColor);
        }
      }

      // 2. Glühender Supernova-Kernblitz
      if (prog < 0.45f) {
        float coreR = (sn.maxRadius * 0.42f) * (1.0f - prog / 0.45f);
        if (coreR >= 1.0f) {
          e.circlefill(sx, sy, coreR, sn.coreColor);
        }
      }

      // 3. Strahlende Sternen-Lichtstrahlen (Cross-Rays)
      float rayLen = sn.maxRadius * 1.45f * (1.0f - prog);
      if (rayLen > 1.0f) {
        e.line(sx - rayLen, sy, sx + rayLen, sy, sn.rayColor);
        e.line(sx, sy - rayLen, sx, sy + rayLen, sn.rayColor);
        e.line(sx - rayLen * 0.7f, sy, sx + rayLen * 0.7f, sy, Colors::White);
        e.line(sx, sy - rayLen * 0.7f, sx, sy + rayLen * 0.7f, Colors::White);

        // Bei 2x Mega-Supernova (K.O.) zusätzliche 8-Punkt Diagonalstrahlen
        if (sn.isMega) {
          float diag = rayLen * 0.707f;
          e.line(sx - diag, sy - diag, sx + diag, sy + diag, Colors::White);
          e.line(sx - diag, sy + diag, sx + diag, sy - diag, Colors::White);
          e.line(sx - diag * 1.2f, sy - diag * 1.2f, sx + diag * 1.2f,
                 sy + diag * 1.2f, sn.ringColor);
          e.line(sx - diag * 1.2f, sy + diag * 1.2f, sx + diag * 1.2f,
                 sy - diag * 1.2f, sn.ringColor);
        }
      }
    }
  }

  // ===========================================================================
  // 12. UI & HUD (Lebensbalken, Rundensterne, Banner)
  // ===========================================================================
  void renderHUD(Engine &e) {
    FighterDef d0 = getFighterDef(players[0].fighter);
    FighterDef d1 = getFighterDef(players[1].fighter);

    // --- SPIELER 1 (LINKS) ---
    // Name & Element
    e.draw_text(10, 6, "P1: " + d0.name, d0.mainColor, 1);
    // Lebensbalken
    int hpW0 = static_cast<int>(
        (static_cast<float>(players[0].hp) / players[0].maxHp) * 85);
    e.rectfill(10, 16, 85, 6, Colors::DarkGray);
    e.rect(9, 15, 87, 8, Colors::White);
    uint8_t col0 = (players[0].hp > players[0].maxHp / 2)
                       ? Colors::LightGreen
                       : (players[0].hp > players[0].maxHp / 4 ? Colors::Yellow
                                                               : Colors::Red);
    e.rectfill(10, 16, hpW0, 6, col0);

    // Runden-Sterne Spieler 1 (Best of 3)
    for (int r = 0; r < 2; ++r) {
      uint8_t starCol =
          (r < players[0].roundsWon) ? Colors::Gold : Colors::DarkGray;
      e.rectfill(102 + r * 10, 16, 6, 6, starCol);
      e.rect(102 + r * 10, 16, 6, 6, Colors::White);
    }

    // --- SPIELER 2 (RECHTS) ---
    // Name & Element
    e.draw_text(225, 6, "P2: " + d1.name, d1.mainColor, 1);
    // Lebensbalken
    int hpW1 = static_cast<int>(
        (static_cast<float>(players[1].hp) / players[1].maxHp) * 85);
    e.rectfill(225, 16, 85, 6, Colors::DarkGray);
    e.rect(224, 15, 87, 8, Colors::White);
    uint8_t col1 = (players[1].hp > players[1].maxHp / 2)
                       ? Colors::LightGreen
                       : (players[1].hp > players[1].maxHp / 4 ? Colors::Yellow
                                                               : Colors::Red);
    e.rectfill(310 - hpW1, 16, hpW1, 6, col1);

    // Runden-Sterne Spieler 2 (Best of 3)
    for (int r = 0; r < 2; ++r) {
      uint8_t starCol =
          (r < players[1].roundsWon) ? Colors::Gold : Colors::DarkGray;
      e.rectfill(200 + r * 10, 16, 6, 6, starCol);
      e.rect(200 + r * 10, 16, 6, 6, Colors::White);
    }

    // --- MITTE: RUNDEN-ANZEIGE ---
    std::string roundTxt = "RUNDE " + std::to_string(currentRound);
    e.draw_text(138, 6, roundTxt, Colors::Gold, 1);

    // Tastatur-Erinnerung
    e.draw_text(
        10, 230,
        "BLOCK:WEG+S  GERADE:VOR,VOR  45 OBEN:VOR,OBEN  BODENWELLE:HIN+S",
        Ramps::Grays[7], 1);
  }

  void renderRoundIntroBanner(Engine &e) {
    e.rectfill(60, 85, 200, 42, Colors::Black);
    e.rect(60, 85, 200, 42, Colors::Gold);

    if (roundStateTimer < 0.7f) {
      std::string txt = "RUNDE " + std::to_string(currentRound);
      e.draw_text(110, 96, txt, Colors::Yellow, 2);
    } else {
      e.draw_text(125, 96, "FIGHT!", Ramps::Fire[10], 2);
    }
  }

  void renderRoundOverBanner(Engine &e) {
    e.rectfill(70, 80, 180, 50, Colors::Black);
    e.rect(70, 80, 180, 50, Colors::Red);

    e.draw_text(130, 90, "K.O.!", Ramps::Fire[12], 2);
    if (players[0].hp <= 0 && players[1].hp <= 0) {
      e.draw_text(95, 112, "UNENTSCHIEDEN!", Colors::White, 1);
    } else if (players[0].hp <= 0) {
      e.draw_text(95, 112, "P2 GEWINNT DIE RUNDE!", Ramps::Red[10], 1);
    } else {
      e.draw_text(95, 112, "P1 GEWINNT DIE RUNDE!", Ramps::Blue[10], 1);
    }
  }

  void renderMatchOverPodium(Engine &e) {
    e.rectfill(40, 40, 240, 150, Colors::Black);
    e.rect(40, 40, 240, 150, Colors::Gold);
    e.rect(42, 42, 236, 146, Ramps::Grays[2]);

    std::string champName =
        (matchWinner == 0)
            ? (players[0].fighter == FighterType::Music ? "DARIO (P1)"
                                                        : "SPIELER 1")
            : "SPIELER 2";
    uint8_t champColor = (matchWinner == 0) ? Ramps::Blue[10] : Ramps::Red[10];

    e.draw_text(70, 55, "CHAMPION SIEG!", Colors::Gold, 2);
    e.draw_text(65, 85, champName + " GEWINNT DAS MATCH!", champColor, 1);

    // Pokal-Icon / Sieger-Statue
    e.rectfill(145, 105, 30, 22, Colors::Gold);
    e.rect(145, 105, 30, 22, Colors::White);
    e.rectfill(155, 127, 10, 12, Colors::Gold);
    e.rectfill(148, 139, 24, 5, Colors::Gold);

    bool blink = (static_cast<int>(globalTimer * 2.5f) % 2 == 0);
    if (blink) {
      e.draw_text(68, 160, "LEERTASTE: NEUES SPIEL", Colors::Yellow, 1);
    }
    e.draw_text(95, 174, "ESC: HAUPTMENUE", Colors::LightGray, 1);
  }

  // --- CHARAKTER-AUSWAHL BILDSCHIRM ---
  void renderCharacterSelect(Engine &e) {
    // Hintergrund
    e.cls(Ramps::Grays[2]);

    // Titel
    e.draw_text(55, 12, "DARIOS SPIEL: ELEMENTAL BRAWL", Colors::Gold, 2);
    e.draw_text(72, 32, "WAEHLE DEINEN ELEMENTAR-KAEMPFER!", Colors::White, 1);

    // 6 Kämpfer-Karten
    for (int i = 0; i < 6; ++i) {
      FighterDef def = getFighterDef(static_cast<FighterType>(i));
      int cardX = 6 + i * 52;
      int cardY = 50;
      int cardW = 48;
      int cardH = 138;

      // Rahmen & Hintergrund der Karte
      bool p1Here = (p1Cursor == i);
      bool p2Here = (p2Cursor == i);

      uint8_t bgCol = Ramps::Grays[4];
      if (p1Here && p2Here)
        bgCol = Ramps::Purple[4];
      else if (p1Here)
        bgCol = Ramps::Blue[4];
      else if (p2Here)
        bgCol = Ramps::Red[4];

      e.rectfill(cardX, cardY, cardW, cardH, bgCol);
      e.rect(cardX, cardY, cardW, cardH, def.mainColor);

      // Name & Titel
      e.draw_text(cardX + 3, cardY + 5, def.name, Colors::White, 1);
      e.draw_text(cardX + 3, cardY + 16, def.title, def.accentColor, 1);

      // Vorschau-Sprite in der Karte
      Player dummy;
      dummy.x = cardX + 24.0f;
      dummy.y = cardY + 70.0f;
      dummy.fighter = static_cast<FighterType>(i);
      dummy.facing = 1;
      renderFighter(e, dummy);

      // Superkraft-Name
      e.draw_text(cardX + 3, cardY + 86, "KRAFT:", Colors::Gold, 1);
      e.draw_text(cardX + 3, cardY + 97, def.powerName, Colors::White, 1);

      // Cursor-Markierungen
      if (p1Here) {
        std::string tag = p1Locked ? "[P1 OK]" : "> P1 <";
        e.draw_text(cardX + 3, cardY + 112, tag, Ramps::Blue[12], 1);
      }
      if (p2Here) {
        std::string tag = p2Locked ? "[P2 OK]" : "> P2 <";
        e.draw_text(cardX + 3, cardY + (p1Here ? 123 : 112), tag,
                    Ramps::Red[12], 1);
      }
    }

    // Fußzeile Steuerung
    e.rectfill(0, 195, 320, 45, Colors::Black);
    e.line(0, 195, 320, 195, Colors::White);
    e.draw_text(16, 204, "SPIELER 1: A / D = WAEHLEN   W / LEERTASTE = LOCK-IN",
                Ramps::Blue[12], 1);
    e.draw_text(16, 218, "SPIELER 2: LINKS / RECHTS     UP / ENTER = LOCK-IN",
                Ramps::Red[12], 1);
  }

  // ===========================================================================
  // 13. PARTIKEL-GENERATOREN
  // ===========================================================================
  void spawnGroundWaveParticles(const Projectile &proj) {
    Particle p;
    p.x = proj.x + (rand() % 8 - 4);
    p.y = proj.y + 2.0f;
    p.vx = -proj.vx * 0.10f + (rand() % 16 - 8);
    p.vy = -15.0f - (rand() % 25);
    p.maxLife = 0.20f + (rand() % 15) / 100.0f;
    p.life = p.maxLife;
    p.size = 1.5f;

    switch (proj.type) {
    case FighterType::Music:
      p.color = (rand() % 2 == 0) ? Colors::Gold : Ramps::Fire[12];
      break;
    case FighterType::Ice:
      p.color = (rand() % 2 == 0) ? Ramps::Cyan[14] : Colors::White;
      break;
    case FighterType::Fire:
      p.color = Ramps::Fire[9 + (rand() % 6)];
      break;
    case FighterType::Lightning:
      p.color = (rand() % 2 == 0) ? Ramps::Yellow[14] : Colors::White;
      break;
    case FighterType::Water:
      p.color = (rand() % 2 == 0) ? Ramps::Cyan[12] : Ramps::Blue[10];
      break;
    case FighterType::Hacker:
      p.color = (rand() % 2 == 0) ? Ramps::Green[14] : Ramps::Cyan[14];
      break;
    }
    particles.push_back(p);
  }

  void spawnProjectileTrail(const Projectile &proj) {
    Particle p;
    p.x = proj.x + (rand() % 7 - 3);
    p.y = proj.y + (rand() % 7 - 3);
    p.vx = -proj.vx * 0.15f + (rand() % 20 - 10);
    p.vy = (rand() % 20 - 10);
    p.maxLife = 0.25f + (rand() % 15) / 100.0f;
    p.life = p.maxLife;
    p.size = 1.0f;

    switch (proj.type) {
    case FighterType::Music:
      p.color = (rand() % 2 == 0) ? Colors::Gold : Colors::White;
      break;
    case FighterType::Ice:
      p.color = (rand() % 2 == 0) ? Ramps::Cyan[13] : Colors::White;
      break;
    case FighterType::Fire:
      p.color = Ramps::Fire[8 + (rand() % 7)];
      break;
    case FighterType::Lightning:
      p.color = (rand() % 2 == 0) ? Ramps::Yellow[14] : Colors::White;
      break;
    case FighterType::Water:
      p.color = (rand() % 2 == 0) ? Ramps::Cyan[10] : Ramps::Blue[12];
      break;
    case FighterType::Hacker:
      p.color = (rand() % 2 == 0) ? Ramps::Green[13] : Colors::White;
      break;
    }
    particles.push_back(p);
  }

  void spawnHitExplosion(float x, float y, FighterType type) {
    for (int i = 0; i < 28; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      float ang = (rand() % 360) * (3.14159265f / 180.0f);
      float spd = 20.0f + (rand() % 90);
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd - 20.0f;
      p.maxLife = 0.35f + (rand() % 25) / 100.0f;
      p.life = p.maxLife;
      p.size = 2.0f;

      switch (type) {
      case FighterType::Music:
        p.color = (i % 2 == 0) ? Colors::Gold : Ramps::Red[9];
        break;
      case FighterType::Ice:
        p.color = (i % 2 == 0) ? Colors::White : Ramps::Cyan[13];
        break;
      case FighterType::Fire:
        p.color = Ramps::Fire[8 + (i % 8)];
        break;
      case FighterType::Lightning:
        p.color = (i % 2 == 0) ? Ramps::Yellow[14] : Colors::White;
        break;
      case FighterType::Water:
        p.color = (i % 2 == 0) ? Ramps::Cyan[11] : Ramps::Blue[10];
        break;
      case FighterType::Hacker:
        p.color = (i % 2 == 0) ? Ramps::Green[14] : Ramps::Cyan[13];
        break;
      }
      particles.push_back(p);
    }
  }

  void spawnFireMegaExplosion(float x, float y) {
    // 💥 RIESIGE FEUER-EXPLOSION: Mehrstufige Druckwelle, Funkenregen &
    // Rauchwolke
    fluidSolver.addRadialImpulse(x, y, 220.0f, 26.0f);
    spawnFireStream(x, y, 0.0f, -50.0f, -1, 22);

    for (int i = 0; i < 65; ++i) {
      Particle p;
      p.x = x + (rand() % 12 - 6);
      p.y = y + (rand() % 12 - 6);
      float ang = (rand() % 360) * (3.14159265f / 180.0f);
      float spd = 25.0f + (rand() % 140);
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd - 25.0f;
      p.maxLife = 0.40f + (rand() % 30) / 100.0f;
      p.life = p.maxLife;
      p.size = (i % 3 == 0) ? 2.5f : 1.5f;

      if (i < 18) {
        p.color = Colors::White; // Weißglühender Explosionskern
      } else if (i < 42) {
        p.color = Ramps::Fire[10 + (rand() % 6)]; // Lodernde Feuerflammen
      } else if (i < 56) {
        p.color = Ramps::Fire[6 + (rand() % 4)]; // Rote Glut
      } else {
        p.color = Ramps::Grays[3 + (rand() % 3)]; // Dunkler Rauch
      }
      particles.push_back(p);
    }
  }

  // --- SUPERNOVA EFFEKTE: KLEIN BEI TREFFER, 2X RIESIG BEI K.O. ---
  void spawnSupernova(float x, float y, FighterType type, bool isMega) {
    Supernova sn;
    sn.x = x;
    sn.y = y;
    sn.isMega = isMega;
    sn.maxRadius = isMega ? 42.0f : 16.0f; // 2x+ RIESIGE SUPERNOVA BEI K.O.!
    sn.maxLife = isMega ? 0.65f : 0.28f;
    sn.life = sn.maxLife;

    switch (type) {
    case FighterType::Music:
      sn.coreColor = Colors::White;
      sn.ringColor = Colors::Gold;
      sn.rayColor = Ramps::Red[9];
      break;
    case FighterType::Ice:
      sn.coreColor = Colors::White;
      sn.ringColor = Ramps::Cyan[14];
      sn.rayColor = Ramps::Cyan[12];
      break;
    case FighterType::Fire:
      sn.coreColor = Colors::White;
      sn.ringColor = Ramps::Fire[12];
      sn.rayColor = Ramps::Fire[14];
      break;
    case FighterType::Lightning:
      sn.coreColor = Colors::White;
      sn.ringColor = Ramps::Yellow[15];
      sn.rayColor = Ramps::Yellow[13];
      break;
    case FighterType::Water:
      sn.coreColor = Colors::White;
      sn.ringColor = Ramps::Cyan[11];
      sn.rayColor = Ramps::Blue[10];
      break;
    case FighterType::Hacker:
      sn.coreColor = Colors::White;
      sn.ringColor = Ramps::Green[14];
      sn.rayColor = Ramps::Cyan[13];
      break;
    }
    supernovas.push_back(sn);

    // Begleitende Supernova-Strahlenpartikel
    int numParts = isMega ? 55 : 20;
    for (int i = 0; i < numParts; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      float ang = (rand() % 360) * (3.14159265f / 180.0f);
      float spd = (isMega ? 45.0f : 20.0f) + (rand() % (isMega ? 150 : 80));
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd;
      p.maxLife = (isMega ? 0.45f : 0.25f) + (rand() % 20) / 100.0f;
      p.life = p.maxLife;
      p.size = (i % 2 == 0) ? (isMega ? 2.5f : 1.5f) : 1.0f;
      p.color = (i % 3 == 0) ? Colors::White
                             : ((i % 3 == 1) ? sn.ringColor : sn.rayColor);
      particles.push_back(p);
    }
  }

  void updateSupernovas(float dt) {
    for (size_t i = 0; i < supernovas.size();) {
      supernovas[i].life -= dt;
      if (supernovas[i].life <= 0.0f) {
        supernovas[i] = supernovas.back();
        supernovas.pop_back();
      } else {
        ++i;
      }
    }
  }

  void spawnShieldParticles(float x, float y, uint8_t col) {
    Particle p;
    p.x = x;
    p.y = y + (rand() % 16 - 8);
    p.vx = (rand() % 20 - 10);
    p.vy = -15.0f - (rand() % 25);
    p.maxLife = 0.20f;
    p.life = p.maxLife;
    p.color = col;
    p.size = 1.5f;
    particles.push_back(p);
  }

  void spawnBlockSparks(float x, float y) {
    for (int i = 0; i < 14; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      p.vx = (rand() % 80 - 40);
      p.vy = -30.0f - (rand() % 50);
      p.maxLife = 0.20f + (rand() % 15) / 100.0f;
      p.life = p.maxLife;
      p.color = (i % 2 == 0) ? Colors::White : Colors::Gold;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void spawnJumpDust(float x, float y) {
    for (int i = 0; i < 8; ++i) {
      Particle p;
      p.x = x + (rand() % 10 - 5);
      p.y = y;
      p.vx = (rand() % 40 - 20);
      p.vy = -10.0f - (rand() % 20);
      p.maxLife = 0.18f;
      p.life = p.maxLife;
      p.color = Ramps::Grays[8];
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void spawnDoubleJumpSparkle(float x, float y, uint8_t col) {
    for (int i = 0; i < 10; ++i) {
      Particle p;
      p.x = x + (rand() % 12 - 6);
      p.y = y - 2.0f;
      p.vx = (rand() % 50 - 25);
      p.vy = 10.0f + (rand() % 20);
      p.maxLife = 0.22f;
      p.life = p.maxLife;
      p.color = (i % 2 == 0) ? col : Colors::White;
      p.size = 1.5f;
      particles.push_back(p);
    }
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
        ++i;
      }
    }
  }

  // ===========================================================================
  // 14. SYNTHESIZER SOUNDS
  // ===========================================================================
  void playMenuBeep(Engine &e) { e.play_tone(Notes::E4, 0.06f); }

  void playLockSound(Engine &e) {
    e.play_tone(Notes::A4, 0.08f);
    e.play_tone(Notes::E5, 0.12f);
  }

  void playFightGong(Engine &e) { e.play_tone(Notes::D3, 0.40f); }

  void playJumpSound(Engine &e) { e.play_tone(Notes::G4, 0.06f); }

  void playGuitarDragonSound(Engine &e) {
    // Epischer E-Gitarren Power Chord + Drachen-Dröhnen
    e.play_tone(Notes::E2, 0.25f);
    e.play_tone(Notes::E3, 0.20f);
    e.play_tone(Notes::B3, 0.20f);
    e.play_tone(Notes::G4, 0.20f);
  }

  void playIceCastSound(Engine &e) {
    e.play_tone(Notes::C5, 0.10f);
    e.play_tone(Notes::Fs5, 0.12f);
  }

  void playFireCastSound(Engine &e) { e.play_tone(Notes::A2, 0.20f); }

  void playFireExplosionSound(Engine &e) {
    // Satter, dröhnender Explosions-Knall
    e.play_tone(Notes::C2, 0.32f);
    e.play_tone(Notes::F2, 0.22f);
    e.play_tone(Notes::G2, 0.16f);
  }

  void playThunderSound(Engine &e) { e.play_tone(Notes::Ds5, 0.08f); }

  void playWaterSplashSound(Engine &e) { e.play_tone(Notes::F3, 0.15f); }

  void playHackerCastSound(Engine &e) {
    // 8-Bit Cyber Modem-Arpeggio & Tastatur-Pew
    e.play_tone(Notes::B5, 0.035f);
    e.play_tone(Notes::E6, 0.045f);
    e.play_tone(Notes::Gs5, 0.055f);
  }

  void playCyberGlitchSound(Engine &e) {
    // Digitaler Glitch- & Terminal-Sound
    e.play_tone(Notes::As4, 0.05f);
    e.play_tone(Notes::F5, 0.07f);
    e.play_tone(Notes::D6, 0.10f);
  }

  void playBlockSound(Engine &e) { e.play_tone(Notes::A5, 0.08f); }

  void playHitSound(Engine &e) { e.play_tone(Notes::B2, 0.12f); }

  void playKoSound(Engine &e) { e.play_tone(Notes::D2, 0.50f); }

  void playVictoryFanfare(Engine &e) {
    e.play_tone(Notes::C4, 0.12f);
    e.play_tone(Notes::E4, 0.12f);
    e.play_tone(Notes::G4, 0.12f);
    e.play_tone(Notes::C5, 0.35f);
  }

  // ===========================================================================
  // NAVIER-STOKES FLUID SIMULATION & ELEMENTAR-PARTIKEL
  // ===========================================================================

  void spawnWaterStream(float startX, float startY, float vx, float vy, int owner, int count) {
    fluidSolver.addWaterDensity(startX, startY, 0.45f, 16.0f);
    for (int i = 0; i < count; ++i) {
      FluidParticle p;
      p.x = startX + (rand() % 8 - 4);
      p.y = startY + (rand() % 8 - 4);
      float spreadAng = ((rand() % 31) - 15) * 0.0174533f; // +/- 15 Grad Streuung
      float cosA = std::cos(spreadAng);
      float sinA = std::sin(spreadAng);
      float spdMult = 0.75f + (rand() % 50) / 100.0f;
      p.vx = (vx * cosA - vy * sinA) * spdMult;
      p.vy = (vx * sinA + vy * cosA) * spdMult;
      p.maxLife = 1.0f + (rand() % 60) / 100.0f; // 1.0 bis 1.6s
      p.life = p.maxLife;
      p.size = (rand() % 3 == 0) ? 2.0f : 1.2f;
      p.type = FluidElementType::Water;
      p.owner = owner;
      p.active = true;
      fluidParticles.push_back(p);
    }
  }

  void spawnFireStream(float startX, float startY, float vx, float vy, int owner, int count) {
    fluidSolver.addFireDensity(startX, startY, 0.50f, 15.0f);
    for (int i = 0; i < count; ++i) {
      FluidParticle p;
      p.x = startX + (rand() % 6 - 3);
      p.y = startY + (rand() % 6 - 3);
      float spreadAng = ((rand() % 35) - 17) * 0.0174533f;
      float cosA = std::cos(spreadAng);
      float sinA = std::sin(spreadAng);
      float spdMult = 0.70f + (rand() % 60) / 100.0f;
      p.vx = (vx * cosA - vy * sinA) * spdMult;
      p.vy = (vx * sinA + vy * cosA) * spdMult;
      p.maxLife = 0.75f + (rand() % 50) / 100.0f; // 0.75 bis 1.25s
      p.life = p.maxLife;
      p.size = (rand() % 2 == 0) ? 2.0f : 1.4f;
      p.type = FluidElementType::Fire;
      p.owner = owner;
      p.active = true;
      fluidParticles.push_back(p);
    }
  }

  void spawnWindParticles(float startX, float startY, float vx, float vy, int owner, int count) {
    for (int i = 0; i < count; ++i) {
      FluidParticle p;
      p.x = startX + (rand() % 10 - 5);
      p.y = startY + (rand() % 10 - 5);
      float spdMult = 0.8f + (rand() % 40) / 100.0f;
      float noiseY = (rand() % 40 - 20);
      p.vx = vx * spdMult;
      p.vy = vy * spdMult + noiseY;
      p.maxLife = 0.45f + (rand() % 30) / 100.0f; // 0.45 bis 0.75s
      p.life = p.maxLife;
      p.size = 1.0f;
      p.type = FluidElementType::Wind;
      p.owner = owner;
      p.active = true;
      fluidParticles.push_back(p);
    }
  }

  void spawnSteamBurst(float x, float y, int count) {
    for (int i = 0; i < count; ++i) {
      FluidParticle p;
      p.x = x + (rand() % 8 - 4);
      p.y = y + (rand() % 8 - 4);
      float ang = (rand() % 360) * 0.0174533f;
      float spd = 20.0f + (rand() % 60);
      p.vx = std::cos(ang) * spd;
      p.vy = std::sin(ang) * spd - 30.0f; // Steigt schnell nach oben
      p.maxLife = 0.60f + (rand() % 40) / 100.0f;
      p.life = p.maxLife;
      p.size = 2.0f + (rand() % 20) / 10.0f;
      p.type = FluidElementType::Steam;
      p.owner = -1; // Neutral
      p.active = true;
      fluidParticles.push_back(p);
    }
  }

  void updateFluidParticles(Engine &e, float dt) {
    // Partikel-Pool begrenzen (maximal 260 gleichzeitige Fluid-Partikel für geschmeidige 60 FPS)
    if (fluidParticles.size() > 260) {
      fluidParticles.erase(fluidParticles.begin(),
                           fluidParticles.begin() + (fluidParticles.size() - 260));
    }

    for (size_t i = 0; i < fluidParticles.size();) {
      auto &p = fluidParticles[i];
      p.life -= dt;
      if (p.life <= 0.0f || !p.active) {
        fluidParticles[i] = fluidParticles.back();
        fluidParticles.pop_back();
        continue;
      }

      // 1. Bilineares Abtasten der Fluid-Strömung aus dem Navier-Stokes Gitter
      float fvx = 0.0f, fvy = 0.0f;
      fluidSolver.sampleVelocity(p.x, p.y, fvx, fvy);

      float drag = (p.type == FluidElementType::Wind)
                       ? 16.0f
                       : ((p.type == FluidElementType::Water) ? 9.0f : 12.0f);
      p.vx += (fvx - p.vx) * std::min(1.0f, drag * dt);
      p.vy += (fvy - p.vy) * std::min(1.0f, drag * dt);

      // 2. Schwerkraft & Auftrieb
      if (p.type == FluidElementType::Water) {
        p.vy += 90.0f * dt; // Schwerkraft zieht Wassertropfen nach unten
      } else if (p.type == FluidElementType::Fire) {
        p.vy -= 70.0f * dt; // Thermischer Auftrieb lässt Flammen aufsteigen
      } else if (p.type == FluidElementType::Steam) {
        p.vy -= 45.0f * dt; // Dampf steigt sanft nach oben
      }

      p.x += p.vx * dt;
      p.y += p.vy * dt;

      // Kopplung der Partikel an das Eulersche Navier-Stokes Gitter:
      // Kontinuierliche Einspeisung von Dichte & Impuls entlang der Flugbahn
      if (p.type == FluidElementType::Water) {
        fluidSolver.addWaterDensity(p.x, p.y, 0.35f * dt, 4.0f);
        fluidSolver.addVelocity(p.x, p.y, p.vx * 0.25f * dt, p.vy * 0.25f * dt, 5.0f);
      } else if (p.type == FluidElementType::Fire) {
        fluidSolver.addFireDensity(p.x, p.y, 0.40f * dt, 4.0f);
        fluidSolver.addVelocity(p.x, p.y, p.vx * 0.20f * dt, (p.vy - 30.0f) * 0.20f * dt, 5.0f);
      } else if (p.type == FluidElementType::Wind) {
        fluidSolver.addVelocity(p.x, p.y, p.vx * 0.50f * dt, p.vy * 0.50f * dt, 6.0f);
      }

      // 3. Bildschirmgrenzen
      if (p.x < 2.0f || p.x > 318.0f || p.y < 0.0f || p.y > 238.0f) {
        p.active = false;
        fluidParticles[i] = fluidParticles.back();
        fluidParticles.pop_back();
        continue;
      }

      // 4. Gelände- & Plattformkollision
      float gy = getGroundHeight(p.x);
      if (p.y >= gy - 1.5f) {
        p.y = gy - 1.5f;
        if (p.type == FluidElementType::Water) {
          p.vy = -p.vy * 0.15f; // Geringe Abprallhöhe
          // Wasser fließt Hänge hinab (Gradienten-Kollision)
          float slope =
              (getGroundHeight(p.x + 3.0f) - getGroundHeight(p.x - 3.0f)) / 6.0f;
          p.vx += slope * 160.0f * dt;
          p.vx *= 0.94f;
          if (p.life < 0.35f && e.rnd(10) == 0) {
            spawnWaterPuddle(p.x, p.owner);
          }
        } else if (p.type == FluidElementType::Fire) {
          p.life -= dt * 2.5f;
          if (p.life > 0.4f && e.rnd(20) == 0) {
            igniteGroundEverywhere(p.x, p.owner);
          }
        } else if (p.type == FluidElementType::Steam) {
          p.life -= dt * 2.0f;
        } else if (p.type == FluidElementType::Wind) {
          p.active = false;
        }
      }

      // Schwebende Plattformen
      for (const auto &plat : platforms) {
        if (p.x >= plat.x1 && p.x <= plat.x2 && p.y >= plat.y - 2.0f &&
            p.y <= plat.y + 4.0f && p.vy > 0.0f) {
          p.y = plat.y - 2.0f;
          p.vy = -p.vy * 0.20f;
          p.vx *= 0.94f;
          break;
        }
      }

      // 5. Elementar-Chemie: Wasser trifft Feuer = Dampf-Explosion!
      if (p.type == FluidElementType::Water || p.type == FluidElementType::Fire) {
        for (size_t j = i + 1; j < fluidParticles.size(); ++j) {
          auto &other = fluidParticles[j];
          if (!other.active)
            continue;
          if ((p.type == FluidElementType::Water &&
               other.type == FluidElementType::Fire) ||
              (p.type == FluidElementType::Fire &&
               other.type == FluidElementType::Water)) {
            float cdx = p.x - other.x;
            float cdy = p.y - other.y;
            if (cdx * cdx + cdy * cdy <= 36.0f) { // <= 6 Pixel Abstand
              float midX = (p.x + other.x) * 0.5f;
              float midY = (p.y + other.y) * 0.5f;
              p.active = false;
              other.active = false;
              fluidSolver.addRadialImpulse(midX, midY, 150.0f, 18.0f);
              spawnSteamBurst(midX, midY, 6);
              playSteamHissSound(e);
              break;
            }
          }
        }
      }

      if (!p.active) {
        fluidParticles[i] = fluidParticles.back();
        fluidParticles.pop_back();
        continue;
      }

      // 6. Trefferprüfung gegen gegnerischen Spieler
      for (int pid = 0; pid < 2; ++pid) {
        if (p.owner != -1 && p.owner == pid)
          continue;
        Player &target = players[pid];

        float dx = p.x - target.x;
        float dy = p.y - (target.y - 9.0f);
        if (dx * dx + dy * dy <= 10.0f * 10.0f) {
          if (target.isBlocking) {
            // Schild reflektiert Fluid-Partikel!
            p.vx = -p.vx * 0.70f + (rand() % 40 - 20);
            p.vy = -p.vy * 0.70f + (rand() % 40 - 20);
            p.owner = target.id;
            spawnBlockSparks(p.x, p.y);
            if (e.rnd(4) == 0)
              playBlockSound(e);
          } else {
            // Treffer! Impulsübertragung & Schaden
            p.active = false;
            target.vx += p.vx * 0.04f;
            if (p.type == FluidElementType::Water) {
              target.slowTimer = std::max(target.slowTimer, 0.45f);
            } else if (p.type == FluidElementType::Wind) {
              target.vy -= 40.0f;
              target.vx += p.vx * 0.15f;
            }

            float &dmgTimer = (pid == 0) ? p1FluidDmgTimer : p2FluidDmgTimer;
            if (dmgTimer <= 0.0f) {
              int dmg = (p.type == FluidElementType::Fire) ? 3 : 2;
              target.hp = std::max(0, target.hp - dmg);
              target.hurtTimer = 0.12f;
              dmgTimer = 0.08f; // Verhindert Sofort-Tod durch viele Partikel
              playHitSound(e);
            }
          }
          break;
        }
      }

      ++i;
    }
  }

  void renderFluidDensity(Engine &e) {
    const auto &densW = fluidSolver.getWaterDensity();
    const auto &densF = fluidSolver.getFireDensity();
    constexpr int GW = FluidSolver2D::GW;
    constexpr int GH = FluidSolver2D::GH;

    for (int gy = 0; gy < GH; ++gy) {
      int screenY = gy * 2;
      int rowIdx = gy * GW;
      for (int gx = 0; gx < GW; ++gx) {
        int i = rowIdx + gx;
        float dw = densW[i];
        float df = densF[i];

        if (dw > 0.06f) {
          int screenX = gx * 2;
          uint8_t col = (dw > 0.30f) ? Ramps::Cyan[11]
                       : (dw > 0.16f) ? Ramps::Cyan[8]
                                      : Ramps::Blue[13];
          e.pset(screenX, screenY, col);
          e.pset(screenX + 1, screenY, col);
          e.pset(screenX, screenY + 1, col);
          e.pset(screenX + 1, screenY + 1, col);
        } else if (df > 0.06f) {
          int screenX = gx * 2;
          uint8_t col = (df > 0.35f) ? Ramps::Fire[12]
                       : (df > 0.18f) ? Ramps::Fire[8]
                                      : Ramps::Fire[4];
          e.pset(screenX, screenY, col);
          e.pset(screenX + 1, screenY, col);
          e.pset(screenX, screenY + 1, col);
          e.pset(screenX + 1, screenY + 1, col);
        }
      }
    }
  }

  void renderFluidParticles(Engine &e) {
    for (const auto &p : fluidParticles) {
      if (!p.active)
        continue;
      int px = static_cast<int>(p.x);
      int py = static_cast<int>(p.y);
      if (px < 0 || px >= 320 || py < 0 || py >= 240)
        continue;

      switch (p.type) {
      case FluidElementType::Water: {
        float spdSq = p.vx * p.vx + p.vy * p.vy;
        if (spdSq > 75.0f * 75.0f) {
          e.pset(px, py, Colors::White);
          int tx = static_cast<int>(p.x - p.vx * 0.015f);
          int ty = static_cast<int>(p.y - p.vy * 0.015f);
          if (tx >= 0 && tx < 320 && ty >= 0 && ty < 240) {
            e.pset(tx, ty, Ramps::Cyan[11]);
          }
        } else {
          e.rectfill(px - 1, py - 1, 2, 2, Ramps::Cyan[9]);
          e.pset(px, py, Ramps::Blue[13]);
        }
        break;
      }
      case FluidElementType::Fire: {
        float t = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
        uint8_t col = Ramps::Fire.sample(t);
        if (t > 0.65f) {
          e.rectfill(px - 1, py - 1, 2, 2, col);
          e.pset(px, py, Colors::White);
        } else {
          e.pset(px, py, col);
        }
        break;
      }
      case FluidElementType::Wind: {
        int tx = static_cast<int>(p.x - p.vx * 0.035f);
        int ty = static_cast<int>(p.y - p.vy * 0.035f);
        e.line(px, py, tx, ty, Ramps::Grays[11]);
        e.pset(px, py, Colors::White);
        break;
      }
      case FluidElementType::Steam: {
        float prog = 1.0f - (p.life / p.maxLife);
        float r = p.size + prog * 3.5f;
        uint8_t steamCol =
            Ramps::Grays[std::clamp(6 + static_cast<int>(p.life * 8.0f), 0, 15)];
        e.circle(px, py, r, steamCol);
        if (r > 2.5f) {
          e.pset(px, py, Colors::White);
        }
        break;
      }
      }
    }
  }

  void playSteamHissSound(Engine &e) {
    e.play_tone(Notes::B6, 0.05f);
    e.play_tone(Notes::F6, 0.07f);
  }

  void playWindWhooshSound(Engine &e) {
    e.play_tone(Notes::G3, 0.08f);
    e.play_tone(Notes::D3, 0.12f);
  }

  // ===========================================================================
  // SUB-PIXEL 2X FLUID-REFRAKTION & GESCHWINDIGKEITS-FALTUNG (640x480)
  // ===========================================================================
  bool customPresent(sf::Image &image, unsigned outW, unsigned outH,
                     const std::vector<uint8_t> &buffer, int srcW, int srcH,
                     const std::array<sf::Color, 256> &palette) override {
    if (state == State::CharacterSelect) {
      return false; // Im Charakter-Auswahlmenü Standard-Pixelverdopplung
    }

    // 2x-Sub-Pixel Convolution & Optical Flow Refraction
    for (int by = 0; by < srcH; ++by) {
      int srcRow = by * srcW;
      unsigned dstY0 = static_cast<unsigned>(by * 2);
      unsigned dstY1 = dstY0 + 1;

      for (int bx = 0; bx < srcW; ++bx) {
        float sx = static_cast<float>(bx);
        float sy = static_cast<float>(by);

        float vx = 0.0f, vy = 0.0f;
        fluidSolver.sampleVelocity(sx, sy, vx, vy);
        float spdSq = vx * vx + vy * vy;
        float waterDens = fluidSolver.sampleWaterDensity(sx, sy);
        float fireDens = fluidSolver.sampleFireDensity(sx, sy);
        float curlVal = std::abs(fluidSolver.sampleCurl(sx, sy));

        unsigned dstX0 = static_cast<unsigned>(bx * 2);
        unsigned dstX1 = dstX0 + 1;

        // Wenn keine nennenswerte Strömung / Dichte / Wirbel vorhanden ist:
        // Gestochen scharfer, nativer 2x Retro-Pixel ohne Rechenaufwand!
        if (spdSq < 20.0f && curlVal < 0.6f && waterDens < 0.010f && fireDens < 0.010f) {
          sf::Color baseCol = palette[buffer[srcRow + bx]];
          image.setPixel({dstX0, dstY0}, baseCol);
          image.setPixel({dstX1, dstY0}, baseCol);
          image.setPixel({dstX0, dstY1}, baseCol);
          image.setPixel({dstX1, dstY1}, baseCol);
          continue;
        }

        // Bei aktiver Strömung: Kontinuierliche Sub-Pixel Refraktion & 3-Tap Faltung
        float dispX = std::clamp(vx * 0.038f, -6.0f, 6.0f);
        float dispY = std::clamp(vy * 0.038f, -6.0f, 6.0f);

        for (int subY = 0; subY < 2; ++subY) {
          for (int subX = 0; subX < 2; ++subX) {
            float curSx = sx + subX * 0.5f;
            float curSy = sy + subY * 0.5f;

            // 3-Tap Abtastung entlang des Strömungsvektors
            float tap1X = std::clamp(curSx - dispX * 1.15f, 0.0f, 319.0f);
            float tap1Y = std::clamp(curSy - dispY * 1.15f, 0.0f, 239.0f);
            float tap2X = std::clamp(curSx - dispX * 0.50f, 0.0f, 319.0f);
            float tap2Y = std::clamp(curSy - dispY * 0.50f, 0.0f, 239.0f);
            float tap3X = std::clamp(curSx + dispX * 0.35f, 0.0f, 319.0f);
            float tap3Y = std::clamp(curSy + dispY * 0.35f, 0.0f, 239.0f);

            int i1 = static_cast<int>(tap1Y) * srcW + static_cast<int>(tap1X);
            int i2 = static_cast<int>(tap2Y) * srcW + static_cast<int>(tap2X);
            int i3 = static_cast<int>(tap3Y) * srcW + static_cast<int>(tap3X);

            sf::Color c1 = palette[buffer[i1]];
            sf::Color c2 = palette[buffer[i2]];
            sf::Color c3 = palette[buffer[i3]];

            // 25% - 50% - 25% Richtungs-Faltung (Bewegungsunschärfe entlang Strömung)
            int r = static_cast<int>(c1.r * 0.25f + c2.r * 0.50f + c3.r * 0.25f);
            int g = static_cast<int>(c1.g * 0.25f + c2.g * 0.50f + c3.g * 0.25f);
            int b = static_cast<int>(c1.b * 0.25f + c2.b * 0.50f + c3.b * 0.25f);

            // Spektrale Tönung durch das Eulersche Dichtefeld:
            // 1. Wasser-Strömung: Kristall-Türkis/Azur Schimmer
            if (waterDens > 0.010f) {
              float wAlpha = std::min(0.40f, waterDens * 0.50f);
              r = static_cast<int>(r * (1.0f - wAlpha) + 35.0f * wAlpha);
              g = static_cast<int>(g * (1.0f - wAlpha) + 165.0f * wAlpha);
              b = static_cast<int>(b * (1.0f - wAlpha) + 250.0f * wAlpha);
            }
            // 2. Feuer/Hitze-Auftrieb: Warmer Bernstein-/Gold-Glanz
            if (fireDens > 0.010f) {
              float fAlpha = std::min(0.45f, fireDens * 0.60f);
              r = static_cast<int>(r * (1.0f - fAlpha) + 255.0f * fAlpha);
              g = static_cast<int>(g * (1.0f - fAlpha) + 130.0f * fAlpha);
              b = static_cast<int>(b * (1.0f - fAlpha) + 25.0f * fAlpha);
            }

            unsigned px = dstX0 + subX;
            unsigned py = dstY0 + subY;
            image.setPixel({px, py}, sf::Color(static_cast<uint8_t>(std::clamp(r, 0, 255)),
                                               static_cast<uint8_t>(std::clamp(g, 0, 255)),
                                               static_cast<uint8_t>(std::clamp(b, 0, 255))));
          }
        }
      }
    }

    return true;
  }
};
