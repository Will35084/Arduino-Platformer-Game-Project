#include "enemy.h"
#include "player.h"
#include "game.h"
#include "movement.h"

unsigned long lastDamageTime = 0; //lastDamageTime uses millis to handle the delay of the "invulnerability" the player has after taking damage
unsigned long lastBossMoveTime = 0; //Unsigned long for the bosses movement. 
int bossFiringBot = 1; //Since the boss fires two lasers, bossFiringBot ensures the bottom laser doesn't overlap with the top laser's logic
std::string message;
bool moveFlag;
void CrawlerHandle(Enemy &C){
  
  if(C.onScreen){ //if crawler will only move around when on screen
    if(currentTime - C.lastEventTime >= CRAWLER_SPEED){ //they will move a tile every second seconds

      //This if else statement ensures that the crawler will always face the direction of the player
      if(Player.X_POS > C.X_POS) {C.Dir = RIGHT_DIR;} 
      else {C.Dir = LEFT_DIR;}

      //the crawler will only move if the two blocks underneath him are not walk-through (coins (1), air (0), laser (2), portal(3))
      if(Level[C.Y_POS+1][C.X_POS+C.Dir] > LASER || Level[C.Y_POS+2][C.X_POS+C.Dir] > LASER){
        //clear the original crawler tile: 
        Level[C.Y_POS][C.X_POS] = AIR;
        lcd.setCursor(C.X_LCD, C.Y_LCD);
        lcd.write(' ');

        if(Level[C.Y_POS][C.X_POS + C.Dir] == AIR){ //if there are no obstructions in front of the crawler, he will walk
	        //if the tile below the target tile is a block, crawler will simply move one tile left.
          if(Level[C.Y_POS+1][C.X_POS + C.Dir] > LASER){ 
            Level[C.Y_POS][C.X_POS + C.Dir] = CRAWLER;
            lcd.setCursor(C.X_LCD+C.Dir,C.Y_LCD);
            lcd.write(5);
          }
          //Otherwise, he will move down one tile and horizontally one tile
          else{
            Level[C.Y_POS+1][C.X_POS + C.Dir] = CRAWLER;
            lcd.setCursor(C.X_LCD+C.Dir,C.Y_LCD+1);
            lcd.write(5);
            C.Y_LCD +=1; //increment y down
            C.Y_POS +=1;
          }
          C.X_LCD += C.Dir; //increment x forward
          C.X_POS += C.Dir;
        }
	      //If there are obstructions in front of the crawler...
        else if (Level[C.Y_POS][C.X_POS + C.Dir] != AIR && Level[C.Y_POS][C.X_POS + C.Dir] != CREEPER) {

          //if there is no block or another crawler above that tile (or directly above him), it will "jump" onto that tile
          if (!(Level[C.Y_POS - 1][C.X_POS + C.Dir] != AIR || Level[C.Y_POS - 1][C.X_POS] > LASER)){
            Level[C.Y_POS-1][C.X_POS + C.Dir] = CRAWLER;
            lcd.setCursor(C.X_LCD+C.Dir,C.Y_LCD-1);
            lcd.write(5);
            C.X_LCD += C.Dir; //X forward, Y up
            C.X_POS += C.Dir;
            C.Y_LCD -= 1;
            C.Y_POS -= 1;
          }
          else{ //otherwise, Crawler remains on current tile
            Level[C.Y_POS][C.X_POS] = CRAWLER;
            lcd.setCursor(C.X_LCD, C.Y_LCD);
            lcd.write(5);
          }
        }
      }
      C.lastEventTime = currentTime;
    }
  }
}

