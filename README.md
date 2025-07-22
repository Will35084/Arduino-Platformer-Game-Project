# Platformer-Game-Project

Overview
This is a 2D platformer game built for Arduino using an LCD screen and joystick controller. The game features:
- A hero character that can jump, walk, sprint, attack, and crawl omnidirectionally
- Various enemies (Crawlers, Creepers, Clingers)
- A boss battle
- Collectible coins and a points system
- Three lives and a damage system
- Obstacles and interactive elements (lasers, portals, movable blocks, disappearing blocks)
- Multiple game states (title screen, gameplay, win/lose screens)

Hardware Requirements
- Arduino Uno R4 Minima (other boards not tested)
- LCD screen with I2C 20x4 interface
- Joystick module
- Push button
- Passive Buzzer for sound effects

Software Requirements
- LiquidCrystalI2C Library
- I used Platformio on VSCode. Create a project for the specified board, install the LiquidCrystalI2C library, add the scripts to the src folder, and you're good to go
  
Pin Connections:
#define BUZZER_PIN 8
#define JOY_BUTTON 4
#define JOY_H A0
#define JOY_V A1
#define A_BUTTON 2

Game Features
- Character Movement:
  - Face left, right, up-left, up-right, straight-up and down with joystick
  - Jumping in a direction specified with a joystick by pressing the pushbutton
  - Crawling (to avoid ceiling enemies) by holding the joystick diagonally down while pressing pushbutton
  - Sprinting by walking continuously for a set amount of time
  - Attacking with sword slash by pressing the joystick module botton while facing a direction

- Enemies:
  - Crawlers: Ground enemies that must be jumped on and can't be attacked. They exhibit similar movement as the player and can jump onto and down from blocks. They constantly move towards the player
  - Creepers: Tall enemies that must be attacked and can't be jumped on. They are more limited in movement and can only move horizontally. From a far distance, they shoot lasers at players. From a short distance, they run away from players until
    they are far enough away to shoot lasers again.
  - Clingers: Ceiling enemies that must be crawled under. They are also on the floor and are used as generic obstacle blocks to be jumped over.
  - Boss: Special enemy with unique behavior. Hard-coded movement forward and backwards. The only enemy that is two tiles tall. Shoots lasers like creepers

Game Elements:
- Coins (gives the player 2 points for each collected)
- Portals (transport to boss level and the win screen)
- Lasers (hazard made by the creeper and boss enemies)
- Movable blocks (are moved by attacking them in the direction you want them moved)
- Fake blocks (normal blocks that turn into disappearing ones if you walk too close to them)
- Debris (the "disappearing blocks". They disappear after a delay if walked on. They can also be broken by attacking them)

Game States
- TITLE_SCREEN: Initial screen with blinking "Hit Button To Play" message
- START: Initializes game variables and player position
- START_2: Main gameplay state
- BOSS: Boss battle state
- LOSE: Game over screen with death message
- WIN: Victory screen showing final score

How to Play
- Connect all hardware components as specified
- Upload the code to your Arduino
- Press the push button to start from the title screen
- Use the joystick and pushbutton to move and the joystick button to attack
- Collect coins for points
- Reach the portal to face the boss
- Defeat the boss to win the game

Sound Effects
The game includes sound effects for:
- Jumping
- Attacking
- Collecting coins
- Taking damage
- Dying
- Killing enemies
- Laser firing

Customization
You can modify:
- Level design directly in the Level array
- Enemy behavior parameters (speed, spawn rate)
- Character sprites (8x8 custom characters)
- Sound effects (note frequencies and durations)

File Structure
- main.cpp: Main game loop and state machine
- game.h: Game definitions and level handling, special block handling, sound handling
- player.h: Player character definitions and controls
- movement.h: Movement and physics handling
- enemy.h: Enemy definitions and behaviors

Future Improvements
- Maybe a level select screen with multiple levels
- Additional enemy types. I was considering adding an enemy that teleports to another part of the screen to shoot at you
- Power-ups: A shield that lets you take a hit without damage, a heart that gives you another life, a OHKO item?
- Save high scores
- More sophisticated sound system. Adding another buzzer would allow level music to play
- Optimise creepers more (Make it so they only shoot you if you are in their line of sight)

Credits
Created by William Dougherty (Will35084) using Arduino and the LiquidCrystal_I2C library.
