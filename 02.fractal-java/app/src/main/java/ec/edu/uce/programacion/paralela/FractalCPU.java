package ec.edu.uce.programacion.paralela;

public class FractalCPU {
    public static int[] pixel_buffer;

    public FractalCPU() {
        pixel_buffer = new int[FractalParams.WIDTH * FractalParams.HEIGHT];
    }

    int acotado_2(double x, double y) {
        int iteraciones = 1;

        double zr = x;
        double zi = y;

        while ((zr * zr + zi * zi) < 4.0 && iteraciones < FractalParams.MAX_ITERACIONES) {
            // Zn+1 = Zn^2 + c

            double dr = zr * zr - zi * zi + FractalParams.c_real;
            double di = 2 * zr * zi + FractalParams.c_imaginaria;

            zr = dr;
            zi = di;

            iteraciones++;
        }

        if (iteraciones < FractalParams.MAX_ITERACIONES) {
            int index = iteraciones % FractalParams.PALETTE_SIZE;
            return FractalParams.color_ramp[index]; // | 0xFF000000; // Agregar canal

        }
        return 0XFF000000; // Negro para los puntos que pertenecen al conjunto de Julia
    }

    void julia_serial_2(double x_min, double y_min, double x_max, double y_max, int width, int height) {
        double dx = (x_max - x_min) / width;
        double dy = (y_max - y_min) / height;

        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                double x = x_min + i * dx;
                double y = y_min + j * dy;

                int color = acotado_2(x, y);

                pixel_buffer[j * width + i] = color;
            }
        }

    }
}
