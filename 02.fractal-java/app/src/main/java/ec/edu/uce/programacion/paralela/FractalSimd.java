package ec.edu.uce.programacion.paralela;

import java.nio.ByteBuffer;

public class FractalSimd {
    ByteBuffer pixel_buffer;
    byte[] pixel_array;

    public FractalSimd() {
        pixel_array = new byte[FractalParams.WIDTH * FractalParams.HEIGHT * 4];
        pixel_buffer = ByteBuffer.allocateDirect(FractalParams.WIDTH * FractalParams.HEIGHT * 4);
    }

    public void juliaSimd() {
        FractalDll.INSTANCE.julia_simd(
                FractalParams.x_min, FractalParams.y_min, FractalParams.x_max, FractalParams.y_max,
                FractalParams.WIDTH, FractalParams.HEIGHT, FractalParams.MAX_ITERACIONES, pixel_array);
        int rowStride = FractalParams.WIDTH * 4;
        byte[] tempRow = new byte[rowStride];
        for (int top = 0, bottom = FractalParams.HEIGHT - 1; top < bottom; top++, bottom--) {
            int topOffset = top * rowStride;
            int bottomOffset = bottom * rowStride;
            System.arraycopy(pixel_array, topOffset, tempRow, 0, rowStride);
            System.arraycopy(pixel_array, bottomOffset, pixel_array, topOffset, rowStride);
            System.arraycopy(tempRow, 0, pixel_array, bottomOffset, rowStride);
        }
        pixel_buffer.clear();
        pixel_buffer.put(pixel_array);
        pixel_buffer.flip();
    }

}
