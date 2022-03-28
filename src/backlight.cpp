#include "backlight.h"

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

int Channel::getDuty() const { 
    return MAX_DUTY_CYCLE;
}

/* -------------------------------------------------------------------- */

/* Custom Constructor */
Backlight::Backlight(const int pin, Channel& pwmChannel)
: pinNum(pin), ledOn(false), nightLightOn(false), outChannel(pwmChannel) {
    backlightSetup();
}

/* Initialize backlight in hardware, to be run in start */
void Backlight::backlightSetup() {
    ledcSetup(outChannel.getChannel(), outChannel.getFreq(), outChannel.getRes());
    ledcAttachPin(pinNum, outChannel.getChannel());
}

/* Gets the nightlight value */
bool Backlight::getState() const {
    return nightLightOn;
}

/* Turns on the nightlight */
void Backlight::turnOn() {
    nightLightOn = true;
    ledcWrite(outChannel.getChannel(), outChannel.getDuty());
}

/* Turns off the backlight */
void Backlight::turnOff() {
    nightLightOn = true;
    ledcWrite(outChannel.getChannel(), 0);
}

void Backlight::startMomentary() {
    if(!nightLightOn) {
        ledOn = true;
        ledcWrite(outChannel.getChannel(), outChannel.getDuty()-1);
        ledTimer = millis();
    }
}

/* Manages the momentary LED effect, needs to be called every frame */
void Backlight::manageBacklight() {
    const unsigned long delayTime = 5000; // 5 seconds
    const int fadeTime = 3000; // 1 second
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