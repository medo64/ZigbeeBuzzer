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


hw_timer_t* PlayTimer;
bool PlayActive = false;
uint32_t PlayBufferFrequency[256];
uint16_t PlayBufferDuration[256];
uint8_t PlayBufferStart = 0;
uint8_t PlayBufferEnd = 0;
uint16_t PlayMs = 0;


ZigbeePowerOutlet ZBSwitch1(ZIGBEE_SWITCH_ENDPOINT_1);
ZigbeePowerOutlet  ZBSwitch2(ZIGBEE_SWITCH_ENDPOINT_2);


void addTone(uint32_t frequency, uint16_t duration) {
    uint8_t newPlayBufferEnd = PlayBufferEnd + 1;
    if (PlayBufferEnd == newPlayBufferEnd) { return; }  // no buffer space
    PlayBufferFrequency[PlayBufferEnd] = frequency;
    PlayBufferDuration[PlayBufferEnd] = duration;
    PlayBufferEnd = newPlayBufferEnd;
}

void setState1(bool value) {
    if (value) {
        Debug("Switch 1 on");
        addTone(440, 250);
        addTone(415, 250);
        addTone(440, 250);
    } else {
        Debug("Switch 1 off");
    }
}

void setState2(bool value) {
    if (value) {
        Debug("Switch 2 on");
        addTone(329, 250);
        addTone(369, 250);
        addTone(329, 250);
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
        addTone(440, 125);
        addTone(493, 125);
        addTone(554, 125);
        addTone(587, 125);
        addTone(659, 125);
        addTone(740, 125);
        addTone(830, 125);
        addTone(880, 125);
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

    if (ZBSwitch1.getPowerOutletState()) { ZBSwitch1.setState(false); }
    if (ZBSwitch2.getPowerOutletState()) { ZBSwitch2.setState(false); }
    delay(50);
}
