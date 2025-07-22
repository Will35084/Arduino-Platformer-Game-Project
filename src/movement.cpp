#include "game.h"
#include "enemy.h"
#include "player.h"
#include "movement.h"
#include <LiquidCrystal_I2C.h>

//for chaning the player's x position within movement handling functions
void ChangeXPos(){
  Player.X_POS += Player.Dir;
  //if the player is close to the edges of the level, the level won't scroll and the player will move across the screen
  if (Player.X_POS < 11 || Player.X_POS > LEVEL_LENGTH - 10) { 
    //an edge case. If the player is moving left from tile 11 and tile 10 is the altered x position, the offset won't change
    if((Player.X_POS == 10 && Player.Dir == LEFT_DIR) || (Player.X_POS == LEVEL_LENGTH - 9 && Player.Dir == RIGHT_DIR)){
      Platformer.Ref.x_offset += Player.Dir; //change the offset
      WriteLevel(); //redraw the level shifted in direction of movement
    }
    //otherwise move the player across the screen, not the level
    else{
      Player.X_LCD += Player.Dir;
      WriteLevel();
      lcd.setCursor(Player.X_LCD-Player.Dir, Player.Y_LCD);
      lcd.write(' ');
    }
  }
  //if not near the end of the level, scroll the level
  else { 
    Platformer.Ref.x_offset += Player.Dir;
    WriteLevel();
  }
}

