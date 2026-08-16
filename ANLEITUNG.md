# 🎮 Die Spiele-Schmiede: Vom Anfänger zum Spiele-Programmierer (Zero to Hero)

Herzlich willkommen in deiner eigenen Spiele-Werkstatt! In diesem Handbuch lernst du Schritt für Schritt, wie du mit echtem C++ eigene Retro-Spiele (wie Pong, Snake, Breakout oder Space-Shooter) programmierst.

---

## 🚀 Kapitel 0: Das große Geheimnis der Videospiele

Hast du schon mal ein **Daumenkino** gebastelt? Wenn man schnell durch die Seiten blättert, erwachen die gezeichneten Figuren zum Leben!

Genauso funktioniert dein Computer:
* Unser Spiel malt **60-mal in jeder einzelnen Sekunde** ein neues Bild auf den Bildschirm.
* Das Herzstück jedes Spiels ist diese eine Funktion:
  ```cpp
  void update(Engine& e) {
      // Alles hier drin wird 60-mal pro Sekunde ausgeführt!
  }
  ```

---

## 🎨 Level 1: Die Malerwerkstatt (Farben & Formen)

### 1. Die Zauberleinwand & Koordinaten
Unser Bildschirm ist **320 Pixel breit** und **240 Pixel hoch**.
* **$X$ (nach rechts)**: Geht von $0$ (ganz links) bis $320$ (ganz rechts).
* **$Y$ (nach unten)**: Geht von $0$ (ganz oben) bis $240$ (ganz unten).
* Die Ecke oben links ist **$(0, 0)$**!

### 2. Der Farbkasten
Du kannst diese Farben überall verwenden:
* `Colors::DarkGreen` (Dunkelgrüner Rasen)
* `Colors::Red` (Leuchtendes Rot)
* `Colors::Blue` (Helles Blau)
* `Colors::Yellow` (Sonnengelb)
* `Colors::White` (Schneeweiß)
* `Colors::Black` (Tiefschwarz)
* `Colors::Orange`, `Colors::LightGreen`, `Colors::DarkGray`, `Colors::LightGray`

### 3. Deine Mal-Werkzeuge
| Befehl | Was er macht | Beispiel |
| :--- | :--- | :--- |
| `e.cls(Farbe)` | **Tafel putzen**: Malt den ganzen Bildschirm mit einer Farbe an. | `e.cls(Colors::DarkGreen);` |
| `e.pset(x, y, Farbe)` | **Punkt malen**: Setzt einen einzelnen Pixel. | `e.pset(160, 120, Colors::Yellow);` |
| `e.line(x1, y1, x2, y2, Farbe)` | **Linie ziehen**: Verbindet zwei Punkte mit einem Strich. | `e.line(0, 0, 320, 240, Colors::White);` |
| `e.rect(x, y, b, h, Farbe)` | **Rechteck-Rahmen**: Zeichnet den Umriss eines Kastens. | `e.rect(10, 10, 50, 30, Colors::Red);` |
| `e.rectfill(x, y, b, h, Farbe)` | **Rechteck füllen**: Zeichnet einen ausgefüllten Kasten. | `e.rectfill(10, 10, 50, 30, Colors::Red);` |
| `e.circle(x, y, r, Farbe)` | **Kreis-Rahmen**: Zeichnet den Umriss eines Kreises. | `e.circle(160, 120, 20, Colors::White);` |
| `e.circlefill(x, y, r, Farbe)` | **Ball malen**: Zeichnet einen ausgefüllten Kreis. | `e.circlefill(160, 120, 8, Colors::Yellow);` |
| `e.draw_text(x, y, "TEXT", Farbe)` | **Schrift schreiben**: Schreibt Text auf den Bildschirm. | `e.draw_text(100, 50, "HALLO WELT", Colors::White);` |
| `e.draw_digit(x, y, zahl, Farbe)` | **Große Zahl**: Schreibt Punktestände. | `e.draw_digit(100, 20, 5, Colors::Yellow, 3);` |

