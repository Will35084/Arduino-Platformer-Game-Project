#ifndef ENEMY_H
#define ENEMY_H
#include <LiquidCrystal_I2C.h>

#define CRAWLER_SPEED 1000
#define CREEPER_SPEED 750
#define LASER_SPEED 200
#define LASER_RATE 2000
#define KILL_DELAY 200
#define BOSS_DELAY 2000
#define MAX_DEBRIS 10
#define MAX_FAKE_BLOCK 3
#define DAMAGE_COOLDOWN 1000

//The struct for the enemies

struct Enemy {
    byte X_POS; //it's coordinates in the level array for moving the sprite
    byte Y_POS; 
    byte X_LCD; // keeps track of max 2 crawlers on a screen at a time
    byte Y_LCD;
    int Dir; //its' direction to know which direction it should walk next
    bool onScreen; //if off screen, crawlers' information will reset
    unsigned long lastEventTime; //for enemy movement. 
    bool eventBool;
    int creFiring; //to check the status of the creeper's laser
};
extern Enemy Crawler_1; //there can be as many as two creepers and two crawlers on the screen at once
extern Enemy Crawler_2;
extern Enemy Creeper_1;
extern Enemy Creeper_2;
extern Enemy Boss;
extern Enemy DebrisArr[MAX_DEBRIS];
extern Enemy FakeBlockArr[MAX_FAKE_BLOCK];

//Instead of an if statement which is used to handle all the other game functions, since there are going to be more than 1 crawlers on the screen at a time, this function is to handle both crawlers
void CrawlerHandle(Enemy &C); 
void CreeperHandle(Enemy &C);
void EnemyHandle();
void EnemyKill();
void BossHandle();

extern byte craCount; //to count how many crawlers there are (should be a maximum of 2)
extern byte creCount; //to count how many creepers there are on the screen (should be a maximum of 2)

extern byte crawler[8];//sprite for crawling enemy (crawler) (dies to jump, not to attack)
extern byte creeper[8];//sprite for tall enemy (creeper) (dies to attack, not to jump)
extern byte clinger[8];//sprite for antlion like enemy (clinger) Attaches to ceiling. You have to crawl under them to avoid dying
extern byte boss1[8];
extern byte boss2[8];

//for the death screen. Depends on cause of death
extern std::string message;


//for delaying the player getting hit
extern unsigned long lastDamageTime;
//for delaying boss moveemnt during boss fight
extern unsigned long lastBossMoveTime;

//for boss laser logic
extern int bossFiringBot;

//for creeper movement
extern bool moveFlag;
#endif