void JumpHandle(){
  if(!Player.falling && Player.Jump){ //if player isn't falling, they can jump
    Player.walking = false; //player can't be simultaneously walking and jumping. Jumping takes precedence
    //jumpProcess determines how far along in the jump the player is from 0-3 (0-4 for a sprint-jump)
    switch (jumpProcess) {
      case 0: //the jump hasn’t started yet. The player is replaced with his jumping sprite
        noteIndex = 0;
        playingJump = true;
        //If the player is under a solid block, the jump is disabled
        if(Level[Player.Y_POS-1][Player.X_POS] > LASER) {
          Player.Jump = false;
          Player.falling = true;
          lastFallTime = currentTime;
        } 
        lcd.setCursor(Player.X_LCD, Player.Y_LCD);
        lcd.write(3); //writing the player jumping
        jumpProcess = 1; //to go to the next case next loop iteration
        lastJumpTime = currentTime; //for the small delay in between jump states
        break;

      case 1: //the player is currently on the tile he started with his jumping sprite. The player will 
              //go one tile up and one tile to the left/right in his standing still phase (if the delay is sufficient)

        if(currentTime - lastJumpTime >= jumpDelay[0]){ //if delay is long enough
          //correct player’s y position
          Player.Y_POS -=1;
          Player.Y_LCD -=1;
          //if the target tile of the player is a solid block, player will simply fall and not change their x coordinate
          if(Level[Player.Y_POS][Player.X_POS+Player.Dir] > LASER) {          
            lcd.setCursor(Player.X_LCD, Player.Y_LCD);
            lcd.write(4);
            Player.Dir = STRAIGHT;
          }
          //shift the level / player
          ChangeXPos();
          //clear the old tile with the character
          lcd.setCursor(Player.X_LCD - Player.Dir, Player.Y_LCD + 1);
          lcd.write(' '); 
          //write the player with its new coordinates
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(4);
          //if the tile underneath the player is a block or a debris, the player's jump finishes
          //jumpProcess is set to 4 to add a small delay in between consecutive jumps
          if (Level[Player.Y_POS+1][Player.X_POS] > LASER) {
            jumpProcess = 4;
            lastJumpTime = currentTime;
            break;
          }
          jumpProcess = 2;
          lastJumpTime = currentTime;
        }
        break;

      case 2: //The player is one block higher and one block to the side of their original position. Next, they will go 
              //through a phase similar to case 0 but will also move one block down and one block adjacent if possible. 
              //For sprint jumping, they will instead move 1 tile horizontally if possible then check if standing on ground

        if(currentTime - lastJumpTime >= jumpDelay[1]){
          //to differentiate between sprint and non sprint jumps
          if(!Player.Sprint){
            Player.Y_POS +=1;
            Player.Y_LCD +=1;
            //If the two tiles in the direction of the fall are solid blocks, the player's x position doesn't 
            //change and he simply falls: 
            if(Level[Player.Y_POS-1][Player.X_POS+Player.Dir] > LASER || Level[Player.Y_POS][Player.X_POS+Player.Dir] > LASER){
              lcd.setCursor(Player.X_LCD, Player.Y_LCD - 1);
              lcd.write(' '); 
              lcd.setCursor(Player.X_LCD, Player.Y_LCD);
              lcd.write(3);
            }
            else{
              //change x direction
              ChangeXPos();
              //reset the current character
              lcd.setCursor(Player.X_LCD - Player.Dir, Player.Y_LCD - 1);
              lcd.write(' '); 
              //If the target block has a crawler on it, the player will go to the tile above it and stop jumping
              if (Level[Player.Y_POS][Player.X_POS] == CRAWLER) {
                Player.Y_LCD -=1;
                Player.Y_POS -=1;
                lcd.setCursor(Player.X_LCD, Player.Y_LCD);
                lcd.write(4);
                Player.Jump = false;
                Player.falling = true;
                lastFallTime = currentTime;
              }
              else{
                lcd.setCursor(Player.X_LCD, Player.Y_LCD);
                lcd.write(3);
              }
            }
          }
          else { //if the sprint flag is enabled: a sprint jump is activated
            //checks if the block adjacent is a block or a debris and if is, player stays put
            if(!(Level[Player.Y_POS][Player.X_POS+Player.Dir] > LASER)){
              //change x pos
              ChangeXPos();
              lcd.setCursor(Player.X_LCD - Player.Dir, Player.Y_LCD);
              lcd.write(' '); 
              //write new player
              lcd.setCursor(Player.X_LCD, Player.Y_LCD);
              lcd.write(4);
              //if the block below the player is a block or debris, stop the jump
              if(Level[Player.Y_POS+1][Player.X_POS] > LASER) {
                jumpProcess = 4;
                lastJumpTime = currentTime;
                break;
              }
            }
          }
          jumpProcess = 3;
          lastJumpTime = currentTime;
        }
        break;

      case 3: //The player will simply return to his normal self on the new tile. 
              //For sprint jumps, it is similar to case 2 for normal jumps

        if (currentTime - lastJumpTime >= jumpDelay[2]){
          if(!Player.Sprint){
            lcd.setCursor(Player.X_LCD, Player.Y_LCD);
            lcd.write(4);
            Player.Jump = false;
            //if the tile underneath the player is not a solid block, or is a crawler, activate the falling flag
            if (Level[Player.Y_POS+1][Player.X_POS] < BLOCK || Level[Player.Y_POS+1][Player.X_POS] == CRAWLER || 
            Level[Player.Y_POS+1][Player.X_POS] == CLINGER) {
              Player.falling = true;
              lastFallTime = currentTime;
            }
          }
          else{
            //see case 2 comments for sprint jump
            Player.Y_POS +=1;
            Player.Y_LCD +=1;
            if(Level[Player.Y_POS-1][Player.X_POS+Player.Dir] > PORTAL || 
            Level[Player.Y_POS][Player.X_POS+Player.Dir] > PORTAL){
              lcd.setCursor(Player.X_LCD, Player.Y_LCD -1);
              lcd.write(' '); 
              lcd.setCursor(Player.X_LCD, Player.Y_LCD);
              lcd.write(3);
            }
            else{
              ChangeXPos();
              lcd.setCursor(Player.X_LCD - Player.Dir, Player.Y_LCD - 1);
              lcd.write(' '); 
              if (Level[Player.Y_POS][Player.X_POS] == CRAWLER) {
                lcd.setCursor(Player.X_LCD, Player.Y_LCD-1);
                lcd.write(4);
                Player.Jump = false;
                Player.falling = true;
                lastFallTime = currentTime;
              }
              else{
                lcd.setCursor(Player.X_LCD, Player.Y_LCD);
                lcd.write(3);
              }
            }
            jumpProcess = 4;
            lastJumpTime = currentTime;
          }
        }
        break;

      case 4: //case 3 for sprint jumping
        if(currentTime-lastJumpTime >= jumpDelay[4]){
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(4);
          Player.Jump = false;
          lastWalkTime = currentTime;
          //if the tile underneath the player is not a block or debris, activate the falling flag
          if (Level[Player.Y_POS+1][Player.X_POS] < BLOCK || Level[Player.Y_POS+1][Player.X_POS] == CRAWLER || 
          Level[Player.Y_POS+1][Player.X_POS] == CLINGER) {
            Player.falling = true;
            lastFallTime = currentTime;
          }
        }
        break;
    }
  }
  //last part of jumping handling is playing the jump sound. Only if all other sounds aren't being played
  if(playingJump && !playingCoin && !playingLaser && !playingKill && !playingOwch){
    PlaySounds(jumpNotes);
  }
}

