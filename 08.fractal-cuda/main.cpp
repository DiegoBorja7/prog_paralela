#include <iostream>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <complex>

#include "palette.h"
#include <cuda_runtime.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define WIDTH 1600
#define HEIGHT 900

int max_iteraciones = 10;

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

std::complex<double> c(-0.7, 0.27015);

uint32_t *pixel_buffer = nullptr;

uint32_t *host_pixel_buffer = nullptr;
uint32_t *device_pixel_buffer = nullptr;

#define CHECK(expr)                                                                                                               \
    {                                                                                                                             \
        auto internal_error = (expr);                                                                                             \
        if (internal_error != cudaSuccess)                                                                                        \
        {                                                                                                                         \
            fmt::println("{}: {} in {} at line {}", (int)internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__); \
            exit(EXIT_FAILURE);                                                                                                   \
        }                                                                                                                         \
    }

extern void
julia_gpu(double centro_real, double centro_img,
          int num_interaciones,
          double x_min, double y_min, double x_max,
          double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer);

extern void copiar_paleta(unsigned int *h_paleta);

int main()
{

    int deviceID = 0;
    cudaSetDevice(deviceID);
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceID);
 
    fmt::print("Device : {}\n", deviceProp.name);
    fmt::print("Total global memory : {} MB\n", deviceProp.totalGlobalMem / 1024.0 / 1024.0);
 
    size_t buffer_size = WIDTH * HEIGHT * sizeof(uint32_t);
    host_pixel_buffer = (uint32_t *)malloc(buffer_size);
    std::memset(host_pixel_buffer, 0, buffer_size);
 
    CHECK(cudaMalloc(&device_pixel_buffer, buffer_size));
 
    //-- inicializar UI
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Cuda Set - SFML");
 
#ifdef _WIN32
    HWND hwnh = window.getNativeHandle();
    ShowWindow(hwnh, SW_MAXIMIZE);
#endif
 
    sf::Texture texture(sf::Vector2u(WIDTH, HEIGHT));
    sf::Sprite sprite(texture);
 
    sf::Font font("arial.ttf");
    sf::Text text(font, "Julua Set", 15);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);
 
    // dibuja el cuadro de opciones para cambiar entre modos
    std::string options = "UP/DOWN : Change Iterations";
    sf::Text textOptions(font, options, 14);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setPosition({10, 30});
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});
 
    int frames = 0;
    int fps = 0;
    sf::Clock clock;
 
    // Copiar la paleta una sola vez antes de empezar a dibujar
    copiar_paleta(color_ramp.data());

    while (window.isOpen())
    {
        // Process events || Eventos por teclado manajado
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();
 
                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                        max_iteraciones = 10;
                    break;
 
                default:
                    break;
                }
            }
        }
 
        // dibujar
        std::string mode = "";
 
        mode = "GPU CUDA";
 
        // dibujar en la GPU
        julia_gpu(c.real(), c.imag(), max_iteraciones,
                  x_min, y_min, x_max, y_max, WIDTH, HEIGHT, device_pixel_buffer);
 
        CHECK(cudaMemcpy(host_pixel_buffer, device_pixel_buffer,
                         buffer_size, cudaMemcpyDeviceToHost));
 
        texture.update((const uint8_t *)host_pixel_buffer);
        frames++;
 
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }
 
        // acttualziar el titulo
        auto msg = fmt::format("Julia GPU: Iteraciones:{} , FPS:{} , Mode:{}", max_iteraciones, fps, mode);
        text.setString(msg);
 
        // dibujar
 
        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }
        window.display();
    }
 
    cudaFree(device_pixel_buffer);
    delete[] host_pixel_buffer;

    return 0;
}