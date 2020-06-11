# music-leds
Control individual LEDs with an Arduino with many modes, including a music visualizer.

## Modes
1. Wave - Send the full 360 degrees of hue down the strip, with almost each LED being about a degree off from the last one.
2. Blink - Blink on and off after a specified number of milliseconds. Config is found in led-test.cpp: `const int RATE_MS`
3. Solid - Set all LEDs to a specific color. Config is found in led-test.cpp: `CHSV color_to_set`
4. Morse Code - Blink specified morse code, using config options found in led-test.cpp: `const char* MORSE`, `const int DOT_MS`,
   `const int DASH_MS`, `const int SPACE_MS`, and `const int SPACE_BETWEEN_LETTERS_MS`.  Note that all values alread set are for
   optimal use (except for `const char* MORSE`, of course).
5. Trail from middle - Have a worm-like effect come from the middle of the strand to the ends of each respective end.
6. Music simple - Simple (not very good in my opinion) music reactive LEDs.
7. Music - Music reactive LEDs.
8. Off - You guessed it.

## Using Music Mode
Music mode is the most complex mode of all the modes.
### Requirements
1. A strand of AT LEAST 256 leds.
2. A power supply
#### Requirements for the PC connected to the Arduino
1. A usb port
2. python 3.7.4+
3. pyserial
4. pyaudio
5. numpy
6. struct
7. scipy
8. pyqtgraph (requrires pyqt5)
9. VoiceMeter Banana audio routing software (you must route your speakers to a virtual out so that pyaudio can read it as a mic)
10. Your output device set to the Voicemeter Input (NOT VOICEMETER INPUT AUX)
