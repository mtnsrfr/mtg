#pragma once

#include "Engine.hpp"
#include <algorithm>
#include <cmath>
#include <string>

// =============================================================================
// DAS 2-SPIELER PONG SPIEL
// =============================================================================
// Spieler 1 (Rot, links):  W = Hoch, S = Runter
// Spieler 2 (Blau, rechts): Pfeil-Hoch = Hoch, Pfeil-Runter = Runter
// Leertaste (Space):       Spiel starten / Neustarten
// =============================================================================
struct PongGame : Game {
  PongGame() {
    resetGame();
  }

  // Diese Funktion wird 60-mal pro Sekunde aufgerufen (das Herzstück des Spiels)
  void update(Engine &e) override {
    // 1. Spielfeld mit dunklem Rasengrün übermalen
    e.cls(Colors::DarkGreen);

    // 2. Spielfeldlinien zeichnen (Netz, Begrenzung, Mittelkreis)
    drawPitch(e);

    // 3. Je nachdem in welcher Phase wir sind, die passende Logik ausführen
    switch (state) {
    case State::Title:
      updateTitle(e); // Startbildschirm: Wartet auf Leertaste
      break;
    case State::Playing:
      updatePlaying(e); // Das aktive Match: Ball fliegt, Schläger bewegen sich
      break;
    case State::PointScored:
      updatePointScored(e); // Jemand hat ein Tor erzielt: Kurze Jubelpause
      break;
    case State::Victory:
      updateVictory(e); // Ein Spieler hat 5 Punkte erreicht und gewonnen!
      break;
    }

    // 4. Schläger, Ball und Spielstand zeichnen
    drawPaddles(e);
    if (state != State::Title) {
      drawBall(e);
    }
    drawScores(e);
  }

  // --- Spiel-Phasen (Zustände) ---
  enum class State {
    Title,       // Start-Menü ("Drücke Leertaste zum Starten")
    Playing,     // Ball im Spiel
    PointScored, // Torpause ("Spieler hat gepunktet!")
    Victory      // Siegesbildschirm ("Rot / Blau gewinnt!")
  };

  State state = State::Title;

  // --- Schläger-Einstellungen (Paddles) ---
  const float paddleWidth = 6.0f;    // Schläger-Breite in Pixeln
  const float paddleHeight = 36.0f;  // Schläger-Höhe in Pixeln
  const float paddleSpeed = 160.0f;  // Schläger-Geschwindigkeit (Pixel pro Sekunde)

  float p1Y = 100.0f; // Y-Position von Spieler 1 (Rot, links)
  float p2Y = 100.0f; // Y-Position von Spieler 2 (Blau, rechts)
  const float p1X = 14.0f;  // Feste X-Position links
  const float p2X = 300.0f; // Feste X-Position rechts

  // --- Ball-Einstellungen ---
  float ballX = 160.0f;  // X-Position des Balls (Bildschirmmitte: 160)
  float ballY = 120.0f;  // Y-Position des Balls (Bildschirmmitte: 120)
  float ballVx = 0.0f;   // Horizontale Geschwindigkeit
  float ballVy = 0.0f;   // Vertikale Geschwindigkeit
  const float ballRadius = 3.5f;        // Ballgröße (Radius in Pixeln)
  const float initialBallSpeed = 150.0f; // Start-Geschwindigkeit des Balls
  const float maxBallSpeed = 320.0f;     // Maximale Geschwindigkeit bei schnellen Ballwechseln

  // --- Spielstand & Regeln ---
  int score1 = 0; // Punkte für Spieler 1 (Rot)
  int score2 = 0; // Punkte für Spieler 2 (Blau)
  const int winScore = 5; // Wer zuerst 5 Punkte hat, gewinnt das Spiel!

  // --- Timer & Hilfsvariablen ---
  float stateTimer = 0.0f; // Wartezeit-Uhr für Torpausen
  int lastScorer = 0;      // Welcher Spieler hat zuletzt gepunktet (1 oder 2)

