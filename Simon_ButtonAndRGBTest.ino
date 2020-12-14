#include "Adafruit_WS2801.h"
#include "LiquidCrystal.h"


// Pin number constants
const uint8_t clockPin = 2;
const uint8_t dataPin = 3;

// const short button0 = 7;
// const short button1 = 6;
// const short button2 = 5;
// const short button3 = 4;

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

// int inputState0;
// int lastInputState0 = HIGH;
// bool inputFlag0 = LOW;
// long lastDebounceTime0 = 0;

// int inputState1;
// int lastInputState1 = HIGH;
// bool inputFlag1 = LOW;
// long lastDebounceTime1 = 0;

// int inputState2;
// int lastInputState2 = HIGH;
// bool inputFlag2 = LOW;
// long lastDebounceTime2 = 0;

// int inputState3;
// int lastInputState3 = HIGH;
// bool inputFlag3 = LOW;
// long lastDebounceTime3 = 0;




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

	// pinMode(button0, INPUT_PULLUP);
	// pinMode(button1, INPUT_PULLUP);
	// pinMode(button2, INPUT_PULLUP);
	// pinMode(button3, INPUT_PULLUP);

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

	// if (inputFlag0 == false && inputFlag1 == false && inputFlag2 == false && inputFlag3 == false)
	// {
	// 	for (uint8_t i = 0; i < numberOfPixels; i++)
	// 	{
	// 		strip.setPixelColor(i, 0, 0, 0);
	// 	}
	// 	strip.show();
	// }
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

	// int reading0 = digitalRead(button0);
	// inputFlag0 = false;
	// if (reading0 != lastInputState0) lastDebounceTime0 = millis();
	// if ((millis() - lastDebounceTime0) > debounceDelay && reading0 != inputState0)
	// {
	// 	inputState0 = reading0;
	// 	inputFlag0 = inputState0 == LOW ? true : false;
	// }
	// lastInputState0 = reading0;

	// int reading1 = digitalRead(button1);
	// inputFlag1 = false;
	// if (reading1 != lastInputState1) lastDebounceTime1 = millis();
	// if ((millis() - lastDebounceTime1) > debounceDelay && reading1 != inputState1)
	// {
	// 	inputState1 = reading1;
	// 	inputFlag1 = inputState1 == LOW ? true : false;
	// }
	// lastInputState1 = reading1;

	// int reading2 = digitalRead(button2);
	// inputFlag2 = false;
	// if (reading2 != lastInputState2) lastDebounceTime2 = millis();
	// if ((millis() - lastDebounceTime2) > debounceDelay && reading2 != inputState2)
	// {
	// 	inputState2 = reading2;
	// 	inputFlag2 = inputState2 == LOW ? true : false;
	// }
	// lastInputState2 = reading2;
	
	// int reading3 = digitalRead(button3);
	// inputFlag3 = false;
	// if (reading3 != lastInputState3) lastDebounceTime3 = millis();

	// if ((millis() - lastDebounceTime3) > debounceDelay && reading3 != inputState3)
	// {
	// 	inputState3 = reading3;
	// 	inputFlag3 = inputState3 == LOW ? true : false;
	// }
	// lastInputState3 = reading3;
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
