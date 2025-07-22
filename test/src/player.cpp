#include "enemy.h"
#include "player.h"
#include "game.h"
#include "movement.h"

Hero Player = {0,0,0,0,0,0,3,0,0,0,0,0,0,0};
int slashX;
int slashType;
unsigned long lastSlashTime;
int slashTotal = 0;

//sprite of hero jumping
byte hero_J[8] = {
  0b01110,
  0b00100,
  0b00100,
  0b01010,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

//hero facing straight (used when player isn't facing left or right)
byte hero_A[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b00100,
  0b00100,
  0b01010
};

//hero facing/attacking to the left
byte hero_L[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b00100,
  0b01100,
  0b00010
};

//hero facing/attacking to the right
byte hero_R[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b00100,
  0b00110,
  0b01000
};

//hero facing up-left
byte hero_UL[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b00100,
  0b00100,
  0b01100,
  0b00010
};

//hero facing up-right
byte hero_UR[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b00100,
  0b00100,
  0b00110,
  0b01000
};

//Shows the player "crawling" under the clinger
byte hero_U[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b01010
};

void SlashHandle(){ //the player's attack
  if(Player.Slash){
    if(currentTime - lastSlashTime >= 800){ //cooldown on attack of 800 ms
      lcd.setCursor(Player.X_LCD + Player.Dir, Player.Y_LCD);

      if(Player.Dir < 0) {lcd.write('(');} //a "slash"(parentheses) will show
      else if(Player.Dir > 0) {lcd.write (')');}

      if(slashTotal != 2 || Level[Player.Y_POS][Player.X_POS + Player.Dir] != CREEPER){PlaySlashSound();}
      if(slashTotal == 2){ //to kill a creeper or break a debris, you need to hit it three times with a slash
        slashTotal = 0;
        if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == CREEPER) {
          Player.kill = true;
          playingKill = true;
        } 
        
        else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == DEBRIS){ //if debris, replace the debris with air in the level array
          Level[Player.Y_POS][Player.X_POS + Player.Dir] = 0;
          lcd.setCursor(Player.X_LCD + Player.Dir, Player.Y_LCD);
          lcd.write(' ');
        }
      }
      else { //if the slash total is less than three, if the player is hitting the same thing, increment slash total. Otherwise, reset it
        if((slashX = Player.X_POS + Player.Dir && slashType == DEBRIS) || slashType == CREEPER) {slashTotal += 1;}
        else {slashTotal = 0;}
      }
      //For hitting fake blocks, move the block in the direction of the attack if there is space available
      if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == MOVABLE_BLOCK){
        lcd.setCursor(0,0);
        lcd.write('T');
        if(Level[Player.Y_POS][Player.X_POS + 2 * Player.Dir] == 0){
          Level[Player.Y_POS][Player.X_POS + Player.Dir] = AIR;
          Level[Player.Y_POS][Player.X_POS + 2 * Player.Dir] = MOVABLE_BLOCK;
          lcd.setCursor(Player.X_LCD + Player.Dir, Player.Y_LCD);
          lcd.write(' ');
          lcd.setCursor(Player.X_LCD + 2 * Player.Dir, Player.Y_LCD);
          lcd.write(0xFF);
        }
      }

      //Get rid of the parentheses and replace it with the block that was there before
      lcd.setCursor(Player.X_LCD + Player.Dir, Player.Y_LCD);
      if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == CREEPER) {lcd.write(6);}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == CRAWLER) {lcd.write(5);}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == CLINGER) {lcd.write(7);}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == MOVABLE_BLOCK) {lcd.write('0');}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == AIR) {lcd.write(' ');}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == BLOCK) {lcd.write(0xFF);}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == DEBRIS) {lcd.write(0xDB);}
      else if (Level[Player.Y_POS][Player.X_POS + Player.Dir] == COIN) {lcd.write('o');}
      slashX = Player.X_POS + Player.Dir;
      Player.Slash = false;
      lastSlashTime = currentTime;
      WriteLevel();
    }
  }
}