void CreeperHandle(Enemy &C){
  //If the creeper isn't on screen while shooting lasers, delete the lasers
  if(!C.onScreen && C.eventBool){
    Level[C.Y_POS][C.X_POS + C.Dir * (C.creFiring)] = AIR;
    lcd.setCursor(C.X_LCD + C.Dir * (C.creFiring), C.Y_LCD);
    lcd.write(' ');
    C.eventBool = false;
    C.creFiring = 1;
  }
  if(C.onScreen){ 
    if(abs(C.X_POS - Player.X_POS) <= 1){ //if there are 1 or less tiles in between the creeper and the player, movement will begin
      moveFlag = true;
    }
    else{
      if(C.eventBool){
        moveFlag = false;
      }
    }

    if(moveFlag){ //if there is still a laser on the screen, get rid of it
      if(C.creFiring > 1) {
        Level[C.Y_POS][C.X_POS + C.Dir * (C.creFiring)] = AIR;
        lcd.setCursor(C.X_LCD + C.Dir * (C.creFiring), C.Y_LCD);
        lcd.write(' ');
        C.eventBool = false;
        C.creFiring = 1;
      }

      if(currentTime - C.lastEventTime >= CREEPER_SPEED){ //the creeper will wait for 750 ms before walking
        //the creeper will face opposite the direction the player is in
        if(Player.X_POS > C.X_POS) {C.Dir = LEFT_DIR;} 
        else if (Player.X_POS < C.X_POS) {C.Dir = RIGHT_DIR;}
        //if the target block is solid, the creeper will run the opposite direction to get away from the player
        if(Level[C.Y_POS][C.X_POS+C.Dir] > PORTAL){
          C.Dir = - C.Dir;
        }
        //if the target tile has a block underneath and the target tiles themselves aren't blocks ordebris
        if(Level[C.Y_POS+1][C.X_POS+C.Dir] > LASER && Level[C.Y_POS][C.X_POS+C.Dir] < PORTAL){
          
          Level[C.Y_POS][C.X_POS] = AIR; //clearing current creeper tile
          lcd.setCursor(C.X_LCD, C.Y_LCD);
          lcd.write(' ');

          Level[C.Y_POS][C.X_POS+C.Dir] = CREEPER; //setting new creeper tile
          lcd.setCursor(C.X_LCD+C.Dir, C.Y_LCD);
          lcd.write(6);

          C.X_POS += C.Dir; //x increment
          C.X_LCD += C.Dir;
        }
        C.lastEventTime = currentTime; //reset cooldown for movement / laser
        if(abs(C.X_POS - Player.X_POS) == 4){
          moveFlag = false;
        }
      }
    }
    else{ //if the creeper is at least 4 tiles away from the player, it will shoot its laser

      //x position of laser = C.X_POS (x position of origin) + C.creFiring (magnitude of distance traveled) * C.Dir (direction of distance. 1 or -1, right or left) 
      int next_x_pos = C.X_POS + C.Dir * (C.creFiring + 1);
      int curr_x_pos = C.X_POS + C.Dir * C.creFiring;
      int next_x_lcd = C.X_LCD + C.Dir * (C.creFiring + 1);
      int curr_x_lcd = C.X_LCD + C.Dir * C.creFiring;
      //This if else statement ensures that the crawler will always face the direction of the player
      if(Player.X_POS > C.X_POS) {C.Dir = RIGHT_DIR;} 
      else {C.Dir = LEFT_DIR;} 

      //the creeper will wait 2s between laser shots
      if(currentTime - C.lastEventTime >= LASER_RATE && !C.eventBool) {
        C.eventBool = true;
        //play the laser sound. Reset sounds if not already playing them
        if(!playingCoin){
          if(!playingLaser){noteIndex = 0;}
          playingLaser = true;
        }
      }
      //The laser will move every 200ms
      if(currentTime - C.lastEventTime >= LASER_SPEED && C.eventBool){
    
        //if the target tile of the laser is air, and not out of bounds, the laser will shoot
        if(Level[C.Y_POS][next_x_pos] == AIR && next_x_lcd >= 0 && next_x_lcd <= 19){

          //if the laser is beginning to shoot, don't accidentially delete the tile with the creeper
          if(Level[C.Y_POS][curr_x_pos] != CREEPER) {
            Level[C.Y_POS][curr_x_pos] = AIR;
            lcd.setCursor(curr_x_lcd, C.Y_LCD);
            lcd.write(' ');
          }
          //update the laser's position
          Level[C.Y_POS][next_x_pos] = LASER;
          lcd.setCursor(next_x_lcd, C.Y_LCD);
          lcd.write('-');
          C.creFiring += 1; //increase the distance of the next laser position
          C.lastEventTime = currentTime;
        }
        //if the laser can't move to the next tile, delete it and reset everything
        else{
          Level[C.Y_POS][curr_x_pos] = AIR;
          lcd.setCursor(curr_x_lcd, C.Y_LCD);
          lcd.write(' ');

          playingLaser = false;
          C.eventBool = false;
          C.creFiring = 1; //creFiring resets to 1
        }
      }
    }
  }
  if(playingLaser && !playingCoin && !playingOwch && !playingCoin) {PlaySounds(laserNotes);} //last part of creeper handling plays the laser sounds in parallel
}