> 🎯 **Mini-Quest 1**: Male ein Haus mit einem dreieckigen Dach (aus Linien) und einer bunten Tür!

---

## 🏃 Level 2: Leben einhauchen! (Variablen & Bewegung)

### Was ist eine Variable?
Eine Variable ist wie eine **beschriftete Schachtel**, in der sich eine Zahl merkt:
```cpp
float ballX = 100.0f; // Eine Schachtel namens ballX mit dem Wert 100
```

### Die Zauberformel für Bewegung
Wenn wir in jedem Bild (`update`) die Zahl ein kleines Stückchen verändern, bewegt sich das Objekt:
```cpp
class MeinSpiel : public Game {
    float ballX = 0.0f;

    void update(Engine& e) override {
        e.cls(Colors::DarkGreen);

        // Den Ball ein Stück nach rechts schieben:
        ballX = ballX + 2.0f;

        // Ball an der neuen Position malen:
        e.circlefill(ballX, 120, 8, Colors::Yellow);
    }
};
```

---

## 🕹️ Level 3: Die Fernbedienung (Tastatur & Maus)

Jetzt machen wir das Spiel interaktiv!

### Tasten abfragen:
* `e.key(Key::...)` $\rightarrow$ Prüft, ob eine Taste **gedrückt gehalten** wird (perfekt für Schläger / Raumschiffe):
  ```cpp
  if (e.key(Key::W)) {
      spielerY = spielerY - 4.0f; // Nach oben fliegen
  }
  if (e.key(Key::S)) {
      spielerY = spielerY + 4.0f; // Nach unten fliegen
  }
  ```
* `e.pressed(Key::...)` $\rightarrow$ Prüft, ob eine Taste **gerade angetippt** wurde (perfekt für Schießen oder Springen):
  ```cpp
  if (e.pressed(Key::Space)) {
      e.play_tone(880, 0.1f); // Schuss-Ton abspielen!
  }
  ```

### Verfügbare Tasten:
* `Key::W`, `Key::S`, `Key::A`, `Key::D`
* `Key::Up`, `Key::Down`, `Key::Left`, `Key::Right`
* `Key::Space`, `Key::Enter`, `Key::Escape`
* `Key::Num1`, `Key::Num2`, `Key::Num3`

### Mit der Maus malen:
* `e.mouse_x()` und `e.mouse_y()` verraten dir die Position des Mauszeigers.
* `e.mouse_down()` prüft, ob die Maustaste gedrückt ist:
  ```cpp
  if (e.mouse_down()) {
      e.circlefill(e.mouse_x(), e.mouse_y(), 5, Colors::Yellow);
  }
  ```

---

## 💥 Level 4: Kollisionen & Spielphysik

### 1. Wand-Abprall (Der Flummi-Trick)
Wenn der Ball oben oder unten anstößt, kehren wir einfach seine vertikale Geschwindigkeit (`ballVy`) um:
```cpp
ballY = ballY + ballVy;

if (ballY <= 10 || ballY >= 230) {
    ballVy = -ballVy;        // Richtung umdrehen: Aus + wird - und aus - wird +!
    e.play_tone(400, 0.05f); // Abprall-Ton!
}
```

### 2. Schläger trifft Ball (Kollisionsprüfung)
Wir prüfen, ob sich der Ball innerhalb der Schläger-Grenzen befindet:
```cpp
if (ballX <= schlaegerX + schlaegerBreite &&
    ballY >= schlaegerY && ballY <= schlaegerY + schlaegerHoehe) {
    ballVx = -ballVx;        // Ball fliegt wieder nach rechts zurück!
    e.play_tone(800, 0.08f); // Treffer-Ton!
}
```

---

## 🎵 Level 5: Töne & Musik (Der Sound-Baukasten)

Töne machen ein Spiel erst richtig lebendig!

### 1. Einzelne Töne abspielen:
`e.play_tone(FrequenzInHertz, DauerInSekunden)`:
* **Heller Piep-Ton (z. B. Sprung / Treffer)**:
  `e.play_tone(784.0, 0.08f);` (Ton G5)
