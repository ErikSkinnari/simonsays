#include "LiquidCrystal.h"
#include "EEPROM.h"
#include <APA102.h>

// Pin number constants
const uint8_t CLOCK_PIN = 14;
const uint8_t DATA_PIN = 15;
const short NO_OF_PIXELS = 4;
const short D4 = 6;
const short D5 = 7;
const short D6 = 8;
const short D7 = 9;
const short DISPLAY_ENABLE = 5;
const short DISPLAY_RS = 4;
const short DEBOUNCE_DELAY = 50;
const short SIZE_OF_ARRAYS = 0;
const short BRIGHTNESS = 40;
const short BUTTON_1 = 3;
const short BUTTON_2 = 2;
const short BUTTON_3 = 17;
const short BUTTON_4 = 16;
const short BUTTON_RS = 10;

short buttons[5] = {BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_RS};
int inputStates[5];
int lastInputStates[5];
bool inputFlags[5];
long lastDebounceTimes[5] = {0, 0, 0, 0, 0};

// Display and leds
APA102<DATA_PIN, CLOCK_PIN> ledStrip;
rgb_color colors[NO_OF_PIXELS];
LiquidCrystal lcd(DISPLAY_RS, DISPLAY_ENABLE, D4, D5, D6, D7);

int pattern[SIZE_OF_ARRAYS];
int turn = 0;
int playerScore = 0;
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

  String loadedHighscores = loadHighScores(0);
  savedHighscores[0] = getValue(loadedHighscores, ';', 0);
  savedHighscores[1] = getValue(loadedHighscores, ';', 1);
  savedHighscores[2] = getValue(loadedHighscores, ';', 2);

  randomSeed(analogRead(19));

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
  Serial.print("main menu");
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

  while (true)
  {
    buttonRead();

    if (inputFlags[0] == true)
    {
      setAllPixels(OFF);
      state = game;
      displayChange = true;
      break;
    }

    if (inputFlags[3] == true)
    {
      setAllPixels(OFF);
      state = highscore;
      displayChange = true;
      break;
    }
  }
}

// Display scrollable list with top 3 highscores
void highscoreList()
{
  Serial.print("Highscore list");
  if (displayChange)
  {
    setAllPixels(OFF);
    setPixel(0, BLUE);
    setPixel(1, BLUE);
    setPixel(3, BLUE);
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

  buttonRead();

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
  Serial.print("Game!");
  randomizePattern();
  turn = 0;
  playerScore = 0;

  writeText("    New Game    ");
  lcd.setCursor(0, 1);
  lcd.print(" Press to Start ");
  setAllPixels(GREEN);
  waitForAnyButton();

  bool gameRunning = true;

  writeText(" Follow pattern ");
  setAllPixels(OFF);
  setAllPixels(GREEN);
  delay(100);
  setAllPixels(OFF);
  delay(1000);

  while (gameRunning)
  {
    for (int i = 0; i <= turn; i++)
    {
      setPixel(pattern[i], BLUE);
      delay(200);
      setAllPixels(OFF);
      delay(300);
    }

    for (int i = 0; i <= turn; i++)
    {
      waitForAnyButton();

      if (inputFlags[pattern[i]] != 1)
      {
        gameRunning == false;
        playerScore = turn;
        gameOverDisplay();
        return;
      }
      else
      {
        setPixel(pattern[i], BLUE);
        delay(100);
        setAllPixels(OFF);
      }
    }
    turn++;
    delay(1000);
  }
}

void gameOverDisplay()
{
  Serial.println("GAME OVER!");

  // Flash leds
  writeText("   Game Over!   ");
  for (int i = 0; i < 5; i++)
  {
    setAllPixels(RED);
    delay(200);
  }

  // Check if highscore
  if (playerScore > getValue(savedHighscores[2], ':', 1).toInt())
  {
    enterNameDisplay();
    return;
  }
  else
  {
    lcd.clear();
    String score = "Score: ";
    score.concat(playerScore);
    lcd.print(score);
    lcd.setCursor(0, 1);
    lcd.print("Press any button");
    waitForAnyButton();
  }
  state = startMenu;
}

void enterNameDisplay()
{
  String name = "";
  char newChar = 65;

  setAllPixels(GREEN);
  lcd.clear();
  lcd.print("Press button");
  lcd.setCursor(0, 1);
  lcd.print("to enter name");
  waitForAnyButton();

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

    // Down button
    if ((inputFlags[0] == true) && newChar > 48)
    {
      newChar--;
    }

    // Up button
    if ((inputFlags[1] == true) && newChar < 90)
    {
      newChar++;
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
  newHighscoreString.concat(playerScore);
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

    waitForButtonPressed(3);
    state = startMenu;
    mainMenu();
  }

  sortHighscores();
  String highscoresToSave = savedHighscores[0] + ";" + savedHighscores[1] + ";" + savedHighscores[2];
  saveHighScores(0, highscoresToSave);
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

void waitForButtonPressed(int buttonIndex)
{
  while (inputFlags[buttonIndex] != 1)
  {
    buttonRead();
  }
}

void waitForAnyButton()
{
  buttonRead();
  while (!stateChanged())
  {
    buttonRead();
  }
}

void resetHighscores()
{
  lcd.clear();
  lcd.print("Reset highscores");
  lcd.setCursor(0, 1);
  lcd.print("Press code");

  waitForAnyButton();
  if (inputFlags[3] != 1)
    mainMenu();
  if (inputFlags[0] != 1)
    mainMenu();
  if (inputFlags[3] != 1)
    mainMenu();
  if (inputFlags[2] != 1)
    mainMenu();

  savedHighscores[0] = " : ";
  savedHighscores[1] = " : ";
  savedHighscores[2] = " : ";

  String highscoresToSave = savedHighscores[0] + ";" + savedHighscores[1] + ";" + savedHighscores[2];
  saveHighScores(0, highscoresToSave);

  lcd.clear();
  lcd.print("Cleared");

  state = startMenu;
}

void setPixel(int pixel, ColorSelection color)
{

  colors[pixel].red = 0;
  colors[pixel].green = 0;
  colors[pixel].blue = 0;

  if (color == RED)
  {
    colors[pixel].red = 255;
  }
  else if (color == GREEN)
  {
    colors[pixel].green = 255;
  }
  else if (color == BLUE)
  {
    colors[pixel].blue = 255;
  }

  ledStrip.write(colors, 4, 31);
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
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String loadHighScores(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
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
  int strIndex[] = {
      0,
      -1};
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