void StationaryControlsCheck(){
  //If the player is horizontally tilting the joystick, make the player face that direction
  if(!Player.Jump && !Player.walking){
    //right
    if(analogRead(JOY_H) > JOY_MIN && !(digitalRead(A_BUTTON) == LOW) && (analogRead(JOY_V) >= JOY_MAX)) { 
      Player.Dir = RIGHT_DIR;
      lcd.createChar(2, hero_R);
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      if(!Player.Crawl) {lcd.write(2);}
      else {lcd.write('_');}
    }
    //left
    else if(analogRead(JOY_H) < JOY_MAX && !(digitalRead(A_BUTTON) == LOW) && (analogRead(JOY_V) >= JOY_MAX)) {
      Player.Dir = LEFT_DIR;
      lcd.createChar(0, hero_L);
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      if(!Player.Crawl) {lcd.write(0);}
      else {lcd.write('_');}
    }
    //straight
    else if ((analogRead(JOY_H) >= JOY_MAX || analogRead(JOY_H) <= JOY_MIN) && !(digitalRead(A_BUTTON) == LOW) && analogRead(JOY_V) >= JOY_MAX){
      Player.Dir = STRAIGHT;
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      if(!Player.Crawl) {lcd.write(4);}
      else {lcd.write('_');}
    }
    //straight-up
    else if(analogRead(JOY_V) < JOY_MAX && analogRead(JOY_H) >= JOY_MAX && !(digitalRead(A_BUTTON) == LOW) && analogRead(JOY_H) <= JOY_MIN && !Player.Jump){
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      lcd.write(1);
    }
    //left-up
    else if(analogRead(JOY_V) < JOY_MAX && analogRead(JOY_H) < JOY_MAX && !(digitalRead(A_BUTTON) == LOW) && !Player.Jump){
      Player.Dir = LEFT_DIR;
      lcd.createChar(0, hero_UL);
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      lcd.write(0);
    }
    //right-up
    else if(analogRead(JOY_V) < JOY_MAX && analogRead(JOY_H) > JOY_MIN && !(digitalRead(A_BUTTON) == LOW) && !Player.Jump){
      Player.Dir = RIGHT_DIR;
      lcd.createChar(2, hero_UR);
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      lcd.write(2);
    }
  }
  //check if the player is using controls to attack
  if(digitalRead(JOY_BUTTON) == LOW){
    if (currentTime - lastSlashTime >= 300){
      Player.Slash = true;
      if (analogRead(JOY_H) > JOY_MIN) {Player.Dir = RIGHT_DIR;}
      if (analogRead(JOY_H) < JOY_MAX) {Player.Dir = LEFT_DIR;}
      if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == CREEPER) {slashType = CREEPER;}
      else if(Level[Player.Y_POS][Player.X_POS + Player.Dir] == DEBRIS) {slashType = DEBRIS;}
    }
  }
}

void MovementControlsCheck(){
  //check if the player is crawling
  if(analogRead(JOY_V) > JOY_MIN+300) {
    Player.Crawl = true;
    Player.Speed_Delay = 200;
    Player.Sprint = false;
    sprintTimer = currentTime;
  }
  else{Player.Crawl = false;}

  //checks if player is walking 
  if(!Player.Jump){ //walking isn't allowed if also jumping and falling
    if(digitalRead(A_BUTTON) == LOW){
      //if the adjacent tile is a block or debris, the player cannot walk there
      if (analogRead(JOY_H) > JOY_MIN && Level[Player.Y_POS][Player.X_POS + 1] != BLOCK && Level[Player.Y_POS][Player.X_POS + 1] != DEBRIS && Level[Player.Y_POS][Player.X_POS + 1] != FAKE_BLOCK && Level[Player.Y_POS][Player.X_POS + 1] != MOVABLE_BLOCK) {
        Player.walking = true;
        Player.Dir = RIGHT_DIR; // Move right
      }
      else if (analogRead(JOY_H) < JOY_MAX && Level[Player.Y_POS][Player.X_POS - 1] != BLOCK && Level[Player.Y_POS][Player.X_POS - 1] != DEBRIS && Level[Player.Y_POS][Player.X_POS - 1] != FAKE_BLOCK && Level[Player.Y_POS][Player.X_POS - 1] != MOVABLE_BLOCK) {
        Player.walking = true;
        Player.Dir = LEFT_DIR; // Move left
      }  
    }
  }
  //check if player is jumping
  if(currentTime-lastJumpTime >= 400){
    if (!Player.Jump && digitalRead(A_BUTTON) == LOW && analogRead(JOY_V) < JOY_MAX){
      Player.Jump = true;
      jumpProcess = 0;
      if (analogRead(JOY_H) < JOY_MAX) {Player.Dir = LEFT_DIR;}
      else if (analogRead(JOY_H) > JOY_MIN) {Player.Dir = RIGHT_DIR;}
      else {Player.Dir = STRAIGHT;}
    }
  }

  //check if the player isn't sprinting. If so, reset sprinting variables
  if(!digitalRead(A_BUTTON) == LOW || ((Player.Dir == LEFT_DIR && analogRead(JOY_H) > JOY_MAX) || (Player.Dir == RIGHT_DIR && analogRead(JOY_H) < JOY_MIN))) {
    if(!Player.Jump){
      sprintTimer = currentTime;
      Player.Sprint = false;
    }
  }
}

//handles the invulneability of theplayer after taking damage. Also prints the player's lives in the top left of the screen
void damageLivesHandle(){
  if(currentTime - lastDamageTime < DAMAGE_COOLDOWN && currentTime > DAMAGE_COOLDOWN){
    lcd.setCursor(Player.X_LCD, Player.Y_LCD);
    lcd.write('!');
  }
  if(playingOwch && !playingKill) {
    PlaySounds(owchNotes);
  }
  lcd.setCursor(0,0);
  lcd.print(Player.lives);
}