#ifndef MOVEMENT_H
#define MOVEMENT_H
#include <LiquidCrystal_I2C.h>
#define FALL_DELAY 150

void ChangeXPos(); //depending on the circumstances, ChangeXPos() moves the player across the screen or moves the level array with WriteLevel()
void JumpHandle(); //JumpHandle manages the process of a jump & jump animation if the player.Jump flag is set to true
void WalkSprintHandle(); //If the player is walking or sprinting, this handles those flags
void FallHandle(); //If the player is falling, this handles that flag

//Direction definitions. 
extern const int LEFT_DIR;
extern const int RIGHT_DIR;
extern const int STRAIGHT;


//for using millis() to keep track of time rather than using delays. 
extern unsigned long currentTime;
extern unsigned long lastJumpTime; // For keeping track of the last time a jump was made with the millis() function
extern unsigned long lastFallTime; // for keeping track of when the last time the player fell a tile was
extern unsigned long lastWalkTime; //For keeping track of when the last time the player walked
extern unsigned long sprintTimer; //for properly timing sprints

//The jumping definitions
extern byte jumpProcess; //determines how far along the jump is. 
extern const int jumpDelay[5]; //holds the delay for each stage of the jump (large delay at peak of jump)

//walking definition
extern byte walkProcess; //keep track of what stage walking animation is at

#endif