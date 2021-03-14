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

short buttons[4] = {7, 6, 5, 4};
int inputStates[4];
int lastInputStates[4];
bool inputFlags[4];
long lastDebounceTimes[4] = {0, 0, 0, 0};

// Display and leds
Adafruit_WS2801 strip = Adafruit_WS2801(NO_OF_PIXELS, DATA_PIN, CLOCK_PIN);
LiquidCrystal lcd(DISPLAY_RS, DISPLAY_ENABLE, D4, D5, D6, D7);

short pattern[SIZE_OF_ARRAYS];
short turn = 0;
short selectedHighscore = 0;

String savedHighscores[3];

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

byte checkMark[8] = {
	B00000,
	B00001,
	B00010,
	B00010,
	B00100,
	B10100,
	B01000,
	B01000};

bool buttonPressed = true;
bool displayChange = true;

void setup()
{
	Serial.begin(9600);

	//To set the scores the first time, uncomment this part.
	savedHighscores[0] = "000000:000";
	savedHighscores[1] = "111111:000";
	savedHighscores[2] = "222222:000";

	String highscoresToSave = savedHighscores[0] + ";" + savedHighscores[1] + ";" + savedHighscores[2];
	saveHighScores(0, highscoresToSave);

	//String loadedHighscores = loadHighScores(0);
	// savedHighscores[0] = getValue(loadedHighscores, ';', 0);
	// savedHighscores[1] = getValue(loadedHighscores, ';', 1);
	// savedHighscores[2] = getValue(loadedHighscores, ';', 2);

	Serial.println(savedHighscores[0]);
	Serial.println(savedHighscores[1]);
	Serial.println(savedHighscores[2]);

	randomSeed(analogRead(0));
	randomizePattern();

	state = startMenu;

	for (int i = 0; i < 4; i++)
	{
		pinMode(buttons[i], INPUT_PULLUP);
	}

	for (int i = 0; i < 4; i++)
	{
		lastInputStates[i] = HIGH;
		inputFlags[i] = LOW;
		lastDebounceTimes[i] = 0;
	}

	lcd.createChar(0, arrowUp);
	lcd.createChar(1, arrowDown);
	lcd.createChar(2, arrowLeft);
	lcd.createChar(3, arrowRight);
	lcd.createChar(4, checkMark);

	lcd.begin(16, 2);

	strip.begin();
	// for (int i = 0; i < NO_OF_PIXELS; i++)
	// {
	// 	strip.setPixelColor(i, 0, 0, BRIGHTNESS);
	// 	if (i > 0)
	// 	{
	// 		strip.setPixelColor(i - 1, 0, 0, 0);
	// 	}
	// 	strip.show();
	// 	delay(1000);
	// }
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
		// gamePlay();
		enterNameDisplay(12);
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
		setAllPixels(OFF);
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

// Display scrollable list with top 3 highscores
void highscoreList()
{
	setPixel(0, BLUE);
	setPixel(1, BLUE);
	setPixel(3, BLUE);
	if (displayChange)
	{
		lcd.clear();
		String score = (String)(selectedHighscore + 1);
		score.concat(".");
		score.concat(highscoreLine(savedHighscores[selectedHighscore]));
		lcd.print(score);
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
		displayChange = true;
	}

	// Down button
	if ((inputFlags[1] == true) && selectedHighscore < 2)
	{
		selectedHighscore++;
		displayChange = true;
	}

	// Back button
	if (inputFlags[3] == true)
	{
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
	// TODO Create game over menu
	state = startMenu;
}

void enterNameDisplay(int score)
{
	// TODO add logic for entering name on new highscore
	String name = "";
	char newChar = 65;

	setAllPixels(GREEN);
	lcd.clear();
	lcd.print("Press button");
	lcd.setCursor(0, 1);
	lcd.print("to enter name");

	while (!stateChanged())
	{
		buttonRead();
	}
	setAllPixels(OFF);

	while (name.length() <= 10)
	{
		if (stateChanged())
		{
			lcd.clear();
			lcd.print(name);
			lcd.print(newChar);
			lcd.setCursor(0, 1);
			lcd.write(byte(0));
			lcd.setCursor(4, 1);
			lcd.write(byte(1));
			lcd.setCursor(9, 1);
			lcd.write(byte(3));
			lcd.setCursor(15, 1);
			lcd.write(byte(4));
		}

		buttonRead();

		// Up button
		if ((inputFlags[0] == true) && newChar < 90)
		{
			newChar++;
		}

		// Down button
		if ((inputFlags[1] == true) && newChar > 65)
		{
			newChar--;
		}

		// Check button
		if ((inputFlags[2] == true))
		{
			name.concat(newChar);
		}

		// Back button
		if (inputFlags[3] == true && name.length() > 0)
		{
			break;
		}
	}

	String newHighscoreString = name;
	newHighscoreString.concat(":");
	newHighscoreString.concat(score);

	addScore(newHighscoreString);

	state = highscore;
}

void addScore(String newScore)
{
	if (getValue(newScore, ':', 1).toInt() > (int)getValue(savedHighscores[2], ':', 1).toInt())
	{
		savedHighscores[2] = newScore;

		writeText("Higscore added");
		lcd.setCursor(0, 1);
		lcd.print("Press red button");

		while (inputFlags[3] != 1)
		{
			buttonRead();
		}
	}

	sortHighscores();
}

void sortHighscores()
{
	if (getValue(savedHighscores[1], ':', 1).toInt() < getValue(savedHighscores[2], ':', 1).toInt())
	{
		String temp = savedHighscores[1];
		savedHighscores[1] = savedHighscores[2];
		savedHighscores[2] = temp;
	}

	if (getValue(savedHighscores[0], ':', 1).toInt() < getValue(savedHighscores[1], ':', 1).toInt())
	{
		String temp = savedHighscores[0];
		savedHighscores[0] = savedHighscores[1];
		savedHighscores[1] = temp;
	}

	if (getValue(savedHighscores[1], ':', 1).toInt() < getValue(savedHighscores[2], ':', 1).toInt())
	{
		String temp = savedHighscores[1];
		savedHighscores[1] = savedHighscores[2];
		savedHighscores[2] = temp;
	}
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
	for (int i = 0; i < 4; i++)
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

void saveHighScores(int addrOffset, const String &strToWrite)
{
	Serial.println("Inside save highscores");
	byte len = strToWrite.length();
	Serial.println("length of string = " + (String)len);
	EEPROM.write(addrOffset, len);
	for (int i = 0; i < len; i++)
	{
		EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
	}
}

String loadHighScores(int addrOffset)
{
	Serial.println("Inside loadHigscores");
	int newStrLen = EEPROM.read(addrOffset);
	Serial.println("length specified in eeprom = " + (String)newStrLen);
	char data[newStrLen + 1];
	for (int i = 0; i < newStrLen; i++)
	{
		data[i] = EEPROM.read(addrOffset + 1 + i);
	}
	data[newStrLen] = '\0';
	return String(data);
}

// Taken from Stackoverflow(Thank you Alvaro):
// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
	int found = 0;
	int strIndex[] = {0, -1};
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++)
	{
		if (data.charAt(i) == separator || i == maxIndex)
		{
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}

	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String highscoreLine(String s)
{
	int stringLength = s.length() - 1;
	String output = getValue(s, ':', 0);
	String whiteSpace = "";
	for (int i = 0; i < (14 - stringLength); i++)
	{
		whiteSpace += " ";
	}
	output.concat(whiteSpace);
	output.concat(getValue(s, ':', 1));

	return output;
}
