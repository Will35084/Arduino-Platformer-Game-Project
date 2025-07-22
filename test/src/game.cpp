#include "game.h"
#include "enemy.h"
#include "player.h"
#include "movement.h"
#include <LiquidCrystal_I2C.h> //for the LCD

//initialize the game struct "Platformer"
Game Platformer = {0,{0},0};

//Initialize I2C LCD (Address: 0x27 or 0x27, Columns: 20, Rows: 4)
LiquidCrystal_I2C lcd(0x27, 20, 4);

//The implementation of the writelevel function. Shifts the level and keeps track of crawlers and creepers
void WriteLevel() {
  //screenBuffer is an array where each row is printed to the lcd
  char screenBuffer[4][21]; // 20 columns x 4 rows + null terminators
  memset(screenBuffer, ' ', sizeof(screenBuffer));

  //if Crawler_1.onScreen is never set to true in the for loops, then there is no crawler on screen
  Crawler_1.onScreen = false;
  Crawler_2.onScreen = false;
  Creeper_1.onScreen = false;
  Creeper_2.onScreen = false;
  //reset creeper and crawler count for each time writeLevel() is called
  craCount = 0;
  creCount = 0;

  //If the boss level starts, alter the x offset to "teleport" the player to the boss area
  if (Platformer.status == BOSS && Platformer.Ref.x_offset < LEVEL_LENGTH-BOSS_LEVEL_LENGTH){
    Platformer.Ref.x_offset = LEVEL_LENGTH - BOSS_LEVEL_LENGTH; 
  }

  //setting debscreen to false at the start of function. The function not getting 
  //set to true means the debris found is not already recorded in DebrisArr[]
  bool debScreen = false;
  
  //nested for loop which goes through the entire LCD screen and replaces 
  //the current tile with the one adjacent in the direction of the offset
  
  for (byte x = 0; x < 20; x++){
    for (byte y = 0; y < 4; y++){
      int currXPOS = x + Platformer.Ref.x_offset;
      int currYPOS = y + 1;

      //go through the whole level array on the lcd screen
      switch (Level[currYPOS][currXPOS]) { //y + 1 since level array has invisible blocks above the player

        case AIR: { //if air, write a space or clear the tile
          screenBuffer[y][x] = ' ';
          break;
        }
        case BLOCK: { //if a block, draw the premade sprite 0xFF
          screenBuffer[y][x] = (char)0xFF;
          break;
        }
        case DEBRIS: {
          //loop through the Debris array. If the debris found is the same as one recorded 
          //in the array, update that debris's coordinates instead of making a new debris
          for (int i = 0; i < MAX_DEBRIS; i++){
            if(DebrisArr[i].onScreen && DebrisArr[i].X_POS == currXPOS){
              DebrisArr[i].X_POS = currXPOS;
              DebrisArr[i].X_LCD = x;
              screenBuffer[y][x] = (char)0xDB;
              debScreen = true; //the debris is already on screen
              break;
            }
          }
          if(!debScreen){ //if no coordinates match, it is a new debris and should be added to array
            for(int j = 0; j<MAX_DEBRIS; j++){
              if(!DebrisArr[j].onScreen){ //at the first open slot, set the debris parameters in
                DebrisArr[j].onScreen = true;
                DebrisArr[j].X_POS = currXPOS;
                DebrisArr[j].Y_POS = currYPOS;
                DebrisArr[j].X_LCD = x;
                DebrisArr[j].Y_LCD = y;
                screenBuffer[y][x] = (char)0xDB;
                break;
              }
            }
          }
          debScreen = false; //reset debScreen for the next debris in Debris
          break;
        }
        case COIN: {//if a coin, draw the premade sprite 'o'
          screenBuffer[y][x] = 'o';
          break;
        }
        case CRAWLER: {//if a crawler in the level array
          //If 0 crawlers on screen, coordinates go to crawler1, otherwise, goes to crawler2
          Enemy* c = (craCount == 0) ? &Crawler_1 : (craCount == 1) ? &Crawler_2 : nullptr;
          if(c){
            //if crawler's direction is straight, it was just born and needs to be initialized
            if(c->Dir == STRAIGHT){
              c->Dir = RIGHT_DIR;
              c->lastEventTime = currentTime;
            }
            //set crawler's stuff
            c->onScreen = true;
            c->X_POS = currXPOS;
            c->Y_POS = currYPOS;
            c->X_LCD = x;
            c->Y_LCD = y;
            craCount ++;
            screenBuffer[y][x] = (char)5;
          }
          break;
        }
        case CREEPER: {//if a creeper, draw the custom creeper sprite

          //since the creeper sprite is replaced with the boss sprite in the 
          //boss area, creeper only happens before the boss area is reached
          if (Platformer.status == START_2){ 
            //If 0 creepers on screen, coordinates go to creeper1, otherwise, goes to creeper2
            Enemy* c = (creCount == 0) ? &Creeper_1 : (creCount == 1) ? &Creeper_2 : nullptr;
            if(c){
              //if crawler's direction is straight, it was just born and needs to be initialized
              if(c->Dir == STRAIGHT){
                c->Dir = RIGHT_DIR;
                c->lastEventTime = currentTime;
              }
              //set crawler's stuff
              c->onScreen = true;
              c->X_POS = currXPOS;
              c->Y_POS = currYPOS;
              c->X_LCD = x;
              c->Y_LCD = y;
              creCount ++;
              screenBuffer[y][x] = (char)6;
            }
            break;
          }
          else{ //if it's the boss area, if the boss is just spawning in, initialize its onScreen and Direction
            if(Boss.Dir == STRAIGHT){
              Boss.onScreen = true;
              Boss.Dir = LEFT_DIR;
            }
            //record boss's x and y coordinates
            Boss.X_LCD = x;
            Boss.Y_LCD = y;
            Boss.X_POS = currXPOS;
            Boss.Y_POS = currYPOS;
            screenBuffer[y][x] = (char)6;
            break;
          }
        }
        case CLINGER: {//if a clinger, draw the custom clinger sprite
          screenBuffer[y][x] = (char)7;
          break;
        }
        case LASER: {
          screenBuffer[y][x] = '-';
          break;
        }
        case FAKE_BLOCK: {
          //loop through the fake block array. If the fake block found is the same as one recorded in the array,
          // update that fake block's coordinates instead of making a new fake block
          for (int i = 0; i < MAX_FAKE_BLOCK; i++){
            if(FakeBlockArr[i].onScreen && FakeBlockArr[i].X_POS == currXPOS){
              FakeBlockArr[i].X_POS = currXPOS;
              FakeBlockArr[i].X_LCD = x;
              screenBuffer[y][x] = 0xFF;
              debScreen = true; //the fake block is already on screen
              break;
            }
          }
          //reusing debScreen for the fake blocks (cause what the heck I do what I want)
          if(!debScreen){ //if no coordinates match, it is a new fake block and should be added to array
            for(int j = 0; j<MAX_FAKE_BLOCK; j++){
              if(!FakeBlockArr[j].onScreen){ //at the first open slot, put the array coordinates in
                FakeBlockArr[j].onScreen = true;
                FakeBlockArr[j].X_POS = currXPOS;
                FakeBlockArr[j].Y_POS = currYPOS;
                FakeBlockArr[j].X_LCD = x;
                FakeBlockArr[j].Y_LCD = y;
                screenBuffer[y][x] = 0xFF;
                break;
              }
            }
          }
          debScreen = false;
          break;
        }
        case MOVABLE_BLOCK: {
        //if there is air below it, it falls down one tile
          if(Level[y+2][currXPOS] == AIR){
            
            screenBuffer[y][x] = ' ';
            Level[y+2][currXPOS] = MOVABLE_BLOCK;
            Level[y+1][currXPOS] = AIR;
          }
          else{
            screenBuffer[y][x] = '0';
          }
          break;
        }
        case PORTAL: {//if a portal is on screen, make it a question mark
          screenBuffer[y][x] = '?';
          break;
        }
      }
    }
  }
  for (byte y = 0; y < 4; y++) {
    //this if statement skips the specific tile of the player
    if(y == Player.Y_LCD){
      char screenBuffer_1[21];
      for(int i = Player.X_LCD-1; i < 20; i++){
        screenBuffer_1[i - Player.X_LCD-1] = screenBuffer[y][i];
      }
      //the null terminate is at player's x position so won't be printed
      screenBuffer[y][Player.X_LCD] = '\0'; // Null terminate
      screenBuffer_1[20- Player.X_LCD - 1] = '\0';
      lcd.setCursor(0, y);
      lcd.print(screenBuffer[y]);
      lcd.setCursor(Player.X_LCD+1, y);
      lcd.print(screenBuffer_1);
    }
    else{
      screenBuffer[y][20] = '\0'; // Null terminate
      lcd.setCursor(0, y);
      lcd.print(screenBuffer[y]); 
    }
  }
}

