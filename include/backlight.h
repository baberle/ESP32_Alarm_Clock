#ifndef __BACKLIGHT_H
#define __BACKLIGHT_H

#include <Arduino.h>

class Channel {
    int PWM_CHANNEL;
    int PWM_FREQ;
    int PWM_RESOLUTION;
    int MAX_DUTY_CYCLE;
    public:
    Channel(int channel, int frequency, int resolution);
    int getChannel() const;
    int getFreq() const;
    int getRes() const;
    int getDuty() const;
};

class Backlight {
    int pinNum;
    bool ledOn = false;
    bool nightLightOn = false;
    unsigned long ledTimer;
    Channel& outChannel;
    public:
    Backlight() = delete;
    Backlight(const int pin, Channel& pwmChannel);
    void backlightSetup();
    bool getState() const;
    void turnOn();
    void turnOff();
    void startMomentary();
    void manageBacklight();
};

#endif