  // ===========================================================================
  // Sound-Effekte für Pong
  // ===========================================================================
  void playPaddleHit(Engine &e) {
    // Heller, knackiger Ton (G5), wenn der Ball den Schläger trifft
    e.play_tone(Notes::G5, 0.08f);
  }

  void playWallHit(Engine &e) {
    // Tieferer Abprall-Ton (G4), wenn der Ball oben/unten anstößt
    e.play_tone(Notes::G4, 0.06f);
  }

  void playStartSound(Engine &e) {
    // Fröhliche 4-Ton-Aufstiegs-Fanfare beim Spielstart (C-Dur Arpeggio: C5 -> E5 -> G5 -> C6)
    e.play_melody({
        {Notes::C5, 0.10f},
        {Notes::E5, 0.10f},
        {Notes::G5, 0.10f},
        {Notes::C6, 0.28f}
    });
  }

  void playSadSound(Engine &e) {
    // Traurige absteigende 4-Ton-Melodie bei Punktverlust (Fs4 -> F4 -> E4 -> Ds4)
    e.play_melody({
        {Notes::Fs4, 0.18f},
        {Notes::F4, 0.18f},
        {Notes::E4, 0.20f},
        {Notes::Ds4, 0.45f}
    });
  }

  // Setzt das Spiel auf den Anfangszustand zurück
  void resetGame() {
    score1 = 0;
    score2 = 0;
    p1Y = 102.0f;
    p2Y = 102.0f;
    state = State::Title;
  }

  // Schlägt den Ball aus der Mitte auf (in Richtung des angegebenen Spielers)
  void serveBall(int serveTowardPlayer) {
    ballX = 160.0f;
    ballY = 120.0f;
    float direction = (serveTowardPlayer == 1) ? -1.0f : 1.0f;
    // Zufälliger Startwinkel zwischen -30 und +30 Grad
    float angle = ((rand() % 60) - 30.0f) * (M_PI / 180.0f);
    ballVx = direction * initialBallSpeed * std::cos(angle);
    ballVy = initialBallSpeed * std::sin(angle);
  }

  // Tastatureingaben der beiden Spieler abfragen und Schläger bewegen
  void handlePaddleMovement(Engine &e) {
    float dt = e.dt(); // Verstrichene Zeit seit dem letzten Frame

    // Spieler 1 (Rot): Steuerung mit W (hoch) und S (runter)
    if (e.key(Key::W)) {
      p1Y -= paddleSpeed * dt;
    }
    if (e.key(Key::S)) {
      p1Y += paddleSpeed * dt;
    }

    // Spieler 2 (Blau): Steuerung mit Pfeiltaste Hoch und Runter
    if (e.key(Key::Up)) {
      p2Y -= paddleSpeed * dt;
    }
    if (e.key(Key::Down)) {
      p2Y += paddleSpeed * dt;
    }

    // Verhindern, dass die Schläger aus dem Spielfeld herausfahren
    const float minY = 18.0f;
    const float maxY = 222.0f - paddleHeight;
    p1Y = std::clamp(p1Y, minY, maxY);
    p2Y = std::clamp(p2Y, minY, maxY);
  }

  // --- Startbildschirm Logik ---
  void updateTitle(Engine &e) {
    handlePaddleMovement(e);

    // Blinkender Starttext (wechselt alle 0.4 Sekunden)
    bool blink = (int(e.time() * 2.5f) % 2) == 0;

    // Titel und Steuerungs-Hinweise anzeigen
    e.draw_text(60, 60, "RETRO PONG 2-PLAYER", Colors::Yellow, 2);
    e.draw_text(26, 175, "P1 (RED) : W / S", Colors::Red, 1);
    e.draw_text(194, 175, "P2 (BLUE): UP / DOWN", Colors::Blue, 1);

    if (blink) {
      e.draw_text(84, 120, "PRESS SPACE TO START", Colors::White, 1);
    }

    // Wenn Leertaste gedrückt wird: Startfanfare spielen und Match beginnen!
    if (e.pressed(Key::Space)) {
      playStartSound(e);
      score1 = 0;
      score2 = 0;
      serveBall(1);
      state = State::Playing;
    }
  }

