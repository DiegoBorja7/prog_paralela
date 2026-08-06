#include <SFML/Graphics.hpp>
#include <omp.h>
#include <fmt/core.h>
#include <cstring>
#include <chrono>
#include "arial_ttf.h"

#define CHANNELS 4

extern "C" double kernel_gaussian_blur(const unsigned char *src_image, unsigned char *dst_image, int width, int height);

// Implementacion OpenMP
double gaussian_blur_openmp(const unsigned char *src, unsigned char *dst, int width, int height)
{
    int K[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}};

    auto start_time = std::chrono::high_resolution_clock::now();

#pragma omp parallel
    {

#pragma omp for
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int r = 0, g = 0, b = 0, a = 0;
                int weight_sum = 0;

                for (int i = -1; i <= 1; ++i)
                {
                    for (int j = -1; j <= 1; ++j)
                    {
                        int cur_x = x + j;
                        int cur_y = y + i;

                        // Manejo de bordes ajustando pesos
                        if (cur_x >= 0 && cur_x < width && cur_y >= 0 && cur_y < height)
                        {
                            int neighbor_weight = K[i + 1][j + 1];
                            int neighbor_idx = (cur_y * width + cur_x) * CHANNELS;

                            r += src[neighbor_idx] * neighbor_weight;
                            g += src[neighbor_idx + 1] * neighbor_weight;
                            b += src[neighbor_idx + 2] * neighbor_weight;
                            a += src[neighbor_idx + 3] * neighbor_weight;
                            weight_sum += neighbor_weight;
                        }
                    }
                }

                int pixel_idx = (y * width + x) * CHANNELS;
                dst[pixel_idx] = r / weight_sum;
                dst[pixel_idx + 1] = g / weight_sum;
                dst[pixel_idx + 2] = b / weight_sum;
                dst[pixel_idx + 3] = a / weight_sum;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = end_time - start_time;
    return ms.count();
}

int main()
{
    sf::Image img;
    // sf::Image para cargar la imagen
    if (!img.loadFromFile("image01.jpg"))
    {
        fmt::println("Error al cargar image01.jpg");
        return 1;
    }

    unsigned int w = img.getSize().x;
    unsigned int h = img.getSize().y;
    size_t buffer_size = w * h * CHANNELS;

    const uint8_t *ptr = img.getPixelsPtr();

    // Buffers
    uint8_t *original_buffer = (uint8_t *)malloc(buffer_size);
    uint8_t *processed_buffer = (uint8_t *)malloc(buffer_size);

    std::memcpy(original_buffer, ptr, buffer_size);

    // Ventana SFML
    sf::RenderWindow window(sf::VideoMode({(unsigned int)(1600), (unsigned int)(900)}), "Examen: Gaussian Blur");

    sf::Texture texture(sf::Vector2u(w, h));
    texture.update(original_buffer);
    sf::Sprite sprite(texture);

    sprite.setScale({window.getSize().x * 1.0f / w, window.getSize().y * 1.0f / h});

    const sf::Font font(arial_ttf::data, arial_ttf::data_len);

    sf::Text textOverlay(font, "", 24);
    textOverlay.setFillColor(sf::Color::Yellow);
    textOverlay.setOutlineColor(sf::Color::Black);
    textOverlay.setOutlineThickness(2.0f);
    textOverlay.setPosition({20, 20});
    textOverlay.setStyle(sf::Text::Bold);

    sf::Text textOptions(font, "TECLAS: [B] Aplicar Filtro | [R] Restaurar Original | [C] Cambiar config (CUDA/OpenMP)", 20);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setOutlineColor(sf::Color::Black);
    textOptions.setOutlineThickness(2.0f);
    textOptions.setPosition({20, window.getView().getSize().y - 40});

    // Variables de estado
    bool is_filtered = false;
    bool use_cuda = true;
    double last_process_time_ms = 0.0;

    // FPS
    int frames = 0;
    int fps = 0;
    sf::Clock clockFrames;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();

                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::R:
                    texture.update(original_buffer);
                    is_filtered = false;
                    last_process_time_ms = 0.0;
                    break;

                case sf::Keyboard::Scan::B:
                    if (use_cuda)
                    {
                        last_process_time_ms = kernel_gaussian_blur(original_buffer, processed_buffer, w, h);
                    }
                    else
                    {
                        last_process_time_ms = gaussian_blur_openmp(original_buffer, processed_buffer, w, h);
                    }
                    texture.update(processed_buffer);
                    is_filtered = true;
                    break;

                case sf::Keyboard::Scan::C:
                    use_cuda = !use_cuda;
                    break;
                }
            }
        }

        // FPS
        frames++;
        if (clockFrames.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clockFrames.restart();
        }

        // Overlay : implementacion activa, tiempo ms, dimensiones, FPS
        std::string overlay = fmt::format("Implementacion activa: {}\nEstado: {}\nTiempo de procesamiento: {:.2f} ms\nDimensiones: {}x{}\nFPS: {}",
                                          use_cuda ? "CUDA (Malla 1D)" : "OpenMP (#pragma omp parallel)",
                                          is_filtered ? "Filtro Aplicado" : "Original",
                                          last_process_time_ms, w, h, fps);
        textOverlay.setString(overlay);

        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.draw(textOverlay);
        window.draw(textOptions);
        window.display();
    }

    free(original_buffer);
    free(processed_buffer);

    return 0;
}