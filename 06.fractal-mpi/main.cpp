#include <iostream>
#include <mpi.h>

#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <complex>

#include "fractal_mpi.h"
#include "draw_text.h"

namespace arial_ttf
{
    extern size_t data_len;
    extern unsigned char data[];
}

#ifdef _WIN32
#include <windows.h>
#endif

// paramtros del fractal julia
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

int max_iteraciones = 10;

std::complex<double> c(-0.7, 0.27015);

// buffer para almacenar los píxeles calculados por cada proceso
uint32_t *pixel_buffer = nullptr;

// buffer para almacenar la textura que se mostrará en la ventana
uint32_t *texture_buffer = nullptr;

int running = 1; // Variable para controlar la ejecución de los procesos secundarios

int row_start, row_end, padding, delta, nproc, rank;

// dimensiones de la imagen
#define WIDTH 1600
#define HEIGHT 900

std::string machine_name()
{
    std::string mname = "";
#ifdef _WIN32
    char hostname[256];
    DWORD size = sizeof(hostname);
    GetComputerNameA(hostname, &size);
    mname = hostname;
#endif
    return mname;
}

void dibujar_texto(int rank)
{
    auto texto = fmt::format("Rank: {}", rank);

    draw_text_to_texture(
        (unsigned char *)pixel_buffer, WIDTH, delta, texto.c_str(), 10, 25, 20);
}

void setup_ui()
{
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    // inicializar la ventana de SFML
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Fractal Julia");

#ifdef _WIN32
    HWND hWnd = window.getNativeHandle(); // Obtener el handle de la ventana SFML
    ShowWindow(hWnd, SW_MAXIMIZE);        // Maximizar la ventana
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    sf::Font font(arial_ttf::data, arial_ttf::data_len);
    sf::Text text(font, "Fractal MPI", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    // opciones
    std::string options = "Options: [1] Fractal MPI |  Up/Down: Change iterations";
    sf::Text textOptions(font, options, 24);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, (float)HEIGHT - 40});

    // fps
    sf::Clock clock;
    int fps = 0;
    int frames = 0;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
                running = 0; // Indicar a los procesos secundarios que la aplicación se está cerrando
            }
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
                default:
                    break;
                }
            }

            std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t)); // Limpiar el buffer de píxeles antes de cada renderizado
        }

        // Notificar a los otros procesos que la app se esta cerrando
        std::vector<int> dummy = {max_iteraciones, running};
        MPI_Bcast(dummy.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

        if (running == 0)
            break; // Salir del bucle si se recibe un valor negativo (indica cierre)

        // dibujar la porcion del RANK 0
        julia_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer);
        dibujar_texto(rank);

        // copiar el pixel buffer del proceso 0 al buffer de textura completo
        std::memcpy(texture_buffer, pixel_buffer, WIDTH * delta * sizeof(uint32_t));

        // Recibir las imagenes parciales de los otros RANKS
        for (int i = 1; i < nproc; i++)
        {
            int new_delta = delta; // 225
            if (i == nproc - 1)
                new_delta = delta - padding; // 225 - 100 = 125

            MPI_Status status;

            MPI_Recv(
                pixel_buffer,
                WIDTH * new_delta,
                MPI_UINT32_T,
                i, // RANK DE ORIGEN
                0, // TAG
                MPI_COMM_WORLD,
                &status);

            std::memcpy(texture_buffer + i * delta * WIDTH, pixel_buffer, WIDTH * new_delta * sizeof(uint32_t));
        }

        // crear la textura a partir del buffer de píxeles
        texture.update((const uint8_t *)texture_buffer);

        // contar FPS
        frames++;
        if (clock.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frames;
            frames = 0;
            clock.restart();
        }

        // actualizar el titulo de la ventana
        auto msg = fmt::format("Fractal MPI Iteraciones: {}, FPS: {}, Mode: MPI", max_iteraciones, fps);
        text.setString(msg);

        // Alinear a la derecha dinámicamente
        float text_width = text.getGlobalBounds().size.x;
        float posX = window.getView().getSize().x - text_width - 10.f;
        text.setPosition({posX, 10.f});

        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }
        window.display();
    }

    delete[] pixel_buffer;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    init_freetype();

    fmt::print("Proceso {} de {}\n", rank, nproc);

    delta = std::ceil(HEIGHT * 1.0 / nproc); // 1600 / 4 = 400

    /*
     * Cada proceso calcula un bloque de filas del fractal
     * r0: start = 0 * 400 = 0, end = 1 * 400 = 400
     * r1: start = 1 * 400 = 400, end = 2 * 400 = 800
     * r2: start = 2 * 400 = 800, end = 3 * 400 = 1200
     * r3: start = 3 * 400 = 1200, end = 4 * 400 = 1600
     * Si el número de procesos no divide exactamente la altura, el último proceso ajusta su bloque
     * para cubrir hasta el final de la imagen. Por ejemplo, si HEIGHT = 1600 y nproc = 3, entonces delta = 533.
     * El último proceso (r2) calculará desde la fila 1066 hasta la fila 1600, asegurando que toda la imagen se procese correctamente.
     */
    row_start = rank * delta;
    row_end = row_start + delta;
    padding = delta * nproc - HEIGHT; // 1600 - 1600 = 0

    if (row_end > HEIGHT)
        row_end = HEIGHT;

    pixel_buffer = new uint32_t[WIDTH * delta];
    std::memset(pixel_buffer, 0, WIDTH * delta * sizeof(uint32_t));

    fmt::print("Proceso {}: Calculando filas {} a {}\n", rank, row_start, row_end);

    if (rank == 0)
        setup_ui();
    else
    {
        // dibujar
        while (true)
        {
            // Esperar a que el proceso principal notifique el número de iteraciones a calcular
            std::vector<int> dummy = {max_iteraciones, 0};
            MPI_Bcast(dummy.data(), 2, MPI_INT, 0, MPI_COMM_WORLD);

            max_iteraciones = dummy[0];
            running = dummy[1];

            if (running == 0)
            {
                fmt::print("Proceso {}: Recibida señal de cierre. Terminando proceso.\n", rank);
                break; // Salir del bucle si se recibe un valor negativo (indica cierre)
            }

            julia_mpi(x_min, y_min, x_max, y_max, WIDTH, HEIGHT, row_start, row_end, pixel_buffer);

            // Dibujar el texto en el pixel_buffer antes de enviarlo al proceso principal, para evitar que sea sobrescrito
            // por las llamadas subsecuentes a MPI_Recv.
            dibujar_texto(rank);

            int send_delta = row_end - row_start;
            MPI_Send(pixel_buffer, WIDTH * send_delta, MPI_UINT32_T, 0, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}