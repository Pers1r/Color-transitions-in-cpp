#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

struct Polar {
    float r;
    float alpha;
    bool valid;
};

void HSLtoRGB(float H, float S, float L, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) {
    float C = (1.0f - std::abs(2.0f * L - 1.0f)) * S;
    float H_prime = H / 60.0f;
    float X = C * (1.0f - std::abs(std::fmod(H_prime, 2.0f) - 1.0f));
    float m = L - C / 2.0f;

    float R1 = 0, G1 = 0, B1 = 0;
    if (H_prime >= 0 && H_prime < 1) { R1 = C; G1 = X; B1 = 0; }
    else if (H_prime >= 1 && H_prime < 2) { R1 = X; G1 = C; B1 = 0; }
    else if (H_prime >= 2 && H_prime < 3) { R1 = 0; G1 = C; B1 = X; }
    else if (H_prime >= 3 && H_prime < 4) { R1 = 0; G1 = X; B1 = C; }
    else if (H_prime >= 4 && H_prime < 5) { R1 = X; G1 = 0; B1 = C; }
    else if (H_prime >= 5 && H_prime < 6) { R1 = C; G1 = 0; B1 = X; }

    r = static_cast<std::uint8_t>((R1 + m) * 255.0f);
    g = static_cast<std::uint8_t>((G1 + m) * 255.0f);
    b = static_cast<std::uint8_t>((B1 + m) * 255.0f);
}