//debris handling has three parts. One part to detect if the block underneath the player is debris, another to check if debris is offscreen, and another to break debris
void DebrisHandle(){
  if(Level[Player.Y_POS+1][Player.X_POS] == DEBRIS){ //checks if the tile below the player is debris
    for(int i = 0; i < MAX_DEBRIS; i++){ //for loop finds which debris in the array of on-screen debris is the one the player is on
      if(DebrisArr[i].X_POS == Player.X_POS && DebrisArr[i].Y_POS == Player.Y_POS + 1){
        if(!DebrisArr[i].eventBool){ //to indicate the debris is stepped on and needs to be destroyed
          DebrisArr[i].lastEventTime = currentTime; //starts the timer which ends with the debris's destruction
          DebrisArr[i].eventBool = true;
        }
        break;
      }
    }
  }
  //second part of debris handling checks if debris is offscreen. If the debris is, free up an element in the array
  for(int i = 0; i < MAX_DEBRIS; i++){ 
    if(DebrisArr[i].X_LCD < 0 || DebrisArr[i].X_LCD > 19 || Level[DebrisArr[i].Y_POS][DebrisArr[i].X_POS] != DEBRIS){
      DebrisArr[i].onScreen = false;
    }
    //third part of debris handling checks if the debris's time has run out. If so, break the debris
    if(DebrisArr[i].eventBool && DebrisArr[i].onScreen){ //checks if the debris has been stepped on
      if(currentTime - DebrisArr[i].lastEventTime >= 1000){ //if the timer runs out, the debris is removed from the game
        Level[DebrisArr[i].Y_POS][DebrisArr[i].X_POS] = AIR;
        DebrisArr[i].onScreen = false;
        DebrisArr[i].eventBool = false;
        lcd.setCursor(DebrisArr[i].X_LCD, DebrisArr[i].Y_LCD);
        lcd.write(' ');
      }
    }
  }
}

