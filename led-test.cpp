/*
Code for controlling individual addressable led lights with an Arduino.

PRO TIP: When a value is set to a light, in order to actually see the changes,
         one must call FastLED.show().

void clearAll():
    Sets all leds to display nothing.
void setAllSolidColor():
    Runs through each led and sets its value to color_to_set.
void updateColorTimer(int rate_ms):
    If the user specified rate in milliseconds is greater than the delta
    system time, then it will toggle the color from CHSV_OFF to a specified
    color and set it to color_to_set.
int getMorseMS(char ch):
    Returns the proper time given a dot, dash, or a space.
void illuminate():
    Writes values to the leds in a certain way based on led_mode.
    If led_mode is SOLID, it calls setAllSolidColor(), without changing any
    values of color_to_set.
    If led_mode is BLINK, it calls updateColorTimer() to correctly set the
    color that is then set in setAllSolidColor().
    If led_mode is TRAIL, it calls updateColorTimer() to correctly set the
    color that is then used when it sets the first LED.  After that, starting
    from the last LED, the value of the current LED is set to the value of the
    LED before it.
    if led_mode is TRAIL_FROM_MIDDLE, it does a similar procedure as TRAIL,
    except it has two trails, both originating from the middle of the strand of
    lights that go to each end of the strip.
    If led_mode is WAVE, all of the leds are set so that the entire 360 degrees
    are spread out evenly between them.  Then, wave_counter is set to itself
    plus (255.0 / 300.0) * WAVE_SPEED_SCALAR.  This insures that we get all 360
    degrees of hue.  WAVE_SPEED_SCALAR is used to either speed up or slow down
    the effect.
    If led_mode if MORSE_CODE, then the leds flash the morse code specified by
    MORSE.  It does this by checking if we are in between a letter, character,
    or are in the middle of displaying a character.  If we are in between a
    letter, and our delta system time is greater than SPACE_BETWEEN_LETTER_MS,
    we will move on to the next character.  If we are in between a character,
    then we do the same thing as if we are in between a letter, except with the
    proper time, SPACE_MS.  If we are in the middle of displaying a character,
    we will set color_to_set to a specified color, else, we will set it to
    nothing.  We then call setAllSolidColor().  Finally, we use FastLED.show()
    to display our lights.
void setup():
    Initializes our LEDs, clears them, and displays the cleared LEDs.  We then
    specify the mode we want to use.  Finally, we delay the leds so that they
    have a chance to react after being initialized, then reset our system
    timer.
void loop():
    This funtion is called over and over by the Arduino until the reset button
    is pressed; it only calls illuminate().
*/

#include "Arduino.h"
#include <FastLED.h>
#include "Utils.h"

const int LED_PIN = 7;
const int NUM_LEDS = 300;

const int RATE_MS = 250;

const int WPM = 12;
const char* MORSE = ".--- .- ...- .- ....- .-.. -.-- ..-.";
int index = 0;
bool inBetweenChar = false;
bool inBetweenLetter = false;
const int DOT_MS = 1200 / WPM;
const int DASH_MS = DOT_MS * 3;
const int SPACE_MS = DOT_MS;
const int SPACE_BETWEEN_LETTER_MS = DOT_MS * 7;
const bool USING_RAMP = true;

long timer;
bool was_on = false;
double wave_counter = 0.0;
const double WAVE_SPEED_SCALAR = 1.0;

enum modes{
    BLINK,
    TRAIL,
    SOLID,
    WAVE,
    MORSE_CODE,
    TRAIL_FROM_MIDDLE,
    MUSIC_SIMPLE,
    MUSIC,
    TESTING,
    OFF
} led_mode;

CRGB leds[NUM_LEDS];

const CHSV CHSV_OFF = CHSV(0, 0, 0);
const CHSV CHSV_RED_FULL = CHSV(0, 255, 255);

CHSV solid_color = CHSV_OFF;
CHSV color_to_set = CHSV_RED_FULL;

void clearAll() {

    FastLED.clear();
}

void ClearSerial() {

    while (Serial.available() > 0) { char ch = Serial.read(); }
}

void setAllSolidColor() {

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = color_to_set;
    }
}

void updateColorTimer(int rate_ms) {

    if ((millis() - timer) >= RATE_MS) {
            
        if (!was_on){
            
            color_to_set = CHSV_RED_FULL;
            was_on = true;
        }
        else {

            color_to_set = CHSV_OFF;
            was_on = false;
        }

        timer = millis();
    }
}

void updateWaveCounter() {

    wave_counter += (255.0 / 300.0) * WAVE_SPEED_SCALAR;
}

int getMorseMS(char ch) {

    if (ch == '.') { return DOT_MS; }
    else if (ch == '-') { return DASH_MS; }
    else { return 5000; }
}

