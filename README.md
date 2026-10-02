# Zigbee Buzzer

Zigbee notification device allowing for multiple notifications using sound.


## Usage

When device is first used, button should be pressed to wake up the device. It
will immediately enter the pairing mode and remain in it for 3 minutes. If
device is not paired within 3 minutes, it will enter deep sleep again.

LED will be normally off. If LED is blinking slowly (about 1 Hz), device is
waiting to be paired. Fast double blinks will indicate communication error.

If device is paired, a short button press will cause device to immediately report
its state upon releasing.

Long press of 3-5s will make device enter pairing mode. LED will turn on once
3s interval starts and turn off after 5th second has passed.

Long press of 30-35 seconds will reset unit to factory settings and place it
into deep sleep until the next button press.

Long press while LED is off, will be ignored.


## Sensors

Each notification sound is exposed as a button.