void EnemyHandle(){ //all of the following if statements handle if the player is dying
  //for the boss
  if(Platformer.status == BOSS){
    if(Level[Player.Y_POS][Player.X_POS] == CLINGER || Level[Player.Y_POS][Player.X_POS] ==  CREEPER || (Level[Player.Y_POS][Player.X_POS] == LASER && !Player.Crawl)){
      if (currentTime - lastDamageTime > DAMAGE_COOLDOWN){ //if player was already hurt, he will be invulnerable for a short time
        Player.lives--;
        lastDamageTime = currentTime;
        //When a player dies, the screen clears and the player shows up in the center before switching to the LOSE status
        if(Player.lives == 0){
          lcd.clear();
          lcd.setCursor(10, 3);
          lcd.write(7);
          lcd.setCursor(10, 2);
          lcd.write(6);
          PlayDeathSound();
          message = "Ouch..at the end too";
          Platformer.status = LOSE;
        }
        playingOwch = true;
      }
    }
  }
  else{
    //If player is touching a crawler, lives will deincrement if player isn't invulnerable. If lives are 0, kill the player
    if(Level[Player.Y_POS][Player.X_POS] == CRAWLER){
      if (currentTime - lastDamageTime > DAMAGE_COOLDOWN){ //if player was already hurt, he will be invulnerable for a short time
        Player.lives--;
        lastDamageTime = currentTime;
        //When a player dies, the screen clears and the player shows up in the center before switching to the LOSE status
        if(Player.lives == 0){
          lcd.clear();
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(5);
          message = "Jump on its head!";
          PlayDeathSound(); //Play the death noise
          Platformer.status = LOSE;
        }
        playingOwch = true;
      }
    }
    //same process for getting hurt by a creeper
    else if(Level[Player.Y_POS][Player.X_POS] == CREEPER){
      if(currentTime - lastDamageTime > DAMAGE_COOLDOWN){
        Player.lives --;
        lastDamageTime = currentTime;
        if(Player.lives == 0){
          lcd.clear();
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(6);
          PlayDeathSound();
          message = "Try hitting it maybe";
          Platformer.status = LOSE;
        }
        playingOwch = true;
      }
    }
    //If there is a clinger the tile on the player, same process
    else if(Level[Player.Y_POS][Player.X_POS] == CLINGER){
      if(currentTime - lastDamageTime > DAMAGE_COOLDOWN){
        Player.lives --;
        lastDamageTime = currentTime;
        if(Player.lives == 0){
          lcd.clear();
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(7);
          PlayDeathSound();
          Platformer.status = LOSE;
          message = "Just don't fall lol";
        }
        playingOwch = true;
      }
    }
    //If the player is killed by a laser, similar process, but print the creeper who fired it and the laser itself
    else if(Level[Player.Y_POS][Player.X_POS] == LASER) {
      if (!Player.Crawl && currentTime - lastDamageTime > DAMAGE_COOLDOWN){
        Player.lives --;
        lastDamageTime = currentTime;
        if(Player.lives == 0){
          lcd.clear();
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write('-');
          if(Creeper_1.eventBool){ //find out which of the creepers shot the laser
            lcd.setCursor(Creeper_1.X_LCD, Creeper_1.Y_LCD);
            lcd.write(6);
          }
          else{
            lcd.setCursor(Creeper_2.X_LCD, Creeper_2.Y_LCD);
            lcd.write(6);
          }
          PlayDeathSound();
          message = "duck to not get shot";
          Platformer.status = LOSE;
        }
        playingOwch = true;
      } 
    }
    //hardest one, if the block above the player is a clinger, the clinger will drop onto the tile with the player, and the player will get hurt with the same process
    if(Level[Player.Y_POS-1][Player.X_POS] == CLINGER && !Player.Crawl){
      if(currentTime - lastDamageTime > DAMAGE_COOLDOWN){
        Player.lives --;
        //the clinger will drop onto the player's sprite
        lcd.setCursor(Player.X_LCD, Player.Y_LCD-1);
        lcd.write(0xFF); 
        lcd.setCursor(Player.X_LCD, Player.Y_LCD);
        lcd.write(7);
        //small delay to make sure the animation is visible
        delay(150);
        lastDamageTime = currentTime;
        if (Player.lives == 0){
          lcd.clear();
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(7);
          PlayDeathSound();
          message = "ducking can save you";
          Platformer.status = LOSE;
        }
        else{ //If the player doesn't have 0 lives, the clinger will go back up to where it was before, and give the player a chance to escape
          lcd.setCursor(Player.X_LCD, Player.Y_LCD);
          lcd.write(4); //print player facing straight
          lcd.setCursor(Player.X_LCD, Player.Y_LCD-1);
          lcd.write(7); // prints clinger above the player
        }
        playingOwch = true;
      }
    }
  }
}

