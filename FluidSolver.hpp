#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

// =============================================================================
// NAVIER-STOKES 2D FLUID SOLVER (Jos Stam "Stable Fluids") - HIGH-RES 160x120
// =============================================================================
// Echtzeit-Strömungssimulation mit feiner 2x2 Pixel-Zellauflösung:
// - 160x120 Gitterzellen (19.200 Zellen, 2x2 Pixel pro Zelle)
// - Bedingungslos stabil (Semi-Lagrangian Advektion)
// - Druck-Projektion für inkompressible Strömungen (scharfe Wirbel & Strudel)
// - Eulersche Dichtefelder (Wasser-Dichte & Feuer-Wärmedichte)
// - Feste Randmaskierung für hügeliges Gelände & schwebende Plattformen
// =============================================================================

class FluidSolver2D {
public:
  static constexpr int GW = 160;           // Gitterbreite (160 * 2 = 320 Pixel)
  static constexpr int GH = 120;           // Gitterhöhe  (120 * 2 = 240 Pixel)
  static constexpr float CELL_SIZE = 2.0f; // 2x2 Pixel pro Gitterzelle
  static constexpr int NUM_CELLS = GW * GH;

  FluidSolver2D() {
    u.assign(NUM_CELLS, 0.0f);
    v.assign(NUM_CELLS, 0.0f);
    u0.assign(NUM_CELLS, 0.0f);
    v0.assign(NUM_CELLS, 0.0f);
    p.assign(NUM_CELLS, 0.0f);
    div.assign(NUM_CELLS, 0.0f);
    densityWater.assign(NUM_CELLS, 0.0f);
    densityFire.assign(NUM_CELLS, 0.0f);
    density0.assign(NUM_CELLS, 0.0f);
    curl.assign(NUM_CELLS, 0.0f);
    solidMask.assign(NUM_CELLS, 0);
  }

  void reset() {
    std::fill(u.begin(), u.end(), 0.0f);
    std::fill(v.begin(), v.end(), 0.0f);
    std::fill(u0.begin(), u0.end(), 0.0f);
    std::fill(v0.begin(), v0.end(), 0.0f);
    std::fill(p.begin(), p.end(), 0.0f);
    std::fill(div.begin(), div.end(), 0.0f);
    std::fill(densityWater.begin(), densityWater.end(), 0.0f);
    std::fill(densityFire.begin(), densityFire.end(), 0.0f);
    std::fill(curl.begin(), curl.end(), 0.0f);
  }

  inline int idx(int x, int y) const {
    return y * GW + x;
  }

  inline bool isInside(int gx, int gy) const {
    return (gx >= 0 && gx < GW && gy >= 0 && gy < GH);
  }

  inline bool isSolid(int gx, int gy) const {
    if (!isInside(gx, gy)) return true;
    return solidMask[idx(gx, gy)] != 0;
  }

  void setSolid(int gx, int gy, bool solid) {
    if (isInside(gx, gy)) {
      solidMask[idx(gx, gy)] = solid ? 1 : 0;
      if (solid) {
        u[idx(gx, gy)] = 0.0f;
        v[idx(gx, gy)] = 0.0f;
        densityWater[idx(gx, gy)] = 0.0f;
        densityFire[idx(gx, gy)] = 0.0f;
      }
    }
  }

