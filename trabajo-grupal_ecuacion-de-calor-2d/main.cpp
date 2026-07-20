#include <iostream>
#include <mpi.h>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <vector>
#include <chrono>
#include "ecuacion_calor.h"
#include "palette.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include "arial_ttf.h"

enum BackendType {
    SERIAL = 1,
    SIMD = 2,
    OPENMP = 3,
    MPI_BACKEND = 4
};

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    ParametrosCalor params;
    std::vector<double> u_old(params.nx * params.ny);
    std::vector<double> u_new(params.nx * params.ny);

    // TODOS los rangos deben inicializar la grilla para tener sus memorias sincronizadas desde el inicio
    inicializar_grilla(params, u_old);
    inicializar_grilla(params, u_new);

    sf::RenderWindow* window = nullptr;
    sf::Texture* texture = nullptr;
    sf::Sprite* sprite = nullptr;
    sf::Font* font = nullptr;
    sf::Text* textInfo = nullptr;
    sf::Text* textMenu = nullptr;
    uint32_t* pixel_buffer = nullptr;

    if (rank == 0) {
        window = new sf::RenderWindow(sf::VideoMode({1600, 900}), "Ecuacion de Calor 2D");
        pixel_buffer = new uint32_t[params.nx * params.ny];
        texture = new sf::Texture();
        texture->resize({(unsigned int)params.nx, (unsigned int)params.ny});
        sprite = new sf::Sprite(*texture);
        sprite->setScale({900.0f / params.nx, 900.0f / params.ny});
        sprite->setPosition({400.0f, 0.0f}); 

        font = new sf::Font(arial_ttf::data, arial_ttf::data_len);
        
        textInfo = new sf::Text(*font, "", 20);
        textInfo->setFillColor(sf::Color::White);
        textInfo->setPosition({10, 10});

        textMenu = new sf::Text(*font, "1: Serial | 2: SIMD | 3: OpenMP | 4: MPI\nEspacio: Pausa/Continuar | R: Reiniciar", 18);
        textMenu->setFillColor(sf::Color::Yellow);
        textMenu->setPosition({10, 900 - 80});
    }

    int current_backend = SERIAL;
    int is_paused = 1; // Empieza pausado por UX
    int running = 1;
    int reset_sim = 0;
    int iter = 0;
    double residuo = 0.0;
    double mflops = 0.0;

    while (running) {
        if (rank == 0) {
            reset_sim = 0;
            while (const std::optional event = window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    running = 0;
                } else if (event->is<sf::Event::KeyReleased>()) {
                    auto key = event->getIf<sf::Event::KeyReleased>();
                    if (key->scancode == sf::Keyboard::Scan::Space) is_paused = !is_paused;
                    else if (key->scancode == sf::Keyboard::Scan::R) reset_sim = 1;
                    else if (key->scancode == sf::Keyboard::Scan::Num1) current_backend = SERIAL;
                    else if (key->scancode == sf::Keyboard::Scan::Num2) current_backend = SIMD;
                    else if (key->scancode == sf::Keyboard::Scan::Num3) current_backend = OPENMP;
                    else if (key->scancode == sf::Keyboard::Scan::Num4) current_backend = MPI_BACKEND;
                }
            }
        }

        // Difundir estado de Rank 0 a todos los demas procesos
        int bcast_data[4] = {running, is_paused, current_backend, reset_sim};
        MPI_Bcast(bcast_data, 4, MPI_INT, 0, MPI_COMM_WORLD);
        running = bcast_data[0];
        is_paused = bcast_data[1];
        current_backend = bcast_data[2];
        reset_sim = bcast_data[3];

        if (!running) break;

        if (reset_sim) {
            inicializar_grilla(params, u_old);
            inicializar_grilla(params, u_new);
            iter = 0;
            residuo = 0.0;
        }

        if (!is_paused) {
            if ((iter < params.max_iter && residuo >= params.tol) || iter == 0) {
                auto start_time = std::chrono::high_resolution_clock::now();
                
                if (current_backend == MPI_BACKEND) {
                    residuo = calcular_calor_mpi(params, u_old, u_new, rank, nproc);
                } else {
                    // Si no es MPI, cada proceso simula redundantemente para mantener sus memorias sincronizadas
                    if (current_backend == SERIAL) {
                        residuo = calcular_calor_serial(params, u_old, u_new);
                    } else if (current_backend == SIMD) {
                        residuo = calcular_calor_simd(params, u_old, u_new);
                    } else if (current_backend == OPENMP) {
                        residuo = calcular_calor_omp(params, u_old, u_new);
                    }
                }

                auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> diff = end_time - start_time;
                
                double flops = 7.0 * (params.nx - 2) * (params.ny - 2);
                mflops = (flops / diff.count()) / 1e6;

                std::swap(u_old, u_new);
                iter++;
            }
        }

        if (rank == 0) {
            for (int i = 0; i < params.nx * params.ny; i++) {
                double temp = u_old[i];
                int color_idx = (int)((temp / 100.0) * 15.0);
                if (color_idx < 0) color_idx = 0;
                if (color_idx > 15) color_idx = 15;
                pixel_buffer[i] = color_ramp[color_idx];
            }

            texture->update((const uint8_t*)pixel_buffer);

            std::string backend_name = "";
            if (current_backend == SERIAL) backend_name = "Serial";
            if (current_backend == SIMD) backend_name = "SIMD (Intrinsecas)";
            if (current_backend == OPENMP) backend_name = "OpenMP";
            if (current_backend == MPI_BACKEND) backend_name = "MPI";

            textInfo->setString(fmt::format(
                "Backend: {}\n"
                "Estado: {}\n"
                "Iteracion: {} / {}\n"
                "Residuo L2: {:.6f}\n"
                "Tol: {:.6f}\n"
                "MFLOPS: {:.2f}",
                backend_name,
                is_paused ? "PAUSADO" : "CORRIENDO",
                iter, params.max_iter,
                residuo, params.tol,
                mflops
            ));

            window->clear(sf::Color(30, 30, 30));
            window->draw(*sprite);
            window->draw(*textInfo);
            window->draw(*textMenu);
            window->display();
        }
    }

    if (rank == 0) {
        delete window;
        delete texture;
        delete sprite;
        delete font;
        delete textInfo;
        delete textMenu;
        delete[] pixel_buffer;
    }

    MPI_Finalize();
    return 0;
}
