mainclass timestable {
    public static void main() {
        int i, j;

        for (i = 1; i <= 10; i = i+1) {
            for (j = 1; j <= 10; j = j+1)
                println(i*j);
        }
    }
}