#include "Adafruit_WS2801.h"
#include "LiquidCrystal.h"
#include "EEPROM.h"

// Pin number constants
const uint8_t CLOCK_PIN = 2;
const uint8_t DATA_PIN = 3;

const short NO_OF_PIXELS = 4;

const short D4 = 9;
const short D5 = 10;
const short D6 = 11;
const short D7 = 12;
const short DISPLAY_ENABLE = 13;
const short DISPLAY_RS = 8;

const short DEBOUNCE_DELAY = 50;
const short SIZE_OF_ARRAYS = 20;
const short BRIGHTNESS = 40;

short buttons[4] = { 7, 6, 5, 4 };
int inputStates[4];
int lastInputStates[4];
bool inputFlags[4];
long lastDebounceTimes[4] = { 0, 0, 0, 0 };

// Display and leds
Adafruit_WS2801 strip = Adafruit_WS2801(NO_OF_PIXELS, DATA_PIN, CLOCK_PIN);
LiquidCrystal lcd( DISPLAY_RS, DISPLAY_ENABLE, D4, D5, D6, D7);

short pattern[SIZE_OF_ARRAYS];
short turn = 0;
short selectedHighscore = 0;

String highscores[3];

enum State
{
	startMenu,
	highscore,
	game,
	gameOver,
	nameInput
} state;

enum ColorSelection
{
	RED,
	GREEN,
	BLUE,
	OFF
};

// Custom characters
byte arrowUp[8] = {
	B00000,
	B00100,
	B01110,
	B10101,
	B00100,
	B00100,
	B00100,
	B00000};

byte arrowDown[8] = {
	B00000,
	B00100,
	B00100,
	B00100,
	B10101,
	B01110,
	B00100,
	B00000};

byte arrowLeft[8] = {
	B00000,
	B00100,
	B01000,
	B11111,
	B01000,
	B00100,
	B00000,
	B00000};

byte arrowRight[8] = {
	B00000,
	B00100,
	B00010,
	B11111,
	B00010,
	B00100,
	B00000,
	B00000};

bool buttonPressed = true;
bool displayChange = true;

void setup()
{
	randomSeed(analogRead(0));

	randomizePattern();

	state = startMenu;

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

	lcd.createChar(0, arrowUp);
	lcd.createChar(1, arrowDown);
	lcd.createChar(2, arrowLeft);
	lcd.createChar(3, arrowRight);

	lcd.begin(16, 2);
	Serial.begin(9600);

	strip.begin();
	for (int i = 0; i < NO_OF_PIXELS; i++)
	{
		strip.setPixelColor(i, 0, 0, BRIGHTNESS);
		if (i > 0)
		{
			strip.setPixelColor(i - 1, 0, 0, 0);
		}
		strip.show();
		delay(1000);
	}
	setAllPixels(RED);
	delay(100);

	setAllPixels(OFF);
}

void loop()
{
	if (state == startMenu)
	{
		mainMenu();
	}
	else if (state == highscore)
	{
		highscoreList();
	}
	else if (state == game)
	{
		gamePlay();
	}
	else if (state == gameOver)
	{
		gameOverDisplay();
	}

	buttonRead();
	buttonPressed = stateChanged();
}

void randomizePattern()
{
	for (int i = 0; i < SIZE_OF_ARRAYS; i++)
	{
		pattern[i] = random(0, 4);
	}
}

void mainMenu()
{
	if (displayChange)
	{
		setPixel(0, BLUE);
		setPixel(3, RED);

		writeText("New Game    BLUE");
		lcd.setCursor(0, 1);
		lcd.print("Highscores   RED");
		displayChange = false;
	}

	if (inputFlags[0] == true)
	{
		setAllPixels(OFF);
		state = game;
		displayChange = true;
	}

	if (inputFlags[3] == true)
	{
		setAllPixels(OFF);
		state = highscore;
		displayChange = true;
	}
}

void highscoreList()
{
	if (displayChange)
	{
		lcd.clear();
		lcd.print(selectedHighscore);
		lcd.setCursor(0, 1);
		lcd.write(byte(0));
		lcd.setCursor(4, 1);
		lcd.write(byte(1));
		lcd.setCursor(15, 1);
		lcd.write(byte(2));
		displayChange = false;
	}

	// Up button
	if ((inputFlags[0] == true) && selectedHighscore > 0)
	{
		selectedHighscore--;
		setAllPixels(RED);
		displayChange = true;
	}

	// Down button
	if ((inputFlags[1] == true) && selectedHighscore < 2)
	{
		selectedHighscore++;
		setAllPixels(BLUE);
		displayChange = true;
	}

	// Back button
	if (inputFlags[3] == true)
	{
		setAllPixels(OFF);
		selectedHighscore = 0;
		state = startMenu;
		displayChange = true;
	}
}

void gamePlay()
{
	if (displayChange)
	{
		displayChange = false;
		lcd.clear();
		lcd.print("Game");
	}
	if (inputFlags[3] == 1)
	{
		displayChange = true;
		state = startMenu;
	}
}

void gameOverDisplay()
{
	state = startMenu;
}

bool stateChanged()
{
	for (int i = 0; i < 4; i++)
	{
		if (inputFlags[i] == 1)
			return true;
	}
	return false;
}

void writeText(String text)
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

		if (reading != lastInputStates[i])
			lastDebounceTimes[i] = millis();
		if ((millis() - lastDebounceTimes[i]) > DEBOUNCE_DELAY && reading != inputStates[i])
		{
			inputStates[i] = reading;
			inputFlags[i] = inputStates[i] == LOW ? true : false;
		}
		lastInputStates[i] = reading;
	}
}

void setPixel(int pixel, ColorSelection color)
{
	if (color == RED)
	{
		strip.setPixelColor(pixel, BRIGHTNESS, 0, 0);
	}
	else if (color == GREEN)
	{
		strip.setPixelColor(pixel, 0, BRIGHTNESS, 0);
	}
	else if (color == BLUE)
	{
		strip.setPixelColor(pixel, 0, 0, BRIGHTNESS);
	}
	else if (color == OFF)
	{
		strip.setPixelColor(pixel, 0, 0, 0);
	}
	strip.show();
}

void setAllPixels(ColorSelection color)
{
	for (int i = 0; i < NO_OF_PIXELS; i++)
	{
		setPixel(i, color);
	}
}