  // Setzt die Geländemaske basierend auf der Bodenhöhe und Plattformen
  template <typename GroundFunc, typename PlatformList>
  void updateSolidMask(const GroundFunc &getGroundY, const PlatformList &platforms) {
    for (int gy = 0; gy < GH; ++gy) {
      float screenY = (gy + 0.5f) * CELL_SIZE;
      for (int gx = 0; gx < GW; ++gx) {
        float screenX = (gx + 0.5f) * CELL_SIZE;

        bool solid = false;
        // 1. Hügeliges Gelände am Boden
        float gyGround = getGroundY(screenX);
        if (screenY >= gyGround) {
          solid = true;
        }

        // 2. Schwebende Plattformen
        for (const auto &plat : platforms) {
          if (screenX >= plat.x1 && screenX <= plat.x2 &&
              screenY >= plat.y && screenY <= plat.y + 6.0f) {
            solid = true;
            break;
          }
        }

        solidMask[idx(gx, gy)] = solid ? 1 : 0;
        if (solid) {
          u[idx(gx, gy)] = 0.0f;
          v[idx(gx, gy)] = 0.0f;
          densityWater[idx(gx, gy)] = 0.0f;
          densityFire[idx(gx, gy)] = 0.0f;
        }
      }
    }
  }

  // Impuls-Einspeisung an Bildschirm-Koordinaten (z.B. Wasserstrahl oder Windstoss)
  void addVelocity(float screenX, float screenY, float fx, float fy, float radius) {
    float cx = screenX / CELL_SIZE;
    float cy = screenY / CELL_SIZE;
    float rCells = radius / CELL_SIZE;
    int minGx = std::clamp(static_cast<int>(cx - rCells), 0, GW - 1);
    int maxGx = std::clamp(static_cast<int>(cx + rCells), 0, GW - 1);
    int minGy = std::clamp(static_cast<int>(cy - rCells), 0, GH - 1);
    int maxGy = std::clamp(static_cast<int>(cy + rCells), 0, GH - 1);

    float rSq = rCells * rCells;
    for (int y = minGy; y <= maxGy; ++y) {
      for (int x = minGx; x <= maxGx; ++x) {
        if (solidMask[idx(x, y)]) continue;
        float dx = x - cx;
        float dy = y - cy;
        float dSq = dx * dx + dy * dy;
        if (dSq <= rSq) {
          float falloff = 1.0f - std::sqrt(dSq) / (rCells + 0.001f);
          u[idx(x, y)] += fx * falloff;
          v[idx(x, y)] += fy * falloff;
        }
      }
    }
  }

  // Dichte einspeisen (Wasser oder Feuer)
  void addWaterDensity(float screenX, float screenY, float amount, float radius) {
    addDensityField(densityWater, screenX, screenY, amount, radius);
  }

  void addFireDensity(float screenX, float screenY, float amount, float radius) {
    addDensityField(densityFire, screenX, screenY, amount, radius);
  }

