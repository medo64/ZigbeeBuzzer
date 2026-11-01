#ifndef ZIGBEE_MODE_ED
    #error "Zigbee end device mode is not selected"
#endif

#include "Zigbee.h"


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
        Serial.println("Switch 1 on");
        addTone(440, 250);
        addTone(415, 250);
        addTone(440, 250);
    } else {
        Serial.println("Switch 1 off");
    }
}

void setState2(bool value) {
    if (value) {
        Serial.println("Switch 2 on");
        addTone(329, 250);
        addTone(369, 250);
        addTone(329, 250);
    } else {
        Serial.println("Switch 2 off");
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
            Serial.print("Play setup for index ");
            Serial.print(PlayBufferStart, DEC);
            Serial.print(" at ");
            Serial.print(frequency, DEC);
            Serial.print(" Hz for ");
            Serial.print(duration, DEC);
            Serial.println("ms");
            if (PlayActive) { ledcDetach(PWM_1_PIN); }
            PlayMs = duration;
            ledcAttachChannel(PWM_1_PIN, frequency, PWM_RESOLUTION, PWM_CHANNEL);
            ledcWriteChannel(PWM_CHANNEL, 50);
            PlayActive = true;
            PlayBufferStart += 1;
        } else if (PlayActive) {  // done with playing
            ledcDetach(PWM_1_PIN);
            PlayActive = false;
            Serial.println("Play done");
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

    Serial.println("Adding endpoints");

    ZBSwitch1.setManufacturerAndModel("Medo64", "Switch 1");
    ZBSwitch1.onPowerOutletChange(setState1);
    Zigbee.addEndpoint(&ZBSwitch1);

    ZBSwitch2.setManufacturerAndModel("Medo64", "Switch 2");
    ZBSwitch2.onPowerOutletChange(setState2);
    Zigbee.addEndpoint(&ZBSwitch2);

    Serial.println("Added endpoints");

    Serial.println("Starting Zigbee");
    if (!Zigbee.begin()) {
        Serial.println("Failed to start; rebooting");
        ESP.restart();
    }
    Serial.println("Started Zigbee");

    Serial.print("Connecting to network");
    while (!Zigbee.connected()) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
    Serial.println("Connected to network");

    PlayTimer = timerBegin(1000);
    timerAttachInterrupt(PlayTimer, &onPlayTimer);
    timerAlarm(PlayTimer, 1, true, 0);

    setUserLed(false);
}

void loop() {
    if (digitalRead(BTN_USER) == LOW) {
        delay(100);  // debounce
        setUserLed(true);

        int startTime = millis();
        while (digitalRead(BTN_USER) == LOW) {
            delay(50);
            if ((millis() - startTime) > 3000) {
                Serial.println("Resetting Zigbee to factory defaults; rebooting");
                delay(1000);
                Zigbee.factoryReset();
            }
        }

        setUserLed(false);
    }

    delay(50);

    if (ZBSwitch1.getPowerOutletState()) { ZBSwitch1.setState(false); }
    if (ZBSwitch2.getPowerOutletState()) { ZBSwitch2.setState(false); }
}
