public class Main {

    public static void main(String[] args) {
        System.out.println("Starting JVM warm-up phase...");
        
        for (int i = 0; i < 5; i++) {
            runCpuHeavyTask();
        }
        
        System.out.println("Warm-up complete. Running benchmark...");
        
        long startTime = System.nanoTime();
        
        int result = runCpuHeavyTask();
        
        long endTime = System.nanoTime();
        
        double durationMs = (endTime - startTime) / 1_000_000.0;
        
        System.out.println("Computation Result (Primes found): " + result);
        System.out.printf("Time taken: %.2f ms%n", durationMs);
    }

    private static int runCpuHeavyTask() {
        int primeCount = 0;
        int limit = 100_000;

        for (int i = 2; i < limit; i++) {
            boolean isPrime = true;
            for (int j = 2; j * j <= i; j++) {
                if (i % j == 0) {
                    isPrime = false;
                    break;
                }
            }
            if (isPrime) {
                primeCount++;
            }
        }
        return primeCount;
    }
}