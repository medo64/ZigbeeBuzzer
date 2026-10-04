namespace ToneTest;

using System;
using System.Runtime.InteropServices;
using System.Threading;

internal class Buzzer : IDisposable {

    public Buzzer() {
        var err = snd_pcm_open(out PcmHandle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_MODE_BLOCKING);
        if (err < 0) { throw new InvalidOperationException($"Unable to open ALSA audio device (error {err})"); }

        err = snd_pcm_set_params(PcmHandle, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SAMPLE_RATE, 1, 100000);
        if (err < 0) { throw new InvalidOperationException($"Failed to configure ALSA parameters (error {err})"); }

    }

    private readonly IntPtr PcmHandle;

    public void PlayNote(int frequency, int durationMs) {
        Console.WriteLine($"[{frequency}/{durationMs}]");

        var sampleCount = SAMPLE_RATE * durationMs / 1000;
        var buffer = new byte[sampleCount];

        var periodSamples = frequency == 0 ? 1 : SAMPLE_RATE / frequency;
        var periodHalf = periodSamples / 2;

        for (int i = 0; i < sampleCount; i++) {
            buffer[i] = (byte)((i % periodSamples < periodHalf) ? 0 : 63);  // 1/4 volume
        }

        var pinnedBuffer = GCHandle.Alloc(buffer, GCHandleType.Pinned);
        try {
            var samplesWritten = 0;
            while (samplesWritten < sampleCount) {
                var bufferAddress = IntPtr.Add(pinnedBuffer.AddrOfPinnedObject(), samplesWritten);
                var framesWritten = snd_pcm_writei(PcmHandle, bufferAddress, (nuint)(sampleCount - samplesWritten));

                if (framesWritten < 0) {
                    var err = snd_pcm_recover(PcmHandle, checked((int)framesWritten), 0);
                    if (err < 0) { throw new InvalidOperationException($"Failed to recover playback (error {err})"); }
                } else if (framesWritten > 0) {
                    samplesWritten += (int)framesWritten;
                } else {
                    throw new InvalidOperationException("Stalled playback");
                }
            }
        } finally {
            pinnedBuffer.Free();
        }

        //Thread.Sleep(10);
    }

    public void PlayNote(string noteName, int durationMs) {
        if (noteName.Length != 2) { throw new InvalidOperationException($"Invalid note name ({noteName}).");}
        var noteChar = noteName[0];
        var octaveChar = noteName[1];
        PlayNote(noteChar, octaveChar, durationMs);
    }

    public void PlayNote(char noteChar, char octaveChar, int durationMs) {
        var octave = (octaveChar is >= '0' and <= '9') ? octaveChar - '0' : throw new InvalidOperationException($"Unknown octave ({octaveChar}).");

        switch (noteChar) {
            case 'C': PlayNote(Frequencies[octave * 12 + 0], durationMs); break;   // C
            case 'c': PlayNote(Frequencies[octave * 12 + 1], durationMs); break;   // C#
            case 'D': PlayNote(Frequencies[octave * 12 + 2], durationMs); break;   // D
            case 'd': PlayNote(Frequencies[octave * 12 + 3], durationMs); break;   // D#
            case 'E': PlayNote(Frequencies[octave * 12 + 4], durationMs); break;   // E
            case 'F': PlayNote(Frequencies[octave * 12 + 5], durationMs); break;   // F
            case 'f': PlayNote(Frequencies[octave * 12 + 6], durationMs); break;   // F#
            case 'G': PlayNote(Frequencies[octave * 12 + 7], durationMs); break;   // G
            case 'g': PlayNote(Frequencies[octave * 12 + 8], durationMs); break;   // G#
            case 'A': PlayNote(Frequencies[octave * 12 + 9], durationMs); break;   // A
            case 'a': PlayNote(Frequencies[octave * 12 + 10], durationMs); break;  // A#
            case 'B': PlayNote(Frequencies[octave * 12 + 11], durationMs); break;  // B
            case '0': PlayNote(0, durationMs); break;  // pause
            default: throw new InvalidOperationException($"Unknown note ({noteChar}).");
        }
    }

    public void PlayNote(char noteChar, char octaveChar, char durationChar) {
        var hundreds = (durationChar is >= '1' and <= '9') ? (durationChar - '0')
                     : (durationChar is >= 'A' and <= 'Z') ? (durationChar - 'A' + 10)
                     : (durationChar is >= 'a' and <= 'z') ? (durationChar - 'a' + 36)
                     : throw new InvalidOperationException($"Unknown duration ({durationChar}).");
        PlayNote(noteChar, octaveChar, hundreds * 10);
    }

    public void Dispose() {
        snd_pcm_close(PcmHandle);
    }


    #region Private

    private const int SND_PCM_STREAM_PLAYBACK = 0;
    private const int SND_PCM_MODE_BLOCKING = 0;

    private const int SND_PCM_FORMAT_U8 = 1;
    private const int SND_PCM_ACCESS_RW_INTERLEAVED = 3;
    private const int SAMPLE_RATE = 11025;

    [DllImport("libasound.so.2")]
    private static extern int snd_pcm_open(out IntPtr pcm, string name, int stream, int mode);

    [DllImport("libasound.so.2")]
    private static extern int snd_pcm_set_params(IntPtr pcm, int format, int access, int channels, int rate, int soft_resample, int latency);

    [DllImport("libasound.so.2")]
    private static extern nint snd_pcm_writei(IntPtr pcm, IntPtr buffer, nuint size);

    [DllImport("libasound.so.2")]
    private static extern int snd_pcm_recover(IntPtr pcm, int err, int silent);

    [DllImport("libasound.so.2")]
    private static extern int snd_pcm_close(IntPtr pcm);

    private static readonly UInt16[] Frequencies = [   // C     C#    D     D#     E      F      F#     G      G#     A      A#     B
                                                          16,   17,   18,   19,    20,    21,    23,    24,    25,    27,    29,    30,  // 0
                                                          32,   34,   36,   38,    41,    43,    46,    49,    51,    55,    58,    61,  // 1
                                                          65,   69,   73,   77,    82,    87,    92,    98,   103,   110,   116,   123,  // 2
                                                         130,  138,  146,  155,   164,   174,   185,   196,   207,   220,   233,   246,  // 3
                                                         261,  277,  293,  311,   329,   349,   369,   392,   415,   440,   466,   493,  // 4
                                                         523,  554,  587,  622,   659,   698,   739,   783,   830,   880,   932,   987,  // 5
                                                        1046, 1108, 1174, 1244,  1318,  1396,  1479,  1567,  1661,  1760,  1864,  1975,  // 6
                                                        2093, 2217, 2349, 2489,  2637,  2793,  2959,  3135,  3322,  3520,  3729,  3951,  // 7
                                                        4186, 4434, 4698, 4978,  5274,  5587,  5919,  6271,  6644,  7040,  7458,  7902,  // 8
                                                        8372, 8869, 9397, 9956, 10548, 11175, 11839, 12543, 13289, 14080, 14917, 15804   // 9
                                                    ];

    #endregion Private

}
