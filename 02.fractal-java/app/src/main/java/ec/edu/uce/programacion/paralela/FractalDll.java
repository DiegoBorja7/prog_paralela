package ec.edu.uce.programacion.paralela;

import jnr.ffi.LibraryLoader;

public interface FractalDll {
    String LIBRARY_NAME = "libfractal-dll";
    FractalDll INSTANCE = LibraryLoader.create(FractalDll.class)
                          .load(LIBRARY_NAME);
                          
    void julia_simd(double x_min, double y_min, double x_max, double y_max, int width, int height, int max_iteraciones, byte[] pixelBuffer);

}