  // --- Haupt-Spiel-Logik während des Matches ---
  void updatePlaying(Engine &e) {
    handlePaddleMovement(e);

    float dt = e.dt();
    // Ball-Position anhand seiner Geschwindigkeit aktualisieren
    ballX += ballVx * dt;
    ballY += ballVy * dt;

    // 1. Abprallen an oberer und unterer Wand
    const float topBound = 16.0f + ballRadius;
    const float bottomBound = 224.0f - ballRadius;

    if (ballY <= topBound) {
      ballY = topBound;
      ballVy = -ballVy;
      playWallHit(e); // Wand-Ton abspielen
    } else if (ballY >= bottomBound) {
      ballY = bottomBound;
      ballVy = -ballVy;
      playWallHit(e); // Wand-Ton abspielen
    }

    // 2. Treffer-Prüfung: Schläger 1 (Rot, links)
    if (ballVx < 0 && ballX - ballRadius <= p1X + paddleWidth &&
        ballX + ballRadius >= p1X) {
      if (ballY + ballRadius >= p1Y && ballY - ballRadius <= p1Y + paddleHeight) {
        ballX = p1X + paddleWidth + ballRadius;

        // Je nachdem, wo der Ball den Schläger trifft (Mitte vs. Rand), ändert sich der Winkel!
        float hitFactor = (ballY - (p1Y + paddleHeight / 2.0f)) / (paddleHeight / 2.0f);
        hitFactor = std::clamp(hitFactor, -1.0f, 1.0f);

        // Ball wird bei jedem Schlag 5% schneller (bis zum Tempolimit)
        float currentSpeed = std::min(maxBallSpeed, std::sqrt(ballVx * ballVx + ballVy * ballVy) * 1.05f);
        float bounceAngle = hitFactor * 50.0f * (M_PI / 180.0f); // Max. 50 Grad Abprallwinkel

        ballVx = currentSpeed * std::cos(bounceAngle);
        ballVy = currentSpeed * std::sin(bounceAngle);

        playPaddleHit(e); // Schläger-Ton abspielen
      }
    }

    // 3. Treffer-Prüfung: Schläger 2 (Blau, rechts)
    if (ballVx > 0 && ballX + ballRadius >= p2X &&
        ballX - ballRadius <= p2X + paddleWidth) {
      if (ballY + ballRadius >= p2Y && ballY - ballRadius <= p2Y + paddleHeight) {
        ballX = p2X - ballRadius;

        // Dynamischer Abprallwinkel für rechten Schläger
        float hitFactor = (ballY - (p2Y + paddleHeight / 2.0f)) / (paddleHeight / 2.0f);
        hitFactor = std::clamp(hitFactor, -1.0f, 1.0f);

        float currentSpeed = std::min(maxBallSpeed, std::sqrt(ballVx * ballVx + ballVy * ballVy) * 1.05f);
        float bounceAngle = hitFactor * 50.0f * (M_PI / 180.0f);

        ballVx = -currentSpeed * std::cos(bounceAngle);
        ballVy = currentSpeed * std::sin(bounceAngle);

        playPaddleHit(e); // Schläger-Ton abspielen
      }
    }

    // 4. Punkte-Erfassung (wenn der Ball links oder rechts ins Aus fliegt)
    if (ballX < 0) {
      // Ball im linken Aus -> Spieler 2 (Blau) bekommt einen Punkt!
      score2++;
      lastScorer = 2;
      playSadSound(e); // Traurige Melodie abspielen
      if (score2 >= winScore) {
        state = State::Victory;
      } else {
        state = State::PointScored;
        stateTimer = 1.3f; // 1.3 Sekunden Jubelpause
      }
    } else if (ballX > 320) {
      // Ball im rechten Aus -> Spieler 1 (Rot) bekommt einen Punkt!
      score1++;
      lastScorer = 1;
      playSadSound(e); // Traurige Melodie abspielen
      if (score1 >= winScore) {
        state = State::Victory;
      } else {
        state = State::PointScored;
        stateTimer = 1.3f; // 1.3 Sekunden Jubelpause
      }
    }
  }

