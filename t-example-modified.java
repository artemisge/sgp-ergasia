mainclass Example {
    public static void main() {
        int c;
        float x, sum, mo;
        c = 0;
        x = 35/10;
        sum = 0/10;
        while (c < 5) {
            sum = sum + x;
            c = c + 1;
            x = x + 15/10;
        }
        mo = sum / 5;
        println(mo);
    }
}
