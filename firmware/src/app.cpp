#ifndef ZIGBEE_MODE_ED
    #error "Zigbee end device mode is not selected"
#endif

#include <mutex>
#include "Preferences.h"
#include "Zigbee.h"

#define Debug(...) if (Serial) { Serial.printf(__VA_ARGS__); Serial.println(); }


#define ZIGBEE_SOUND_COUNT        15
#define ZIGBEE_SOUND_ENDPOINT_BASE 1
#define ZIGBEE_ANALOG_ENDPOINT   101

#define BTN_BOOT BOOT_PIN
#define BTN_USER 20
#define LED_USER 15

#define PWM_RESOLUTION 8

#define PWM_1_PIN  2
#define PWM_2_PIN 21
#define PWM_3_PIN 22

static constexpr uint16_t Frequencies[] = {  //    C     C#    D     D#     E      F      F#     G      G#     A      A#     B
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
                                            };


std::mutex PlayMutex;
bool PlayActive = false;  // if currently playing
uint8_t PlayVolume = 0;  // PWMs that were configured
volatile uint8_t PlayVolumeNext = 0;  // next PWM setup
uint32_t PlayBufferFrequency[256];
uint8_t PlayBufferCounter[256];
uint8_t PlayBufferStart = 0;
uint8_t PlayBufferEnd = 0;
uint8_t PlayCounter = 0;

ZigbeeBinary* ZBSounds[ZIGBEE_SOUND_COUNT] = {};
ZigbeeAnalog ZBVolume(ZIGBEE_ANALOG_ENDPOINT);

void queueTones(char* chars) {
    std::lock_guard<std::mutex> guard(PlayMutex);

    while(true) {
        if (*chars == '\0') { break; }

        while (*chars == ' ') { chars++; }
        if (*chars == '\0') { break; }
        char noteChar = *chars;
        chars++;

        while (*chars == ' ') { chars++; }
        if (*chars == '\0') { break; }
        char octaveChar = *chars;
        chars++;

        while (*chars == ' ') { chars++; }
        if (*chars == '\0') { break; }
        char durationChar = *chars;
        chars++;

        uint8_t newPlayBufferEnd = PlayBufferEnd + 1;
        if (PlayBufferEnd == newPlayBufferEnd) {
            Debug("Buffer exhausted!");
            return;
        }

        uint8_t octave;
        if ((octaveChar >= '0') && (octaveChar <= '9')) {
            octave = octaveChar - '0';
        } else {
            Debug("Unrecognized octave char '%c'!", octaveChar);
            return;
        }

        uint32_t frequency;
        switch (noteChar) {
            case 'C': frequency = Frequencies[octave * 12 + 0]; break;   // C
            case 'c': frequency = Frequencies[octave * 12 + 1]; break;   // C#
            case 'D': frequency = Frequencies[octave * 12 + 2]; break;   // D
            case 'd': frequency = Frequencies[octave * 12 + 3]; break;   // D#
            case 'E': frequency = Frequencies[octave * 12 + 4]; break;   // E
            case 'F': frequency = Frequencies[octave * 12 + 5]; break;   // F
            case 'f': frequency = Frequencies[octave * 12 + 6]; break;   // F#
            case 'G': frequency = Frequencies[octave * 12 + 7]; break;   // G
            case 'g': frequency = Frequencies[octave * 12 + 8]; break;   // G#
            case 'A': frequency = Frequencies[octave * 12 + 9]; break;   // A
            case 'a': frequency = Frequencies[octave * 12 + 10]; break;  // A#
            case 'B': frequency = Frequencies[octave * 12 + 11]; break;  // B
            case '0': frequency = 0; break;  // pause
            default: Debug("Unrecognized note char '%c'!", noteChar); return;
        }

        uint8_t hundredths;
        switch (durationChar) {
            case '0' : hundredths = 100; break;
            case '1' : hundredths =  50; break;
            case '2' : hundredths =  25; break;
            case '3' : hundredths =  12; break;
            case '4' : hundredths =   6; break;
            case '5' : hundredths =   3; break;
            case '6' : hundredths =   2; break;
            case '7' : hundredths =   1; break;
            default:
                if ((durationChar >= 'A') && (durationChar <= 'Z')) {
                    hundredths = durationChar - 'A' + 1;
                } else if ((durationChar >= 'a') && (durationChar <= 'z')) {
                    hundredths = durationChar - 'a' + 27;
                } else {
                    Debug("Unrecognized duration char '%c'!", durationChar);
                    return;
                }
                break;
        }

        PlayBufferFrequency[PlayBufferEnd] = frequency;
        PlayBufferCounter[PlayBufferEnd] = hundredths;
        PlayBufferEnd = newPlayBufferEnd;

        Debug("Queued %c%c%c", noteChar, octaveChar, durationChar);
    }

    PlayBufferFrequency[PlayBufferEnd] = 0;
    PlayBufferCounter[PlayBufferEnd] = 10; // 100ms
    PlayBufferEnd++;
    Debug("Queued a moment of silence");

    Debug("Queuing done");
}

