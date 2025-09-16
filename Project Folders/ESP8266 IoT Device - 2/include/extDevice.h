#include <Arduino.h>

class extDevice {

    public:
        extDevice(int pin) : _pin(pin) {
            pinMode(_pin, OUTPUT);
        }

        void on() {
            digitalWrite(_pin, HIGH);
        }
        void off() {
            digitalWrite(_pin, LOW);
        }
        void wake() {
            pinMode(_pin, OUTPUT);
        }
        void sleep() {
            pinMode(_pin, INPUT);
            digitalWrite(_pin, LOW);
        }
        int read() {
            return digitalRead(_pin);
        }

    private:
        int _pin;
        
};