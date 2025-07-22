#include "game.h"
#include "enemy.h"
#include "player.h"
#include "movement.h"

bool title = false;
unsigned long lastBlinkTime = 0;
bool blink = false;
void setup() {
  lcd.init();        // Initialize LCD
  lcd.backlight();   // Turn on backlight

  //initializing custom charcters in LCD
  lcd.createChar(0,hero_L); //hero facing left
  lcd.createChar(1,hero_U); // hero facing up
  lcd.createChar(2,hero_R); //her facing right
  lcd.createChar(3,hero_J); //hero jumping
  lcd.createChar(4,hero_A); //hero facing straight
  lcd.createChar(5,crawler); //crawler
  lcd.createChar(6,creeper); //creeper
  lcd.createChar(7,clinger); //clinger

  //establishing inputs and outputs
  pinMode(JOY_BUTTON, INPUT_PULLUP); 
  pinMode(A_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(JOY_BUTTON, HIGH);

  //starting the title screen
  Platformer.status = TITLE_SCREEN;
}

void loop() {
  // put your main code here, to run repeatedly:
  //The different games states show up here
  switch(Platformer.status){
    case TITLE_SCREEN: {
      currentTime = millis();
      if(title == false){
        title = true;
        for(byte i = 0; i < 20; i++){
          for (byte j = 0; j < 4; j++){
            lcd.setCursor(i,j);
            switch(TitleScreen[j][i]){
              case AIR: 
                lcd.write(' ');
                break;
              case BLOCK: 
                lcd.write(0xFF);
                break;
              case DEBRIS: 
                lcd.write(0xDB);
                break;
              case CRAWLER:
                lcd.write(5);
                break;
              case CREEPER: 
                lcd.write(6);
                break;
              case CLINGER: 
                lcd.write(7);
                break;
              case PORTAL: 
                lcd.write(0);
                break;
              case COIN: 
                lcd.write('o');
                break;
              case LASER: 
                lcd.write('-');
                break;
            }
          }
        }
      }
      if(currentTime - lastBlinkTime > 1000){
        lcd.setCursor(1,1);
        if(blink){
          lcd.print("Hit Button To Play");
          blink = false;
        }
        else{
          lcd.print("                  ");
          blink = true;
        }
        lastBlinkTime = currentTime;
      }
      if(digitalRead(A_BUTTON) == LOW){
        delay(500);
        lcd.clear();
        Platformer.status = START;
      }
      break;
    }

    case START:{
      //position in level array
      Player.X_POS = 1; //2nd row (first row is just walls)
      Player.Y_POS = 2; //2nd column
      //position on LCD
      Player.X_LCD = 0; //1st row of screen
      Player.Y_LCD = 1; //2nd row of screen
      Platformer.score = 0;
      //initializing the "camera offset" of the game
      Platformer.Ref.x_offset = Player.X_POS; //the offset starts at x = 1 on the level array

      Crawler_1.Dir = STRAIGHT;
      Crawler_2.Dir = STRAIGHT;
      craCount = 0;
      creCount = 0;
      
      //drawing the player in its position before the game begins
      lcd.setCursor(Player.X_LCD, Player.Y_LCD);
      lcd.write(4);

      WriteLevel();
      Platformer.status = START_2;
      break;
    }
    //====================== START OF GAMEPLAY ============================
    case START_2:{
      //current time used to time all the different events instead of using the delay function
      currentTime = millis();
      JumpHandle(); //handle jumping movement.h
      FallHandle(); //hanlde falling movement.h
      WalkSprintHandle(); //handle walking movement.h
      SlashHandle(); //handle attacking player.h
      DebrisHandle(); //handle debris game.h

      //coin handling
      if(Level[Player.Y_POS][Player.X_POS] == COIN){
        Level[Player.Y_POS][Player.X_POS] = AIR;
        Platformer.score += 2; //add 2 points to score for collecting coin
        if(!playingCoin){noteIndex = 0;}
        playingCoin = true;
      }
      //order of precedence for sounds. Coin is always played even if other sounds are being played
      if(playingCoin && !playingKill && !playingOwch){PlaySounds(coinNotes);}

      //checks if the player is holding down controls player.h
      StationaryControlsCheck();
      MovementControlsCheck();

      //Portal Handling: Teleports player to boss level
      if(Level[Player.Y_POS][Player.X_POS] == PORTAL){
        lcd.clear();

        //"teleport" player to boss location
        Player.X_POS = LEVEL_LENGTH - 24; 
        Player.Y_POS = 4; 
        Player.X_LCD = 10; 
        Player.Y_LCD = 3; 
        lcd.setCursor(Player.X_LCD, Player.Y_LCD);
        lcd.write(4);

        //sets up boss sprites
        lcd.createChar(6, boss1);
        lcd.createChar(7,boss2);
        //switch status
        Platformer.status = BOSS;
        WriteLevel();
        break;
      }
      
      //see if enemies were killed enemy.h
      EnemyKill();

      //enemy handling functions enemy.h
      CrawlerHandle(Crawler_1);
      CrawlerHandle(Crawler_2);
      CreeperHandle(Creeper_1);
      CreeperHandle(Creeper_2);
      FakeBlockHandle();
      damageLivesHandle();
      EnemyHandle(); 
      break;
    }

    case LOSE: {
      lcd.setCursor(0,0);
      lcd.print("Game Over");
      lcd.setCursor(0,1);
      lcd.print(message.c_str());
      if(digitalRead(A_BUTTON) == LOW) {Platformer.status = TITLE_SCREEN;}
      break;
    }
    case BOSS: {
      currentTime = millis();
      damageLivesHandle();
      JumpHandle(); //handle jumping movement.h
      FallHandle(); //hanlde falling movement.h
      WalkSprintHandle(); //handle walking movement.h
      SlashHandle(); //handle attacking player.h
      DebrisHandle(); //handle debris game.h
      //see if enemies were killed enemy.h
      EnemyKill();
      BossHandle();
      StationaryControlsCheck();
      MovementControlsCheck();
      //enemy handling functions enemy.h
      EnemyHandle(); 
      //portal handling
      if(Level[Player.Y_POS][Player.X_POS] == PORTAL){
        Platformer.status = WIN;
        lcd.clear();
        break;
      }
      break;

    }
    case WIN: {
      lcd.setCursor(0,0);
      lcd.print("Winner!");
      lcd.setCursor(0,1);
      lcd.print("Score: ");
      lcd.setCursor(0,2);
      lcd.print(Platformer.score);
      delay(1000);
      if(digitalRead(A_BUTTON) == LOW) {Platformer.status = TITLE_SCREEN;}
      break;
    }
  }
}