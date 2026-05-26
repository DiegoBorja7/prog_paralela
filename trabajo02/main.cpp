#include <SFML/Graphics.hpp>
#include <fmt/core.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "grayscale.h"
#include "image_io.h"

namespace
{
enum class ViewMode
{
    Original = 0,
    SimdGray,
    OpenMpGray
};

void gray_to_rgba(const std::vector<uint8_t>& gray_pixels, std::vector<uint8_t>& rgba_pixels)
{
    const int total = static_cast<int>(gray_pixels.size());
    rgba_pixels.resize(total * 4);

    for (int i = 0; i < total; ++i)
    {
        const uint8_t g = gray_pixels[i];
        const int px = i * 4;
        rgba_pixels[px + 0] = g;
        rgba_pixels[px + 1] = g;
        rgba_pixels[px + 2] = g;
        rgba_pixels[px + 3] = 255;
    }
}

void print_instructions()
{
    fmt::print("===========================================\n");
    fmt::print(" Trabajo 02 - RGBA a Escala de Grises\n");
    fmt::print("===========================================\n");
    fmt::print("Tecla 1: Mostrar imagen original\n");
    fmt::print("Tecla 2: Aplicar filtro SIMD y mostrar\n");
    fmt::print("Tecla 3: Aplicar filtro OpenMP y mostrar\n");
    fmt::print("Tecla S: Guardar salida (simd/openmp)\n");
    fmt::print("ESC o cerrar ventana: salir\n");
    fmt::print("===========================================\n");
}
}

int main()
{
    print_instructions();

    int width = 0;
    int height = 0;
    std::vector<uint8_t> original_rgba;

    if (!load_image_rgba("img.jpg", original_rgba, width, height))
    {
        if (!load_image_rgba("img.png", original_rgba, width, height))
        {
            fmt::print("No se pudo cargar img.jpg ni img.png\n");
            return 1;
        }
    }

    const int total_pixels = width * height;
    std::vector<uint8_t> gray_simd(total_pixels, 0);
    std::vector<uint8_t> gray_openmp(total_pixels, 0);
    std::vector<uint8_t> view_rgba = original_rgba;

    bool simd_ready = false;
    bool openmp_ready = false;

    ViewMode mode = ViewMode::Original;
    std::string title = "Trabajo02 - 1 Original | 2 SIMD | 3 OpenMP | S Guardar";

    sf::RenderWindow window(sf::VideoMode({static_cast<unsigned>(width), static_cast<unsigned>(height)}), title);
    sf::Texture texture({static_cast<unsigned>(width), static_cast<unsigned>(height)});
    sf::Sprite sprite(texture);

    texture.update(view_rgba.data());

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                const bool is1 = (key->scancode == sf::Keyboard::Scancode::Num1) ||
                                 (key->scancode == sf::Keyboard::Scancode::Numpad1);
                const bool is2 = (key->scancode == sf::Keyboard::Scancode::Num2) ||
                                 (key->scancode == sf::Keyboard::Scancode::Numpad2);
                const bool is3 = (key->scancode == sf::Keyboard::Scancode::Num3) ||
                                 (key->scancode == sf::Keyboard::Scancode::Numpad3);
                const bool isSave = (key->scancode == sf::Keyboard::Scancode::S);

                if (key->scancode == sf::Keyboard::Scancode::Escape)
                {
                    window.close();
                }
                else if (is1)
                {
                    mode = ViewMode::Original;
                    texture.update(original_rgba.data());
                    window.setTitle("Modo ORIGINAL - Teclas: 1/2/3/S");
                    fmt::print("[Tecla 1] Modo Original\n");
                }
                else if (is2)
                {
                    auto t0 = std::chrono::high_resolution_clock::now();
                    rgba_to_gray_simd(original_rgba.data(), gray_simd.data(), width, height);
                    auto t1 = std::chrono::high_resolution_clock::now();

                    gray_to_rgba(gray_simd, view_rgba);
                    texture.update(view_rgba.data());

                    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
                    mode = ViewMode::SimdGray;
                    simd_ready = true;
                    window.setTitle("Modo SIMD - Teclas: 1/2/3/S");
                    fmt::print("[Tecla 2][SIMD] Filtro aplicado en {:.3f} ms\n", ms);
                }
                else if (is3)
                {
                    auto t0 = std::chrono::high_resolution_clock::now();
                    rgba_to_gray_openmp_manual(original_rgba.data(), gray_openmp.data(), width, height);
                    auto t1 = std::chrono::high_resolution_clock::now();

                    gray_to_rgba(gray_openmp, view_rgba);
                    texture.update(view_rgba.data());

                    const double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
                    mode = ViewMode::OpenMpGray;
                    openmp_ready = true;
                    window.setTitle("Modo OPENMP - Teclas: 1/2/3/S");
                    fmt::print("[Tecla 3][OpenMP] Filtro aplicado en {:.3f} ms\n", ms);
                }
                else if (isSave)
                {
                    if (mode == ViewMode::SimdGray && simd_ready)
                    {
                        if (write_image_gray_png("salida_simd.png", gray_simd.data(), width, height))
                        {
                            fmt::print("[Guardar] salida_simd.png OK\n");
                        }
                        else
                        {
                            fmt::print("[Guardar] Error al escribir salida_simd.png\n");
                        }
                    }
                    else if (mode == ViewMode::OpenMpGray && openmp_ready)
                    {
                        if (write_image_gray_png("salida_openmp.png", gray_openmp.data(), width, height))
                        {
                            fmt::print("[Guardar] salida_openmp.png OK\n");
                        }
                        else
                        {
                            fmt::print("[Guardar] Error al escribir salida_openmp.png\n");
                        }
                    }
                    else
                    {
                        fmt::print("[Guardar] Primero aplica SIMD (2) u OpenMP (3)\n");
                    }
                }
            }
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