  // Radiale Explosion / Druckwelle (z.B. Dampf-Explosion oder Supernova)
  void addRadialImpulse(float screenX, float screenY, float strength, float radius) {
    float cx = screenX / CELL_SIZE;
    float cy = screenY / CELL_SIZE;
    float rCells = radius / CELL_SIZE;
    int minGx = std::clamp(static_cast<int>(cx - rCells), 0, GW - 1);
    int maxGx = std::clamp(static_cast<int>(cx + rCells), 0, GW - 1);
    int minGy = std::clamp(static_cast<int>(cy - rCells), 0, GH - 1);
    int maxGy = std::clamp(static_cast<int>(cy + rCells), 0, GH - 1);

    float rSq = rCells * rCells;
    for (int y = minGy; y <= maxGy; ++y) {
      for (int x = minGx; x <= maxGx; ++x) {
        if (solidMask[idx(x, y)]) continue;
        float dx = x - cx;
        float dy = y - cy;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > 0.001f && dist <= rCells) {
          float falloff = (1.0f - dist / rCells) * (strength / dist);
          u[idx(x, y)] += dx * falloff;
          v[idx(x, y)] += dy * falloff;
        }
      }
    }
  }

  // Thermischer Auftrieb für Feuer (Warme Luft steigt nach oben)
  void addBuoyancy(float screenX, float screenY, float buoyancyForce, float radius) {
    addVelocity(screenX, screenY, 0.0f, -buoyancyForce, radius);
  }

  // Hauptsimulations-Schritt (Jos Stam Operator Splitting mit Vorticity Confinement)
  void step(float dt, float viscosity = 0.0f, float vorticity = -1.0f,
            float damping = -1.0f, int solverIters = 12) {
    dt = std::clamp(dt, 0.001f, 0.04f);

    float vortStr = (vorticity < 0.0f) ? defaultVorticity : vorticity;
    float dampRate = (damping < 0.0f) ? defaultDamping : damping;

    // 1. Zähigkeit / Viskose Diffusion (nur falls Viskosität gesetzt)
    if (viscosity > 0.0f) {
      diffuse(dt, viscosity, solverIters);
    }

    // 2. VORTICITY CONFINEMENT (Fedkiw, Stam, Jensen 2001)
    // Wirkt der numerischen Dissipation der Semi-Lagrangian Advektion entgegen:
    // Regeneriert kleinräumige Wirbelstrukturen, Strudel und agile Zirkulation!
    if (vortStr > 0.0f) {
      applyVorticityConfinement(dt, vortStr);
    }

    // 3. Druck-Projektion vor Advektion (macht das Feld divergenzfrei)
    project(solverIters);

    // 4. Semi-Lagrangian Advektion (Selbst-Transport der Geschwindigkeit)
    advect(dt);

    // 5. Druck-Projektion nach Advektion (beseitigt Divergenzen aus Advektion)
    project(solverIters);

    // 6. Dichtefelder advektieren (Wasser & Feuer mit agiler Persistenz)
    advectDensity(densityWater, dt, 0.70f);
    advectDensity(densityFire, dt, 1.20f);

    // 7. Sanfte Luftreibung / Abklingen (Minimal gedämpft für maximale Fluid-Agilität)
    float decay = std::clamp(1.0f - dampRate * dt, 0.0f, 1.0f);
    for (int i = 0; i < NUM_CELLS; ++i) {
      if (solidMask[i]) {
        u[i] = 0.0f;
        v[i] = 0.0f;
        densityWater[i] = 0.0f;
        densityFire[i] = 0.0f;
        curl[i] = 0.0f;
      } else {
        u[i] *= decay;
        v[i] *= decay;
      }
    }
  }

  // Bilineares Auslesen der Fluidgeschwindigkeit an beliebigen Bildschirm-Pixeln
  void sampleVelocity(float screenX, float screenY, float &outVx, float &outVy) const {
    float gx = (screenX / CELL_SIZE) - 0.5f;
    float gy = (screenY / CELL_SIZE) - 0.5f;

    int x0 = static_cast<int>(std::floor(gx));
    int y0 = static_cast<int>(std::floor(gy));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float s1 = gx - x0;
    float s0 = 1.0f - s1;
    float t1 = gy - y0;
    float t0 = 1.0f - t1;

    auto getU = [this](int x, int y) -> float {
      if (x < 0 || x >= GW || y < 0 || y >= GH) return 0.0f;
      int i = idx(x, y);
      return solidMask[i] ? 0.0f : u[i];
    };
    auto getV = [this](int x, int y) -> float {
      if (x < 0 || x >= GW || y < 0 || y >= GH) return 0.0f;
      int i = idx(x, y);
      return solidMask[i] ? 0.0f : v[i];
    };

    outVx = s0 * (t0 * getU(x0, y0) + t1 * getU(x0, y1)) +
            s1 * (t0 * getU(x1, y0) + t1 * getU(x1, y1));
    outVy = s0 * (t0 * getV(x0, y0) + t1 * getV(x0, y1)) +
            s1 * (t0 * getV(x1, y0) + t1 * getV(x1, y1));
  }

  // Dichtefelder an Bildschirmkoordinaten abtasten
  float sampleWaterDensity(float screenX, float screenY) const {
    return sampleScalar(densityWater, screenX, screenY);
  }

  float sampleFireDensity(float screenX, float screenY) const {
    return sampleScalar(densityFire, screenX, screenY);
  }

  const std::vector<float> &getWaterDensity() const { return densityWater; }
  const std::vector<float> &getFireDensity() const { return densityFire; }

  // Wirbelstärke an Bildschirmkoordinaten abtasten
  float sampleCurl(float screenX, float screenY) const {
    return sampleScalar(curl, screenX, screenY);
  }

  void setVorticityStrength(float s) { defaultVorticity = s; }
  void setDamping(float d) { defaultDamping = d; }
  float getVorticityStrength() const { return defaultVorticity; }
  float getDamping() const { return defaultDamping; }

