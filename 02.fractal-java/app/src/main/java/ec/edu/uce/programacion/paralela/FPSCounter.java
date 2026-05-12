package ec.edu.uce.programacion.paralela;

public class FPSCounter {
    private int fps;
    private int frames;
    private long lastTime;

    public FPSCounter() {
        fps = 0;
        frames = 0;
        lastTime = System.currentTimeMillis();
    }

    public boolean update() {
        frames++;
        long now = System.currentTimeMillis();

        if (now - lastTime >= 1000) {
            fps = frames;
            frames = 0;
            lastTime = now;
            return true;
        }

        return false;
    }

    public int getFps() {
        return fps;
    }
}