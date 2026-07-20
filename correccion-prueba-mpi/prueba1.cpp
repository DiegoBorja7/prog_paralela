#include <iostream>
#include <mpi.h>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <vector>
#include <cstring>

#include "fractal_newton_mpi.h"
#include "draw_text.h"

namespace arial_ttf {
    extern size_t data_len;
    extern unsigned char data[];
}

#ifdef _WIN32
#include <windows.h>
#endif

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;
int max_iteraciones = 50;

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;
int running = 1;
int row_start, row_end, padding, delta, nproc, rank;

#define WIDTH 1600
#define HEIGHT 900

void dibujar_texto(int rank, int max_iter, int max_compute_ms, int total_iters, int fps) {
    auto texto = fmt::format("RANK: {} | Iteraciones maximas: {} | computo maximo en: {}ms | total iteraciones: {} | FPS: {}", 
                             rank, max_iter, max_compute_ms, total_iters, fps);
    draw_text_to_texture((unsigned char *)pixel_buffer, WIDTH, delta, texto.c_str(), 10, 25, 20);
}

void setup_ui() {
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Fractal Newton MPI - Prueba 1");

#ifdef _WIN32
    HWND hWnd = window.getNativeHandle();
    ShowWindow(hWnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);
    
    // Overlay dinámico
    sf::Text textTop(font, "", 20);
    textTop.setFillColor(sf::Color::White);
    textTop.setPosition({10, 10});
    textTop.setStyle(sf::Text::Bold);

    // Ayuda de controles
    sf::Text textHelp(font, "Opciones: Up/Down (Iteraciones) | ESC (cerrar)", 20);
    textHelp.setFillColor(sf::Color::White);
    textHelp.setStyle(sf::Text::Bold);
    textHelp.setPosition({10, (float)HEIGHT - 35});

    sf::Clock clock;
    int fps = 0, frames = 0;
    int max_compute_ms = 0;
    int global_total_iters = 0;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                running = 0;
            }
            else if (event->is<sf::Event::KeyReleased>()) {
                auto eventKey = event->getIf<sf::Event::KeyReleased>();
                switch (eventKey->scancode) {
                    case sf::Keyboard::Scan::Up:
                        max_iteraciones += 10;
                        break;
                    case sf::Keyboard::Scan::Down:
                        max_iteraciones -= 10;
                        if (max_iteraciones < 10) max_iteraciones = 10;
                        break;
                    case sf::Keyboard::Scan::Escape:
                        window.close();
                        running = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        double bcast_data[4] = { (double)max_iteraciones, 0.0, 0.0, (double)running };
        MPI_Bcast(bcast_data, 4, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        if (running == 0) break;

        // Medir tiempo
        auto t_start = std::chrono::high_resolution_clock::now();
        ComputeResult res = newton_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer, max_iteraciones);
        auto t_end = std::chrono::high_resolution_clock::now();
        int compute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();

        int local_iters = res.total_iterations;

        // Reduce para MAX compute y SUM iters
        MPI_Reduce(&compute_ms, &max_compute_ms, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_iters, &global_total_iters, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        // Copiar el buffer local
        std::memcpy(texture_buffer, pixel_buffer, WIDTH * delta * sizeof(uint32_t));

        for (int i = 1; i < nproc; i++) {
            int new_delta = delta;
            if (i == nproc - 1) new_delta = delta - padding;

            MPI_Status status;
            MPI_Recv(pixel_buffer, WIDTH * new_delta, MPI_UINT32_T, i, 0, MPI_COMM_WORLD, &status);
            std::memcpy(texture_buffer + i * delta * WIDTH, pixel_buffer, WIDTH * new_delta * sizeof(uint32_t));
        }

        texture.update((const uint8_t *)texture_buffer);

        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f) {
            fps = frames; frames = 0; clock.restart();
        }

        textTop.setString(fmt::format("RANK: {} | Iteraciones maximas: {} | computo maximo en: {}ms | total iteraciones: {} | FPS: {}", 
                                      rank, max_iteraciones, max_compute_ms, global_total_iters, fps));

        window.clear();
        window.draw(sprite);
        window.draw(textTop);
        window.draw(textHelp);
        window.display();
    }
    delete[] pixel_buffer;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    init_freetype();

    delta = std::ceil(HEIGHT * 1.0 / nproc);
    row_start = rank * delta;
    row_end = row_start + delta;
    padding = delta * nproc - HEIGHT;

    if (row_end > HEIGHT) row_end = HEIGHT;

    pixel_buffer = new uint32_t[WIDTH * delta];
    std::memset(pixel_buffer, 0, WIDTH * delta * sizeof(uint32_t));

    if (rank == 0) {
        setup_ui();
    } else {
        while (true) {
            double bcast_data[4] = {0.0, 0.0, 0.0, 0.0};
            MPI_Bcast(bcast_data, 4, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            max_iteraciones = (int)bcast_data[0];
            running = (int)bcast_data[3];

            if (running == 0) break;

            auto t_start = std::chrono::high_resolution_clock::now();
            ComputeResult res = newton_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer, max_iteraciones);
            auto t_end = std::chrono::high_resolution_clock::now();
            
            int compute_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
            int local_iters = res.total_iterations;

            // Reduce variables enviadas al maestro
            int dummy_max, dummy_sum;
            MPI_Reduce(&compute_ms, &dummy_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(&local_iters, &dummy_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

            int send_delta = row_end - row_start;
            MPI_Send(pixel_buffer, WIDTH * send_delta, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
