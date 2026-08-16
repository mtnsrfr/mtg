#include "Engine.hpp"
#include "PongGame.hpp"

// =============================================================================
// Hier startet unser Spiel! (Hauptprogramm)
// =============================================================================
int main() {
  // Erstelle das Spielfenster:
  // - 320x240 Pixel (wie bei alten Retro-Konsolen)
  // - 4-fache Vergrößerung (Fenstergröße: 1280x960 Pixel für gestochen scharfe Retro-Pixel)
  // - Fenstertitel: "Retro 2-Spieler Pong"
  EngineApp app(320, 240, 4, "Retro 2-Spieler Pong");

  // Setze das Pong-Spiel als unser aktives Spiel
  app.setGame(std::make_unique<PongGame>());

  // Starte die Spiel-Schleife (läuft mit flüssigen 60 Bildern pro Sekunde)
  app.run();

  return 0;
}