package ec.edu.uce.programacion.paralela;

import org.lwjgl.*;
import org.lwjgl.glfw.*;
import org.lwjgl.opengl.*;

import static org.lwjgl.glfw.Callbacks.*;
import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.system.MemoryUtil.*;

public class FractalMain {
    // Directorio DLL y directorio bin de MinGW para cargar bibliotecas nativas
    private static final String DLL_DIR = "native";
    private static final String MINGW_BIN_DIR = "D:/Tools/mingw64/bin";

    // The window handle
    private long window;
    private int textureId;
    private final int cpuCores;

    FractalCPU fractalCPU;
    FractalSimd fractalSimd;
    FPSCounter fpsCounter;

    int modo = 1; // 1: CPU, 2: SIMD
    private int lastLoggedMode = -1;
    private int lastLoggedIterations = -1;
    private boolean forceLog = true;

    public FractalMain() {
        cpuCores = Runtime.getRuntime().availableProcessors();
        fractalCPU = new FractalCPU();
        fractalSimd = new FractalSimd();
        fpsCounter = new FPSCounter();
    }

    public void run() {
        verifyDllLoad();
        System.out.println("Hello LWJGL " + Version.getVersion() + "!");
        System.out.println("Cores detectados: " + cpuCores);
        System.out.println("Iteraciones actuales: " + FractalParams.MAX_ITERACIONES);
        System.out.println("Modo actual: CPU (Threads)\n");

        init();
        loop();

        // Free the window callbacks and destroy the window
        glfwFreeCallbacks(window);
        glfwDestroyWindow(window);

        // Terminate GLFW and free the error callback
        glfwTerminate();
        glfwSetErrorCallback(null).free();
    }

