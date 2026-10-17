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
        if (args.Length == 0) { args = ["tones/21.tones"]; }

        var buzzer = new Buzzer();

        var isFirst = true;
        foreach (var fileName in args) {
            var file = new FileInfo(fileName);
            if (!file.Exists) { throw new InvalidOperationException($"File not found ({file.FullName})"); }
            if (isFirst) { isFirst = false; } else { Thread.Sleep(1000); }

            char[] chars;
            if (file.Extension.Equals(".melody", StringComparison.InvariantCultureIgnoreCase)) {
                chars = GetCharsFromMelody(file);
            } else {
                chars = GetCharsFromNotes(file);
            }

            Console.ForegroundColor = ConsoleColor.Cyan;
            Console.WriteLine([.. chars]);
            Console.ResetColor();

            for (var i = 0; i < chars.Length; i += 3) {
                var noteChar = chars[i + 0];
                var octaveChar = chars[i + 1];
                var durationChar = chars[i + 2];
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.Write($"{noteChar}{octaveChar}{durationChar} ");
                Console.ResetColor();
                buzzer.PlayNote(noteChar, octaveChar, durationChar);
            }
        }
    }

    internal static char[] GetCharsFromNotes(FileInfo file) {
        var lines = new List<string>();
        foreach (var line in File.ReadAllLines(file.FullName)) {
            if (!line.StartsWith('#')) { lines.Add(line); }
        }

        var chars = new List<char>();
        foreach (var ch in string.Join('\n', lines.ToArray())) {
            if (!char.IsWhiteSpace(ch)) { chars.Add(ch); }
        }

        if (chars.Count % 3 != 0) { throw new InvalidDataException("Invalid tone count"); }
        return chars.ToArray();
    }

    internal static char[] GetCharsFromMelody(FileInfo file) {
        var lines = new List<string>();
        foreach (var line in File.ReadAllLines(file.FullName)) {
            if (!line.StartsWith('#')) { lines.Add(line); }
        }
        var text = string.Join(" ", lines);

        var indexOfMelody = text.IndexOf("melody", 0);
        var indexOfMelodyStart = text.IndexOf("{", indexOfMelody);
        var indexOfMelodyEnd = text.IndexOf("}", indexOfMelodyStart);

        var indexOfDurations = text.IndexOf("durations", 0);
        var indexOfDurationsStart = text.IndexOf("{", indexOfDurations);
        var indexOfDurationsEnd = text.IndexOf("}", indexOfDurationsStart);

        if (indexOfMelodyEnd == -1) { throw new InvalidDataException("Cannot find melody"); }
        if (indexOfDurationsEnd == -1) { throw new InvalidDataException("Cannot find durations"); }
        if ((indexOfMelodyStart == indexOfDurationsStart) || (indexOfMelodyEnd == indexOfDurationsEnd)) { throw new InvalidDataException("Overlapping melody and durations"); }

        var melodyText = text[(indexOfMelodyStart + 1)..indexOfMelodyEnd];
        var durationsText = text[(indexOfDurationsStart + 1)..indexOfDurationsEnd];
        var melody = melodyText.Split(',', StringSplitOptions.TrimEntries);
        var durations = durationsText.Split(',', StringSplitOptions.TrimEntries);
        if (melody.Length != durations.Length) { throw new InvalidDataException($"Mismatch melody/duration count ({melody.Length}/{durations.Length})"); }

        var chars = new List<char>();
        for (var i = 0; i < melody.Length; i++) {
            var note = melody[i].Replace("NOTE_", "");
            if (note[1] == 'S') {note = note[0..1].ToLower() + note[2]; }
            chars.AddRange([.. note]);

            var duration = 1000 / int.Parse(durations[i]);
            var duration100 = duration / 10;
            while (duration100 > 0) {
                if (duration100 < 10) {
                    chars.Add((char)('0' + duration100));
                    duration100 = 0;
                } else if (duration100 < 36) {
                    chars.Add((char)('A' + duration100 - 10));
                    duration100 = 0;
                } else if (duration100 < 62) {
                    chars.Add((char)('a' + duration100 - 10 - 26));
                    duration100 = 0;
                } else {
                    chars.Add('z');
                    duration100 -= 62;
                    chars.AddRange([.. note]);
                }
            }

            chars.AddRange("00");
            var durationSilent = (int)(duration * 0.30);
            var durationSilent100 = durationSilent / 10;
            if (durationSilent100 == 0) { durationSilent100 = 1; }
                if (durationSilent100 < 10) {
                    chars.Add((char)('0' + durationSilent100));
                } else if (durationSilent100 < 36) {
                    chars.Add((char)('A' + durationSilent100 - 10));
                } else if (durationSilent100 < 62) {
                    chars.Add((char)('a' + durationSilent100 - 10 - 26));
                } else {
                    throw new InvalidDataException($"Too much delay ({durationSilent100} for {duration} ms duration)");
                }
        }

        if (chars.Count % 3 != 0) { throw new InvalidDataException("Invalid tone count"); }
        return chars.ToArray();
    }
}
