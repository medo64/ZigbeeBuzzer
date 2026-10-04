namespace ToneTest;

using System;

internal class App {

    public const int C3 = 131, E3 = 165, GS3 = 208, A3 = 220;
    public const int C4 = 262, E4 = 330, G4 = 392, A4 = 440, B4 = 494;
    public const int C5 = 523, D5 = 587, DS5 = 622, E5 = 659, F5 = 698, G5 = 784, A5 = 880, B5 = 988;

    internal static void Main(string[] args) {
        using var buzzer = new Buzzer();

        // buzzer.PlayNote("D5", 60);
        // buzzer.PlayNote("F5", 60);
        // buzzer.PlayNote("A5", 90);
        // buzzer.PlayNote("D6", 140);

        buzzer.PlayNote("E5", 250);
        buzzer.PlayNote("d5", 250);
        buzzer.PlayNote("E5", 250);
        buzzer.PlayNote("d5", 250);
        buzzer.PlayNote("E5", 250);
        buzzer.PlayNote("B4", 250);
        buzzer.PlayNote("D5", 250);
        buzzer.PlayNote("C5", 250);
        buzzer.PlayNote("A4", 500);

        buzzer.PlayNote("C4", 250);
        buzzer.PlayNote("E4", 250);
        buzzer.PlayNote("A4", 250);
        buzzer.PlayNote("B4", 500);

        buzzer.PlayNote("E4", 250);
        buzzer.PlayNote("C5", 250);
        buzzer.PlayNote("B4", 250);
        buzzer.PlayNote("A4", 500);

        buzzer.PlayNote("E4", 250);
        buzzer.PlayNote("E5", 250);
        buzzer.PlayNote("d5", 250);
        buzzer.PlayNote("E5", 250);
        buzzer.PlayNote("d5", 250);
        buzzer.PlayNote("E5", 250);
        buzzer.PlayNote("B4", 250);
        buzzer.PlayNote("D5", 250);
        buzzer.PlayNote("C5", 250);
        buzzer.PlayNote("A4", 500);

        buzzer.PlayNote("C4", 250);
        buzzer.PlayNote("E4", 250);
        buzzer.PlayNote("A4", 250);
        buzzer.PlayNote("B4", 500);

        buzzer.PlayNote("E4", 250);
        buzzer.PlayNote("C5", 250);
        buzzer.PlayNote("B4", 250);
        buzzer.PlayNote("A4", 500);
    }

}
