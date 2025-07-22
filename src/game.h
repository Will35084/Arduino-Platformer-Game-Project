#ifndef GAME_H
#define GAME_H
#include <LiquidCrystal_I2C.h>

//defining the numbers in the level array with their corresponding numbers
#define AIR 0
#define COIN 1
#define LASER 2
#define PORTAL 3
#define BLOCK 4
#define MOVABLE_BLOCK 5
#define FAKE_BLOCK 6
#define DEBRIS 7
#define CLINGER 8
#define CRAWLER 9
#define CREEPER 10

#define BOSS_LEVEL_LENGTH 34 //amount of tiles the boss part of the level takes up
#define LEVEL_LENGTH 100 //amount of "cells" or blocks the level takes up

//The different game states
#define TITLE_SCREEN 0 //(haven't implemented yet)
#define START 1 //(Initializes all the variables & player/level structs. finished)
#define START_2 2 //(The actual playable game, working on now)
#define LOSE 3 //(the lose screen. Haven't implemented yet)
#define BOSS 4 // boss fight
#define WIN 5 // (the win screen. Haven't implemented yet)


//The buzzer which makes the sounds of the game (not implemented yet) is located at digital pin 8
#define BUZZER_PIN 8
//The joystick definitions: 
#define JOY_BUTTON 4 //the joystick "button" is at digital pin 4 (used for attacking)
#define JOY_H A0 //the horizontal left-right axis of the joystick is at analog pin A0
#define JOY_V A1 //vertical axis of joystick at analog pin A1
#define JOY_MAX 50 //Maximum sensitivity that would activate a "left" or "up" on the joystick
#define JOY_MIN 600 //minimum sensitivity that would activate a "right" or "down" on the joystick
//The pushbutton declaration (used for "actions" such as moving and jumping)
#define A_BUTTON 2 //action "A" button at digital pin 2

//used for the lenth of the notes themselves and for the delay between notes
#define LONG_SOUND_DELAY 100 
#define SHORT_SOUND_DELAY 50

//declare the lcd
extern LiquidCrystal_I2C lcd;

// keeps track of the position of the character in relation to the rest of the background and the status of the game (title screen, etc.). 
struct Game {
    byte status; //what state is the game in? Title screen, game over screen, gameplay
    struct Reference{ //keeps track of the x axis offset of the game so the background moves with the character
      int x_offset;
    };
    Reference Ref;
    byte score; //keeps track of the score of the player. Coins give 2 points, killing an enemy gives 3 points, breaking debris gives 1 point
};

extern Game Platformer;

//WriteLevel() activates whenever the player moves. It "scrolls" through a level array. It also keeps track of the coordinates for the enemies
void WriteLevel();  
void DebrisHandle();//for the debris and its behavior handling
void FakeBlockHandle();//for fake blocks and their handling
void PlayDeathSound(); //for playing the sounds of the player dying
void PlaySlashSound();//for playing the sounds of the player attacking
void PlaySounds(const int* notes); //For playing the remaining sounds

extern byte Level[6][LEVEL_LENGTH];
extern const byte TitleScreen[4][20];

//sound playing variables
extern bool playingNote;
extern int noteIndex;
extern unsigned long lastNoteTime;

//the frequencies corresponding to notes for the buzzer to make sounds
extern const int jumpNotes[2]; 
extern const int attackNotes[4]; 
extern const int deathNotes[10];
extern const int laserNotes[2];
extern const int coinNotes[2];
extern const int owchNotes[2];
extern const int killNotes[2];
extern bool playingCoin;
extern bool playingLaser;
extern bool playingJump;
extern bool playingOwch;
extern bool playingKill;
#endif