//for handling fake blocks
void FakeBlockHandle(){
  for(int i = 0; i < MAX_FAKE_BLOCK; i++){
    //if the player is close to a fake block in the aray (and it is on screen), delete the block and set it to debris
    if(abs(Player.X_POS - FakeBlockArr[i].X_POS) <= 1 && FakeBlockArr[i].onScreen){
      FakeBlockArr[i].onScreen = false;
      Level[FakeBlockArr[i].Y_POS][FakeBlockArr[i].X_POS] = DEBRIS;
      lcd.setCursor(FakeBlockArr[i].X_LCD, FakeBlockArr[i].Y_LCD);
      lcd.write(0xDB);
    }
    //reset the fake block array element if offscreen
    if(FakeBlockArr[i].X_LCD < 0 || FakeBlockArr[i].X_LCD > 19 || Level[FakeBlockArr[i].Y_POS][FakeBlockArr[i].X_POS] != FAKE_BLOCK){
      FakeBlockArr[i].onScreen = false;
    }
  }
}

//plays death sound
void PlayDeathSound(){
  for(int i : deathNotes){
    tone(BUZZER_PIN,i,LONG_SOUND_DELAY); //goes through death notes array with a delay to make the sound
    delay(LONG_SOUND_DELAY);
  }
  noTone(BUZZER_PIN);
}
//same process for slash sound
void PlaySlashSound(){
  for(int i : attackNotes){
    tone(BUZZER_PIN,i,SHORT_SOUND_DELAY);
    delay(SHORT_SOUND_DELAY);
  }
  noTone(BUZZER_PIN);
}

