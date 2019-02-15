#include <TimerOne.h>

#define ERROR_TONE 98
#define SUCCESS_TONE 882

//consts
const int numOfPlayers = 2;
const int playersLEDs []= { 2,3,4,5,6,7};
const int playerButtons [] = {12,13,14,15};
const int startButton = 8;
const int gameLed = 9;
const long gameLedPeriod = 250000; //250ms
const long secondsToPush = 5;
const int buzzerOutput = 11;

//state
int playersScores [] = {0,0,0,0};
bool playerHasFailed [] = {false, false, false, false};
bool startButtonHasBeenPushed = false;
int gameLedState = LOW;
volatile unsigned long blinkCount = 0; // La definimos como volatile

void setPlayerOutputs()
{
    for(int i = 0; i< numOfPlayers; i++)
    {
      pinMode(playersLEDs[i*3], OUTPUT);
      pinMode(playersLEDs[i*3 + 1], OUTPUT);
      pinMode(playersLEDs[i*3 + 2], OUTPUT);
    }
}

void setOutputs()
{
    setPlayerOutputs();
    pinMode(gameLed, OUTPUT);
    pinMode(buzzerOutput, OUTPUT);
}

void setInputs()
{
    for(int i = 0; i < numOfPlayers; i++)
    {
        pinMode(playerButtons[i], INPUT);
    }  
    pinMode(startButton, INPUT);
}

bool timeOutHasPassed()
{
    return (blinkCount * gameLedPeriod / 1000) > (secondsToPush * 1000);
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
    digitalWrite(gameLed, gameLedState);
}

void InitTimer()
{
    Timer1.initialize(gameLedPeriod);
    Timer1.attachInterrupt(ISR_Blink); // Activa la interrupcion y la asocia a ISR_Blink
}

void setup()
{
    setOutputs();
    setInputs();
    InitTimer();
    noInterrupts();
    //Serial.begin(9600);
}

void setPlayerScores()
{
    for(int i = 0; i < numOfPlayers; i++)
    {
        setPlayerScore(i);        
    }
}

void setPlayerScore(int player)
{
    if (playersScores[player] == 0) {
        digitalWrite(playersLEDs[player*3], LOW);
        digitalWrite(playersLEDs[player*3 + 1], LOW);
        digitalWrite(playersLEDs[player*3 + 2], LOW);
    }
    else if (playersScores[player] == 1) {
        digitalWrite(playersLEDs[player*3], HIGH);
        digitalWrite(playersLEDs[player*3 + 1], LOW);
        digitalWrite(playersLEDs[player*3 + 2], LOW);
    }
    else if (playersScores[player] == 2) {
        digitalWrite(playersLEDs[player*3], HIGH);
        digitalWrite(playersLEDs[player*3 + 1], HIGH);
        digitalWrite(playersLEDs[player*3 + 2], LOW);
    }
    else {
        digitalWrite(playersLEDs[player*3], HIGH);
        digitalWrite(playersLEDs[player*3 + 1], HIGH);
        digitalWrite(playersLEDs[player*3 + 2], HIGH);
    }
}

bool allPlayersHaveFailed()
{  
    for(int i = 0; i < numOfPlayers; i++)
    {
        if(!playerHasFailed[i]) return false;
    }
    return true;
}

void setWaitingState()
{
    blinkCount = 0;
    noInterrupts();                 // Autoriza las interrupciones
    startButtonHasBeenPushed = digitalRead(startButton) == HIGH;
    for(int i = 0; i < numOfPlayers; i++)
    {
        playerHasFailed[i] = false;
    }        
    gameLedState = LOW;
    digitalWrite(gameLed, gameLedState);
}

void playTone(int toneFrequency)
{    
    tone(buzzerOutput, toneFrequency);
    delay(200);
    noTone(buzzerOutput);
}


void checkPlayerButton(bool timeHasPassed, int player)
{
    bool playerButton = digitalRead(playerButtons[player]) == HIGH;
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
                delay(200);
                playTone(SUCCESS_TONE);
                delay(200);
                playTone(SUCCESS_TONE);
            }
        }
    }
}

void loop()
{
    if (startButtonHasBeenPushed && !allPlayersHaveFailed())
    {
        interrupts();
        bool timeHasPassed = timeOutHasPassed();
        for(int i = 0; i < numOfPlayers; i++)
        {
            checkPlayerButton(timeHasPassed, i);    
        }    
        setPlayerScores();
    }
    else
    {
        setWaitingState();
    }
}
