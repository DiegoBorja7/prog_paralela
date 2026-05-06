package ec.edu.uce.programacion.paralela;

public class FractalParams {
    public static final int MAX_ITERACIONES_DEFAULT = 10;
    public static int MAX_ITERACIONES = MAX_ITERACIONES_DEFAULT;

    public static final int WIDTH = 1600;
    public static final int HEIGHT = 900;

    public static final double x_min = -1.5;
    public static final double x_max = 1.5;

    public static final double y_min = -1.0;
    public static final double y_max = 1.0;

    public static final double c_real = -0.7;
    public static final double c_imaginaria = 0.27015;

    public static final int PALETTE_SIZE = 16;

    public static final int[] color_ramp = {
            0xF0F921FF,
            0xF8DD24FF,
            0xFDC12AFF,
            0xFBA836FF,
            0xF59044FF,
            0xED7953FF,
            0xE06561FF,
            0xD35170FF,
            0xC23D81FF,
            0xB02A90FF,
            0x9C179EFF,
            0x830AA4FF,
            0x6901A6FF,
            0x4F03A1FF,
            0x2F0595FF,
            0x0D0887FF
    };
}