void PlaySounds(const int* notes){
  if(noteIndex < 2){ //hard coded 2 since there are 2 elements in each notes array this function would use
    //if a note is being played past its duration, stop playing the note
    if(playingNote && currentTime - lastNoteTime >= SHORT_SOUND_DELAY){
      noTone(BUZZER_PIN);
      lastNoteTime = currentTime;
      playingNote = false;
    }

    //if the delay between two notes is longer than it should be, play the next note
    else if (!playingNote && currentTime - lastNoteTime >= SHORT_SOUND_DELAY){
      tone(BUZZER_PIN, notes[noteIndex], SHORT_SOUND_DELAY);
      lastNoteTime = currentTime;
      playingNote = true;
      noteIndex ++;
    }
  }
  else{
    playingCoin = false;
    playingLaser = false;
    playingJump = false;
    playingKill = false;
    playingOwch = false;
    noteIndex = 0;
  }
}


//The level Array & key

//AIR 0
//COIN 1
//LASER 2
//PORTAL 3
//BLOCK 4
//MOVABLE_BLOCK 5
//FAKE_BLOCK 6
//DEBRIS 7
//CLINGER 8
//CRAWLER 9
//CREEPER 10
byte Level[6][LEVEL_LENGTH] = { 
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,4,4,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,4,4,4,0,0,0,4,4,4,0,0,0,0,0,0},
  {4,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,4,4,0,0,0,0,0,0,0,4,4,0,5,0,0,0,4,4,4,0,4,4,0,0,0,0,0,0,4,4,0,0,0,0,0},
  {4,0,0,4,0,0,0,0,0,0,0,0,7,7,0,0,7,0,7,7,4,0,6,0,4,4,4,0,0,0,0,0,0,4,0,0,0,4,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,4,4,0,0,0,0,0,0,0,4,4,4,4,4,0,0,0,0,0,10,4,4,4,0,0,0,4,4,4,4,0,0,0,0,0},
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,8,8,8,8,8,8,8,8,4,4,4,8,4,4,4,0,4,4,0,0,4,4,4,0,9,0,9,4,8,8,8,4,0,0,0,0,0,10,0,0,0,0,0,0,4,4,4,4,3,4,4,0,0,0,0,0,0,0,4,4,0,0,5,0,0,0,0,0,8,7,0,0,3,0,0,4,0,4,0,0,0,0,0,0},
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4}
};
const byte TitleScreen[4][20] = {
  {4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7,4,7},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {4,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,7},
  {7,4,4,0,10,0,2,0,3,0,0,0,0,9,0,0,0,4,4,4}
};
//the notes that play during each event
unsigned long lastNoteTime;
const int attackNotes[4] = {294,392,622};
const int deathNotes[10] = {800,750,700,650,600,550,500,450,400,350};

const int jumpNotes[2] = {500,740}; 
const int laserNotes[2] = {1000,1200};
const int coinNotes[2] = {800,1000};
const int owchNotes[2] = {1100,850};
const int killNotes[2] = {950,950};
int noteIndex = 0;
bool playingNote = false;
bool playingOwch = false;
bool playingKill = false;
bool playingCoin = false;
bool playingLaser = false;
bool playingJump = false;