void EnemyKill(){
  if (Player.kill){
    //if player is killing a crawler or the boss, they will be falling
    if (Player.falling){
      //hard-coded 200ms delay for killing bosses
      if(currentTime - lastFallTime >= 200){
        Level[Player.Y_POS+1][Player.X_POS] = AIR; //set tile below player to air, deleting enemy
        //if the enemy is a boss, also delete the second tile underneath since the boss is 2 tiles
        if(Platformer.status == BOSS){
          Level[Player.Y_POS + 2][Player.X_POS] = AIR;
          Boss.onScreen = false;
          Platformer.score += 7; //10 total points for killing the boss
        }
        Player.kill = false;
        lastFallTime = currentTime;
        Platformer.score += 3; // add 3 points to score for killing crawler

        //Reset the crawler who got killed to make space for another one. 
        if(Crawler_1.X_POS == Player.X_POS){
          Crawler_1.X_POS = 0;
          Crawler_1.Y_POS = 0;
          Crawler_1.X_LCD = 0;
          Crawler_1.Y_LCD = 0;
          Crawler_1.onScreen = false;
        }
        else if (Crawler_2.X_POS == Player.X_POS){
          Crawler_2.X_POS = 0;
          Crawler_2.Y_POS = 0;
          Crawler_2.X_LCD = 0;
          Crawler_2.Y_LCD = 0;
          Crawler_2.onScreen = false;
        }
      }
    }
    //else if player is killing a creeper
    else{
      //same process as crawler
      Level[Player.Y_POS][Player.X_POS + Player.Dir] = AIR;
      Player.kill = false;
      lastFallTime = currentTime;
      Platformer.score += 3; // add 3 points to score for killing creeper
      
      if(Creeper_1.X_POS == Player.X_POS + Player.Dir){
        Creeper_1.X_POS = 0;
        Creeper_1.Y_POS = 0;
        Creeper_1.X_LCD = 0;
        Creeper_1.Y_LCD = 0;
        Creeper_1.onScreen = false;
      }
      else{
        Creeper_2.X_POS = 0;
        Creeper_2.Y_POS = 0;
        Creeper_2.X_LCD = 0;
        Creeper_2.Y_LCD = 0;
        Creeper_2.onScreen = false;
      }
      lcd.setCursor(Player.X_LCD + Player.Dir, Player.Y_LCD);
      lcd.write(' ');
    }
  }
  if(playingKill) {PlaySounds(killNotes);}
}

