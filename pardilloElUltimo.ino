#include <TimerOne.h>

#define ERROR_TONE 98
#define SUCCESS_TONE 882

//Musical Note Frequencies
#define C6 1047
#define Ab5 831
#define Bb5 932
#define G5 784
#define F5 698
#define E5 659
#define Eb5 622
#define D5 587
#define Db5 554
#define C5 523
#define B4 494
#define Bb4 466
#define nA4 440
#define Ab4 415
#define G4 392
#define Gb4 370
#define F4 349
#define E4 330
#define Eb4 311
#define D4 294
#define Db4 277
#define C4 262
#define B3 247
#define Bb3 233
#define nA3 220
#define G3 196
#define Gb3 185
#define F3 175
#define E3 165

//consts
const int NUM_OF_PLAYERS = 2;
const int PLAYERS_LEDS []= { 2,3,4,5,6,7};
const int START_BUTTON = 8;
const int GAME_LED = 9;
const int BUZZER_OUTPUT = 11;
const int PLAYER_BUTTONS [] = {12,13,14,15};
const int ANALOG_RANDOM_SEED_PIN = 5; //A05 must not be connected
const long GAME_LED_PERIOD = 250000; //250ms

const int melody[] = {G3, C4, E4, G4, C5, E5, G5, E5,
                      E3, C4, Eb4, Ab4, C5, Eb5, Ab5, Eb5,
                      Bb3, D4, F4, Bb4, D5, F5, Bb5, 0, Bb5, 0, Bb5, 0, Bb5, C6};
//Mario main them tempo
const int tempo[] = {166, 166, 166, 166, 166, 166, 500, 500,
                     166, 166, 166, 166, 166, 166, 500, 500,
                     166, 166, 166, 166, 166, 166, 500, 50, 166, 50, 166, 50, 166, 2000};

//state
int playersScores [] = {0,0,0,0};
bool playerHasFailed [] = {false, false, false, false};
bool startButtonHasBeenPushed = false;
int gameLedState = LOW;
volatile unsigned long blinkCount = 0; // La definimos como volatile
long secondsToPush = 5;
bool readyToStartTurn = false;

void setPlayerOutputs()
{
    for(int i = 0; i< NUM_OF_PLAYERS; i++)
    {
      pinMode(PLAYERS_LEDS[i*3], OUTPUT);
      pinMode(PLAYERS_LEDS[i*3 + 1], OUTPUT);
      pinMode(PLAYERS_LEDS[i*3 + 2], OUTPUT);
    }
}

void setOutputs()
{
    setPlayerOutputs();
    pinMode(GAME_LED, OUTPUT);
    pinMode(BUZZER_OUTPUT, OUTPUT);
}

void setInputs()
{
    for(int i = 0; i < NUM_OF_PLAYERS; i++)
    {
        pinMode(PLAYER_BUTTONS[i], INPUT);
    }  
    pinMode(START_BUTTON, INPUT);
}

bool timeOutHasPassed()
{
    return (blinkCount * GAME_LED_PERIOD / 1000) > (secondsToPush * 1000);
}

void ISR_Blink()
{
    if (startButtonHasBeenPushed && !timeOutHasPassed())
    {
        gameLedState = !gameLedState;
        blinkCount++;
    }
    else
    {
        gameLedState = LOW;
    }
    digitalWrite(GAME_LED, gameLedState);
}

void initTimer()
{
    Timer1.initialize(GAME_LED_PERIOD);
    Timer1.attachInterrupt(ISR_Blink); // Activa la interrupcion y la asocia a ISR_Blink
}

void checkLEDS()
{
    for(int i = 0; i< NUM_OF_PLAYERS; i++)
    {
        digitalWrite(PLAYERS_LEDS[i*3], HIGH);
        delay(200);
        digitalWrite(PLAYERS_LEDS[i*3 + 1], HIGH);
        delay(200);
        digitalWrite(PLAYERS_LEDS[i*3 + 2], HIGH);
        delay(200);
        digitalWrite(PLAYERS_LEDS[i*3], LOW);
        delay(200);
        digitalWrite(PLAYERS_LEDS[i*3 + 1], LOW);
        delay(200);
        digitalWrite(PLAYERS_LEDS[i*3 + 2], LOW);
        delay(200);      
    }
}