void playSound(uint8_t index) {
    Debug("Sound %d triggered", index)
    switch (index) {
        case  1: queueTones("A43"); break;
        case  2: queueTones("E42"); break;
        case  3: queueTones("C52"); break;
        case  4: queueTones("A42 C53"); break;
        case  5: queueTones("C52 E52"); break;
        case  6: queueTones("F41 A42"); break;
        case  7: queueTones("A42 g42 A42"); break;
        case  8: queueTones("D42 F42 A52"); break;
        case  9: queueTones("G51 E52 C53"); break;
        case 10: queueTones("C52 E53 G52 C63"); break;
        case 11: queueTones("C42 G42 E52 g52"); break;
        case 12: queueTones("C41 E41 A41 B41"); break;
        case 13: queueTones("B4Y G4y D4Y E4Y C4y"); break;
        case 14: queueTones("G5O G5O A5O B5O C6e"); break;
        case 15: queueTones("B5R B5R A5R g5R f5R E5e"); break;
    }
}

void playSound1(bool state) { if (state) { playSound(1); } }
void playSound2(bool state) { if (state) { playSound(2); } }
void playSound3(bool state) { if (state) { playSound(3); } }
void playSound4(bool state) { if (state) { playSound(4); } }
void playSound5(bool state) { if (state) { playSound(5); } }
void playSound6(bool state) { if (state) { playSound(6); } }
void playSound7(bool state) { if (state) { playSound(7); } }
void playSound8(bool state) { if (state) { playSound(8); } }
void playSound9(bool state) { if (state) { playSound(9); } }
void playSound10(bool state) { if (state) { playSound(10); } }
void playSound11(bool state) { if (state) { playSound(11); } }
void playSound12(bool state) { if (state) { playSound(12); } }
void playSound13(bool state) { if (state) { playSound(13); } }
void playSound14(bool state) { if (state) { playSound(14); } }
void playSound15(bool state) { if (state) { playSound(15); } }

void setVolume(float value) {
    uint8_t newValue = (uint8_t)value;
    PlayVolumeNext = newValue;
    Debug("Volume set to %d", newValue);
}

void setUserLed(bool value) {
    digitalWrite(LED_USER, value ? LOW : HIGH);
}


Preferences Preferences;

