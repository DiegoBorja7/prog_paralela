#include <iostream>
#include <mpi.h>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstring>
#include <string>

#include "fractal_burning_ship_mpi.h"
#include "draw_text.h"

namespace arial_ttf {
    extern size_t data_len;
    extern unsigned char data[];
}

#ifdef _WIN32
#include <windows.h>
#endif

double x_min = -1.8;
double x_max = -1.7;
double y_min = -0.1;
double y_max = 0.05;
int max_iteraciones = 100;

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;
int running = 1;
int row_start, row_end, padding, delta, nproc, rank;

#define WIDTH 1600
#define HEIGHT 900

void setup_ui() {
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Fractal Burning Ship MPI - Prueba 2");

#ifdef _WIN32
    HWND hWnd = window.getNativeHandle();
    ShowWindow(hWnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);
    
    // Textos
    sf::Text textTop(font, "", 16);
    textTop.setFillColor(sf::Color::White);
    textTop.setPosition({10, 10});
    textTop.setStyle(sf::Text::Bold);

    sf::Text textHelp(font, "Options: Up/Down (Iteraciones) | ESC (cerrar)", 18);
    textHelp.setFillColor(sf::Color::White);
    textHelp.setStyle(sf::Text::Bold);
    textHelp.setPosition({10, (float)HEIGHT - 35});

    sf::Clock clock;
    int fps = 0, frames = 0;

    uint32_t global_hist[16] = {0};
    uint32_t* gathered_hist = new uint32_t[16 * nproc];

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

        // Bcast de arreglo {max_iter, x_min, x_max, y_min, y_max, running}
        double bcast_data[6] = { (double)max_iteraciones, x_min, x_max, y_min, y_max, (double)running };
        MPI_Bcast(bcast_data, 6, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        if (running == 0) break;

        // Cálculo Local
        uint32_t local_hist[16] = {0};
        burning_ship_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer, max_iteraciones, local_hist);

        // MPI_Gather para recolectar histogramas en rank 0
        MPI_Gather(local_hist, 16, MPI_UINT32_T, gathered_hist, 16, MPI_UINT32_T, 0, MPI_COMM_WORLD);

        // Sumar todos los histogramas locales en el global
        std::memset(global_hist, 0, 16 * sizeof(uint32_t));
        for (int p = 0; p < nproc; p++) {
            for (int i = 0; i < 16; i++) {
                global_hist[i] += gathered_hist[p * 16 + i];
            }
        }

        // Ensamblar Imagen
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

        // Construir string del histograma
        std::string hist_str = "Hist: ";
        for(int i=0; i<16; i++) {
            hist_str += fmt::format("[{}]:{} ", i, global_hist[i]);
            if (i == 7) hist_str += "\n      "; // Salto de linea para que no se salga de pantalla
        }

        textTop.setString(fmt::format("RANKS: {} | Iteraciones maximas: {} | Dominio: [{}, {}]x[{}, {}] | FPS: {}\n{}", 
                                      nproc, max_iteraciones, x_min, x_max, y_min, y_max, fps, hist_str));

        window.clear();
        window.draw(sprite);
        window.draw(textTop);
        window.draw(textHelp);
        window.display();
    }
    delete[] pixel_buffer;
    delete[] gathered_hist;
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
            double bcast_data[6];
            MPI_Bcast(bcast_data, 6, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            max_iteraciones = (int)bcast_data[0];
            x_min = bcast_data[1];
            x_max = bcast_data[2];
            y_min = bcast_data[3];
            y_max = bcast_data[4];
            running = (int)bcast_data[5];

            if (running == 0) break;

            uint32_t local_hist[16] = {0};
            burning_ship_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer, max_iteraciones, local_hist);

            MPI_Gather(local_hist, 16, MPI_UINT32_T, nullptr, 0, MPI_UINT32_T, 0, MPI_COMM_WORLD);

            // Enviar pixeles con Punto a Punto
            int send_delta = row_end - row_start;
            MPI_Send(pixel_buffer, WIDTH * send_delta, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
