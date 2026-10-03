#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// =============================================================================
// SWARMFRONT: NANITE WARS (Game 8 - Real-Time Swarm Strategy)
// =============================================================================
// Features:
// - 1000+ Units Flocking Simulation (Boids: Separation, Alignment, Cohesion)
// - Rock-Paper-Scissors Swarm Triad:
//     1. ⚡ NEEDLE (Melee/Swarm) -> Strong against Pulse Snipers
//     2. 💥 ACID (AoE Splash)   -> Strong against Needle Swarms
//     3. 🎯 PULSE (Laser Beam)  -> Strong against Acid Spitters
//     4. 👁️ SCOUT (Recon)       -> Clears Fog of War & Fast Harvester
// - Zero-Waste Biomass Recycling: Fallen units drop biomass to be eaten & reborn
// - Macro Pheromone Beacon Steering (Click/Drag to command the swarm flow)
// - Fog of War & Seismic Minimap Radar
// =============================================================================

struct SwarmGame : Game {
public:
  enum class UnitType : uint8_t {
    Needle = 0, // Schnelle Nahkampf-Stacheln
    Acid = 1,   // Säure-Spucker (AoE Flächenschaden)
    Pulse = 2,  // Impuls-Strahler (Reichweiten-Laser)
    Scout = 3   // Späher-Fliege (Schnell, hohe Sicht)
  };

  enum class Directive : uint8_t {
    Attack = 0,   // Schwarm fließt zum Angriffs-Leuchtfeuer
    Defend = 1,   // Schwarm bildet Schutzwall um die eigene Basis
    Harvester = 2,// Schwarm sammelt Biomasse & Mineralien
    Disperse = 3  // Schwarm streut sich weit aus (weicht AoE-Säure aus)
  };

  enum class State {
    Playing,
    Victory,
    Defeat
  };

  struct Unit {
    float x, y;
    float vx, vy;
    int hp, maxHp;
    uint8_t team; // 0 = Spieler (Cyan/Blau), 1 = KI (Rot/Orange)
    UnitType type;
    float attackTimer;
    bool active;
  };

  struct Projectile {
    float x, y;
    float vx, vy;
    float targetX, targetY;
    uint8_t team;
    UnitType type;
    int damage;
    float life;
    float maxLife;
    bool active;
  };

  struct AcidPool {
    float x, y;
    float radius;
    float life;
    float maxLife;
    int damagePerSec;
    uint8_t team;
    bool active;
  };

  struct BiomassDrop {
    float x, y;
    int value;
    float life;
    bool active;
  };

  struct ResourceNode {
    float x, y;
    float radius;
    int owner; // -1 = Neutral, 0 = Spieler, 1 = KI
    float captureProgress; // -100.0f (KI) bis +100.0f (Spieler)
    float harvestTimer;
  };

  struct Hive {
    float x, y;
    float radius;
    int hp, maxHp;
    int biomass;
    uint8_t team;
    std::vector<UnitType> queue;
    float spawnTimer;
  };

  struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    uint8_t color;
    float size;
  };

  struct LaserBeam {
    float x1, y1, x2, y2;
    float life;
    uint8_t color;
  };

  // ===========================================================================
  // 1. SPIEL-KONSTANTEN & FELDER
  // ===========================================================================
  static constexpr int MAX_UNITS = 1200;
  static constexpr int GRID_W = 20; // 320 / 16
  static constexpr int GRID_H = 15; // 240 / 16
  static constexpr int FOG_W = 32;  // 320 / 10
  static constexpr int FOG_H = 24;  // 240 / 10

  State state = State::Playing;
  float matchTimer = 0.0f;
  float screenShake = 0.0f;

  Hive playerHive;
  Hive enemyHive;

  // Pheromon-Leuchtfeuer
  float pBeaconX = 160.0f;
  float pBeaconY = 120.0f;
  bool pBeaconActive = false;
  Directive pDirective = Directive::Attack;
  bool autoBuild = true;

  float eBeaconX = 160.0f;
  float eBeaconY = 120.0f;
  Directive eDirective = Directive::Attack;
  float aiDecisionTimer = 0.0f;

  std::vector<Unit> units;
  std::vector<Projectile> projectiles;
  std::vector<AcidPool> acidPools;
  std::vector<BiomassDrop> biomassDrops;
  std::vector<ResourceNode> nodes;
  std::vector<Particle> particles;
  std::vector<LaserBeam> lasers;

  // Spatial Partitioning Grid für Boids
  std::vector<int> grid[GRID_W][GRID_H];

  // Fog of War (0 = Unentdeckt, 1 = Sichtbar)
  std::array<uint8_t, FOG_W * FOG_H> fogOfWar;

  // ===========================================================================
  // 2. INITIALISIERUNG
  // ===========================================================================
  SwarmGame() {
    initMatch();
  }

  void initMatch() {
    state = State::Playing;
    matchTimer = 0.0f;
    screenShake = 0.0f;

    units.clear();
    projectiles.clear();
    acidPools.clear();
    biomassDrops.clear();
    nodes.clear();
    particles.clear();
    lasers.clear();

    // 1. Basen initialisieren
    playerHive.x = 28.0f;
    playerHive.y = 120.0f;
    playerHive.radius = 18.0f;
    playerHive.maxHp = 1000;
    playerHive.hp = playerHive.maxHp;
    playerHive.biomass = 80;
    playerHive.team = 0;
    playerHive.queue.clear();
    playerHive.spawnTimer = 0.0f;

    enemyHive.x = 292.0f;
    enemyHive.y = 120.0f;
    enemyHive.radius = 18.0f;
    enemyHive.maxHp = 1000;
    enemyHive.hp = enemyHive.maxHp;
    enemyHive.biomass = 80;
    enemyHive.team = 1;
    enemyHive.queue.clear();
    enemyHive.spawnTimer = 0.0f;

    pBeaconX = 100.0f;
    pBeaconY = 120.0f;
    pBeaconActive = false;
    pDirective = Directive::Attack;

    eBeaconX = 220.0f;
    eBeaconY = 120.0f;
    eDirective = Directive::Attack;

    // 2. Ressourcen-Knoten (Geysire)
    nodes.push_back({90.0f, 45.0f, 12.0f, -1, 0.0f, 0.0f});   // Nord-West
    nodes.push_back({90.0f, 195.0f, 12.0f, -1, 0.0f, 0.0f});  // Süd-West
    nodes.push_back({160.0f, 120.0f, 16.0f, -1, 0.0f, 0.0f}); // Zentrum (Mega-Geysir)
    nodes.push_back({230.0f, 45.0f, 12.0f, -1, 0.0f, 0.0f});  // Nord-Ost
    nodes.push_back({230.0f, 195.0f, 12.0f, -1, 0.0f, 0.0f}); // Süd-Ost

    // 3. Start-Schwärme spawnen
    for (int i = 0; i < 24; ++i) {
      spawnUnit(playerHive.x + (rand() % 20 - 10), playerHive.y + (rand() % 20 - 10), 0, UnitType::Needle);
      spawnUnit(enemyHive.x + (rand() % 20 - 10), enemyHive.y + (rand() % 20 - 10), 1, UnitType::Needle);
    }
    for (int i = 0; i < 6; ++i) {
      spawnUnit(playerHive.x + (rand() % 20 - 10), playerHive.y + (rand() % 20 - 10), 0, UnitType::Acid);
      spawnUnit(enemyHive.x + (rand() % 20 - 10), enemyHive.y + (rand() % 20 - 10), 1, UnitType::Acid);
      spawnUnit(playerHive.x + (rand() % 20 - 10), playerHive.y + (rand() % 20 - 10), 0, UnitType::Pulse);
      spawnUnit(enemyHive.x + (rand() % 20 - 10), enemyHive.y + (rand() % 20 - 10), 1, UnitType::Pulse);
    }
    for (int i = 0; i < 4; ++i) {
      spawnUnit(playerHive.x + (rand() % 20 - 10), playerHive.y + (rand() % 20 - 10), 0, UnitType::Scout);
      spawnUnit(enemyHive.x + (rand() % 20 - 10), enemyHive.y + (rand() % 20 - 10), 1, UnitType::Scout);
    }

    fogOfWar.fill(0);
  }

  int getUnitCost(UnitType t) const {
    switch (t) {
    case UnitType::Needle: return 6;
    case UnitType::Acid: return 14;
    case UnitType::Pulse: return 20;
    case UnitType::Scout: return 4;
    }
    return 6;
  }

  void spawnUnit(float x, float y, uint8_t team, UnitType type) {
    if (units.size() >= MAX_UNITS) return;
    Unit u;
    u.x = x;
    u.y = y;
    u.vx = (rand() % 20 - 10) * 1.5f;
    u.vy = (rand() % 20 - 10) * 1.5f;
    u.team = team;
    u.type = type;
    u.attackTimer = (rand() % 30) / 100.0f;
    u.active = true;

    switch (type) {
    case UnitType::Needle:
      u.maxHp = 18;
      break;
    case UnitType::Acid:
      u.maxHp = 32;
      break;
    case UnitType::Pulse:
      u.maxHp = 24;
      break;
    case UnitType::Scout:
      u.maxHp = 12;
      break;
    }
    u.hp = u.maxHp;
    units.push_back(u);
  }

  // ===========================================================================
  // 3. HAUPT-UPDATE LOOP
  // ===========================================================================
  void update(Engine &e) override {
    float dt = e.dt();
    matchTimer += dt;

    if (screenShake > 0.0f) {
      screenShake = std::max(0.0f, screenShake - dt * 2.5f);
    }

    if (state == State::Playing) {
      handleInput(e);
      updateAI(dt, e);
      updateHives(dt, e);
      updateNodes(dt, e);
      updateSpatialGrid();
      updateBoidsAndCombat(dt, e);
      updateProjectiles(dt, e);
      updateAcidPools(dt);
      updateBiomass(dt);
      updateLasers(dt);
      updateParticles(dt);
      updateFogOfWar();

      // Sieg / Niederlage prüfen
      if (enemyHive.hp <= 0) {
        state = State::Victory;
        playVictorySound(e);
        screenShake = 1.0f;
      } else if (playerHive.hp <= 0) {
        state = State::Defeat;
        playDefeatSound(e);
        screenShake = 1.0f;
      }
    } else {
      if (e.pressed(Key::Space) || e.pressed(Key::R) || e.pressed(Key::Enter)) {
        initMatch();
      }
    }

    render(e);
  }

  // ===========================================================================
  // 4. SPIELER-EINGABE (Maus & Tastatur)
  // ===========================================================================
  void handleInput(Engine &e) {
    int mx = e.mouse_x();
    int my = e.mouse_y();

    // 1. Pheromon-Beacon per Mausklick / Drag platzieren
    if (e.mouse_down(sf::Mouse::Button::Left)) {
      if (mx >= 6 && mx <= 314 && my >= 18 && my <= 204) {
        pBeaconX = static_cast<float>(mx);
        pBeaconY = static_cast<float>(my);
        pBeaconActive = true;
        pDirective = Directive::Attack;

        // Pheromon-Aura Partikel am Mauszeiger
        if (rand() % 2 == 0) {
          Particle p;
          p.x = pBeaconX + (rand() % 10 - 5);
          p.y = pBeaconY + (rand() % 10 - 5);
          p.vx = (rand() % 16 - 8);
          p.vy = (rand() % 16 - 8);
          p.maxLife = 0.25f;
          p.life = p.maxLife;
          p.color = Ramps::Cyan[14];
          p.size = 1.5f;
          particles.push_back(p);
        }
      }
    }

    if (e.mouse_pressed(sf::Mouse::Button::Left)) {
      playBeaconSound(e);
    }

    if (e.mouse_pressed(sf::Mouse::Button::Right)) {
      // Rechtsklick: Defensiv-Rückzug zur Basis
      pBeaconX = playerHive.x;
      pBeaconY = playerHive.y;
      pDirective = Directive::Defend;
      playBeaconSound(e);
    }

    // 2. Einheiten-Rekrutierung (Tasten 1, 2, 3, 4)
    if (e.pressed(Key::Num1)) queuePlayerUnit(UnitType::Needle, e);
    if (e.pressed(Key::Num2)) queuePlayerUnit(UnitType::Acid, e);
    if (e.pressed(Key::Num3)) queuePlayerUnit(UnitType::Pulse, e);
    if (e.pressed(Key::Num4)) queuePlayerUnit(UnitType::Scout, e);

    // 3. Taktische Direktiven
    if (e.pressed(Key::Q)) {
      pDirective = Directive::Defend;
      pBeaconX = playerHive.x + 20.0f;
      pBeaconY = playerHive.y;
      pBeaconActive = true;
      playBeaconSound(e);
    }
    if (e.pressed(Key::W)) {
      pDirective = Directive::Attack;
      pBeaconX = enemyHive.x;
      pBeaconY = enemyHive.y;
      pBeaconActive = true;
      playBeaconSound(e);
    }
    if (e.pressed(Key::E)) {
      pDirective = (pDirective == Directive::Disperse) ? Directive::Attack : Directive::Disperse;
      playBeaconSound(e);
    }
    if (e.pressed(Key::Tab) || e.pressed(Key::Num5) || e.pressed(Key::A)) {
      autoBuild = !autoBuild;
      e.play_tone(autoBuild ? Notes::E5 : Notes::C4, 0.08f);
    }
  }

  void queuePlayerUnit(UnitType type, Engine &e) {
    int cost = getUnitCost(type);
    if (playerHive.biomass >= cost) {
      playerHive.biomass -= cost;
      playerHive.queue.push_back(type);
      e.play_tone(Notes::A5, 0.05f);
    } else {
      e.play_tone(Notes::G3, 0.08f); // Zu wenig Biomasse
    }
  }

  // ===========================================================================
  // 5. ADAPTIVE GEGNER-KI
  // ===========================================================================
  void updateAI(float dt, Engine &e) {
    aiDecisionTimer += dt;
    if (aiDecisionTimer < 0.6f) return;
    aiDecisionTimer = 0.0f;

    // 1. Zähle Zusammensetzung der Spieler-Armee (Scouting-Analyse)
    int pNeedles = 0, pAcids = 0, pPulses = 0;
    int eNeedles = 0, eAcids = 0, ePulses = 0;

    for (const auto &u : units) {
      if (!u.active) continue;
      if (u.team == 0) {
        if (u.type == UnitType::Needle) pNeedles++;
        else if (u.type == UnitType::Acid) pAcids++;
        else if (u.type == UnitType::Pulse) pPulses++;
      } else {
        if (u.type == UnitType::Needle) eNeedles++;
        else if (u.type == UnitType::Acid) eAcids++;
        else if (u.type == UnitType::Pulse) ePulses++;
      }
    }

    // 2. Schere-Stein-Papier Konter-Rekrutierung:
    UnitType preferred = UnitType::Needle;
    if (pNeedles >= pAcids && pNeedles >= pPulses) {
      preferred = UnitType::Acid; // Kontert Nadeln mit Flächenschaden!
    } else if (pAcids >= pNeedles && pAcids >= pPulses) {
      preferred = UnitType::Pulse; // Kontert Säure mit Reichweiten-Lasern!
    } else {
      preferred = UnitType::Needle; // Kontert Sniper mit schnellen Nadel-Massen!
    }

    int cost = getUnitCost(preferred);
    if (enemyHive.biomass >= cost && enemyHive.queue.size() < 6) {
      enemyHive.biomass -= cost;
      enemyHive.queue.push_back(preferred);
    } else if (enemyHive.biomass >= getUnitCost(UnitType::Scout) && rand() % 4 == 0) {
      enemyHive.biomass -= getUnitCost(UnitType::Scout);
      enemyHive.queue.push_back(UnitType::Scout);
    }

    // 3. Taktische Angriffs- & Sammelpunkte der KI
    int eTotal = eNeedles + eAcids + ePulses;
    if (eTotal >= 35) {
      // Großangriff auf Spielerbasis
      eBeaconX = playerHive.x;
      eBeaconY = playerHive.y;
      eDirective = Directive::Attack;
    } else if (eTotal >= 15) {
      // Besetze zentralen oder umkämpften Geysir
      eBeaconX = 160.0f;
      eBeaconY = (rand() % 2 == 0) ? 120.0f : 45.0f;
      eDirective = Directive::Attack;
    } else {
      // Verteidige eigene Basis
      eBeaconX = enemyHive.x - 25.0f;
      eBeaconY = enemyHive.y;
      eDirective = Directive::Defend;
    }
  }

  // ===========================================================================
  // 6. BRUTSTÄTTEN (HIVES) & AUTO-BUILD
  // ===========================================================================
  void updateHives(float dt, Engine &e) {
    // Auto-Build für Spieler
    if (autoBuild && playerHive.queue.size() < 4) {
      UnitType nextType = UnitType::Needle;
      int r = rand() % 10;
      if (r < 5) nextType = UnitType::Needle;
      else if (r < 8) nextType = UnitType::Acid;
      else nextType = UnitType::Pulse;

      int cost = getUnitCost(nextType);
      if (playerHive.biomass >= cost + 12) {
        playerHive.biomass -= cost;
        playerHive.queue.push_back(nextType);
      }
    }

    // Spieler-Hive Brutprozess
    playerHive.spawnTimer += dt;
    if (!playerHive.queue.empty() && playerHive.spawnTimer >= 0.22f) {
      playerHive.spawnTimer = 0.0f;
      UnitType t = playerHive.queue.front();
      playerHive.queue.erase(playerHive.queue.begin());
      spawnUnit(playerHive.x + 12.0f, playerHive.y + (rand() % 16 - 8), 0, t);
      playSpawnSound(e);
    }

    // KI-Hive Brutprozess
    enemyHive.spawnTimer += dt;
    if (!enemyHive.queue.empty() && enemyHive.spawnTimer >= 0.22f) {
      enemyHive.spawnTimer = 0.0f;
      UnitType t = enemyHive.queue.front();
      enemyHive.queue.erase(enemyHive.queue.begin());
      spawnUnit(enemyHive.x - 12.0f, enemyHive.y + (rand() % 16 - 8), 1, t);
    }
  }

  // ===========================================================================
  // 7. RESSOURCEN-KNOTEN (GEYSIRE)
  // ===========================================================================
  void updateNodes(float dt, Engine &e) {
    for (auto &n : nodes) {
      int pNear = 0;
      int eNear = 0;

      for (const auto &u : units) {
        if (!u.active) continue;
        float d2 = (u.x - n.x) * (u.x - n.x) + (u.y - n.y) * (u.y - n.y);
        if (d2 <= (n.radius + 15.0f) * (n.radius + 15.0f)) {
          if (u.team == 0) pNear++;
          else eNear++;
        }
      }

      // Einnahme-Fortschritt
      if (pNear > eNear) {
        n.captureProgress = std::min(100.0f, n.captureProgress + dt * (15.0f + pNear * 2.0f));
        if (n.captureProgress >= 100.0f) n.owner = 0;
      } else if (eNear > pNear) {
        n.captureProgress = std::max(-100.0f, n.captureProgress - dt * (15.0f + eNear * 2.0f));
        if (n.captureProgress <= -100.0f) n.owner = 1;
      }

      // Kontinuierliche Biomasse-Förderung
      n.harvestTimer += dt;
      if (n.harvestTimer >= 1.2f) {
        n.harvestTimer = 0.0f;
        int yield = (n.radius > 14.0f) ? 5 : 3; // Mega-Geysir gibt mehr!

        if (n.owner == 0) {
          playerHive.biomass += yield;
          spawnNodeSparkles(n.x, n.y, Ramps::Cyan[14]);
        } else if (n.owner == 1) {
          enemyHive.biomass += yield;
          spawnNodeSparkles(n.x, n.y, Ramps::Fire[12]);
        }
      }
    }
  }

  // ===========================================================================
  // 8. SPATIAL GRID & BOIDS FLOCKING PHYSIK
  // ===========================================================================
  void updateSpatialGrid() {
    for (int gx = 0; gx < GRID_W; ++gx) {
      for (int gy = 0; gy < GRID_H; ++gy) {
        grid[gx][gy].clear();
      }
    }

    for (size_t i = 0; i < units.size(); ++i) {
      if (!units[i].active) continue;
      int gx = std::clamp(static_cast<int>(units[i].x / 16.0f), 0, GRID_W - 1);
      int gy = std::clamp(static_cast<int>(units[i].y / 16.0f), 0, GRID_H - 1);
      grid[gx][gy].push_back(static_cast<int>(i));
    }
  }

  void updateBoidsAndCombat(float dt, Engine &e) {
    for (size_t i = 0; i < units.size(); ++i) {
      auto &u = units[i];
      if (!u.active) continue;

      u.attackTimer -= dt;

      // 1. Boids-Kräfte: Separation, Alignment, Cohesion
      float sepX = 0.0f, sepY = 0.0f;
      float alignX = 0.0f, alignY = 0.0f;
      float cohX = 0.0f, cohY = 0.0f;
      int neighborCount = 0;

      float closestEnemyDistSq = 999999.0f;
      int closestEnemyIdx = -1;

      int gx = std::clamp(static_cast<int>(u.x / 16.0f), 0, GRID_W - 1);
      int gy = std::clamp(static_cast<int>(u.y / 16.0f), 0, GRID_H - 1);

      for (int ox = -1; ox <= 1; ++ox) {
        int nx = gx + ox;
        if (nx < 0 || nx >= GRID_W) continue;
        for (int oy = -1; oy <= 1; ++oy) {
          int ny = gy + oy;
          if (ny < 0 || ny >= GRID_H) continue;

          for (int otherIdx : grid[nx][ny]) {
            if (otherIdx == static_cast<int>(i)) continue;
            const auto &other = units[otherIdx];
            if (!other.active) continue;

            float dx = other.x - u.x;
            float dy = other.y - u.y;
            float d2 = dx * dx + dy * dy;

            if (other.team == u.team) {
              // Eigene Schwarm-Kameraden
              if (d2 < 64.0f && d2 > 0.01f) {
                float d = std::sqrt(d2);
                sepX -= (dx / d) * (8.0f - d) * 16.0f;
                sepY -= (dy / d) * (8.0f - d) * 16.0f;
              }
              if (d2 < 400.0f) {
                alignX += other.vx;
                alignY += other.vy;
                cohX += other.x;
                cohY += other.y;
                neighborCount++;
              }
            } else {
              // Gegnerische Einheit
              if (d2 < closestEnemyDistSq) {
                closestEnemyDistSq = d2;
                closestEnemyIdx = otherIdx;
              }
            }
          }
        }
      }

      // 2. Leuchtfeuer- & Ziel-Steuerung (Beacon Attraction)
      float targetX = (u.team == 0) ? pBeaconX : eBeaconX;
      float targetY = (u.team == 0) ? pBeaconY : eBeaconY;
      Directive dir = (u.team == 0) ? pDirective : eDirective;

      if (dir == Directive::Defend) {
        targetX = (u.team == 0) ? playerHive.x + 25.0f : enemyHive.x - 25.0f;
        targetY = (u.team == 0) ? playerHive.y : enemyHive.y;
      }

      // 3. Kampf-KI & Angriffsreichweiten
      float attackRange = 10.0f;
      if (u.type == UnitType::Acid) attackRange = 40.0f;
      else if (u.type == UnitType::Pulse) attackRange = 72.0f;
      else if (u.type == UnitType::Scout) attackRange = 14.0f;

      bool inCombat = false;

      if (closestEnemyIdx >= 0 && closestEnemyDistSq <= (attackRange + 25.0f) * (attackRange + 25.0f)) {
        inCombat = true;
        const auto &target = units[closestEnemyIdx];
        float edx = target.x - u.x;
        float edy = target.y - u.y;
        float ed = std::sqrt(closestEnemyDistSq);

        if (ed > attackRange * 0.75f) {
          u.vx += (edx / ed) * 45.0f * dt;
          u.vy += (edy / ed) * 45.0f * dt;
        }

        // Angriff ausführen
        if (closestEnemyDistSq <= attackRange * attackRange && u.attackTimer <= 0.0f) {
          executeAttack(u, target.x, target.y, closestEnemyIdx, e);
        }
      } else {
        // Wenn keine Gegner in der Nähe: Prüfe feindliche Basis
        const auto &enemyBase = (u.team == 0) ? enemyHive : playerHive;
        float bdx = enemyBase.x - u.x;
        float bdy = enemyBase.y - u.y;
        float bd2 = bdx * bdx + bdy * bdy;

        if (bd2 <= (attackRange + enemyBase.radius) * (attackRange + enemyBase.radius)) {
          inCombat = true;
          if (u.attackTimer <= 0.0f) {
            executeBaseAttack(u, e);
          }
        }
      }

      // 4. Boids-Geschwindigkeit aktualisieren
      if (!inCombat) {
        float bdx = targetX - u.x;
        float bdy = targetY - u.y;
        float bd = std::sqrt(bdx * bdx + bdy * bdy);
        if (bd > 12.0f) {
          float steerForce = (dir == Directive::Disperse) ? 18.0f : 48.0f;
          u.vx += (bdx / bd) * steerForce * dt;
          u.vy += (bdy / bd) * steerForce * dt;
        }

        if (neighborCount > 0) {
          alignX /= neighborCount;
          alignY /= neighborCount;
          cohX = (cohX / neighborCount) - u.x;
          cohY = (cohY / neighborCount) - u.y;

          u.vx += alignX * 0.12f * dt;
          u.vy += alignY * 0.12f * dt;
          u.vx += cohX * 0.25f * dt;
          u.vy += cohY * 0.25f * dt;
        }
      }

      u.vx += sepX * dt;
      u.vy += sepY * dt;

      // Reibung / Dämpfung & Höchstgeschwindigkeit
      float maxSpd = 55.0f;
      if (u.type == UnitType::Needle) maxSpd = 70.0f;
      else if (u.type == UnitType::Acid) maxSpd = 44.0f;
      else if (u.type == UnitType::Pulse) maxSpd = 38.0f;
      else if (u.type == UnitType::Scout) maxSpd = 105.0f;

      float curSpd = std::sqrt(u.vx * u.vx + u.vy * u.vy);
      if (curSpd > maxSpd) {
        u.vx = (u.vx / curSpd) * maxSpd;
        u.vy = (u.vy / curSpd) * maxSpd;
      }
      u.vx *= 0.96f;
      u.vy *= 0.96f;

      u.x += u.vx * dt;
      u.y += u.vy * dt;

      // Spielfeld-Grenzen (320x240, oberer HUD 18px frei, unterer Balken 36px frei)
      u.x = std::clamp(u.x, 8.0f, 312.0f);
      u.y = std::clamp(u.y, 22.0f, 202.0f);
    }
  }

  void executeAttack(Unit &attacker, float tx, float ty, int targetIdx, Engine &e) {
    switch (attacker.type) {
    case UnitType::Needle: {
      attacker.attackTimer = 0.28f;
      if (targetIdx >= 0 && targetIdx < static_cast<int>(units.size())) {
        damageUnit(targetIdx, 4, attacker.team);
        spawnBiteSparkles(tx, ty, attacker.team == 0 ? Colors::Cyan : Colors::Red);
        if (rand() % 4 == 0) playBiteSound(e);
      }
      break;
    }
    case UnitType::Acid: {
      attacker.attackTimer = 0.85f;
      Projectile p;
      p.x = attacker.x;
      p.y = attacker.y;
      p.targetX = tx;
      p.targetY = ty;
      float dx = tx - attacker.x;
      float dy = ty - attacker.y;
      float d = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
      p.vx = (dx / d) * 85.0f;
      p.vy = (dy / d) * 85.0f;
      p.team = attacker.team;
      p.type = UnitType::Acid;
      p.damage = 12;
      p.maxLife = d / 85.0f;
      p.life = p.maxLife;
      p.active = true;
      projectiles.push_back(p);
      playAcidSound(e);
      break;
    }
    case UnitType::Pulse: {
      attacker.attackTimer = 1.15f;
      if (targetIdx >= 0 && targetIdx < static_cast<int>(units.size())) {
        damageUnit(targetIdx, 16, attacker.team);
        LaserBeam lb;
        lb.x1 = attacker.x;
        lb.y1 = attacker.y;
        lb.x2 = tx;
        lb.y2 = ty;
        lb.life = 0.12f;
        lb.color = (attacker.team == 0) ? Ramps::Cyan[14] : Ramps::Fire[14];
        lasers.push_back(lb);
        playLaserSound(e);
      }
      break;
    }
    case UnitType::Scout: {
      attacker.attackTimer = 0.35f;
      if (targetIdx >= 0 && targetIdx < static_cast<int>(units.size())) {
        damageUnit(targetIdx, 2, attacker.team);
      }
      break;
    }
    }
  }

  void executeBaseAttack(Unit &attacker, Engine &e) {
    auto &enemyBase = (attacker.team == 0) ? enemyHive : playerHive;
    attacker.attackTimer = (attacker.type == UnitType::Pulse) ? 1.2f : 0.40f;
    int dmg = (attacker.type == UnitType::Pulse) ? 18 : (attacker.type == UnitType::Acid ? 10 : 3);
    enemyBase.hp = std::max(0, enemyBase.hp - dmg);
    spawnBiteSparkles(enemyBase.x + (rand() % 20 - 10), enemyBase.y + (rand() % 20 - 10),
                      attacker.team == 0 ? Colors::Cyan : Colors::Red);
    if (rand() % 3 == 0) playBiteSound(e);
  }

  void damageUnit(int idx, int dmg, uint8_t byTeam) {
    if (idx < 0 || idx >= static_cast<int>(units.size())) return;
    auto &target = units[idx];
    if (!target.active) return;

    target.hp -= dmg;
    if (target.hp <= 0) {
      target.active = false;
      // Hinterlässt recycelbare Biomasse!
      BiomassDrop bd;
      bd.x = target.x;
      bd.y = target.y;
      bd.value = std::max(2, getUnitCost(target.type) / 2);
      bd.life = 12.0f; // Bleibt 12 Sekunden liegen
      bd.active = true;
      biomassDrops.push_back(bd);

      spawnDeathExplosion(target.x, target.y, target.team == 0 ? Colors::Cyan : Colors::Red);
    }
  }

  // ===========================================================================
  // 9. PROJEKTILE & SÄUREPFÜTZEN (AoE)
  // ===========================================================================
  void updateProjectiles(float dt, Engine &e) {
    for (size_t i = 0; i < projectiles.size();) {
      auto &p = projectiles[i];
      p.life -= dt;
      p.x += p.vx * dt;
      p.y += p.vy * dt;

      if (p.life <= 0.0f) {
        // Säure-Einschlag -> Erzeugt ätzende Säurelache am Boden!
        AcidPool ap;
        ap.x = p.targetX;
        ap.y = p.targetY;
        ap.radius = 15.0f;
        ap.maxLife = 2.0f;
        ap.life = ap.maxLife;
        ap.damagePerSec = 14;
        ap.team = p.team;
        ap.active = true;
        acidPools.push_back(ap);

        spawnAcidSplashFX(p.targetX, p.targetY, p.team == 0 ? Ramps::Cyan[12] : Ramps::Green[13]);
        playSplashSound(e);

        projectiles[i] = projectiles.back();
        projectiles.pop_back();
        continue;
      }
      ++i;
    }
  }

  void updateAcidPools(float dt) {
    for (size_t i = 0; i < acidPools.size();) {
      auto &ap = acidPools[i];
      ap.life -= dt;
      if (ap.life <= 0.0f) {
        acidPools[i] = acidPools.back();
        acidPools.pop_back();
        continue;
      }

      // AoE Schaden an allen gegnerischen Einheiten im Umkreis
      for (size_t uIdx = 0; uIdx < units.size(); ++uIdx) {
        auto &u = units[uIdx];
        if (!u.active || u.team == ap.team) continue;
        float dx = u.x - ap.x;
        float dy = u.y - ap.y;
        if (dx * dx + dy * dy <= ap.radius * ap.radius) {
          if (rand() % 4 == 0) {
            damageUnit(static_cast<int>(uIdx), 3, ap.team);
          }
        }
      }

      ++i;
    }
  }

  // ===========================================================================
  // 10. BIOMASSE-RECYCLING (Zero-Waste)
  // ===========================================================================
  void updateBiomass(float dt) {
    for (size_t i = 0; i < biomassDrops.size();) {
      auto &bd = biomassDrops[i];
      bd.life -= dt;
      if (bd.life <= 0.0f || !bd.active) {
        biomassDrops[i] = biomassDrops.back();
        biomassDrops.pop_back();
        continue;
      }

      // Einheiten in der Nähe saugen die Biomasse automatisch auf
      for (const auto &u : units) {
        if (!u.active) continue;
        float d2 = (u.x - bd.x) * (u.x - bd.x) + (u.y - bd.y) * (u.y - bd.y);
        if (d2 <= 14.0f * 14.0f) {
          if (u.team == 0) {
            playerHive.biomass += bd.value;
            spawnBiomassFX(bd.x, bd.y, Ramps::Cyan[14]);
          } else {
            enemyHive.biomass += bd.value;
            spawnBiomassFX(bd.x, bd.y, Ramps::Fire[12]);
          }
          bd.active = false;
          break;
        }
      }
      ++i;
    }
  }

  void updateLasers(float dt) {
    for (size_t i = 0; i < lasers.size();) {
      lasers[i].life -= dt;
      if (lasers[i].life <= 0.0f) {
        lasers[i] = lasers.back();
        lasers.pop_back();
      } else {
        ++i;
      }
    }
  }

  // ===========================================================================
  // 11. NEBEL DES KRIEGES (FOG OF WAR)
  // ===========================================================================
  void updateFogOfWar() {
    // 0 = Nicht sichtbar, 1 = Sichtbar
    fogOfWar.fill(0);

    // Eigene Basis hat permanente Sicht
    int hgx = static_cast<int>(playerHive.x / 10.0f);
    int hgy = static_cast<int>(playerHive.y / 10.0f);
    revealFogCircle(hgx, hgy, 5);

    // Alle eigenen Einheiten decken Sicht auf
    for (const auto &u : units) {
      if (!u.active || u.team != 0) continue;
      int ux = static_cast<int>(u.x / 10.0f);
      int uy = static_cast<int>(u.y / 10.0f);
      int visRad = (u.type == UnitType::Scout) ? 5 : 3;
      revealFogCircle(ux, uy, visRad);
    }
  }

  void revealFogCircle(int cx, int cy, int r) {
    for (int ox = -r; ox <= r; ++ox) {
      int x = cx + ox;
      if (x < 0 || x >= FOG_W) continue;
      for (int oy = -r; oy <= r; ++oy) {
        int y = cy + oy;
        if (y < 0 || y >= FOG_H) continue;
        if (ox * ox + oy * oy <= r * r) {
          fogOfWar[y * FOG_W + x] = 1;
        }
      }
    }
  }

  // ===========================================================================
  // 12. RENDERING (320x240 Retro Mode 13h)
  // ===========================================================================
  void render(Engine &e) {
    // Bildschirm-Wackeln
    e.cls(Ramps::Grays[1]);

    // 1. Terrain & Geysire zeichnen
    renderTerrain(e);
    renderNodes(e);
    renderAcidPools(e);
    renderBiomass(e);

    // 2. Basen (Hives)
    renderHive(e, playerHive);
    renderHive(e, enemyHive);

    // 3. Pheromon-Leuchtfeuer
    renderBeacons(e);

    // 4. Einheiten & Schwärme
    renderUnits(e);

    // 5. Laserstrahlen & Projektile
    renderLasers(e);
    renderProjectiles(e);
    renderParticles(e);

    // 6. Nebel des Krieges (Fog of War)
    renderFogOfWar(e);

    // 7. HUD, Minimap & Befehlskonsole
    renderHUD(e);
    renderMinimap(e);

    if (state == State::Victory) {
      renderEndBanner(e, "SIEG! DER FEINDLICHE SCHWARM WURDE VERNICHTET!", Colors::Yellow);
    } else if (state == State::Defeat) {
      renderEndBanner(e, "NIEDERLAGE! DEIN SCHWARM-KERN WURDE ZERSTOERT!", Colors::Red);
    }
  }

  void renderTerrain(Engine &e) {
    // Alien-Mineralienboden mit hexagonalen Naniten-Gittern
    for (int y = 20; y < 204; y += 16) {
      e.line(8, y, 312, y, Ramps::Grays[2]);
    }
    for (int x = 8; x < 312; x += 16) {
      e.line(x, 20, x, 204, Ramps::Grays[2]);
    }

    // Rocky Outcroppings / Choke Point Barrieren
    e.rectfill(154, 70, 12, 22, Ramps::Grays[4]);
    e.rect(154, 70, 12, 22, Ramps::Grays[6]);
    e.rectfill(154, 148, 12, 22, Ramps::Grays[4]);
    e.rect(154, 148, 12, 22, Ramps::Grays[6]);
  }

  void renderNodes(Engine &e) {
    for (const auto &n : nodes) {
      int nx = static_cast<int>(n.x);
      int ny = static_cast<int>(n.y);

      uint8_t ringCol = (n.owner == 0) ? Ramps::Cyan[12] : (n.owner == 1 ? Ramps::Fire[12] : Colors::LightGray);
      e.circlefill(nx, ny, n.radius, Ramps::Grays[3]);
      e.circle(nx, ny, n.radius, ringCol);

      // Pulsierender Mineral-Kern
      float pulse = std::sin(matchTimer * 6.0f + n.x) * 2.0f;
      e.circlefill(nx, ny, 4.0f + pulse, (n.owner == 0) ? Ramps::Cyan[14] : (n.owner == 1 ? Ramps::Fire[14] : Colors::White));

      // Einnahmebalken
      if (n.captureProgress != 0.0f) {
        int barW = static_cast<int>((std::abs(n.captureProgress) / 100.0f) * 16.0f);
        e.rectfill(nx - 8, ny - static_cast<int>(n.radius) - 4, barW, 2, ringCol);
      }
    }
  }

  void renderHive(Engine &e, const Hive &h) {
    int hx = static_cast<int>(h.x);
    int hy = static_cast<int>(h.y);

    uint8_t colMain = (h.team == 0) ? Ramps::Cyan[10] : Ramps::Fire[10];
    uint8_t colCore = (h.team == 0) ? Ramps::Cyan[14] : Ramps::Fire[14];

    // Basis-Kuppel
    e.circlefill(hx, hy, h.radius, colMain);
    e.circle(hx, hy, h.radius, Colors::White);
    e.circle(hx, hy, h.radius * 0.55f, colCore);

    // Pulsierende Bio-Adern
    for (int a = 0; a < 4; ++a) {
      float ang = a * 1.5708f + matchTimer * 2.0f;
      int px = hx + static_cast<int>(std::cos(ang) * (h.radius + 3.0f));
      int py = hy + static_cast<int>(std::sin(ang) * (h.radius + 3.0f));
      e.pset(px, py, colCore);
    }

    // Lebensbalken über der Basis
    int hpW = static_cast<int>((static_cast<float>(h.hp) / h.maxHp) * 28.0f);
    e.rectfill(hx - 14, hy - 25, 28, 4, Colors::DarkGray);
    e.rect(hx - 15, hy - 26, 30, 6, Colors::White);
    e.rectfill(hx - 14, hy - 25, hpW, 4, (h.team == 0) ? Colors::LightGreen : Colors::Red);
  }

  void renderBeacons(Engine &e) {
    if (pBeaconActive) {
      int bx = static_cast<int>(pBeaconX);
      int by = static_cast<int>(pBeaconY);
      float r = 6.0f + std::sin(matchTimer * 10.0f) * 2.5f;
      e.circle(bx, by, r, Ramps::Cyan[13]);
      e.pset(bx, by, Colors::White);
      e.line(bx - 3, by, bx + 3, by, Ramps::Cyan[14]);
      e.line(bx, by - 3, bx, by + 3, Ramps::Cyan[14]);
    }
  }

  void renderUnits(Engine &e) {
    for (const auto &u : units) {
      if (!u.active) continue;
      int ux = static_cast<int>(u.x);
      int uy = static_cast<int>(u.y);

      // Gegner außerhalb des Fog of War ausblenden
      int fx = std::clamp(ux / 10, 0, FOG_W - 1);
      int fy = std::clamp(uy / 10, 0, FOG_H - 1);
      if (u.team == 1 && fogOfWar[fy * FOG_W + fx] == 0) {
        continue;
      }

      uint8_t col = (u.team == 0) ? Ramps::Cyan[13] : Ramps::Fire[12];
      uint8_t coreCol = Colors::White;

      switch (u.type) {
      case UnitType::Needle:
        // ⚡ Nadel: Schneller spitzer Pfeil
        e.pset(ux, uy, col);
        e.pset(ux + static_cast<int>(u.vx * 0.04f), uy + static_cast<int>(u.vy * 0.04f), coreCol);
        break;

      case UnitType::Acid:
        // 💥 Säure: 2x2 Runder Kugelkäfer
        e.rectfill(ux - 1, uy - 1, 3, 3, col);
        e.pset(ux, uy, coreCol);
        break;

      case UnitType::Pulse:
        // 🎯 Pulse: Rhombus / Diamant
        e.pset(ux, uy - 1, col);
        e.pset(ux - 1, uy, col);
        e.pset(ux + 1, uy, col);
        e.pset(ux, uy + 1, col);
        e.pset(ux, uy, coreCol);
        break;

      case UnitType::Scout:
        // 👁️ Scout: Winziger schneller Flitzer
        e.pset(ux, uy, (u.team == 0) ? Colors::Yellow : Colors::Pink);
        break;
      }
    }
  }

  void renderLasers(Engine &e) {
    for (const auto &l : lasers) {
      e.line(l.x1, l.y1, l.x2, l.y2, l.color);
      e.line(l.x1 + 0.5f, l.y1 + 0.5f, l.x2 + 0.5f, l.y2 + 0.5f, Colors::White);
    }
  }

  void renderProjectiles(Engine &e) {
    for (const auto &p : projectiles) {
      int px = static_cast<int>(p.x);
      int py = static_cast<int>(p.y);
      uint8_t col = (p.team == 0) ? Ramps::Cyan[14] : Ramps::Green[14];
      e.circlefill(px, py, 2.0f, col);
      e.pset(px, py, Colors::White);
    }
  }

  void renderAcidPools(Engine &e) {
    for (const auto &ap : acidPools) {
      int ax = static_cast<int>(ap.x);
      int ay = static_cast<int>(ap.y);
      uint8_t col = (ap.team == 0) ? Ramps::Cyan[10] : Ramps::Green[11];
      e.circlefill(ax, ay, ap.radius * (ap.life / ap.maxLife), col);
      if (rand() % 3 == 0) {
        e.pset(ax + (rand() % 10 - 5), ay + (rand() % 10 - 5), Colors::White);
      }
    }
  }

  void renderBiomass(Engine &e) {
    for (const auto &bd : biomassDrops) {
      if (!bd.active) continue;
      int bx = static_cast<int>(bd.x);
      int by = static_cast<int>(bd.y);
      e.pset(bx, by, Ramps::Gold[14]);
      if (static_cast<int>(matchTimer * 12.0f + bd.x) % 2 == 0) {
        e.pset(bx, by - 1, Colors::White);
      }
    }
  }

  void renderParticles(Engine &e) {
    for (const auto &p : particles) {
      int px = static_cast<int>(p.x);
      int py = static_cast<int>(p.y);
      if (px >= 0 && px < 320 && py >= 0 && py < 240) {
        e.pset(px, py, p.color);
      }
    }
  }

  void renderFogOfWar(Engine &e) {
    for (int fy = 0; fy < FOG_H; ++fy) {
      for (int fx = 0; fx < FOG_W; ++fx) {
        if (fogOfWar[fy * FOG_W + fx] == 0) {
          // Halbdunkles Dithering über unentdeckten Bereichen
          int sx = fx * 10;
          int sy = fy * 10;
          for (int py = sy; py < sy + 10; py += 2) {
            for (int px = sx; px < sx + 10; px += 2) {
              e.pset(px, py, Colors::Black);
            }
          }
        }
      }
    }
  }

  void renderHUD(Engine &e) {
    // Oberes HUD (Basis-HP)
    e.rectfill(0, 0, 320, 18, Colors::Black);
    e.line(0, 18, 320, 18, Colors::White);

    e.draw_text(6, 4, "SPIELER NEXUS: " + std::to_string(playerHive.hp), Ramps::Cyan[13], 1);
    e.draw_text(190, 4, "FEIND HIVE: " + std::to_string(enemyHive.hp), Ramps::Fire[12], 1);

    // Unteres Command Console HUD
    e.rectfill(0, 206, 320, 34, Colors::Black);
    e.line(0, 206, 320, 206, Colors::White);

    // Biomasse-Stand
    std::string bioTxt = "BIO: " + std::to_string(playerHive.biomass);
    e.draw_text(6, 211, bioTxt, Colors::Gold, 1);

    // Einheiten-Produktions-Buttons
    e.draw_text(68, 211, "1:NADEL(6)", Ramps::Cyan[12], 1);
    e.draw_text(130, 211, "2:SAEURE(14)", Ramps::Green[13], 1);
    e.draw_text(204, 211, "3:PULSE(20)", Ramps::Cyan[14], 1);
    e.draw_text(270, 211, "4:SCOUT", Colors::Yellow, 1);

    // Steuerungs-Tipps
    std::string autoTxt = autoBuild ? "[AUTO: AN]" : "[AUTO: AUS]";
    e.draw_text(6, 224, autoTxt, autoBuild ? Colors::LightGreen : Colors::DarkGray, 1);
    e.draw_text(80, 224, "L-KLICK: BEACON  R-KLICK: DEF  TAB: AUTO-ZUCHT", Ramps::Grays[7], 1);
  }

  void renderMinimap(Engine &e) {
    // Minimap in oberer rechter Ecke (60x36 Pixel)
    int mapX = 254;
    int mapY = 22;
    int mapW = 60;
    int mapH = 36;

    e.rectfill(mapX, mapY, mapW, mapH, Colors::Black);
    e.rect(mapX, mapY, mapW, mapH, Colors::White);

    // Basen auf Minimap
    e.pset(mapX + static_cast<int>(playerHive.x * mapW / 320.0f),
           mapY + static_cast<int>(playerHive.y * mapH / 240.0f), Ramps::Cyan[14]);
    e.pset(mapX + static_cast<int>(enemyHive.x * mapW / 320.0f),
           mapY + static_cast<int>(enemyHive.y * mapH / 240.0f), Ramps::Fire[14]);

    // Geysire auf Minimap
    for (const auto &n : nodes) {
      uint8_t ncol = (n.owner == 0) ? Ramps::Cyan[13] : (n.owner == 1 ? Ramps::Fire[13] : Colors::LightGray);
      e.pset(mapX + static_cast<int>(n.x * mapW / 320.0f),
             mapY + static_cast<int>(n.y * mapH / 240.0f), ncol);
    }

    // Einheiten auf Minimap
    for (const auto &u : units) {
      if (!u.active) continue;
      int ux = static_cast<int>(u.x * mapW / 320.0f);
      int uy = static_cast<int>(u.y * mapH / 240.0f);
      if (u.team == 0) {
        e.pset(mapX + ux, mapY + uy, Ramps::Cyan[12]);
      } else {
        int fx = std::clamp(static_cast<int>(u.x / 10.0f), 0, FOG_W - 1);
        int fy = std::clamp(static_cast<int>(u.y / 10.0f), 0, FOG_H - 1);
        if (fogOfWar[fy * FOG_W + fx] == 1) {
          e.pset(mapX + ux, mapY + uy, Ramps::Fire[12]);
        }
      }
    }
  }

  void renderEndBanner(Engine &e, const std::string &txt, uint8_t color) {
    e.rectfill(20, 90, 280, 50, Colors::Black);
    e.rect(20, 90, 280, 50, color);
    e.draw_text(30, 104, txt, color, 1);
    e.draw_text(60, 122, "DRUECKE LEERTASTE ODER R ZUM NEUSTART", Colors::White, 1);
  }

  // ===========================================================================
  // 13. PARTIKEL-GENERATOREN & EFFEKTE
  // ===========================================================================
  void spawnDeathExplosion(float x, float y, uint8_t col) {
    for (int i = 0; i < 8; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      p.vx = (rand() % 40 - 20);
      p.vy = (rand() % 40 - 20);
      p.maxLife = 0.20f + (rand() % 15) / 100.0f;
      p.life = p.maxLife;
      p.color = (i % 2 == 0) ? col : Colors::White;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void spawnBiteSparkles(float x, float y, uint8_t col) {
    for (int i = 0; i < 3; ++i) {
      Particle p;
      p.x = x + (rand() % 4 - 2);
      p.y = y + (rand() % 4 - 2);
      p.vx = (rand() % 20 - 10);
      p.vy = (rand() % 20 - 10);
      p.maxLife = 0.12f;
      p.life = p.maxLife;
      p.color = col;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void spawnAcidSplashFX(float x, float y, uint8_t col) {
    for (int i = 0; i < 14; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      p.vx = (rand() % 60 - 30);
      p.vy = (rand() % 60 - 30);
      p.maxLife = 0.28f;
      p.life = p.maxLife;
      p.color = (i % 2 == 0) ? col : Colors::White;
      p.size = 1.5f;
      particles.push_back(p);
    }
  }

  void spawnBiomassFX(float x, float y, uint8_t col) {
    for (int i = 0; i < 4; ++i) {
      Particle p;
      p.x = x;
      p.y = y;
      p.vx = (rand() % 16 - 8);
      p.vy = -10.0f - (rand() % 15);
      p.maxLife = 0.22f;
      p.life = p.maxLife;
      p.color = col;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void spawnNodeSparkles(float x, float y, uint8_t col) {
    for (int i = 0; i < 2; ++i) {
      Particle p;
      p.x = x + (rand() % 12 - 6);
      p.y = y + (rand() % 12 - 6);
      p.vx = (rand() % 10 - 5);
      p.vy = -12.0f;
      p.maxLife = 0.30f;
      p.life = p.maxLife;
      p.color = col;
      p.size = 1.0f;
      particles.push_back(p);
    }
  }

  void updateParticles(float dt) {
    for (size_t i = 0; i < particles.size();) {
      particles[i].life -= dt;
      if (particles[i].life <= 0.0f) {
        particles[i] = particles.back();
        particles.pop_back();
      } else {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        ++i;
      }
    }
  }

  // ===========================================================================
  // 14. SYNTHESIZER SOUNDS
  // ===========================================================================
  void playBeaconSound(Engine &e) {
    e.play_tone(Notes::C5, 0.04f);
    e.play_tone(Notes::G5, 0.06f);
  }

  void playSpawnSound(Engine &e) {
    e.play_tone(Notes::E4, 0.04f);
  }

  void playBiteSound(Engine &e) {
    e.play_tone(Notes::A2, 0.03f);
  }

  void playAcidSound(Engine &e) {
    e.play_tone(Notes::D3, 0.08f);
    e.play_tone(Notes::Fs3, 0.06f);
  }

  void playSplashSound(Engine &e) {
    e.play_tone(Notes::F2, 0.12f);
  }

  void playLaserSound(Engine &e) {
    e.play_tone(Notes::Ds6, 0.05f);
    e.play_tone(Notes::B5, 0.07f);
  }

  void playVictorySound(Engine &e) {
    e.play_tone(Notes::C5, 0.15f);
    e.play_tone(Notes::E5, 0.15f);
    e.play_tone(Notes::G5, 0.15f);
    e.play_tone(Notes::C6, 0.40f);
  }

  void playDefeatSound(Engine &e) {
    e.play_tone(Notes::G3, 0.20f);
    e.play_tone(Notes::Ds3, 0.20f);
    e.play_tone(Notes::C3, 0.45f);
  }
};