void setup() {
    Serial.begin(115200);

    Debug("Setup preferences");
    Preferences.begin("Volume", false);
    PlayVolumeNext = Preferences.getUChar("Volume", 3);;

    Debug("Setup pins");
    pinMode(LED_USER, OUTPUT);
    pinMode(BTN_USER, INPUT_PULLUP);

    Debug("Adding endpoints");

    Zigbee.role(ZIGBEE_END_DEVICE);

    for (size_t i = 0; i < ZIGBEE_SOUND_COUNT; i++) {
        uint8_t endpointIndex = i + 1;
        char description[16];
        snprintf(description, sizeof(description), "Sound %u", static_cast<unsigned>(endpointIndex));
        ZBSounds[i] = new ZigbeeBinary(ZIGBEE_SOUND_ENDPOINT_BASE + i);
        ZBSounds[i]->addBinaryOutput();
        ZBSounds[i]->setBinaryOutputDescription(description);
        switch(endpointIndex) {
            case  1: ZBSounds[i]->onBinaryOutputChange(playSound1); break;
            case  2: ZBSounds[i]->onBinaryOutputChange(playSound2); break;
            case  3: ZBSounds[i]->onBinaryOutputChange(playSound3); break;
            case  4: ZBSounds[i]->onBinaryOutputChange(playSound4); break;
            case  5: ZBSounds[i]->onBinaryOutputChange(playSound5); break;
            case  6: ZBSounds[i]->onBinaryOutputChange(playSound6); break;
            case  7: ZBSounds[i]->onBinaryOutputChange(playSound7); break;
            case  8: ZBSounds[i]->onBinaryOutputChange(playSound8); break;
            case  9: ZBSounds[i]->onBinaryOutputChange(playSound9); break;
            case 10: ZBSounds[i]->onBinaryOutputChange(playSound10); break;
            case 11: ZBSounds[i]->onBinaryOutputChange(playSound11); break;
            case 12: ZBSounds[i]->onBinaryOutputChange(playSound12); break;
            case 13: ZBSounds[i]->onBinaryOutputChange(playSound13); break;
            case 14: ZBSounds[i]->onBinaryOutputChange(playSound14); break;
            case 15: ZBSounds[i]->onBinaryOutputChange(playSound15); break;
        }
        Zigbee.addEndpoint(ZBSounds[i]);
    }

    ZBVolume.setManufacturerAndModel("Medo64", "Buzzer Volume");
    ZBVolume.addAnalogOutput();
    ZBVolume.setAnalogOutputDescription("Volume");
    ZBVolume.setAnalogOutputMinMax(1, 3);
    ZBVolume.setAnalogOutputResolution(1);
    ZBVolume.onAnalogOutputChange(setVolume);
    Zigbee.addEndpoint(&ZBVolume);

    Debug("Added endpoints");

    Debug("Starting Zigbee");
    if (!Zigbee.begin()) {
        Debug("Failed to start; restarting");
        ESP.restart();
    }
    Debug("Started Zigbee");

    Debug("Connecting to network");
    int64_t connectMillis = millis();
    uint64_t connectDuration = 0;
    while (!Zigbee.connected()) {
        delay(50);
        connectDuration = millis() - connectMillis;
        setUserLed((connectDuration / 500) % 2 == 0);
    }
    if (connectDuration > 200) {  // assume the first connection
        queueTones("A43 B43 c53 D53 E53 f53 g53 A53");
    }
    Debug("Connected to network");

    ZBVolume.setAnalogOutput((float)PlayVolumeNext);

    setUserLed(false);
}


int64_t ButtonMillis = 0;

