public class Main {
    public static void main(String[] args) {
        System.out.println("Running fib(i=50)");

        int iterations = 50;
        long _n = 0;
        long n = 1;
        for (int i = 0; i < iterations; i++) {
            long tmp = n;
            n = n + _n;
            System.out.println("fib(" + i + "): " + n);
            _n = tmp;
        }
    }
}