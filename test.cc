#include <SFML/Graphics.hpp>
#include <array>
#include <iostream>
#include <vector>

// Define the palette size
constexpr int PALETTE_SIZE = 256;

// Function to initialize the palette with grayscale values
void InitializePalette(std::array<sf::Color, PALETTE_SIZE>& palette) {
    for (int i = 0; i < PALETTE_SIZE; ++i) {
        palette[i] = sf::Color(i, i, i); // Grayscale values
    }
}
// Bresenham line drawing algorithm
void DrawLine(std::vector<uint8_t>& buffer, int width, int height, int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            buffer[y0 * width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

int main(int ac, char** av)
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "My window");

    // Define the buffer size
    constexpr int WIDTH = 320;
    constexpr int HEIGHT = 240;

    // Create the buffer and palette
    std::vector<uint8_t> buffer(WIDTH * HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        buffer[i] = i % 256;
    }
    std::array<sf::Color, PALETTE_SIZE> palette;
    InitializePalette(palette);

    // Create an image to render the buffer
    sf::Image image({WIDTH, HEIGHT}, sf::Color::Red); // Create an empty
    
    // Main loop
    while (window.isOpen())
    {
        // Check all the window's events that were triggered since the last iteration of the loop
        while (std::optional event = window.pollEvent())
        {
            // "close requested" event: we close the window
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Get the mouse position
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        // Clear the buffer
        std::fill(buffer.begin(), buffer.end(), 0);

        // Draw a line from the center of the buffer to the mouse position
        DrawLine(buffer, WIDTH, HEIGHT, WIDTH / 2, HEIGHT / 2, mousePos.x * WIDTH / window.getSize().x, mousePos.y * HEIGHT / window.getSize().y, 255);

        // Update the image with the buffer data
        for (uint32_t y = 0; y < HEIGHT; ++y) {
            for (uint32_t x = 0; x < WIDTH; ++x) {
                uint8_t pixelValue = buffer[y * WIDTH + x];
                image.setPixel({x, y}, palette[pixelValue]);
            }
        }

        // Create a texture and sprite to display the image
        sf::Texture texture;
        if (!texture.loadFromImage(image)) {
            std::cout << "ERROR: Failed to load texture from image." << std::endl;
            return 0;
        }
        sf::Sprite sprite(texture);
        // Scale the sprite to fit the window size
        sf::Vector2u windowSize = window.getSize();
        sprite.setScale(
            {static_cast<float>(windowSize.x) / WIDTH,
            static_cast<float>(windowSize.y) / HEIGHT}
        );

        // Clear the window and draw the sprite
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}