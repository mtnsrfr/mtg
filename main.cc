#include "Engine.hpp"
#include "MenuGame.hpp"

// =============================================================================
// Hier startet unsere Retro-Spielebox! (Hauptprogramm)
// =============================================================================
int main() {
  // Erstelle das Spielfenster:
  // - 320x240 Pixel (Retro-Auflösung)
  // - 4-fache Vergrößerung (Fenstergröße: 1280x960 Pixel für gestochen scharfe Retro-Pixel)
  // - Fenstertitel: "MTG Retro Spielebox"
  EngineApp app(320, 240, 4, "MTG Retro Spielebox");

  // Starte mit dem Hauptmenü (hier kann man zwischen Pong und Snake wählen)
  app.setGame(std::make_unique<MenuGame>());

  // Starte die Spiel-Schleife (läuft mit flüssigen 60 Bildern pro Sekunde)
  app.run();

  return 0;
}