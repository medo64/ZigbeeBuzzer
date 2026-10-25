#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected"
#endif

#include "Zigbee.h"

#define ZIGBEE_LIGHT_ENDPOINT 10
constexpr uint8_t userLed = 15;
constexpr uint8_t userButton = BOOT_PIN;

ZigbeeLight zbLight = ZigbeeLight(ZIGBEE_LIGHT_ENDPOINT);

void setLight(bool value) {
    if (value) {
        Serial.println("Light on");
        digitalWrite(userLed, LOW);
    } else {
        Serial.println("Light off");
        digitalWrite(userLed, HIGH);
    }
}

void setUserLed(bool value) {
    digitalWrite(userLed, value ? LOW : HIGH);
}

void setup() {
    Serial.begin(115200);

    pinMode(userLed, OUTPUT);
    pinMode(userButton, INPUT_PULLUP);

    Serial.println("Adding endpoint");
    zbLight.setManufacturerAndModel("Medo64", "TestBulb");
    zbLight.onLightChange(setLight);
    Zigbee.addEndpoint(&zbLight);

    if (!Zigbee.begin()) {
        Serial.println("Failed to start; rebooting...");
        ESP.restart();
    }

    Serial.print("Connecting to network...");
    while (!Zigbee.connected()) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
    Serial.println("Connected to network");

    setUserLed(false);
}

void loop() {
    if (digitalRead(userButton) == LOW) {
        delay(100);  // debounce
        setUserLed(true);

        int startTime = millis();
        while (digitalRead(userButton) == LOW) {
            delay(50);
            if ((millis() - startTime) > 3000) {
                Serial.println("Resetting Zigbee to factory defaults; rebooting...");
                delay(1000);
                Zigbee.factoryReset();
            }
        }

        setUserLed(false);

        Serial.println("Manual light toggle");
        zbLight.setLight(!zbLight.getLightState());
    }

    delay(50);
}