  // --- Tor-Jubelpause Logik ---
  void updatePointScored(Engine &e) {
    handlePaddleMovement(e);
    stateTimer -= e.dt();

    // Text anzeigen, wer gerade gepunktet hat
    if (lastScorer == 1) {
      e.draw_text(96, 95, "PLAYER 1 SCORED!", Colors::Red, 1);
    } else {
      e.draw_text(96, 95, "PLAYER 2 SCORED!", Colors::Blue, 1);
    }

    // Wenn die Pause vorbei ist: Ball neu aufschlagen
    if (stateTimer <= 0.0f) {
      serveBall(lastScorer == 1 ? 2 : 1);
      state = State::Playing;
    }
  }

  // --- Siegesbildschirm Logik ---
  void updateVictory(Engine &e) {
    bool blink = (int(e.time() * 3.0f) % 2) == 0;

    // Sieger verkünden
    if (score1 >= winScore) {
      e.draw_text(80, 80, "PLAYER 1 (RED) WINS!", Colors::Red, 1);
    } else {
      e.draw_text(74, 80, "PLAYER 2 (BLUE) WINS!", Colors::Blue, 1);
    }

    // Blinkende Aufforderung für ein neues Match
    if (blink) {
      e.draw_text(76, 130, "PRESS SPACE TO RESTART", Colors::White, 1);
    }

    if (e.pressed(Key::Space)) {
      playStartSound(e);
      score1 = 0;
      score2 = 0;
      serveBall(1);
      state = State::Playing;
    }
  }

  // ===========================================================================
  // Grafische Zeichenfunktionen für das Spiel
  // ===========================================================================

  // Zeichnet das Tennis-/Fußball-Rasenfeld
  void drawPitch(Engine &e) {
    // Obere und untere weiße Spielfeldbegrenzung
    e.rectfill(0, 14, 320, 2, Colors::White);
    e.rectfill(0, 224, 320, 2, Colors::White);

    // Gestrichelte Mittellinie (Netz)
    for (int y = 18; y < 222; y += 8) {
      e.rectfill(159, y, 2, 4, Colors::LightGreen);
    }

    // Mittelkreis
    e.circle(160, 119, 28, Colors::LightGreen);
  }

  // Zeichnet die beiden Spielerschläger
  void drawPaddles(Engine &e) {
    // Schläger 1: Rot (links)
    e.rectfill(p1X, p1Y, paddleWidth, paddleHeight, Colors::Red);

    // Schläger 2: Blau (rechts)
    e.rectfill(p2X, p2Y, paddleWidth, paddleHeight, Colors::Blue);
  }

  // Zeichnet den leuchtend gelben Ball
  void drawBall(Engine &e) {
    e.circlefill(ballX, ballY, ballRadius, Colors::Yellow);
  }

  // Zeichnet die großen digitalen Spielstandszahlen oben am Bildschirm
  void drawScores(Engine &e) {
    // Punkte Spieler 1 (Rot)
    e.draw_digit(120, 22, score1, Colors::Red, 3);

    // Doppelpunkt-Trennzeichen in der Mitte
    e.rectfill(158, 26, 3, 3, Colors::White);
    e.rectfill(158, 34, 3, 3, Colors::White);

    // Punkte Spieler 2 (Blau)
    e.draw_digit(180, 22, score2, Colors::Blue, 3);
  }
};
