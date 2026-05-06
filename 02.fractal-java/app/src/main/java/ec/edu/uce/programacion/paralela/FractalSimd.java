package ec.edu.uce.programacion.paralela;

import java.nio.ByteBuffer;

public class FractalSimd {
    ByteBuffer pixel_buffer;

    public FractalSimd() {
        pixel_buffer = ByteBuffer.allocate(FractalParams.WIDTH * FractalParams.HEIGHT * 4);
    }

    public void juliaSimd() {
        FractalDll.INSTANCE.julia_simd(
                FractalParams.x_min, FractalParams.y_min, FractalParams.x_max, FractalParams.y_max,
                FractalParams.WIDTH, FractalParams.HEIGHT, FractalParams.MAX_ITERACIONES, pixel_buffer.array());
    }

}