void HSVtoRGB(float H, float S, float V, std::uint8_t& r, std::uint8_t& g, std::uint8_t& b) {
    float C = V * S;
    float H_prime = H / 60.0f;
    float X = C * (1.0f - std::abs(std::fmod(H_prime, 2.0f) - 1.0f));
    float m = V - C;

    float R1 = 0, G1 = 0, B1 = 0;
    if (H_prime >= 0 && H_prime < 1) { R1 = C; G1 = X; B1 = 0; }
    else if (H_prime >= 1 && H_prime < 2) { R1 = X; G1 = C; B1 = 0; }
    else if (H_prime >= 2 && H_prime < 3) { R1 = 0; G1 = C; B1 = X; }
    else if (H_prime >= 3 && H_prime < 4) { R1 = 0; G1 = X; B1 = C; }
    else if (H_prime >= 4 && H_prime < 5) { R1 = X; G1 = 0; B1 = C; }
    else if (H_prime >= 5 && H_prime < 6) { R1 = C; G1 = 0; B1 = X; }

    r = static_cast<std::uint8_t>((R1 + m) * 255.0f);
    g = static_cast<std::uint8_t>((G1 + m) * 255.0f);
    b = static_cast<std::uint8_t>((B1 + m) * 255.0f);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({750, 600}), "Modele i przestrzenie barw.", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Clock clock;
    unsigned int last_frame_time = 0;

    const int TEX_SIZE = 240;
    const float RADIUS = 120.0f;

    std::vector<std::uint8_t> pixelsHSL(TEX_SIZE * TEX_SIZE * 4, 0);
    std::vector<std::uint8_t> pixelsHSV(TEX_SIZE * TEX_SIZE * 4, 0);
    std::vector<std::uint8_t> pixelsCMY(TEX_SIZE * TEX_SIZE * 4, 0);
    std::vector<std::uint8_t> pixelsRGB(TEX_SIZE * TEX_SIZE * 4, 0);

    std::vector<Polar> polarMap(TEX_SIZE * TEX_SIZE);
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            float dx = static_cast<float>(x) - RADIUS;
            float dy = static_cast<float>(y) - RADIUS;
            float distance = std::sqrt(dx * dx + dy * dy);

            int idx = y * TEX_SIZE + x;
            if (distance <= RADIUS) {
                polarMap[idx].valid = true;
                polarMap[idx].r = distance / RADIUS;

                float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
                if (angle < 0) angle += 360.0f;
                polarMap[idx].alpha = angle;
            } else {
                polarMap[idx].valid = false;
            }
        }
    }

    sf::Texture texHSL, texHSV, texCMY, texRGB;
    (void)texHSL.resize({TEX_SIZE, TEX_SIZE});
    (void)texHSV.resize({TEX_SIZE, TEX_SIZE});
    (void)texCMY.resize({TEX_SIZE, TEX_SIZE});
    (void)texRGB.resize({TEX_SIZE, TEX_SIZE});

    sf::Sprite sprHSL(texHSL), sprHSV(texHSV), sprCMY(texCMY), sprRGB(texRGB);

    sprHSL.setPosition({50.f, 40.f});
    sprHSV.setPosition({350.f, 40.f});
    sprCMY.setPosition({50.f, 320.f});
    sprRGB.setPosition({350.f, 320.f});

    sf::Font font;
    if (!font.openFromFile("TerminusTTF-4.49.3.ttf")) {
        return -1;
    }

    auto createText = [&](const std::string& str, float x, float y) {
        sf::Text text(font, str, 12);
        text.setFillColor(sf::Color::Black);
        text.setPosition({x, y});
        return text;
    };

    sf::Text txtTitleHSL = createText("HSL", 50, 20);
    sf::Text txtTitleHSV = createText("HSV", 350, 20);
    sf::Text txtTitleCMY = createText("CMY", 50, 300);
    sf::Text txtTitleRGB = createText("RGB", 350, 300);

    sf::Text txtValHSL = createText("L=0.537", 240, 260);
    sf::Text txtValHSV = createText("V=0.537", 540, 260);
    sf::Text txtValCMY = createText("Y=54%", 240, 540);
    sf::Text txtValRGB = createText("B=137", 540, 540);
    sf::Text txtTime   = createText("Refresh time:\n0 us", 630, 350);

    float sliderValue = 0.537f;
    float slX = 660, slY = 60, slW = 30, slH = 255;

    sf::VertexArray sliderTrack(sf::PrimitiveType::TriangleStrip, 4);
    sliderTrack[0] = { {slX, slY}, sf::Color::Black };
    sliderTrack[1] = { {slX + slW, slY}, sf::Color::Black };
    sliderTrack[2] = { {slX, slY + slH}, sf::Color::White };
    sliderTrack[3] = { {slX + slW, slY + slH}, sf::Color::White };

    sf::RectangleShape sliderHandle({slW + 10.f, 6.f});
    sliderHandle.setFillColor(sf::Color::Black);
    sliderHandle.setOutlineColor(sf::Color::White);
    sliderHandle.setOutlineThickness(1.0f);

    std::ignore = clock.restart().asMicroseconds();

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            sf::Vector2i mPos = sf::Mouse::getPosition(window);
            if (mPos.x >= slX - 10 && mPos.x <= slX + slW + 10 && mPos.y >= slY && mPos.y <= slY + slH) {
                sliderValue = 1.0f - (static_cast<float>(mPos.y) - slY) / slH;
                if (sliderValue < 0.0f) sliderValue = 0.0f;
                if (sliderValue > 1.0f) sliderValue = 1.0f;
            }
        }

        sliderHandle.setPosition({slX - 5.f, slY + (1.0f - sliderValue) * slH - 3.f});

        txtValHSL.setString("L=" + std::to_string(sliderValue).substr(0, 5));
        txtValHSV.setString("V=" + std::to_string(sliderValue).substr(0, 5));
        txtValCMY.setString("Y=" + std::to_string(static_cast<int>(sliderValue * 100)) + "%");
        txtValRGB.setString("B=" + std::to_string(static_cast<int>(sliderValue * 255)));

        for (int y = 0; y < TEX_SIZE; ++y) {
            for (int x = 0; x < TEX_SIZE; ++x) {
                int idx = y * TEX_SIZE + x;
                int pxIdx = idx * 4;

                if (polarMap[idx].valid) {
                    float alpha = polarMap[idx].alpha;
                    float r_norm = polarMap[idx].r;
                    std::uint8_t R, G, B;

                    HSLtoRGB(alpha, r_norm, sliderValue, R, G, B);
                    pixelsHSL[pxIdx] = R; pixelsHSL[pxIdx+1] = G; pixelsHSL[pxIdx+2] = B; pixelsHSL[pxIdx+3] = 255;

                    HSVtoRGB(alpha, r_norm, sliderValue, R, G, B);
                    pixelsHSV[pxIdx] = R; pixelsHSV[pxIdx+1] = G; pixelsHSV[pxIdx+2] = B; pixelsHSV[pxIdx+3] = 255;

                    R = static_cast<std::uint8_t>((1.0f - r_norm) * 255.0f);
                    G = static_cast<std::uint8_t>((1.0f - (alpha / 360.0f)) * 255.0f);
                    B = static_cast<std::uint8_t>((1.0f - sliderValue) * 255.0f);
                    pixelsCMY[pxIdx] = R; pixelsCMY[pxIdx+1] = G; pixelsCMY[pxIdx+2] = B; pixelsCMY[pxIdx+3] = 255;

                    pixelsRGB[pxIdx] = static_cast<std::uint8_t>(r_norm * 255.0f);
                    pixelsRGB[pxIdx+1] = static_cast<std::uint8_t>((alpha / 360.0f) * 255.0f);
                    pixelsRGB[pxIdx+2] = static_cast<std::uint8_t>(sliderValue * 255.0f);
                    pixelsRGB[pxIdx+3] = 255;
                }
            }
        }

        texHSL.update(pixelsHSL.data());
        texHSV.update(pixelsHSV.data());
        texCMY.update(pixelsCMY.data());
        texRGB.update(pixelsRGB.data());

        last_frame_time = static_cast<unsigned int>(clock.getElapsedTime().asMilliseconds());
        clock.restart();
        txtTime.setString("Refresh time:\n" + std::to_string(last_frame_time) + " ms");

        window.clear(sf::Color::White);

        window.draw(sprHSL);
        window.draw(sprHSV);
        window.draw(sprCMY);
        window.draw(sprRGB);

        window.draw(txtTitleHSL);
        window.draw(txtTitleHSV);
        window.draw(txtTitleCMY);
        window.draw(txtTitleRGB);

        window.draw(txtValHSL);
        window.draw(txtValHSV);
        window.draw(txtValCMY);
        window.draw(txtValRGB);
        window.draw(txtTime);

        window.draw(sliderTrack);
        window.draw(sliderHandle);

        window.display();
    }
    return 0;
}