* **Tiefer Wumm (z. B. Wandstoß / Explosion)**:
  `e.play_tone(140.0, 0.20f);` (Tiefer Bass)
* **Sehr hoher Münzen-Sound**:
  `e.play_tone(1200.0, 0.05f);`

### 2. Eigene Melodien komponieren:
Mit `e.play_melody` kannst du mehrere Töne nacheinander abspielen:
```cpp
// Eine fröhliche 3-Ton-Fanfare:
e.play_melody({
    {523.25, 0.10f}, // C5
    {659.25, 0.10f}, // E5
    {783.99, 0.25f}  // G5 (länger)
});
```

---

## 🏓 Level 6: Spiel 1 – Das Retro-Pong Match ([PongGame.hpp](file:///Users/mtn/code/mtg/PongGame.hpp))

In **Pong** lernen wir, wie zwei Spieler gegeneinander antreten:
1. **Zwei Schläger**: Spieler 1 (Rot mit `W`/`S`), Spieler 2 (Blau mit Pfeiltasten).
2. **Dynamischer Abprallwinkel**: Trifft der Ball die Schlägermitte, fliegt er geradeaus. Trifft er den Rand, prallt er im steilen Winkel ab!
3. **Tempo-Steigerung**: Bei jedem Schlag wird der Ball 5% schneller (`speed *= 1.05f`).

---

## 🐍 Level 7: Spiel 2 – Das Schlangen-Duell ([SnakeGame.hpp](file:///Users/mtn/code/mtg/SnakeGame.hpp))

In **Snake** lernen wir zwei mächtige neue Programmier-Tricks kennen:

### 1. Das Kachel-Gitter (Grid)
Statt in einzelnen Pixeln rechnen wir hier in Kacheln (jede Kachel ist 8x8 Pixel groß):
* Unser Spielfeld hat **40 Kacheln Breite** und **30 Kacheln Höhe**.
* Eine Kachel-Position ist einfach:
  ```cpp
  struct Cell { int x, y; };
  ```

### 2. Wie bewegt sich eine Schlange?
Die Schlange ist eine Kette von Kacheln (`std::deque<Cell>`). In jedem Schritt passiert Magie:
```cpp
// 1. Vorne einen neuen Kopf in Gehrichtung anfügen:
body.push_front(neuerKopf);

// 2. Hat die Schlange einen Apfel gefressen?
if (apfelGefressen) {
    score++; // Schlange wächst automatisch, weil wir den Schwanz NICHT löschen!
} else {
    body.pop_back(); // Schwanz nachziehen: Letztes Glied hinten abschneiden
}
```

### 3. Der „Apfel-Klau“-Mechanismus (Modell B)
Wenn eine Schlange an die Wand oder in den Gegner fährt:
```cpp
// Jeden Körperteil der Schlange in einen leckeren Apfel auf dem Feld verwandeln:
for (size_t i = 1; i < body.size(); ++i) {
    apples.push_back(body[i]);
}
// Schlange startet sofort wieder klein in ihrer Ecke neu!
respawnSnake();
```
Der überlebende Spieler kann die Trümmer-Äpfel blitzschnell fressen und riesig werden!

---

## 🚀 Level 8: Spiel 3 – Die Weltraum-Schlacht ([StarfighterGame.hpp](file:///Users/mtn/code/mtg/StarfighterGame.hpp))

Hier steigen wir ein in die Welt der **Partikel-Systeme** und **Pixel-Art-Schiffe**:

### 1. Das 3D-Parallaxe Sternenfeld
Um dem Weltraum echte Tiefe zu verleihen, lassen wir 70 Sterne in **3 verschiedenen Tiefen-Ebenen** fliegen:
* **Hintergrund** (dunkelgrau, langsam: 25 px/s) $\rightarrow$ Weit entfernte Sterne
* **Mittlere Sterne** (hellgrau, mittel: 65 px/s)
* **Vordergrund** (strahlend weiß, schnell: 130 px/s) $\rightarrow$ Nahe Sterne

```cpp
struct Star {
    float x, y, speed;
    uint8_t color;
};

// In jedem Frame nach unten wandern:
s.y += s.speed * e.dt();
if (s.y >= 240.0f) { s.y = 0.0f; s.x = rand() % 320; }
```

### 2. 16x16 Pixel-Art Raumschiffe mit Bitmasken
Wir können Raumschiffe direkt im Code als Pixelmuster zeichnen:
```cpp
// Das 16x16 Delta X-Wing Schiff (Zeile für Zeile):
static const uint16_t playerSprite[16] = {
  0b0000000110000000, // Spitze
  0b0000001111000000, // Cockpit
  0b0100001111000010, // Flügelkanonen vorn
  0b0100111111110010, // Delta-Flügel
  0b1111111111111111, // Rumpf
  0b0000001001000000, // Schubflamme
};
```

### 3. Schiffs-Explosionen mit echter Physik (Trägheit & Farbverlauf)
Wenn ein Schiff explodiert, sprengen sich die Trümmer nicht nur im Kreis auseinander, sondern **nehmen den vollen Geschwindigkeitsvektor des Schiffs mit**:
```cpp
// 1. Schiffsimpuls + radiale Sprengung
p.vx = shipVx + burstSpeed * cos(angle);
p.vy = shipVy + burstSpeed * sin(angle);

// 2. Thermischer Farbverlauf über die Lebensdauer der Trümmer:
// Frisch (100%-75%): Weißer Blitz -> Gelb -> Orange -> Rote Glut -> Dunkler Rauch
```

---

## 🕹️ Level 9: Die Retro-Konsole ([MenuGame.hpp](file:///Users/mtn/code/mtg/MenuGame.hpp))

Wie baut man ein Hauptmenü, um zwischen mehreren Spielen umzuschalten?

```cpp
class MenuGame : public Game {
    std::unique_ptr<PongGame> pong;
    std::unique_ptr<SnakeGame> snake;
    std::unique_ptr<StarfighterGame> starfighter;

    void update(Engine& e) override {
        // Mit Escape immer zurück ins Menü!
        if (e.pressed(Key::Escape)) { currentGame = CurrentGame::Menu; }

        if (e.pressed(Key::Num1)) { currentGame = CurrentGame::Pong; }
        if (e.pressed(Key::Num2)) { currentGame = CurrentGame::Snake; }
        if (e.pressed(Key::Num3)) { currentGame = CurrentGame::Starfighter; }
    }
};
```

---

## 📋 Der Spickzettel (Cheat Sheet) auf einen Blick

```cpp
// --- Malen ---
e.cls(Farbe);                             // Bildschirm leeren
e.pset(x, y, Farbe);                     // Pixel setzen
e.line(x0, y0, x1, y1, Farbe);           // Linie ziehen
e.rectfill(x, y, breite, hoehe, Farbe);  // Ausgefülltes Rechteck
e.circlefill(x, y, radius, Farbe);       // Ausgefüllter Kreis
e.draw_text(x, y, "TEXT", Farbe);        // Text schreiben
e.draw_digit(x, y, zahl, Farbe, 3);      // Große Ziffer malen

// --- Steuerung ---
if (e.key(Key::W)) { ... }               // Taste gedrückt halten
if (e.pressed(Key::Space)) { ... }       // Taste neu angetippt
e.mouse_x();                             // Maus X
e.mouse_y();                             // Maus Y
e.mouse_down();                          // Maustaste gedrückt?

// --- Sound ---
e.play_tone(frequenz, dauer);            // Ton abspielen
e.play_melody({ {523, 0.1}, {659, 0.1} }); // Melodie

// --- Farben ---
Colors::DarkGreen, Colors::Red, Colors::Blue,
Colors::Yellow, Colors::White, Colors::Black,
Colors::Orange, Colors::LightGreen, Colors::DarkGray, Colors::LightGray
```

**Viel Spaß beim Erfinden und Programmieren deiner eigenen Spiele! 🚀**