void BossHandle() {
  //If boss is on screen and after a delay, the boss will move one tile
  if(currentTime - lastBossMoveTime > BOSS_DELAY && Boss.onScreen){

    //reset current boss tiles
    Level[Boss.Y_POS][Boss.X_POS] = AIR;
    lcd.setCursor(Boss.X_LCD, Boss.Y_LCD);
    lcd.write(' ');
    Level[Boss.Y_POS+1][Boss.X_POS] = AIR;
    lcd.setCursor(Boss.X_LCD, Boss.Y_LCD + 1);
    lcd.write(' ');

    //move boss by one tile
    Boss.X_POS = Boss.X_POS + Boss.Dir;
    Boss.X_LCD = Boss.X_LCD + Boss.Dir;
    Level[Boss.Y_POS][Boss.X_POS] = CREEPER;
    lcd.setCursor(Boss.X_LCD, Boss.Y_LCD);
    lcd.write(6);
    Level[Boss.Y_POS+1][Boss.X_POS] = CLINGER;
    lcd.setCursor(Boss.X_LCD, Boss.Y_LCD + 1);
    lcd.write(7);

    //flip directions for next time
    Boss.Dir = -Boss.Dir;

    lastBossMoveTime = currentTime;
    //for playing the laser sound
    if(Boss.eventBool == false){
      Boss.eventBool = true;
      if(!playingLaser){noteIndex = 0;}
      playingLaser = true;
    }
    else{
      Boss.eventBool = false;
    }
  }
  //The laser will move every 200ms
  if (currentTime - Boss.lastEventTime >= LASER_SPEED && Boss.eventBool) {

    //defining the laser positions for clarity
    int nextXTop = Boss.X_POS + LEFT_DIR * (Boss.creFiring + 1);
    int curXTop  = Boss.X_POS + LEFT_DIR * Boss.creFiring;
    int nextXBot = Boss.X_POS + LEFT_DIR * (bossFiringBot + 1);
    int curXBot  = Boss.X_POS + LEFT_DIR * bossFiringBot;
  
    //To indicate if the top laser/bottom laser are finished moving
    bool topDone = false;
    bool botDone = false;
  
    //top laser handling: 
    //if the laser isn't reset or out of bounds, 
    if (Boss.creFiring >= 0 && Boss.X_LCD + LEFT_DIR * (Boss.creFiring + 1) >= 0 && Boss.X_LCD + LEFT_DIR * (Boss.creFiring + 1) <= 19) {
      //if the current laser position isn't on the boss, set that block to air
      if (Level[Boss.Y_POS][curXTop] != CREEPER && Level[Boss.Y_POS][curXTop] != CLINGER) {
        Level[Boss.Y_POS][curXTop] = AIR;
        lcd.setCursor(Boss.X_LCD + LEFT_DIR * Boss.creFiring, Boss.Y_LCD);
        lcd.write(' ');
      }
      //It the target block is air, move the laser and incremend creFiring
      if (Level[Boss.Y_POS][nextXTop] == AIR) {
        Level[Boss.Y_POS][nextXTop] = LASER;
        lcd.setCursor(Boss.X_LCD + LEFT_DIR * (Boss.creFiring + 1), Boss.Y_LCD);
        lcd.write('-');
        Boss.creFiring++;
      } 
      else {
        topDone = true; //if the laser's target block isn't air, finish the laser
      }
    } 
    else {
      topDone = true; //if the laser is out of bounds or reset, laser is done. 
    }
  
    //bottom laser handling (same process)
    if (bossFiringBot >= 0 && Boss.X_LCD + LEFT_DIR * (bossFiringBot + 1) >= 0 && Boss.X_LCD + LEFT_DIR * (bossFiringBot + 1) <= 19) {
      if (Level[Boss.Y_POS + 1][curXBot] != CREEPER && Level[Boss.Y_POS + 1][curXBot] != CLINGER) {
        Level[Boss.Y_POS + 1][curXBot] = AIR;
        lcd.setCursor(Boss.X_LCD + LEFT_DIR * bossFiringBot, Boss.Y_LCD + 1);
        lcd.write(' ');
      }
  
      if (Level[Boss.Y_POS + 1][nextXBot] == AIR) {
        Level[Boss.Y_POS + 1][nextXBot] = LASER;
        lcd.setCursor(Boss.X_LCD + LEFT_DIR * (bossFiringBot + 1), Boss.Y_LCD + 1);
        lcd.write('-');
        bossFiringBot++;
      } 
      else {
        botDone = true;
      }
    } 
    else {
      botDone = true;
    }
  
    // If both are done, reset everything until eventBool set to true again
    if (topDone && botDone) {
      Boss.eventBool = false;
      Boss.creFiring = 1;
      bossFiringBot = 1;
      playingLaser = false;
    }
  
    Boss.lastEventTime = currentTime;
  }
  if(playingLaser && !playingCoin && !playingKill && !playingOwch)
    PlaySounds(laserNotes); //play the laser sounds
}
Enemy Crawler_1 = {0,0,0,0,0,0,0,0,0}; //there can be as many as two creepers and two crawlers on the screen at once
Enemy Crawler_2 = {0,0,0,0,0,0,0,0,0};
Enemy Creeper_1 = {0,0,0,0,0,0,0,0,1};
Enemy Creeper_2 = {0,0,0,0,0,0,0,0,1};

//The debris array for handling debris
Enemy DebrisArr[MAX_DEBRIS];
//the fake block array for handling fake blocks
Enemy FakeBlockArr[MAX_FAKE_BLOCK];
//The Boss
Enemy Boss = {0,0,0,0,0,0,0,0,1};

byte craCount; //to count how many crawlers there are (should be a maximum of 2)
byte creCount; //to count how many creepers there are on the screen (should be a maximum of 2)

//sprite for crawling enemy (crawler) (dies to jump, not to attack)
byte crawler[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00100,
  0b01110,
  0b01110,
  0b10101
};

//sprite for tall enemy (creeper) (dies to attack, not to jump)
byte creeper[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b01010
};

//sprite for antlion like enemy (clinger) Attaches to ceiling. You have to crawl under them to avoid dying
byte clinger[8] = {
  0b00100,
  0b10101,
  0b10101,
  0b11111,
  0b11111,
  0b10101,
  0b10101,
  0b00100
};

//sprite of top of boss
byte boss1[8] = {
  0b00000,
  0b01110,
  0b10101,
  0b11111,
  0b01010,
  0b00100,
  0b01110,
  0b10101
};

//sprite of bottom of boss
byte boss2[8] = {
  0b10101,
  0b11111,
  0b01110,
  0b00100,
  0b01110,
  0b01010,
  0b10001,
  0b10001
};