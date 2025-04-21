# Maze Runner Game

A console-based C++ game where you navigate a procedurally generated maze, avoid enemies, collect food, shoot bullets, and reach the exit to progress through levels. Developed using Windows console graphics and logic-based movement AI.

## Features

- Procedural maze generation (recursive backtracking algorithm)
- Color-coded game elements
- Enemies with random movement
- Food pickups for points and extra ammo
- Bullet shooting in four directions
- Level progression and increasing difficulty
- Console graphics using Windows API
- Adaptive HUD with controls and stats

## Gameplay

You start with 3 lives and 5 bullets. Each level is a new randomly generated maze filled with enemies (x), food (*), and an exit (E). Your goal is to reach the exit while collecting food and avoiding or shooting enemies. The game ends when your lives drop to 0.

### Controls

- W - Move Up  
- A - Move Left  
- S - Move Down  
- D - Move Right  
- Space - Shoot in the direction you're facing  
- ESC - Quit the game

### Scoring

- +10 points for each food collected  
- +15 points for each enemy shot  
- Extra ammo is rewarded via food pickups

### Ammo

- You begin with 5 bullets (increased slightly each level)
- Each shot costs 1 ammo
- Food gives +2 ammo per pickup

## Winning Criteria

- Reach the exit (E) to complete the level
- Survive as many levels as you can
- Score as high as possible

## Game Over Conditions

- Player loses all 3 lives
- Collision with enemies reduces life
- When lives reach 0, the game ends

## Installation & Compilation

### Requirements

- Windows OS
- C++ compiler that supports Windows API (e.g., MinGW, MSVC)

### How to Run

Using codeblocks on Windows:
