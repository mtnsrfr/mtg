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
    ballVy = -ballVy;      // Richtung umdrehen: Aus + wird - und aus - wird +!
    e.play_tone(400, 0.05f); // Abprall-Ton!
}
```

### 2. Schläger trifft Ball (Kollisionsprüfung)
Wir prüfen, ob sich der Ball innerhalb der Schläger-Grenzen befindet:
```cpp
if (ballX <= schlaegerX + schlaegerBreite &&
    ballY >= schlaegerY && ballY <= schlaegerY + schlaegerHoehe) {
    ballVx = -ballVx;      // Ball fliegt wieder nach rechts zurück!
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
  `e.play_tone(220.0, 0.15f);` (Ton A3)
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

## 🏆 Level 6: Wie man ein komplettes Spiel baut

Schau dir **[PongGame.hpp](file:///Users/mtn/code/mtg/PongGame.hpp)** und **[SnakeGame.hpp](file:///Users/mtn/code/mtg/SnakeGame.hpp)** an. Ein gutes Spiel hat meistens 3 Phasen:

```
+-------------------------------------------------------+
|  1. Startbildschirm: "DRÜCKE LEERTASTE ZUM STARTEN"   |
+---------------------------+---------------------------+
                            | (Spieler drückt Space)
                            v
+-------------------------------------------------------+
|  2. Das Spiel läuft: Punkte sammeln, Bälle schlagen   |
+---------------------------+---------------------------+
                            | (Jemand hat das Ziel erreicht)
                            v
+-------------------------------------------------------+
|  3. Sieger-Bildschirm: "ROT GEWINNT! NEUES SPIEL?"    |
+-------------------------------------------------------+
```

---

## 💡 Level 7: Die Spiele-Bibliothek

In deinem Projekt findest du bereits drei fertige Spiele, die du spielen, verändern und erweitern kannst:

### 1. 🏓 [PongGame.hpp](file:///Users/mtn/code/mtg/PongGame.hpp) (2-Spieler Retro Pong)
* Spieler 1 (Rot) mit `W`/`S`, Spieler 2 (Blau) mit `Pfeiltasten`.
* Physik mit dynamischen Abprallwinkeln und Treffer-Tönen!

### 2. 🐍 [SnakeGame.hpp](file:///Users/mtn/code/mtg/SnakeGame.hpp) (2-Spieler Schlangen-Duell - Modell B)
* Beide Schlangen jagen nach roten Äpfeln.
* **Der Apfel-Klau-Trick**: Wenn eine Schlange anstößt, zerplatzt ihr Körper in Äpfel, die der Gegner fressen kann!
* Wer zuerst 12 Punkte hat, gewinnt!

### 3. 🚀 [StarfighterGame.hpp](file:///Users/mtn/code/mtg/StarfighterGame.hpp) (Weltraum-Schlacht)
* **Partikel-Sternenfeld**: 70 funkelnde Sterne fliegen mit 3 verschiedenen Geschwindigkeiten an dir vorbei.
* **16x16 Pixel-Art Raumschiffe**: Dein eigener Delta-Wing Jäger mit Doppellaser gegen feindliche TIE-Attacker.
* **Explosionen**: Treffer lassen bunte Trümmerteilchen (Partikel) in alle Richtungen sprühen!
* Wer die meisten Angreifer abschießt, stellt den neuen Highscore auf!

### 4. 🕹️ [MenuGame.hpp](file:///Users/mtn/code/mtg/MenuGame.hpp) (Die Spiele-Auswahl)
* Taste `1` startet Pong, Taste `2` startet Snake, Taste `3` startet Starfighter.
* Mit `Escape` kommst du jederzeit zurück ins Menü!

---

## ✨ Level 8: Das Geheimnis der Partikel (Sterne & Explosionen)

Ein **Partikel** ist einfach ein kleiner bunter Punkt, der eine Position $(X, Y)$ und eine Geschwindigkeit hat:

```cpp
struct Stern {
    float x;
    float y;
    float geschwindigkeit; // z.B. 30 (langsam/dunkel) oder 120 (schnell/hell)
};

// In jedem Frame wandert der Stern nach unten:
stern.y += stern.geschwindigkeit * e.dt();
```
Wenn du viele davon auf den Bildschirm zauberst, entsteht ein wunderschöner, lebendiger 3D-Weltraum!

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
Colors::Yellow, Colors::White, Colors::Black
```

**Viel Spaß beim Erfinden und Programmieren deiner eigenen Spiele! 🚀**
