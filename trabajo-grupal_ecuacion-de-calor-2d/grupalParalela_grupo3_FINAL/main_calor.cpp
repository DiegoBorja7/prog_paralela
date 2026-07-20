#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <vector>
#include "calor_serial.h"
#include "calor_simd.h"
#include "calor_openmp.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define WIDTH 1600
#define HEIGHT 900

enum class runtime_type
{
    SERIAL = 0,
    SIMD,
    OPENMP,
    MPI
};

int main()
{
    int nx = 128;
    int ny = 128;
    double lx = 1.0;
    double ly = 1.0;
    double alpha = 0.25;
    double dt = 5.0e-7;
    int max_iters = 100000;
    double tol = 1.0e-4;

    runtime_type r_type = runtime_type::SERIAL;

    uint32_t *pixel_buffer = new uint32_t[WIDTH * HEIGHT];

    std::vector<double> u_old(nx * ny);
    std::vector<double> u_new(nx * ny);
    iniciar_calor(u_old, nx, ny);
    iniciar_calor(u_new, nx, ny);

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Ecuacion de Calor 2D - SFML");

#ifdef _WIN32
    HWND hwnh = window.getNativeHandle();
    ShowWindow(hwnh, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    sf::Sprite sprite(texture);

    sf::Font font("arial.ttf");
    sf::Text text(font, "Ecuacion de Calor", 15);
    text.setFillColor(sf::Color::Black);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Options: [1]Serial [2]SIMD [3]OpenMP | P: Pause/Resume | R: Reset";
    sf::Text textOptions(font, options, 14);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setPosition({10, window.getView().getSize().y - 40});
    textOptions.setStyle(sf::Text::Bold);

    int frames = 0;
    int fps = 0;
    sf::Clock clock;

    int iter_actual = 0;
    double residuo_L2 = 0.0;
    bool paused = false;

    double tiempo_acumulado_serial = 0.0;
    double tiempo_acumulado_simd   = 0.0;
    double tiempo_acumulado_openmp = 0.0;

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
                case sf::Keyboard::Scan::Num1:
                    r_type = runtime_type::SERIAL;
                    break;
                case sf::Keyboard::Scan::Num2:
                    r_type = runtime_type::SIMD;
                    break;
                case sf::Keyboard::Scan::Num3:
                    r_type = runtime_type::OPENMP;
                    break;
                case sf::Keyboard::Scan::P:
                    paused = !paused;
                    break;
                case sf::Keyboard::Scan::R:
                    iniciar_calor(u_old, nx, ny);
                    iniciar_calor(u_new, nx, ny);
                    iter_actual = 0;
                    residuo_L2 = 0.0;
                    paused = false;
                    tiempo_acumulado_serial = 0.0;
                    tiempo_acumulado_simd   = 0.0;
                    tiempo_acumulado_openmp = 0.0;
                    break;
                default:
                    break;
                }
            }
        }

        double mflops = 0.0;

        if (!paused && iter_actual < max_iters && (iter_actual == 0 || residuo_L2 > tol))
        {
            sf::Clock step_clock;
            int iters_por_frame = 50;

            for (int k = 0; k < iters_por_frame && iter_actual < max_iters; k++)
            {
                if (r_type == runtime_type::SERIAL)
                {
                    residuo_L2 = calor_serial(u_old, u_new, nx, ny, lx, ly, alpha, dt);
                }
                else if (r_type == runtime_type::SIMD)
                {
                    residuo_L2 = calor_simd(u_old, u_new, nx, ny, lx, ly, alpha, dt);
                }
                else if (r_type == runtime_type::OPENMP)
                {
                    residuo_L2 = calor_openmp(u_old, u_new, nx, ny, lx, ly, alpha, dt);
                }
                
                u_old = u_new;
                iter_actual++;

                if (residuo_L2 <= tol)
                    break;
            }

            float step_time = step_clock.getElapsedTime().asSeconds();

            if (r_type == runtime_type::SERIAL)
                tiempo_acumulado_serial += step_time;
            else if (r_type == runtime_type::SIMD)
                tiempo_acumulado_simd += step_time;
            else if (r_type == runtime_type::OPENMP)
                tiempo_acumulado_openmp += step_time;

            double ops = (double)(nx - 2) * (ny - 2) * 7.0 * iters_por_frame;
            if (step_time > 0)
            {
                mflops = (ops / step_time) / 1e6;
            }
        }

        std::string mode = "Serial";
        double t_actual = tiempo_acumulado_serial;

        if (r_type == runtime_type::SIMD) {
            mode = "SIMD";
            t_actual = tiempo_acumulado_simd;
        } else if (r_type == runtime_type::OPENMP) {
            mode = "OpenMP";
            t_actual = tiempo_acumulado_openmp;
        }

        acotado(u_old, pixel_buffer, nx, ny, WIDTH, HEIGHT);
        texture.update((const uint8_t *)pixel_buffer);
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format("Calor 2D | Iter: {}/{} | L2: {:.6e} | Tiempo: {:.3f}s | FPS: {} | Backend: {} | {:.2f} MFLOPS",
                               iter_actual, max_iters, residuo_L2, t_actual, fps, mode, mflops);
        text.setString(msg);

        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.draw(textOptions);
        window.display();
    }

    delete[] pixel_buffer;
    return 0;
}