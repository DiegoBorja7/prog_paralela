#include <fmt/core.h>
#include <SFML/Graphics.hpp>

#include "fractal_serial.h"
#include "fractal_simd.h"
#include "fractal_openmp.h"
#include <complex>
#include <omp.h>

#define WIDTH 1600
#define HEIGHT 900

//-- parametros
int max_iteraciones = 10;

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

std::complex<double> c(-0.7, 0.27015);

//-- Textura
uint32_t *pixel_buffer = nullptr;

enum class runtime_type
{
    SERIAL_1 = 0,
    SERIAL_2,
    SIMD,
    OPENMP,
    OPENMP_FOR,
    OPENMP_FOR_SIMD
};

#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
    runtime_type r_type = runtime_type::SERIAL_2;
    pixel_buffer = new uint32_t[WIDTH * HEIGHT];

    // sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Julia Set - SFML", sf::Style::Default);

#ifdef _WIN32
    //
    HWND hWnd = window.getNativeHandle(); // Obtener el handle de la ventana SFML
    ShowWindow(hWnd, SW_MAXIMIZE);        // Maximizar la ventana
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Julia Set", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    // opciones
    std::string options = "Options: [1] Serial 1 [2] Serial 2 [3] SIMD 3 [4] OpenMP [5] OpenMP_FOR [6] OpenMP_FOR_SIMD |  Up/Down: Change iterations";
    sf::Text textOptions(font, options, 24);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    // fps
    sf::Clock clock;
    int fps = 0;
    int frames = 0;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto eventKey = event->getIf<sf::Event::KeyReleased>();

                switch (eventKey->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iteraciones += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iteraciones -= 10;
                    if (max_iteraciones < 10)
                    {
                        max_iteraciones = 10;
                    }
                    break;
                case sf::Keyboard::Scan::Num1:
                    r_type = runtime_type::SERIAL_1;
                    break;
                case sf::Keyboard::Scan::Num2:
                    r_type = runtime_type::SERIAL_2;
                    break;
                case sf::Keyboard::Scan::Num3:
                    r_type = runtime_type::SIMD;
                    break;
                case sf::Keyboard::Scan::Num4:
                    r_type = runtime_type::OPENMP;
                    break;
                case sf::Keyboard::Scan::Num5:
                    r_type = runtime_type::OPENMP_FOR;
                    break;
                case sf::Keyboard::Scan::Num6:
                    r_type = runtime_type::OPENMP_FOR_SIMD;
                    break;
                default:
                    break;
                }
            }

            std::memset(pixel_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t)); // Limpiar el buffer de píxeles antes de cada renderizado  
        }

        // crear la textura a partir del buffer de píxeles
        std::string mode = "";
        if (r_type == runtime_type::SERIAL_1)
        {
            julia_serial_1(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SERIAL_1";
        }
        else if (r_type == runtime_type::SERIAL_2)
        {
            julia_serial_2(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SERIAL_2";
        }
        else if (r_type == runtime_type::SIMD)
        {
            julia_simd(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "SIMD";
        }
        else if (r_type == runtime_type::OPENMP)
        {
            julia_openmp_regiones(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "OPENMP | Threads: " + std::to_string(omp_get_max_threads());
        }
        else if (r_type == runtime_type::OPENMP_FOR)
        {
            julia_openmp_for(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "OPENMP_FOR | Threads: " + std::to_string(omp_get_max_threads());
        }
        else if (r_type == runtime_type::OPENMP_FOR_SIMD)
        {
            julia_openmp_for_simd(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, pixel_buffer);
            mode = "OPENMP_FOR_SIMD | Threads: " + std::to_string(omp_get_max_threads());
        }
        texture.update((const uint8_t *)pixel_buffer);

        // contar FPS
        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // actualizar el titulo de la ventana
        auto msg = fmt::format("Julia Set: Iteraciones: {}, FPS: {}, Mode: {}", max_iteraciones, fps, mode);
        text.setString(msg);

        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }
        window.display();
    }

    delete[] pixel_buffer;

    return 0;
}