private:
  std::vector<float> u, v;             // Horizontale & vertikale Geschwindigkeiten (Pixel/s)
  std::vector<float> u0, v0;           // Scratch-Puffer
  std::vector<float> p, div;          // Druck & Divergenz
  std::vector<float> densityWater;     // Wasser-Dichtefeld
  std::vector<float> densityFire;      // Feuer-Wärmedichtefeld
  std::vector<float> density0;         // Scratch-Puffer für Dichte
  std::vector<float> curl;             // Wirbelstärke (omega) für Vorticity Confinement
  std::vector<uint8_t> solidMask;      // 1 = Festes Hindernis / Boden

  float defaultVorticity = 3.6f;       // Standard-Wirbelkraft (stabile Zirkulation & Agilität)
  float defaultDamping = 0.10f;        // Minimale Luftdämpfung (vorher 0.70f)

  void addDensityField(std::vector<float> &field, float screenX, float screenY, float amount, float radius) {
    float cx = screenX / CELL_SIZE;
    float cy = screenY / CELL_SIZE;
    float rCells = radius / CELL_SIZE;
    int minGx = std::clamp(static_cast<int>(cx - rCells), 0, GW - 1);
    int maxGx = std::clamp(static_cast<int>(cx + rCells), 0, GW - 1);
    int minGy = std::clamp(static_cast<int>(cy - rCells), 0, GH - 1);
    int maxGy = std::clamp(static_cast<int>(cy + rCells), 0, GH - 1);

    float rSq = rCells * rCells;
    for (int y = minGy; y <= maxGy; ++y) {
      for (int x = minGx; x <= maxGx; ++x) {
        if (solidMask[idx(x, y)]) continue;
        float dx = x - cx;
        float dy = y - cy;
        float dSq = dx * dx + dy * dy;
        if (dSq <= rSq) {
          float falloff = 1.0f - std::sqrt(dSq) / (rCells + 0.001f);
          field[idx(x, y)] = std::min(1.0f, field[idx(x, y)] + amount * falloff);
        }
      }
    }
  }

  float sampleScalar(const std::vector<float> &field, float screenX, float screenY) const {
    float gx = (screenX / CELL_SIZE) - 0.5f;
    float gy = (screenY / CELL_SIZE) - 0.5f;

    int x0 = static_cast<int>(std::floor(gx));
    int y0 = static_cast<int>(std::floor(gy));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float s1 = gx - x0;
    float s0 = 1.0f - s1;
    float t1 = gy - y0;
    float t0 = 1.0f - t1;

    auto getVal = [this, &field](int x, int y) -> float {
      if (x < 0 || x >= GW || y < 0 || y >= GH) return 0.0f;
      int i = idx(x, y);
      return solidMask[i] ? 0.0f : field[i];
    };

    return s0 * (t0 * getVal(x0, y0) + t1 * getVal(x0, y1)) +
           s1 * (t0 * getVal(x1, y0) + t1 * getVal(x1, y1));
  }

  // ===========================================================================
  // VORTICITY CONFINEMENT (Fedkiw, Stam, Jensen 2001)
  // ===========================================================================
  // Wirkt der numerischen Dissipation der Semi-Lagrangian Advektion entgegen:
  // 1. Berechnet die lokale Wirbelstärke omega = curl(u, v) = dv/dx - du/dy
  // 2. Bestimmt den Gradienten der Wirbelstärke-Magnitude |omega| (Richtung zum Wirbelzentrum)
  // 3. Speist eine tangentiale Beschleunigung F = eps * dx * (N x omega) ein,
  //    die Wirbel aktiv beschleunigt und agil aufrecht erhält!
  void applyVorticityConfinement(float dt, float strength) {
    if (strength <= 0.0f) return;

    float invTwoDx = 0.5f / CELL_SIZE;

    // 1. Wirbelstärke (omega) an jedem Gitterpunkt berechnen
    for (int y = 1; y < GH - 1; ++y) {
      int rowIdx = y * GW;
      for (int x = 1; x < GW - 1; ++x) {
        int i = rowIdx + x;
        if (solidMask[i]) {
          curl[i] = 0.0f;
          continue;
        }
        float du_dy = (u[i + GW] - u[i - GW]) * invTwoDx;
        float dv_dx = (v[i + 1] - v[i - 1]) * invTwoDx;
        curl[i] = dv_dx - du_dy;
      }
    }

    // 2. Gradient von |omega| und tangentiale Confinement-Kraft
    float maxF = 450.0f; // Begrenzung für bedingungslose Stabilität
    for (int y = 2; y < GH - 2; ++y) {
      int rowIdx = y * GW;
      for (int x = 2; x < GW - 2; ++x) {
        int i = rowIdx + x;
        if (solidMask[i]) continue;

        float dabs_x = (std::abs(curl[i + 1]) - std::abs(curl[i - 1])) * invTwoDx;
        float dabs_y = (std::abs(curl[i + GW]) - std::abs(curl[i - GW])) * invTwoDx;

        float len = std::sqrt(dabs_x * dabs_x + dabs_y * dabs_y) + 1e-5f;
        float nx = dabs_x / len;
        float ny = dabs_y / len;

        float omega = curl[i];

        // 2D Kreuzprodukt: N x omega = (ny * omega, -nx * omega)
        float fx = std::clamp(strength * CELL_SIZE * (ny * omega), -maxF, maxF);
        float fy = std::clamp(strength * CELL_SIZE * (-nx * omega), -maxF, maxF);

        u[i] += fx * dt;
        v[i] += fy * dt;
      }
    }
  }

  // Viskose Diffusion (Implizit via Gauss-Seidel)
  void diffuse(float dt, float visc, int iters) {
    float a = dt * visc * (GW * GH);
    float cRecip = 1.0f / (1.0f + 4.0f * a);

    u0 = u;
    v0 = v;

    for (int k = 0; k < iters; ++k) {
      for (int y = 1; y < GH - 1; ++y) {
        for (int x = 1; x < GW - 1; ++x) {
          int i = idx(x, y);
          if (solidMask[i]) continue;

          float uSum = (solidMask[idx(x - 1, y)] ? 0.0f : u[idx(x - 1, y)]) +
                       (solidMask[idx(x + 1, y)] ? 0.0f : u[idx(x + 1, y)]) +
                       (solidMask[idx(x, y - 1)] ? 0.0f : u[idx(x, y - 1)]) +
                       (solidMask[idx(x, y + 1)] ? 0.0f : u[idx(x, y + 1)]);

          float vSum = (solidMask[idx(x - 1, y)] ? 0.0f : v[idx(x - 1, y)]) +
                       (solidMask[idx(x + 1, y)] ? 0.0f : v[idx(x + 1, y)]) +
                       (solidMask[idx(x, y - 1)] ? 0.0f : v[idx(x, y - 1)]) +
                       (solidMask[idx(x, y + 1)] ? 0.0f : v[idx(x, y + 1)]);

          u[i] = (u0[i] + a * uSum) * cRecip;
          v[i] = (v0[i] + a * vSum) * cRecip;
        }
      }
      setBoundaries(u, 1);
      setBoundaries(v, 2);
    }
  }

  // Semi-Lagrangian Advektion für Geschwindigkeitsfeld
  void advect(float dt) {
    u0 = u;
    v0 = v;

    float dt0x = dt / CELL_SIZE;
    float dt0y = dt / CELL_SIZE;

    for (int y = 1; y < GH - 1; ++y) {
      for (int x = 1; x < GW - 1; ++x) {
        int i = idx(x, y);
        if (solidMask[i]) {
          u[i] = 0.0f;
          v[i] = 0.0f;
          continue;
        }

        float backX = x - dt0x * u0[i];
        float backY = y - dt0y * v0[i];

        backX = std::clamp(backX, 0.5f, static_cast<float>(GW - 1.5f));
        backY = std::clamp(backY, 0.5f, static_cast<float>(GH - 1.5f));

        int i0 = static_cast<int>(backX);
        int i1 = i0 + 1;
        int j0 = static_cast<int>(backY);
        int j1 = j0 + 1;

        float s1 = backX - i0;
        float s0 = 1.0f - s1;
        float t1 = backY - j0;
        float t0 = 1.0f - t1;

        u[i] = s0 * (t0 * u0[idx(i0, j0)] + t1 * u0[idx(i0, j1)]) +
               s1 * (t0 * u0[idx(i1, j0)] + t1 * u0[idx(i1, j1)]);

        v[i] = s0 * (t0 * v0[idx(i0, j0)] + t1 * v0[idx(i0, j1)]) +
               s1 * (t0 * v0[idx(i1, j0)] + t1 * v0[idx(i1, j1)]);
      }
    }
    setBoundaries(u, 1);
    setBoundaries(v, 2);
  }

  // Semi-Lagrangian Advektion für Skalar-Dichtefelder
  void advectDensity(std::vector<float> &d, float dt, float dissipation) {
    density0 = d;
    float dt0x = dt / CELL_SIZE;
    float dt0y = dt / CELL_SIZE;
    float decay = std::clamp(1.0f - dissipation * dt, 0.0f, 1.0f);

    for (int y = 1; y < GH - 1; ++y) {
      for (int x = 1; x < GW - 1; ++x) {
        int i = idx(x, y);
        if (solidMask[i]) {
          d[i] = 0.0f;
          continue;
        }

        float backX = x - dt0x * u[i];
        float backY = y - dt0y * v[i];

        backX = std::clamp(backX, 0.5f, static_cast<float>(GW - 1.5f));
        backY = std::clamp(backY, 0.5f, static_cast<float>(GH - 1.5f));

        int i0 = static_cast<int>(backX);
        int i1 = i0 + 1;
        int j0 = static_cast<int>(backY);
        int j1 = j0 + 1;

        float s1 = backX - i0;
        float s0 = 1.0f - s1;
        float t1 = backY - j0;
        float t0 = 1.0f - t1;

        float val = s0 * (t0 * density0[idx(i0, j0)] + t1 * density0[idx(i0, j1)]) +
                    s1 * (t0 * density0[idx(i1, j0)] + t1 * density0[idx(i1, j1)]);
        d[i] = val * decay;
      }
    }
  }

  // Helmholtz-Hodge Zerlegung: Enforce Incompressibility (div = 0)
  void project(int iters) {
    float h = 1.0f / GW;

    // 1. Divergenz berechnen
    for (int y = 1; y < GH - 1; ++y) {
      for (int x = 1; x < GW - 1; ++x) {
        int i = idx(x, y);
        if (solidMask[i]) {
          div[i] = 0.0f;
          p[i] = 0.0f;
          continue;
        }

        float uRight = solidMask[idx(x + 1, y)] ? -u[i] : u[idx(x + 1, y)];
        float uLeft  = solidMask[idx(x - 1, y)] ? -u[i] : u[idx(x - 1, y)];
        float vDown  = solidMask[idx(x, y + 1)] ? -v[i] : v[idx(x, y + 1)];
        float vUp    = solidMask[idx(x, y - 1)] ? -v[i] : v[idx(x, y - 1)];

        div[i] = -0.5f * h * (uRight - uLeft + vDown - vUp);
        p[i] = 0.0f;
      }
    }
    setBoundaries(div, 0);
    setBoundaries(p, 0);

    // 2. Poisson-Gleichung für Druck p lösen (Gauss-Seidel)
    for (int k = 0; k < iters; ++k) {
      for (int y = 1; y < GH - 1; ++y) {
        for (int x = 1; x < GW - 1; ++x) {
          int i = idx(x, y);
          if (solidMask[i]) continue;

          float pLeft  = solidMask[idx(x - 1, y)] ? p[i] : p[idx(x - 1, y)];
          float pRight = solidMask[idx(x + 1, y)] ? p[i] : p[idx(x + 1, y)];
          float pUp    = solidMask[idx(x, y - 1)] ? p[i] : p[idx(x, y - 1)];
          float pDown  = solidMask[idx(x, y + 1)] ? p[i] : p[idx(x, y + 1)];

          p[i] = (div[i] + pLeft + pRight + pUp + pDown) * 0.25f;
        }
      }
      setBoundaries(p, 0);
    }

    // 3. Druck-Gradient abziehen -> Geschwindigkeit divergenzfrei machen
    for (int y = 1; y < GH - 1; ++y) {
      for (int x = 1; x < GW - 1; ++x) {
        int i = idx(x, y);
        if (solidMask[i]) continue;

        float pLeft  = solidMask[idx(x - 1, y)] ? p[i] : p[idx(x - 1, y)];
        float pRight = solidMask[idx(x + 1, y)] ? p[i] : p[idx(x + 1, y)];
        float pUp    = solidMask[idx(x, y - 1)] ? p[i] : p[idx(x, y - 1)];
        float pDown  = solidMask[idx(x, y + 1)] ? p[i] : p[idx(x, y + 1)];

        u[i] -= 0.5f * (pRight - pLeft) / h;
        v[i] -= 0.5f * (pDown - pUp) / h;
      }
    }
    setBoundaries(u, 1);
    setBoundaries(v, 2);
  }

  // Randbedingungen (Wände & feste Objekte)
  void setBoundaries(std::vector<float> &field, int b) {
    // 1. Bildschirm-Aussenränder
    for (int x = 0; x < GW; ++x) {
      field[idx(x, 0)]      = (b == 2) ? -field[idx(x, 1)]      : field[idx(x, 1)];
      field[idx(x, GH - 1)] = (b == 2) ? -field[idx(x, GH - 2)] : field[idx(x, GH - 2)];
    }
    for (int y = 0; y < GH; ++y) {
      field[idx(0, y)]      = (b == 1) ? -field[idx(1, y)]      : field[idx(1, y)];
      field[idx(GW - 1, y)] = (b == 1) ? -field[idx(GW - 2, y)] : field[idx(GW - 2, y)];
    }

    // Ecken
    field[idx(0, 0)]           = 0.5f * (field[idx(1, 0)] + field[idx(0, 1)]);
    field[idx(0, GH - 1)]      = 0.5f * (field[idx(1, GH - 1)] + field[idx(0, GH - 2)]);
    field[idx(GW - 1, 0)]      = 0.5f * (field[idx(GW - 2, 0)] + field[idx(GW - 1, 1)]);
    field[idx(GW - 1, GH - 1)] = 0.5f * (field[idx(GW - 2, GH - 1)] + field[idx(GW - 1, GH - 2)]);

    // 2. Feste Hindernisse (Gelände & Plattformen)
    for (int y = 1; y < GH - 1; ++y) {
      for (int x = 1; x < GW - 1; ++x) {
        int i = idx(x, y);
        if (solidMask[i]) {
          field[i] = 0.0f;
        }
      }
    }
  }
};
