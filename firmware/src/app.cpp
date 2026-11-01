#ifndef ZIGBEE_MODE_ED
    #error "Zigbee end device mode is not selected"
#endif

#include "Zigbee.h"

#define Debug(...) if (Serial) { Serial.printf(__VA_ARGS__); Serial.println(); }


#define ZIGBEE_SWITCH_ENDPOINT_1 1
#define ZIGBEE_SWITCH_ENDPOINT_2 2

#define BTN_BOOT BOOT_PIN
#define BTN_USER 20
#define LED_USER 15

#define PWM_CHANNEL 0
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


hw_timer_t* PlayTimer;
bool PlayActive = false;
uint32_t PlayBufferFrequency[256];
uint16_t PlayBufferDuration[256];
uint8_t PlayBufferStart = 0;
uint8_t PlayBufferEnd = 0;
uint16_t PlayMs = 0;


ZigbeePowerOutlet ZBSwitch1(ZIGBEE_SWITCH_ENDPOINT_1);
ZigbeePowerOutlet  ZBSwitch2(ZIGBEE_SWITCH_ENDPOINT_2);

void queueTone(char noteChar, char octaveChar, char durationChar) {
    uint8_t newPlayBufferEnd = PlayBufferEnd + 1;
    if (PlayBufferEnd == newPlayBufferEnd) { return; }  // no buffer space

    uint8_t octave;
    if ((octaveChar >= '0') && (octaveChar <= '9')) {
        octave = octaveChar - '0';
    } else {
        Debug("Unrecognized octave char '%c'", octaveChar);
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
        default: Debug("Unrecognized note char '%c'", noteChar); return;
    }

    uint8_t hundreds;
    switch (durationChar) {
        case '0' : hundreds = 100; break;
        case '1' : hundreds =  50; break;
        case '2' : hundreds =  25; break;
        case '3' : hundreds =  12; break;
        case '4' : hundreds =   6; break;
        case '5' : hundreds =   3; break;
        case '6' : hundreds =   2; break;
        case '7' : hundreds =   1; break;
        default:
            if ((durationChar >= 'A') && (durationChar <= 'Z')) {
                hundreds = durationChar - 'A' + 1;
            } else if ((durationChar >= 'a') && (durationChar <= 'z')) {
                hundreds = durationChar - 'a' + 27;
            } else {
                Debug("Unrecognized duration char '%c'", durationChar);
                return;
            }
            break;
    }

    PlayBufferFrequency[PlayBufferEnd] = frequency;
    PlayBufferDuration[PlayBufferEnd] = hundreds * 10;
    PlayBufferEnd = newPlayBufferEnd;
}

void queueTone(char* chars) {
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

        queueTone(noteChar, octaveChar, durationChar);
        Debug("Queued %c%c%c", noteChar, octaveChar, durationChar);
    }
    Debug("Queuing done");
}

void setState1(bool value) {
    if (value) {
        Debug("Switch 1 on");
        queueTone("A42 g42 A42");
    } else {
        Debug("Switch 1 off");
    }
}

void setState2(bool value) {
    if (value) {
        Debug("Switch 2 on");
        queueTone("E42 f42 E42");
    } else {
        Debug("Switch 2 off");
    }
}

void setUserLed(bool value) {
    digitalWrite(LED_USER, value ? LOW : HIGH);
}

void ARDUINO_ISR_ATTR onPlayTimer() {
    if (PlayMs == 0) {
        if (PlayBufferStart != PlayBufferEnd) {  // next note
            uint32_t frequency = PlayBufferFrequency[PlayBufferStart];
            uint16_t duration = PlayBufferDuration[PlayBufferStart];
            Debug("Play %d Hz for %d ms (%d:%d)", frequency, duration, PlayBufferStart, PlayBufferEnd);
            if (PlayActive) { ledcDetach(PWM_1_PIN); }
            PlayMs = duration;
            ledcAttachChannel(PWM_1_PIN, frequency, PWM_RESOLUTION, PWM_CHANNEL);
            ledcWriteChannel(PWM_CHANNEL, 50);
            PlayActive = true;
            PlayBufferStart += 1;
        } else if (PlayActive) {  // done with playing
            ledcDetach(PWM_1_PIN);
            PlayActive = false;
            Debug("Play done (%d:%d)", PlayBufferStart, PlayBufferEnd);
        }
    } else {  // note not done yet
        PlayMs--;
    }
}


void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(LED_USER, OUTPUT);
    pinMode(BTN_USER, INPUT_PULLUP);

    Debug("Adding endpoints");

    ZBSwitch1.setManufacturerAndModel("Medo64", "Switch 1");
    ZBSwitch1.onPowerOutletChange(setState1);
    Zigbee.addEndpoint(&ZBSwitch1);

    ZBSwitch2.setManufacturerAndModel("Medo64", "Switch 2");
    ZBSwitch2.onPowerOutletChange(setState2);
    Zigbee.addEndpoint(&ZBSwitch2);

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
        queueTone("A43 B43 c53 D53 E53 f53 g53 A53");
    }
    Debug("Connected to network");

    PlayTimer = timerBegin(1000);
    timerAttachInterrupt(PlayTimer, &onPlayTimer);
    timerAlarm(PlayTimer, 1, true, 0);

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

    if (ZBSwitch1.getPowerOutletState()) { Debug("Switch 1 to off"); ZBSwitch1.setState(false); }
    if (ZBSwitch2.getPowerOutletState()) { Debug("Switch 2 to off"); ZBSwitch2.setState(false); }
    delay(50);
}
