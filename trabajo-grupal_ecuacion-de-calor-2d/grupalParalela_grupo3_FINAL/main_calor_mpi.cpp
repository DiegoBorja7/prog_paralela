#include <iostream>
#include <fmt/core.h>
#include <vector>
#include <cmath>
#include <mpi.h>
#include <SFML/Graphics.hpp>
#include "draw_text.h"
#include "calor_serial.h"
#include "calor_mpi.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define WIDTH 1600
#define HEIGHT 900

namespace arial_ttf
{
    extern size_t data_len;
    extern unsigned char data[];
}

int nx = 128;
int ny = 128;
double lx = 1.0;
double ly = 1.0;
double alpha = 0.25;
double dt = 5.0e-7;
int max_iters = 100000;
double tol = 1.0e-4;

int frames = 0;
uint32_t *global_pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;
int running = 1;

int nprocs, rank;
int delta;
int row_start;
int row_end;
int padding;
int local_ny;
int pixel_delta;

void dibujar_texto(int rank_id, uint32_t* buffer, int height_delta)
{
    char host_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    
    MPI_Get_processor_name(host_name, &name_len);

    auto texto = fmt::format("RANK_{} ({})", rank_id, host_name);

    draw_text_to_texture(
        (unsigned char *)buffer,
        WIDTH, height_delta,
        texto.c_str(),
        10, 25, 20); 
}

