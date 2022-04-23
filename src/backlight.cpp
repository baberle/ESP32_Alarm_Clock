#include "backlight.h"

/* ==================== PWM CHANNEL ==================== */

// These input variables are limited by the clock speed of the ESP-32 hardware
Channel::Channel(int channel, int frequency, int resolution) {
    PWM_CHANNEL = channel;
    PWM_FREQ = frequency;
    PWM_RESOLUTION = resolution;
    MAX_DUTY_CYCLE = (int)(pow(2,PWM_RESOLUTION)-1);
}

int Channel::getChannel() const { 
    return PWM_CHANNEL;
}

int Channel::getFreq() const { 
    return PWM_FREQ;
}

int Channel::getRes() const { 
    return PWM_RESOLUTION;
}

// This is the number of 'levels' of brightness 
int Channel::getDuty() const { 
    return MAX_DUTY_CYCLE;
}


/* ==================== BACKLIGHT ==================== */

Backlight::Backlight(const int pin, Channel& pwmChannel)
: pinNum(pin), ledOn(false), nightLightOn(false), outChannel(pwmChannel) {
    setup();
}

// Initialize backlight in hardware, to be run at start
void Backlight::setup() {
    ledcSetup(outChannel.getChannel(), outChannel.getFreq(), outChannel.getRes());
    ledcAttachPin(pinNum, outChannel.getChannel());
}

bool Backlight::getState() const {
    return nightLightOn;
}

void Backlight::turnOn() {
    nightLightOn = true;
    ledcWrite(outChannel.getChannel(), outChannel.getDuty());
}

void Backlight::turnOff() {
    nightLightOn = false;
    ledcWrite(outChannel.getChannel(), 0);
}

void Backlight::startMomentary() {
    if(!nightLightOn) {
        ledOn = true;
        ledcWrite(outChannel.getChannel(), outChannel.getDuty()-1);
        ledTimer = millis();
    }
}

/* Manages the momentary LED effect, needs to be called every cycle */
void Backlight::manage() {
    const unsigned long delayTime = 5000; // 5 seconds
    const int fadeTime = 3000; // 3 second
    if(ledOn && !nightLightOn) {
        if(millis() - ledTimer > delayTime) {
            ledOn = false;
            ledcWrite(outChannel.getChannel(), 0);
        } else
        if(millis() - ledTimer > delayTime - fadeTime) {
            float brightness = outChannel.getDuty()-((float)outChannel.getDuty()/delayTime)*(millis() - ledTimer);
            ledcWrite(outChannel.getChannel(), (int)brightness);
        }
    }
}