void loop() {
    bool buttonPressed = digitalRead(BTN_USER) == LOW;
    if (buttonPressed && (ButtonMillis == 0)) {  // button pressed
        setUserLed(true);
        ButtonMillis = millis();
    } else if (!buttonPressed && (ButtonMillis != 0)) {  // button released
        setUserLed(false);
        uint64_t buttonDuration = millis() - ButtonMillis;
        ButtonMillis = 0;
        if ((buttonDuration >= 5000) && (buttonDuration <= 10000)) {
            Debug("Resetting Zigbee to factory defaults; restarting");
            delay(10);
            Zigbee.factoryReset();
        } else if (Serial && (buttonDuration > 10000)) {  // if debugging, just restart device
            Debug("Restarting");
            delay(10);
            ESP.restart();
        } else if (buttonDuration >= 50) {  // if nothing was done (delay for debounce)
            Debug("No button action");
        }
    } else if (buttonPressed) {  // button held
        uint64_t buttonDuration = millis() - ButtonMillis;
        if ((buttonDuration >= 5000) && (buttonDuration <= 10000)) {  // if held for 5-10, blink
            setUserLed((buttonDuration / 200) % 2 == 0);
        } else if (buttonDuration > 10000) {  // give up on waiting
            setUserLed(true);
        }
    }

    if (PlayCounter == 0) {  // nothing playing or playing tone just finished

        if (PlayBufferStart != PlayBufferEnd) {  // next note exists
            std::lock_guard<std::mutex> guard(PlayMutex);
            uint32_t frequency = PlayBufferFrequency[PlayBufferStart];
            uint8_t hundredths = PlayBufferCounter[PlayBufferStart];
            Debug("Play %d Hz for %d0 ms (%d:%d)", frequency, hundredths, PlayBufferStart, PlayBufferEnd);
            PlayCounter = hundredths;

            uint8_t newDutyCycle = (frequency > 0) ? 50 : 0;
            uint32_t newFrequency = (frequency > 0) ? frequency : 1000;

            if (PlayVolume == PlayVolumeNext) {  // use  the same PWM
                Debug("Play changing frequency for PWM %d", PlayVolume);
                switch (PlayVolume) {
                    case 1: ledcChangeFrequency(PWM_1_PIN, newFrequency, PWM_RESOLUTION); break;
                    case 2: ledcChangeFrequency(PWM_2_PIN, newFrequency, PWM_RESOLUTION); break;
                    case 3: ledcChangeFrequency(PWM_3_PIN, newFrequency, PWM_RESOLUTION); break;
                }
            } else {  // there was a change in play volume
                if (PlayVolume > 0) {  // volume already exists
                    Debug("Play detaching PWM %d", PlayVolume);
                    switch (PlayVolume) {
                        case 1: ledcDetach(PWM_1_PIN); pinMode(PWM_1_PIN, INPUT); break;
                        case 2: ledcDetach(PWM_2_PIN); pinMode(PWM_2_PIN, INPUT); break;
                        case 3: ledcDetach(PWM_3_PIN); pinMode(PWM_3_PIN, INPUT); break;
                    }
                }
                PlayVolume = PlayVolumeNext;
                Debug("Play attaching PWM %d", PlayVolume);
                switch (PlayVolume) {
                    case 1: ledcAttach(PWM_1_PIN, newFrequency, PWM_RESOLUTION); break;
                    case 2: ledcAttach(PWM_2_PIN, newFrequency, PWM_RESOLUTION); break;
                    case 3: ledcAttach(PWM_3_PIN, newFrequency, PWM_RESOLUTION); break;
                }
                Preferences.putUChar("Volume", PlayVolume);
            }

            Debug("Play setting duty cycle for PWM %d", PlayVolume);
            switch (PlayVolume) {
                case 1: ledcWrite(PWM_1_PIN, newDutyCycle); break;
                case 2: ledcWrite(PWM_2_PIN, newDutyCycle); break;
                case 3: ledcWrite(PWM_3_PIN, newDutyCycle); break;
            }
            PlayActive = true;
            PlayBufferStart += 1;

        } else if (PlayActive) {  // done with playing
            Debug("Play silencing PWM %d", PlayVolume);
            switch (PlayVolume) {
                case 1: ledcWrite(PWM_1_PIN, 0); break;
                case 2: ledcWrite(PWM_2_PIN, 0); break;
                case 3: ledcWrite(PWM_3_PIN, 0); break;
            }
            PlayActive = false;
            Debug("Play done (%d:%d)", PlayBufferStart, PlayBufferEnd);

        } else {  // nothing to do
            delay(50);
        }

    } else {  // note currently in progress
        PlayCounter--;
        delay(10);
    }

    for (size_t i = 0; i < ZIGBEE_SOUND_COUNT; i++) {
        if (ZBSounds[i]->getBinaryOutput()) {
            ZBSounds[i]->setBinaryOutput(false);
            ZBSounds[i]->reportBinaryOutput();
        }
    }
}
