import java.util.ArrayList;
import java.util.List;

public class Main {
    static {
        System.loadLibrary("gctester");
    }
    public static native void GCTESTER_logMemUsage();
    public static void main(String[] args) {
        System.out.println("Running GC Test...");
        List<byte[]> memoryBurner = new ArrayList<>();
        try {
            // allocate 4GB over time
            for (int i = 0; i < 4096; i++) {
                byte[] data = new byte[1024 * 1024];
                memoryBurner.add(data);

                if (memoryBurner.size() % 128 == 0) {
                    System.out.println("Allocated: " + memoryBurner.size() + " MB");
                    memoryBurner.clear();
                    Thread.sleep(50);
                    //GCTESTER_logMemUsage();
                }
                
                Thread.sleep(5);
            }
        } catch (InterruptedException e) {
            System.err.println("Test interrupted.");
        }
    }
}