namespace ToneTest;

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;

internal class App {

    public const int C3 = 131, E3 = 165, GS3 = 208, A3 = 220;
    public const int C4 = 262, E4 = 330, G4 = 392, A4 = 440, B4 = 494;
    public const int C5 = 523, D5 = 587, DS5 = 622, E5 = 659, F5 = 698, G5 = 784, A5 = 880, B5 = 988;

    internal static void Main(string[] args) {
        if (args.Length == 0) { args = ["FurElise.tones"]; }

        var buzzer = new Buzzer();
        foreach (var file in args) {
            if (!File.Exists(file)) { throw new InvalidOperationException($"File not found ({file})"); }

            var chars = new List<char>();
            foreach (var ch in File.ReadAllText(file)) {
                if (!char.IsWhiteSpace(ch)) { chars.Add(ch); }
            }

            if (chars.Count % 3 != 0) { throw new InvalidDataException("Invalid tone count"); }
            for (var i = 0; i < chars.Count; i += 3) {
                var noteChar = chars[i + 0];
                var octaveChar = chars[i + 1];
                var durationChar = chars[i + 2];
                Console.Write($"{noteChar}{octaveChar}{durationChar} ");
                buzzer.PlayNote(noteChar, octaveChar, durationChar);
            }

            Thread.Sleep(1000);
        }
    }

}