void setup()
{
    setOutputs();
    setInputs();
    checkLEDS();
    initTimer();
    noInterrupts();
    randomSeed(analogRead(ANALOG_RANDOM_SEED_PIN));
    secondsToPush = random(3,8);    
    Serial.println(secondsToPush);
    Serial.begin(9600);
}

void setPlayerScores()
{
    for(int i = 0; i < NUM_OF_PLAYERS; i++)
    {
        setPlayerScore(i);        
    }
}

void setPlayerScore(int player)
{
    if (playersScores[player] == 0) {
        digitalWrite(PLAYERS_LEDS[player*3], LOW);
        digitalWrite(PLAYERS_LEDS[player*3 + 1], LOW);
        digitalWrite(PLAYERS_LEDS[player*3 + 2], LOW);
    }
    else if (playersScores[player] == 1) {
        digitalWrite(PLAYERS_LEDS[player*3], HIGH);
        digitalWrite(PLAYERS_LEDS[player*3 + 1], LOW);
        digitalWrite(PLAYERS_LEDS[player*3 + 2], LOW);
    }
    else if (playersScores[player] == 2) {
        digitalWrite(PLAYERS_LEDS[player*3], HIGH);
        digitalWrite(PLAYERS_LEDS[player*3 + 1], HIGH);
        digitalWrite(PLAYERS_LEDS[player*3 + 2], LOW);
    }
    else {
        digitalWrite(PLAYERS_LEDS[player*3], HIGH);
        digitalWrite(PLAYERS_LEDS[player*3 + 1], HIGH);
        digitalWrite(PLAYERS_LEDS[player*3 + 2], HIGH);
    }
}

bool allPlayersHaveFailed()
{  
    for(int i = 0; i < NUM_OF_PLAYERS; i++)
    {
        if(!playerHasFailed[i]) return false;
    }
    return true;
}

void setWaitingState()
{
    blinkCount = 0;
    noInterrupts();                 // Autoriza las interrupciones    
    for(int i = 0; i < NUM_OF_PLAYERS; i++)
    {
        playerHasFailed[i] = false;
    }        
    gameLedState = LOW;
    secondsToPush = random(3,8);
    Serial.println(secondsToPush);
    digitalWrite(GAME_LED, gameLedState);
}

void playTone(int toneFrequency)
{    
    tone(BUZZER_OUTPUT, toneFrequency);
    delay(200);
    noTone(BUZZER_OUTPUT);
}


void checkPlayerButton(bool timeHasPassed, int player)
{
    bool playerButton = digitalRead(PLAYER_BUTTONS[player]) == HIGH;
    if (playerButton && !playerHasFailed[player])
    {
        if (!timeHasPassed)
        {
            if (playersScores[player] > 0) playersScores[player] --;
            playerHasFailed[player] = true;
            playTone(ERROR_TONE);
        }
        else
        {
            playersScores[player] ++;
            startButtonHasBeenPushed = false;
            playTone(SUCCESS_TONE);
            if(playersScores[player] == 3) 
            {
                sing();
                //delay(200);
                //playTone(SUCCESS_TONE);
                //delay(200);
                //playTone(SUCCESS_TONE);
            }
        }
    }
}

void sing() {
    int size = sizeof(melody) / sizeof(int);
    for (int thisNote = 0; thisNote < size; thisNote++) {
      if(melody[thisNote] > 0) tone(BUZZER_OUTPUT, melody[thisNote]);
      else noTone(BUZZER_OUTPUT);
      delay(tempo[thisNote]);
    }
    noTone(BUZZER_OUTPUT);
}

void loop()
{
    if (startButtonHasBeenPushed && !allPlayersHaveFailed())
    {
        interrupts();
        bool timeHasPassed = timeOutHasPassed();
        for(int i = 0; i < NUM_OF_PLAYERS; i++)
        {
            checkPlayerButton(timeHasPassed, i);    
        }    
        setPlayerScores();
        readyToStartTurn = false;
    }
    else if (!readyToStartTurn)
    {
        setWaitingState();
        readyToStartTurn = true;
    }
    else
    {
        startButtonHasBeenPushed = digitalRead(START_BUTTON) == HIGH;
    }    
}