    private void init() {
        GLFWErrorCallback.createPrint(System.err).set();

        if (!glfwInit())
            throw new IllegalStateException("Unable to initialize GLFW");

        // Configure GLFW
        glfwDefaultWindowHints(); // optional, the current window hints are already the default
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // the window will stay hidden after creation
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // the window will be resizable

        // Create the window
        String windowTitle = "Fractal Julia | Cores: " + cpuCores + " | 1=CPU, 2=SIMD";
        window = glfwCreateWindow(FractalParams.WIDTH, FractalParams.HEIGHT, windowTitle, NULL, NULL);
        if (window == NULL)
            throw new RuntimeException("Failed to create the GLFW window");

        // Setup a key callback. It will be called every time a key is pressed, repeated
        // or released.
        glfwSetKeyCallback(window, (window, key, scancode, action, mods) -> {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
                glfwSetWindowShouldClose(window, true); // We will detect this in the rendering loop
            if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
                FractalParams.MAX_ITERACIONES += 10;
                forceLog = true;
            }

            if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
                FractalParams.MAX_ITERACIONES -= 10;
                if (FractalParams.MAX_ITERACIONES < 0)
                    FractalParams.MAX_ITERACIONES = 10;
                forceLog = true;
            }

            if (key == GLFW_KEY_1 && action == GLFW_RELEASE) {
                System.out.println("Modo CPU (Threads: " + cpuCores + ")");
                modo = 1;
                forceLog = true;
            }

            if (key == GLFW_KEY_2 && action == GLFW_RELEASE) {
                System.out.println("Modo SIMD (DLL)");
                modo = 2;
                forceLog = true;
            }
        });

        GLFWVidMode vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowPos(window,
                (vidmode.width() - FractalParams.WIDTH) / 2,
                (vidmode.height() - FractalParams.HEIGHT) / 2);

        // Make the OpenGL context current
        glfwMakeContextCurrent(window);

        GL.createCapabilities();
        GL.createCapabilitiesWGL();

        // --VERSION DE OPENGL
        String version = GL11.glGetString(GL11.GL_VERSION);
        String vendor = GL11.glGetString(GL11.GL_VENDOR);
        String renderer = GL11.glGetString(GL11.GL_RENDERER);

        System.out.println("OpenGL version: " + version);
        System.out.println("OpenGL vendor: " + vendor);
        System.out.println("OpenGL renderer: " + renderer);

        // conf. proyecion
        GL11.glEnable(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, 1, -1, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_TEXTURE_2D);
        glLoadIdentity();

        // Enable v-sync
        glfwSwapInterval(1); // comentar sí se quiere probar sin v-sync (más FPS pero tearing)

        // Make the window visible
        glfwShowWindow(window);

        setupTexture();
    }

    private void verifyDllLoad() {
        String path = System.getenv("PATH");
        if (path != null && !path.contains(MINGW_BIN_DIR)) {
            System.setProperty("java.library.path", MINGW_BIN_DIR + ";" + path);
        }
        if (System.getProperty("jnr.ffi.library.path") == null) {
            java.nio.file.Path baseDir = java.nio.file.Paths.get(System.getProperty("user.dir"));
            java.nio.file.Path dllDir = baseDir.resolve(DLL_DIR);
            if (!java.nio.file.Files.exists(dllDir)) {
                dllDir = baseDir.resolve("app").resolve(DLL_DIR);
            }
            System.setProperty("jnr.ffi.library.path", dllDir.toAbsolutePath().toString());
        }
        try {
            System.out.println("DLL cargada OK: " + FractalDll.LIBRARY_NAME);
        } catch (Throwable t) {
            System.err.println("No se pudo cargar la DLL: " + FractalDll.LIBRARY_NAME);
            System.err.println("jnr.ffi.library.path=" + System.getProperty("jnr.ffi.library.path"));
            System.err.println("java.library.path=" + System.getProperty("java.library.path"));
            t.printStackTrace();
        }
    }

    private void setupTexture() {
        textureId = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA,
                FractalParams.WIDTH, FractalParams.HEIGHT, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    private void loop() {
        GL.createCapabilities();

        // Set the clear color
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            paint();

            glfwSwapBuffers(window);

            glfwPollEvents();
        }
    }

    private void paint() {
        boolean tick = fpsCounter.update();
        if (tick || forceLog || lastLoggedMode != modo || lastLoggedIterations != FractalParams.MAX_ITERACIONES) {
            lastLoggedMode = modo;
            lastLoggedIterations = FractalParams.MAX_ITERACIONES;
            forceLog = false;
            System.out.println("Estado | FPS: " + fpsCounter.getFps() +
                    " | Modo: " + getModeLabel() +
                    " | Iteraciones: " + FractalParams.MAX_ITERACIONES +
                    " | Cores: " + cpuCores);
        }

        if (modo == 1) {
            fractalCPU.julia_parallel_2(FractalParams.x_min, FractalParams.y_min,
                    FractalParams.x_max, FractalParams.y_max,
                    FractalParams.WIDTH, FractalParams.HEIGHT, cpuCores);
        } else if (modo == 2) {
            fractalSimd.juliaSimd();
        }

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureId);

        if (modo == 2) {
            glTexSubImage2D(
                    GL_TEXTURE_2D, 0, 0, 0,
                    FractalParams.WIDTH, FractalParams.HEIGHT,
                    GL_RGBA, GL_UNSIGNED_BYTE, fractalSimd.pixel_buffer);
        } else {
            glTexSubImage2D(
                    GL_TEXTURE_2D, 0, 0, 0,
                    FractalParams.WIDTH, FractalParams.HEIGHT,
                    GL_RGBA, GL_UNSIGNED_BYTE, FractalCPU.pixel_buffer);
        }

        glBegin(GL_QUADS);
        {
            glTexCoord2d(0.0f, 0.0f);
            glVertex2d(-1, -1);

            glTexCoord2d(0.0f, 1.0f);
            glVertex2d(-1, 1);

            glTexCoord2d(1.0f, 1.0f);
            glVertex2d(1, 1);

            glTexCoord2d(1.0f, 0.0f);
            glVertex2d(1, -1);
        }
        glEnd();
    }

    public native void julia_simd(double x_min, double y_min, double x_max, double y_max, int width, int height,
            int max_iteraciones, byte[] pixelBuffer);

    public static void main(String[] args) {
        new FractalMain().run();
    }

    private String getModeLabel() {
        return (modo == 2) ? "SIMD" : "CPU";
    }
}