void setup_ui()
{
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Ecuacion de Calor 2D - MPI");

#ifdef _WIN32
    HWND hwnh = window.getNativeHandle();
    ShowWindow(hwnh, SW_MAXIMIZE);
#endif
    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    const sf::Font font(arial_ttf::data, arial_ttf::data_len);
    sf::Text text(font, "Calor 2D MPI", 15);
    text.setFillColor(sf::Color::Black);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "Options: P: Pause/Resume | R: Reset";
    sf::Text textOptions(font, options, 14);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    int fps = 0;
    sf::Clock clock;

    int iter_actual = 0;
    double residuo_L2 = 0.0;
    int paused = 0;
    int do_reset = 0;

    std::vector<double> u_old((local_ny + 2) * nx, 0.0);
    std::vector<double> u_new((local_ny + 2) * nx, 0.0);
    
    if (rank == 0) {
        for (int i = 0; i < nx; i++) {
            u_old[1 * nx + i] = 100.0;
            u_new[1 * nx + i] = 100.0;
        }
    }
    int draw_height_rank0 = (0 == nprocs - 1) ? (pixel_delta - padding) : pixel_delta;
    uint32_t* local_pixel_buffer = new uint32_t[WIDTH * draw_height_rank0];

    while (window.isOpen())
    {
        do_reset = 0;
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                running = 0;
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();
                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::P:
                    paused = !paused;
                    break;
                case sf::Keyboard::Scan::R:
                    do_reset = 1;
                    iter_actual = 0;
                    residuo_L2 = 0.0;
                    paused = 0;
                    break;
                default:
                    break;
                }
            }
        }

        std::vector<int> dummy = {running, paused, do_reset};
        MPI_Bcast(dummy.data(), 3, MPI_INT, 0, MPI_COMM_WORLD);

        if (running == 0) break;
        
        if (do_reset) {
            std::fill(u_old.begin(), u_old.end(), 0.0);
            std::fill(u_new.begin(), u_new.end(), 0.0);
            if (rank == 0) {
                for (int i = 0; i < nx; i++) {
                    u_old[1 * nx + i] = 100.0;
                    u_new[1 * nx + i] = 100.0;
                }
            }
        }

        double mflops = 0.0;

        if (!paused && iter_actual < max_iters && (iter_actual == 0 || residuo_L2 > tol))
        {
            sf::Clock step_clock;
            int iters_por_frame = 50;
            
            for (int k = 0; k < iters_por_frame && iter_actual < max_iters; k++) {
                if (rank > 0) {
                    MPI_Sendrecv(&u_old[1 * nx], nx, MPI_DOUBLE, rank - 1, 0,
                                 &u_old[0 * nx], nx, MPI_DOUBLE, rank - 1, 1,
                                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                if (rank < nprocs - 1) {
                    MPI_Sendrecv(&u_old[local_ny * nx], nx, MPI_DOUBLE, rank + 1, 1,
                                 &u_old[(local_ny + 1) * nx], nx, MPI_DOUBLE, rank + 1, 0,
                                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }

                bool is_top = (rank == 0);
                bool is_bottom = (rank == nprocs - 1);

                double dif_cuad_local = calor_mpi_step(u_old, u_new, nx, local_ny, ny, lx, ly, alpha, dt, is_top, is_bottom);
                
                double dif_cuad_global = 0.0;
                MPI_Allreduce(&dif_cuad_local, &dif_cuad_global, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
                
                residuo_L2 = std::sqrt(dif_cuad_global / (nx * ny));
                u_old = u_new;
                iter_actual++;
                
                if (residuo_L2 <= tol) break;
            }
            
            float step_time = step_clock.getElapsedTime().asSeconds();
            double ops = (double)(nx - 2) * (ny - 2) * 7.0 * iters_por_frame;
            if (step_time > 0) mflops = (ops / step_time) / 1e6;
        }

        std::vector<double> u_render(nx * local_ny);
        for(int j=0; j<local_ny; j++) {
            for(int i=0; i<nx; i++) {
                u_render[j*nx + i] = u_old[(j+1)*nx + i];
            }
        }
        
        int draw_height = (rank == nprocs - 1) ? (pixel_delta - padding) : pixel_delta;
        acotado(u_render, local_pixel_buffer, nx, local_ny, WIDTH, draw_height);
        dibujar_texto(0, local_pixel_buffer, draw_height);
        
        std::memcpy(texture_buffer, local_pixel_buffer, WIDTH * draw_height * sizeof(uint32_t));

        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = pixel_delta;
            if (i == nprocs - 1) new_delta = pixel_delta - padding;

            MPI_Recv(local_pixel_buffer, WIDTH * new_delta, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::memcpy(texture_buffer + (i * pixel_delta * WIDTH), local_pixel_buffer, WIDTH * new_delta * sizeof(uint32_t));
        }

        texture.update((const uint8_t *)texture_buffer);
        frames++;

        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        auto msg = fmt::format("Calor 2D | Iter: {}/{} | L2: {:.6e} | Tol: {} | FPS: {} | Backend: MPI | {:.2f} MFLOPS", 
                               iter_actual, max_iters, residuo_L2, tol, fps, mflops);
        text.setString(msg);

        window.clear();
        window.draw(sprite);
        window.draw(text);
        window.draw(textOptions);
        window.display();
    }

    delete[] local_pixel_buffer;
    delete[] texture_buffer;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    init_freetype();

    delta = std::ceil(ny * 1.0 / nprocs);
    row_start = rank * delta;
    row_end = row_start + delta;
    
    if (row_end > ny) row_end = ny;
    local_ny = row_end - row_start;
    
    pixel_delta = std::ceil(HEIGHT * 1.0 / nprocs);
    padding = (pixel_delta * nprocs) - HEIGHT;

    if (rank == 0)
    {
        setup_ui();
    }
    else
    {
        int paused = 0;
        int do_reset = 0;
        int iter_actual = 0;
        double residuo_L2 = 0.0;

        std::vector<double> u_old((local_ny + 2) * nx, 0.0);
        std::vector<double> u_new((local_ny + 2) * nx, 0.0);
        
        if (rank == nprocs - 1) { 

        }


        int draw_height = (rank == nprocs - 1) ? (pixel_delta - padding) : pixel_delta;
        uint32_t* local_pixel_buffer = new uint32_t[WIDTH * draw_height];

        while (true)
        {
            std::vector<int> dummy = {running, paused, do_reset};
            MPI_Bcast(dummy.data(), 3, MPI_INT, 0, MPI_COMM_WORLD);

            running = dummy[0];
            paused = dummy[1];
            do_reset = dummy[2];

            if (running == 0) break;
            
            if (do_reset) {
                std::fill(u_old.begin(), u_old.end(), 0.0);
                std::fill(u_new.begin(), u_new.end(), 0.0);
                iter_actual = 0;
                residuo_L2 = 0.0;
            }

            if (!paused && iter_actual < max_iters && (iter_actual == 0 || residuo_L2 > tol))
            {
                int iters_por_frame = 50;
                for (int k = 0; k < iters_por_frame && iter_actual < max_iters; k++) {
                    if (rank > 0) {
                        MPI_Sendrecv(&u_old[1 * nx], nx, MPI_DOUBLE, rank - 1, 0,
                                     &u_old[0 * nx], nx, MPI_DOUBLE, rank - 1, 1,
                                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }
                    if (rank < nprocs - 1) {
                        MPI_Sendrecv(&u_old[local_ny * nx], nx, MPI_DOUBLE, rank + 1, 1,
                                     &u_old[(local_ny + 1) * nx], nx, MPI_DOUBLE, rank + 1, 0,
                                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    }

                    bool is_top = (rank == 0);
                    bool is_bottom = (rank == nprocs - 1);

                    double dif_cuad_local = calor_mpi_step(u_old, u_new, nx, local_ny, ny, lx, ly, alpha, dt, is_top, is_bottom);
                    
                    double dif_cuad_global = 0.0;
                    MPI_Allreduce(&dif_cuad_local, &dif_cuad_global, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
                    
                    residuo_L2 = std::sqrt(dif_cuad_global / (nx * ny));
                    u_old = u_new;
                    iter_actual++;
                    
                    if (residuo_L2 <= tol) break;
                }
            }

            std::vector<double> u_render(nx * local_ny);
            for(int j=0; j<local_ny; j++) {
                for(int i=0; i<nx; i++) {
                    u_render[j*nx + i] = u_old[(j+1)*nx + i];
                }
            }
            
            acotado(u_render, local_pixel_buffer, nx, local_ny, WIDTH, draw_height);
            dibujar_texto(rank, local_pixel_buffer, draw_height);
            
            MPI_Send(local_pixel_buffer, WIDTH * draw_height, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
        }

        delete[] local_pixel_buffer;
    }

    MPI_Finalize();
    return 0;
}
