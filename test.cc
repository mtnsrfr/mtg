#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <iostream>
#include <vector>
#include <cmath> // Include cmath for M_PI

// Define the palette size
constexpr int PALETTE_SIZE = 256;

// Function to initialize the palette with grayscale values
void InitializePalette(std::array<sf::Color, PALETTE_SIZE> &palette)
{
    for (int i = 0; i < PALETTE_SIZE; ++i)
    {
        palette[i] = sf::Color(i, i, i); // Grayscale values
    }
}

// Function to compute the target frequency for a MIDI note
double computeFrequency(int midiNote)
{
    return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
}

class Buffer
{
public:
    Buffer(int width, int height)
        : width(width), height(height), data(width * height, 0)
    {
        InitializePalette(palette);
    }

    void clear()
    {
        std::fill(data.begin(), data.end(), 0);
    }

    void setPixel(int x, int y, uint8_t color)
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            data[y * width + x] = color;
        }
    }

    uint8_t getPixel(int x, int y) const
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            return data[y * width + x];
        }
        return 0;
    }

    // Bresenham line drawing algorithm
    void DrawLine(int x0, int y0, int x1, int y1, uint8_t color)
    {
        int dx = std::abs(x1 - x0);
        int dy = -std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;

        while (true)
        {
            this->setPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1)
                break;
            int e2 = 2 * err;
            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    const std::vector<uint8_t> &getData() const
    {
        return data;
    }

    const std::array<sf::Color, PALETTE_SIZE> &getPalette() const
    {
        return palette;
    }

    int getWidth() const
    {
        return width;
    }

    int getHeight() const
    {
        return height;
    }

private:
    int width;
    int height;
    std::vector<uint8_t> data;
    std::array<sf::Color, PALETTE_SIZE> palette;
};

std::vector<int16_t> generateSineWave(unsigned sampleRate, unsigned amplitude, double frequency)
{
    const double TWO_PI = 2 * M_PI;
    std::vector<int16_t> samples(sampleRate);
    for (unsigned i = 0; i < sampleRate; ++i)
    {
        samples[i] = amplitude * std::sin((TWO_PI * frequency * i) / sampleRate);
    }
    return samples;
}

class SoundSystem
{
public:
    SoundSystem()
    {
        // Sound buffer parameters
        const unsigned SAMPLE_RATE = 44100;
        const unsigned AMPLITUDE = 30000;
        const double FREQUENCY = 440.0;

        // Generate a sine wave
        std::vector<int16_t> samples = generateSineWave(SAMPLE_RATE, AMPLITUDE, FREQUENCY);

        // Load samples into the sound buffer
        if (!soundBuffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE, {sf::SoundChannel::Mono}))
        {
            std::cerr << "Failed to load sound buffer." << std::endl;
            throw std::runtime_error("Failed to load sound buffer.");
        }

        // Set the buffer to the sound
        sound = std::make_unique<sf::Sound>(soundBuffer);
    }

    void play()
    {
        sound->play();
    }

private:
    sf::SoundBuffer soundBuffer;
    std::unique_ptr<sf::Sound> sound;
};


int main(int ac, char **av)
{
    // Define the buffer size
    constexpr int WIDTH = 320;
    constexpr int HEIGHT = 240;

    sf::RenderWindow window(sf::VideoMode({WIDTH * 4, HEIGHT * 4}), "My window");

    // Create the buffer
    Buffer buffer(WIDTH, HEIGHT);

    // Create an image to render the buffer
    sf::Image image({WIDTH, HEIGHT}, sf::Color::Red); // Create an empty

    int midiNote = 69; // Example MIDI note
    double frequency = computeFrequency(midiNote);
    std::cout << "Frequency for MIDI note " << midiNote << " is " << frequency << " Hz" << std::endl;

    SoundSystem sound_system;
    sound_system.play();


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
        buffer.clear();

        // Draw a line from the center of the buffer to the mouse position
        buffer.DrawLine(WIDTH / 2, HEIGHT / 2, mousePos.x * WIDTH / window.getSize().x, mousePos.y * HEIGHT / window.getSize().y, 255);

        // Update the image with the buffer data
        for (uint32_t y = 0; y < HEIGHT; ++y) {
            for (uint32_t x = 0; x < WIDTH; ++x) {
                uint8_t pixelValue = buffer.getPixel(x, y);
                image.setPixel({x, y}, buffer.getPalette()[pixelValue]);
            }
        }

        // Create a texture and sprite to display the image
        sf::Texture texture;
        if (!texture.loadFromImage(image))
        {
            std::cout << "ERROR: Failed to load texture from image." << std::endl;
            return 0;
        }
        sf::Sprite sprite(texture);
        // Scale the sprite to fit the window size
        sf::Vector2u windowSize = window.getSize();
        sprite.setScale(
            {static_cast<float>(windowSize.x) / WIDTH,
             static_cast<float>(windowSize.y) / HEIGHT});

        // Clear the window and draw the sprite
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}