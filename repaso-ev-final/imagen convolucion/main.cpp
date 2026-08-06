#include <SFML/Graphics.hpp>
#include <cuda_runtime.h>
#include <fmt/core.h>
#include <cstring>
#include "arial_ttf.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define WIDTH 1600
#define HEIGHT 900

// buffers
uint8_t *original_pixel_buffer = nullptr;
uint8_t *host_pixel_buffer = nullptr;

uint8_t *device_pixel_buffer_in = nullptr;
uint8_t *device_pixel_buffer_out = nullptr;

//-- CUDA error checking
#define CHECK(expr)                                                                                                               \
    {                                                                                                                             \
        auto internal_error = (expr);                                                                                             \
        if (internal_error != cudaSuccess)                                                                                        \
        {                                                                                                                         \
            fmt::println("{}: {} in {} at line {}", (int)internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__); \
            exit(EXIT_FAILURE);                                                                                                   \
        }                                                                                                                         \
    }

//-- CUDA kernel
extern "C" void kernel_blur(unsigned char *src_image, unsigned char *dst_image, int width, int height, int blur_step);

int main()
{
    sf::Image img;

    if (!img.loadFromFile("image01.jpg"))
    {
        fmt::println("No se pudo cargar imagen");
        return 1;
    }

    int deviceId = 0;

    CHECK(cudaSetDevice(deviceId));

    cudaDeviceProp props;
    CHECK(cudaGetDeviceProperties(&props, deviceId));

    //--inicializar
    unsigned int w = img.getSize().x;
    unsigned int h = img.getSize().y;

    size_t buffer_size = w * h * 4;

    host_pixel_buffer = (uint8_t *)malloc(buffer_size);
    original_pixel_buffer = (uint8_t *)malloc(buffer_size);

    std::memcpy(host_pixel_buffer, img.getPixelsPtr(), buffer_size);
    std::memcpy(original_pixel_buffer, img.getPixelsPtr(), buffer_size);

    CHECK(cudaMalloc(&device_pixel_buffer_in, buffer_size));
    CHECK(cudaMalloc(&device_pixel_buffer_out, buffer_size));

    CHECK(cudaMemcpy(device_pixel_buffer_in, host_pixel_buffer, buffer_size, cudaMemcpyHostToDevice));

    //--inicializr la UI
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Examen Set - SFML");

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Vector2u size = {w, h};
    sf::Texture texture(size);
    texture.update(host_pixel_buffer);
    sf::Sprite sprite(texture);

    const sf::Font font(arial_ttf::data, arial_ttf::data_len);

    sf::Text text(font, "Examen Set", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Opciones: [R] Normal | Regresar [B] Bordes";
    sf::Text textOptions(font, options, 24);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    // FPS
    int frames = 0;
    int fps = 0;
    sf::Clock clockFrames;

    kernel_blur((unsigned char *)device_pixel_buffer_in, (unsigned char *)device_pixel_buffer_out, w, h, 1);
    CHECK(cudaGetLastError());

    CHECK(cudaMemcpy(host_pixel_buffer, device_pixel_buffer_out, buffer_size, cudaMemcpyDeviceToHost));

    texture.update((const uint8_t *)original_pixel_buffer);

    bool mode = false;

    float xx = window.getSize().x * 1.0f / w;

    sprite.setScale({window.getSize().x * 1.0f / w, window.getSize().y * 1.0f / h});

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();

                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::R:
                    texture.update(original_pixel_buffer);
                    mode = true;
                    break;
                case sf::Keyboard::Scan::B:
                    texture.update(host_pixel_buffer);
                    mode = false;
                    break;
                }
            }
        }

        // contar FPS
        frames++;
        if (clockFrames.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clockFrames.restart();
        }

        // actualizar el titulo
        auto msg = fmt::format("EXAMEN: FPS: {} | Mode: {}", fps, mode ? "NORMAL" : "BORDES");
        text.setString(msg);

        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }
        window.display();
    }

    //--liberar memoria
    free(original_pixel_buffer);
    free(host_pixel_buffer);
    CHECK(cudaFree(device_pixel_buffer_in));
    CHECK(cudaFree(device_pixel_buffer_out));

    return 0;
}