void FallHandle(){
  if (Player.falling){ //checks if the player is currently falling
    if (currentTime - lastFallTime >= FALL_DELAY){//if delay is sufficient. 

      //If a crawler, player will activate the kill flag which will get rid of the crawler. When the player 
      //loops back to the falling function, he will fall normally
      if (Level[Player.Y_POS+1][Player.X_POS] == CRAWLER || (Level[Player.Y_POS+1][Player.X_POS] == CREEPER 
      && Platformer.status == BOSS)){
        if(!Player.kill) {lastFallTime = currentTime;}
        Player.kill = true;
        playingKill = true;
      }

      else{  //otherwise, player will need to move
        lcd.setCursor(Player.X_LCD, Player.Y_LCD);
        lcd.write(' ');

        //if a solid block, player will land on it and will stop falling
        if (Level[Player.Y_POS+1][Player.X_POS] > PORTAL && Level[Player.Y_POS+1][Player.X_POS] != CLINGER && 
        Level[Player.Y_POS+1][Player.X_POS] != CREEPER) {
          Player.falling = false;
          lastJumpTime = currentTime;
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(4);
        }

        //if air, the player will continue falling
        else {
          lastFallTime = currentTime;
          Player.Y_POS +=1;
          Player.Y_LCD +=1;
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(4);
        }
      }
    }
  }

  //checks if the player is falling
  if(!Player.Jump && (Level[Player.Y_POS+1][Player.X_POS] < BLOCK || Level[Player.Y_POS+1][Player.X_POS] == CLINGER) || 
  Level[Player.Y_POS+1][Player.X_POS] == CREEPER) { 
    if(currentTime - lastFallTime >= 100){
      if(Player.falling = false) {lastFallTime = currentTime;} //if player isn't already falling, reset the fall timer
      Player.falling = true;
    }
  }
}

void WalkSprintHandle(){
  if(Player.walking){
    //reseting the walking sprites
    lcd.createChar(0, hero_L);
    lcd.createChar(2, hero_R);
    if(Level[Player.Y_POS][Player.X_POS+Player.Dir] < BLOCK ){ //if the adjacent blocks aren't walls
      if(walkProcess == 0){ //the first phase of walking
        lcd.setCursor(Player.X_LCD, Player.Y_LCD);
        if(!Player.Crawl) {lcd.write(0);}
        else {lcd.write('_');}
        //If lastWalkTime has been reset, it's the beginning of the walk, and the sprint timer is set
        if(lastWalkTime == 0){
          sprintTimer = currentTime; //If the player walks continuously for long enough, a sprint will start
          lastWalkTime = currentTime;
          walkProcess = 1;
        }
        //if the walk isn't starting, don't reset the sprinting timer. Instead wait for the delay
        if (lastWalkTime != 0 && currentTime - lastWalkTime >= Player.Speed_Delay){ 
          lastWalkTime = currentTime;
          walkProcess = 1;
        }
      }
      if(walkProcess == 1){ //at stage 2, set the player to his left direction
        if(currentTime - lastWalkTime >= Player.Speed_Delay){
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          if(!Player.Crawl) {lcd.write(0);} //player facing left
          else {lcd.write('_');}
          walkProcess = 2;
          lastWalkTime = currentTime;
        }
      }
      else if(walkProcess == 2){ //at stage 3, draw the player standing straight again 
        if(currentTime - lastWalkTime >= Player.Speed_Delay){
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          if(!Player.Crawl) {lcd.write(2);}
          else {lcd.write('_');}
          walkProcess = 3;
          lastWalkTime = currentTime;
        }
      }
      else if(walkProcess == 3){ //at stage 4, set the player to his right direction. This is the stage in the 
                                //middle of the animation where the player's sprite actually shifts in the level
        if(currentTime - lastWalkTime >= Player.Speed_Delay){ 
          ChangeXPos(); //shift the level / the player
          //draw the player again
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          if(!Player.Crawl) {lcd.write(2);}
          else {lcd.write('_');}
          walkProcess = 4;
          lastWalkTime = currentTime;
          if(Level[Player.Y_POS+1][Player.X_POS] == CRAWLER){
            Player.kill = true;
            playingKill = true;
            Player.falling = true;
            lastFallTime = currentTime;
          }
        }
      }
      else if(walkProcess == 4){
        if(currentTime - lastWalkTime >= Player.Speed_Delay){
          //after a cooldown, will check if the player lets go of the controls, he will continue walking otherwise
          if(!digitalRead(A_BUTTON) == LOW || ((Player.Dir == LEFT_DIR && analogRead(JOY_H) > JOY_MAX) || 
          (Player.Dir == RIGHT_DIR && analogRead(JOY_H) < JOY_MIN))) {
            lastWalkTime = 0;
            Player.walking = false;
            sprintTimer = currentTime;
            Player.Sprint = false;
          }
          else {lastWalkTime = currentTime;}
          if (currentTime - sprintTimer >= 800) {Player.Sprint = true;}
          walkProcess = 0;
        }
      }
    }
    else{Player.walking = false;}
  }
  //sprint handling
  if(Player.Sprint) {Player.Speed_Delay = 45;}
  else {if(!Player.Crawl && !Player.Jump){Player.Speed_Delay = 100;}}
}

//Right = 1, left = -1, straight (neither) = 0
const int LEFT_DIR = -1;
const int RIGHT_DIR = 1;
const int STRAIGHT = 0;

unsigned long currentTime = 0;
unsigned long lastJumpTime = 0; 
unsigned long lastFallTime = 0; 
unsigned long lastWalkTime = 0; 
unsigned long sprintTimer = 0; 

//no need to initialize jumpProcess and walkProcess as they are set after flag activated
byte jumpProcess;
const int jumpDelay[5] = {100, 300, 300, 100, 200}; //holds the delay for each stage of the jump (large delay at peak of jump)

byte walkProcess; 