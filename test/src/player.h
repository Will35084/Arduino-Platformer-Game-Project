#ifndef PLAYER_H
#define PLAYER_H
#include <LiquidCrystal_I2C.h>


//Hero struct is the player's many attributes. 
struct Hero {

  unsigned short Speed_Delay; //The delay before character moves to the next block when holding in the movement button

  //Two sets of X and Y positions are recorded to keep track of where the background should be positioned
  byte X_POS; //the X position of the character in the game
  byte Y_POS; //The Y position of the character in the game
  byte X_LCD; //The X position of the character on the LCD
  byte Y_LCD; //The X position of the character on the LCD

  //To keep track of whether the hero is facing to the left, right, or diagonally
  int Dir; 
  //player's lives
  int lives;

  //various bool definitions
  bool Jump; //To keep track of when the character is jumping
  bool Slash; //To keep track of whether the character is doing an attack
  bool Crawl; //To keep track of whether the character is crawling
  bool Sprint; //to keep track of whether the character is sprinting
  bool kill; //if the player has killed an enemy (jumped on crawler, attacked creeper), this will be the flag to indicate that
  bool falling; //to check if the player is falling
  bool walking; //to keep track if the player is walking
};

void SlashHandle();
void StationaryControlsCheck();
void MovementControlsCheck();
void damageLivesHandle();
extern Hero Player;


extern byte hero_J[8]; //sprite of hero jumping
extern byte hero_A[8]; //hero facing straight (used when player isn't facing left or right)
extern byte hero_L[8]; //hero facing/attacking to the left
extern byte hero_R[8]; //hero facing/attacking to the right
extern byte hero_UL[8]; //hero facing up-left
extern byte hero_UR[8]; //hero facing up-right
extern byte hero_U[8];//Shows the player "crawling" under the clinger

extern int slashType;
extern unsigned long lastSlashTime;
extern int slashTotal;
extern int slashX;
#endif