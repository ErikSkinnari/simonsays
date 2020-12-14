#include "Adafruit_WS2801.h"
#include "LiquidCrystal.h"


// Pin number constants
const uint8_t clockPin = 2;
const uint8_t dataPin = 3;

const short numberOfPixels = 4;

const short D4 = 9;
const short D5 = 10;
const short D6 = 11;
const short D7 = 12;
const short DISPLAY_ENABLE = 13;
const short DISPLAY_RS = 8;

const short debounceDelay = 50;
const short SIZE_OF_ARRAYS = 20;
const short BRIGHTNESS = 80;

short buttons[4] = { 7, 6, 5, 4 };
int inputStates[4];
int lastInputStates[4];
bool inputFlags[4];
long lastDebounceTimes[4] = { 0, 0, 0, 0 };

Adafruit_WS2801 strip = Adafruit_WS2801(numberOfPixels, dataPin, clockPin);
LiquidCrystal lcd( DISPLAY_RS, DISPLAY_ENABLE, D4, D5, D6, D7);

short answers[SIZE_OF_ARRAYS];
short guesses[SIZE_OF_ARRAYS];
short turn = 0;

void setup()
{
	for(int i = 0; i < 4; i++)
	{
		pinMode(buttons[i], INPUT_PULLUP);
	}

	for(int i = 0; i < 4; i++) 
	{
		lastInputStates[i] = HIGH;
		inputFlags[i] = LOW;
		lastDebounceTimes[i] = 0;
	}

	lcd.begin(16, 2);

	Serial.begin(9600);

	strip.begin();
	for (int i = 0; i < numberOfPixels; i++)
	{
		strip.setPixelColor(i, 0, 0, 0);
	}
	strip.show();

	lcd.print("Simon says:");
	lcd.setCursor(0,1);
	lcd.print("IMPOSTOR!");
	delay(2000);
}

void loop()
{
	buttonRead();

	if (inputFlags[0] == true) {
		all_on(0);
		writeText("Button 1");
	}
	
	if (inputFlags[1] == true) {
		all_on(1);
		writeText("Button 2");
	}
	if (inputFlags[2] == true) {
		all_on(2);
		writeText("Button 3");
	}
	if (inputFlags[3] == true) {
		all_on(3);
		writeText("Button 4");
	}
}

void writeText(char* text) 
{
	lcd.clear();
	lcd.print(text);
}

void buttonRead()
{
	for(int i = 0; i < 4; i++)
	{
		int reading = digitalRead(buttons[i]);
		inputFlags[i] = false;
		if(reading != lastInputStates[i]) lastDebounceTimes[i] = millis();
		if ((millis() - lastDebounceTimes[i]) > debounceDelay && reading != inputStates[i])
		{
			inputStates[i] = reading;
			inputFlags[i] = inputStates[i] == LOW ? true : false;
		}
		lastInputStates[i] = reading;
	}
}

void all_on(int c)
{
	if (c == 0)
	{
		for (int i = 0; i < numberOfPixels; i++)
		{
			strip.setPixelColor(i, BRIGHTNESS, BRIGHTNESS, 0);
		}
		strip.show();
	}
	else if (c == 1)
	{
		Serial.println("rGb");
		for (int i = 0; i < numberOfPixels; i++)
		{
			strip.setPixelColor(i, 0, BRIGHTNESS, 0);
		}
		strip.show();
	}
	else if (c == 2)
	{
		Serial.println("rgB");
		for (int i = 0; i < numberOfPixels; i++)
		{
			strip.setPixelColor(i, 0, 0, BRIGHTNESS);
		}
		strip.show();
	}
	else if (c == 3)
	{
		Serial.println("Rgb");
		for (int i = 0; i < numberOfPixels; i++)
		{
			strip.setPixelColor(i, BRIGHTNESS, 0, 0);
		}
		strip.show();
	}
	else
	{
		Serial.println("RGB");
		for (int i = 0; i < numberOfPixels; i++)
		{
			strip.setPixelColor(i, BRIGHTNESS, BRIGHTNESS, BRIGHTNESS);
		}
		strip.show();
	}
}