void illuminate() {

    if (led_mode == SOLID) {

        setAllSolidColor();
    }

    else if (led_mode == BLINK) {

        updateColorTimer(RATE_MS);
        setAllSolidColor();
    }

    else if (led_mode == TRAIL) {

        updateColorTimer(RATE_MS);

        leds[0] = color_to_set;
        for (int i = NUM_LEDS - 1; i > 0; i--) {

            leds[i] = leds[i - 1];
        }
    }

    else if (led_mode == TRAIL_FROM_MIDDLE) {

        updateColorTimer(RATE_MS);

        leds[NUM_LEDS / 2] = color_to_set;
        leds[NUM_LEDS / 2 - 1] = color_to_set;
        for (int i = 0; i < NUM_LEDS / 2 - 1; i++) {

            leds[i] = leds[i + 1];
        }
        for (int i = NUM_LEDS - 1; i > NUM_LEDS / 2; i--) {

            leds[i] = leds[i - 1];
        }
    }

    else if (led_mode == WAVE) {

        for (int i = 0; i < NUM_LEDS; i++) {

            leds[i] = CHSV((i * 255.0 / 300.0) + wave_counter * WAVE_SPEED_SCALAR, 255, 255);
        }
        updateWaveCounter();
    }

    else if (led_mode == MORSE_CODE) {

        if (inBetweenLetter) {

            if ((millis() - timer) >= SPACE_BETWEEN_LETTER_MS) {

                //Example: --.--.. --.-
                //               | |
                //We are here----| |--|
                //We need to go here--|
                index++;
                index++;
                inBetweenLetter = false;
                timer = millis();
            }
        }

        else if (inBetweenChar) {

            if ((millis() - timer) >= SPACE_MS) {

                index++;
                inBetweenChar = false;
                timer = millis();
            }
        }

        else if ((millis() - timer) >= getMorseMS(MORSE[index])) {

            if (MORSE[index + 1] == ' ') {

                inBetweenLetter = true;
            }
            else if (index + 1 > strlen(MORSE) - 1) {

                index = -2;
                inBetweenLetter = true;
            }
            else {

                inBetweenChar = true;
            }
            
            timer = millis();
        }

        if (inBetweenChar || inBetweenLetter) {

            color_to_set = CHSV_OFF;
        }
        
        else {

            color_to_set = CHSV_RED_FULL;
        }

        setAllSolidColor();
    }

    else if (led_mode == MUSIC_SIMPLE) {

        unsigned char buffer[128] = {0};

        //Make sure there is data to be read :)
        if (Serial.available() >= 0) {

            //Read until the number of bytes has been read, or until the
            //timeout has been reached
            Serial.readBytes(buffer, 128);
            ClearSerial();

            for (int i = 0; i < 128; i++) {

                leds[i] = CHSV(i * 2 + wave_counter, 255, (int)buffer[127 - i]);
            }
            for (int i = 128; i < 256; i++) {

                leds[i] = CHSV((255 - i) * 2 + wave_counter, 255, (int)buffer[i - 128]);
            }
        }
    }

    else if (led_mode == MUSIC) {

        unsigned char buffer[128] = {0};
        double ledsEffects[128] = {0};

        //Make sure there is data to be read :)
        if (Serial.available() >= 0) {

            //Read until the number of bytes has been read, or until the
            //timeout has been reached
            Serial.readBytes(buffer, 128);
            ClearSerial();

            for (int i = 0; i < 128; i++) {

                int masterVal = (int)buffer[i];
                double masterScalar = masterVal / 255.0;

                if (masterVal != 0) {

                    //middle
                    ledsEffects[i] = masterScalar / 1.0;
                    
                    if (USING_RAMP) {
                        
                        int numOfFades = RoundLit(masterVal * 4 / 255.0);
                        int valToSet;
                        int valAlready;
                        //left side
                        for (int j = 0; j < numOfFades; j++) {
                            
                            valToSet = j + 2;
                            valAlready = ledsEffects[i - j - 1];
                            //                                          less than bc it is the bottom of a fraction
                            if ((i - j - 1 >= 0 && i - j - 1 <= 127) && ((valToSet < valAlready) || valAlready == 0)) {

                                ledsEffects[i - j - 1] = masterScalar / valToSet;
                            }
                        }
                        //right side
                        for (int j = 0; j < numOfFades; j++) {
                            
                            valToSet = j + 2;
                            valAlready = ledsEffects[i + j + 1];
                            //                                          less than bc it is the bottom of a fraction
                            if ((i + j + 1 >= 0 && i + j + 1 <= 127) && ((valToSet < valAlready) || valAlready == 0)) {

                                ledsEffects[i + j + 1] = masterScalar / valToSet;
                            }
                        }
                    }
                }
                else if (ledsEffects[i] == 0 ) ledsEffects[i] = 0;
            }

            for (int i = 0; i < 128; i++) {

                leds[i] = CHSV(i * 2 + wave_counter, 255, (ledsEffects[127 - i] == 0) ? 0 : (int)(ledsEffects[127 - i] * 256) - 1);
            }
            for (int i = 128; i < 256; i++) {

                leds[i] = CHSV((255 - i) * 2 + wave_counter, 255, (ledsEffects[i - 128] == 0) ? 0 : (int)(ledsEffects[i - 128] * 256) - 1);
            }
        }
        updateWaveCounter();
    }

    else if (led_mode == TESTING) {

        for (int i = 0; i < 255; i++) { leds[i] = CHSV(0, 255, i); }
    }

    FastLED.show();
}

void setup() {

    Serial.begin(2000000);
    Serial.setTimeout(1000);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    clearAll();
    FastLED.show();

    led_mode = MUSIC;

    delay(500);

    timer = millis();
}

void loop() {

    illuminate();
}
