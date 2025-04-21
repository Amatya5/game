#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <stack>
#include <conio.h>
#include <windows.h>
#include <algorithm> // For shuffle

using namespace std;

// Colors and game symbols
enum Colors { BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3, RED = 4, MAGENTA = 5, YELLOW = 6, WHITE = 7, BRIGHT = 8 };
const char PLAYER = 'P', ENEMY = 'x', FOOD = '*', EXIT = 'E', WALL = '#', PATH = ' ', BULLET = '.';

// Game variables
int score = 0, level = 1, lives = 3, playerRow = 1, playerCol = 1;
int WIDTH = 15, HEIGHT = 11;
bool gameRunning = true;
vector<pair<int, int>> enemies;
vector<pair<int, int>> bullets;
vector<int> bulletDirections; // 0=up, 1=right, 2=down, 3=left
int playerDirection = 1; // Default direction player is facing (right)
int ammo = 5; // Starting ammo

// Random number generator (global to avoid initialization issues)
random_device rd;
mt19937 gen(rd());

// Console functions
void setColor(int textColor, int bgColor = BLACK) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (bgColor << 4) | textColor);
}

void gotoxy(int x, int y) {
    COORD coord = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO cursor;
    cursor.dwSize = 100;
    cursor.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor);
}

// Check if a cell is within bounds
bool isValid(int row, int col) {
    return (row >= 0 && row < HEIGHT && col >= 0 && col < WIDTH);
}

// Generate maze using recursive backtracker algorithm
void generateMaze(vector<vector<char>>& maze, int startRow, int startCol) {
    stack<pair<int, int>> s;
    s.push({startRow, startCol});
    maze[startRow][startCol] = PATH;

    while (!s.empty()) {
        pair<int, int> current = s.top();
        s.pop();

        int row = current.first;
        int col = current.second;

        int dr[] = {-2, 0, 2, 0};
        int dc[] = {0, 2, 0, -2};
        vector<int> directions = {0, 1, 2, 3};
        shuffle(directions.begin(), directions.end(), gen);

        for (int dir : directions) {
            int newRow = row + dr[dir];
            int newCol = col + dc[dir];

            if (isValid(newRow, newCol) && maze[newRow][newCol] == WALL) {
                maze[row + dr[dir]/2][col + dc[dir]/2] = PATH;
                maze[newRow][newCol] = PATH;
                s.push({newRow, newCol});
            }
        }
    }
}

// Find a random empty path location in the maze
pair<int, int> findEmptyPath(const vector<vector<char>>& maze) {
    uniform_int_distribution<> rowDist(1, HEIGHT-2);
    uniform_int_distribution<> colDist(1, WIDTH-2);

    int row, col;
    do {
        row = rowDist(gen);
        col = colDist(gen);
    } while (maze[row][col] != PATH);

    return {row, col};
}

// Place game elements in the maze
void placeGameElements(vector<vector<char>>& maze) {
    uniform_int_distribution<> dist(1, HEIGHT-2);

    // Place player
    playerRow = playerCol = 1;
    if (maze[playerRow][playerCol] != PATH) {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (maze[i][j] == PATH) {
                    playerRow = i;
                    playerCol = j;
                    i = HEIGHT; // Break outer loop
                    break;
                }
            }
        }
    }

    maze[playerRow][playerCol] = PLAYER;
    enemies.clear();
    bullets.clear();
    bulletDirections.clear();
    ammo = 5 + level; // Reset ammo for new level

    // Place exit
    bool exitPlaced = false;
    for (int i = HEIGHT-2; i > HEIGHT/2 && !exitPlaced; i--) {
        for (int j = WIDTH-2; j > WIDTH/2 && !exitPlaced; j--) {
            if (maze[i][j] == PATH) {
                maze[i][j] = EXIT;
                exitPlaced = true;
            }
        }
    }

    // Fallback if exit wasn't placed
    if (!exitPlaced) {
        for (int i = 1; i < HEIGHT-1 && !exitPlaced; i++) {
            for (int j = 1; j < WIDTH-1 && !exitPlaced; j++) {
                if (maze[i][j] == PATH && (i != playerRow || j != playerCol)) {
                    maze[i][j] = EXIT;
                    exitPlaced = true;
                }
            }
        }
    }

    // Place enemies
    int enemyCount = min(2 + level, (WIDTH * HEIGHT) / 10);
    while (enemies.size() < enemyCount) {
        pair<int, int> pos = findEmptyPath(maze);
        int r = pos.first;
        int c = pos.second;

        if ((r != playerRow || c != playerCol)) {
            maze[r][c] = ENEMY;
            enemies.push_back({r, c});
        }
    }

    // Place food (includes ammo pickups now)
    int foodCount = min(5 + level, (WIDTH * HEIGHT) / 8);
    int foodPlaced = 0;
    while (foodPlaced < foodCount) {
        pair<int, int> pos = findEmptyPath(maze);
        int r = pos.first;
        int c = pos.second;

        if ((r != playerRow || c != playerCol) && maze[r][c] == PATH) {
            maze[r][c] = FOOD;
            foodPlaced++;
        }
    }
}

// Print the maze with colors
void printMaze(const vector<vector<char>>& maze) {
    // Create a copy of the maze with bullets for display
    vector<vector<char>> displayMaze = maze;

    // Add bullets to the display maze
    for (auto& bullet : bullets) {
        if (isValid(bullet.first, bullet.second) && displayMaze[bullet.first][bullet.second] == PATH) {
            displayMaze[bullet.first][bullet.second] = BULLET;
        }
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            gotoxy(j * 2, i + 3);

            switch(displayMaze[i][j]) {
                case WALL:
                    setColor(BRIGHT | CYAN, CYAN);
                    cout << WALL << WALL;
                    break;
                case PATH:
                    setColor(BLACK, BLACK);
                    cout << "  ";
                    break;
                case PLAYER:
                    setColor(BRIGHT | GREEN, BLACK);
                    cout << PLAYER << ' ';
                    break;
                case ENEMY:
                    setColor(BRIGHT | RED, BLACK);
                    cout << ENEMY << ' ';
                    break;
                case FOOD:
                    setColor(BRIGHT | YELLOW, BLACK);
                    cout << FOOD << ' ';
                    break;
                case EXIT:
                    setColor(BRIGHT | MAGENTA, BLACK);
                    cout << EXIT << ' ';
                    break;
                case BULLET:
                    setColor(BRIGHT | WHITE, BLACK);
                    cout << BULLET << ' ';
                    break;
                default:
                    cout << displayMaze[i][j] << ' ';
            }
        }
    }
}

// Print the HUD
void printHUD() {
    gotoxy(0, 0);
    setColor(WHITE, BLACK);
    cout << "MAZE RUNNER - Level: " << level << " | Score: " << score << " | Lives: " << lives << " | Ammo: " << ammo;
    gotoxy(0, 1);
    cout << "Controls: W,A,S,D to move | SPACE to shoot | ESC to quit | Collect " << FOOD << " for points & ammo";
}

// Shoot bullet in the direction player is facing
void shootBullet() {
    if (ammo <= 0) return;

    ammo--;

    // Set initial bullet position
    int bulletRow = playerRow;
    int bulletCol = playerCol;

    // Move bullet one space in the firing direction initially
    const int dr[] = {-1, 0, 1, 0}; // up, right, down, left
    const int dc[] = {0, 1, 0, -1};

    bulletRow += dr[playerDirection];
    bulletCol += dc[playerDirection];

    // Only add bullet if it's in a valid position
    if (isValid(bulletRow, bulletCol) && (bulletCol != WALL)) {
        bullets.push_back({bulletRow, bulletCol});
        bulletDirections.push_back(playerDirection);
    }
}

// Move bullets and check collisions
void updateBullets(vector<vector<char>>& maze) {
    const int dr[] = {-1, 0, 1, 0}; // up, right, down, left
    const int dc[] = {0, 1, 0, -1};

    for (int i = 0; i < bullets.size(); i++) {
        // Current position
        int row = bullets[i].first;
        int col = bullets[i].second;

        // Calculate new position
        int newRow = row + dr[bulletDirections[i]];
        int newCol = col + dc[bulletDirections[i]];

        // Check if bullet is out of bounds or hits a wall
        if (!isValid(newRow, newCol) || maze[newRow][newCol] == WALL) {
            // Remove this bullet
            bullets.erase(bullets.begin() + i);
            bulletDirections.erase(bulletDirections.begin() + i);
            i--; // Adjust index after removing
            continue;
        }

        // Check if bullet hits an enemy
        if (maze[newRow][newCol] == ENEMY) {
            // Find and remove the enemy
            for (int j = 0; j < enemies.size(); j++) {
                if (enemies[j].first == newRow && enemies[j].second == newCol) {
                    enemies.erase(enemies.begin() + j);
                    break;
                }
            }

            // Update maze and add score
            maze[newRow][newCol] = PATH;
            score += 15; // More points for shooting an enemy

            // Remove the bullet
            bullets.erase(bullets.begin() + i);
            bulletDirections.erase(bulletDirections.begin() + i);
            i--; // Adjust index after removing
            continue;
        }

        // Move the bullet
        bullets[i].first = newRow;
        bullets[i].second = newCol;
    }
}

// Move enemies randomly
void moveEnemies(vector<vector<char>>& maze) {
    uniform_int_distribution<> dirDist(0, 3);

    for (auto& enemy : enemies) {
        int oldRow = enemy.first;
        int oldCol = enemy.second;

        const int dr[] = {-1, 1, 0, 0};
        const int dc[] = {0, 0, -1, 1};

        int dir = dirDist(gen);
        int newRow = oldRow + dr[dir];
        int newCol = oldCol + dc[dir];

        if (isValid(newRow, newCol) && maze[newRow][newCol] != WALL &&
            maze[newRow][newCol] != EXIT && maze[newRow][newCol] != ENEMY &&
            maze[newRow][newCol] != FOOD) {

            if (maze[newRow][newCol] == PLAYER) {
                lives--;
                if (lives <= 0) {
                    gameRunning = false;
                    return;
                }

                maze[playerRow][playerCol] = PATH;
                pair<int, int> pos = findEmptyPath(maze);
                playerRow = pos.first;
                playerCol = pos.second;
                maze[playerRow][playerCol] = PLAYER;
                continue;
            }

            maze[oldRow][oldCol] = PATH;
            maze[newRow][newCol] = ENEMY;
            enemy = {newRow, newCol};
        }
    }
}

// Handle player movement
bool movePlayer(vector<vector<char>>& maze, int newRow, int newCol) {
    if (!isValid(newRow, newCol) || maze[newRow][newCol] == WALL) {
        return false;
    }

    // Update player direction based on movement
    if (newRow < playerRow) playerDirection = 0; // up
    else if (newCol > playerCol) playerDirection = 1; // right
    else if (newRow > playerRow) playerDirection = 2; // down
    else if (newCol < playerCol) playerDirection = 3; // left

    char cell = maze[newRow][newCol];

    // Handle player collision with enemy
    if (cell == ENEMY) {
        lives--;
        if (lives <= 0) {
            gameRunning = false;
            return false;
        }

        maze[playerRow][playerCol] = PATH;
        pair<int, int> pos = findEmptyPath(maze);
        playerRow = pos.first;
        playerCol = pos.second;
        maze[playerRow][playerCol] = PLAYER;
        return false;
    }
    // Handle food collection
    else if (cell == FOOD) {
        score += 10;
        ammo += 2; // Each food gives 2 bullets
    }
    // Handle reaching exit
    else if (cell == EXIT) {
        level++;
        return true; // Level complete
    }

    maze[playerRow][playerCol] = PATH;
    maze[newRow][newCol] = PLAYER;
    playerRow = newRow;
    playerCol = newCol;
    return false;
}

// Main function
int main() {
    system("title Maze Runner Game");
    system("mode con: cols=80 lines=30");
    hideCursor();

    // Seed random number generator
    srand(time(NULL));

    // Main game loop
    while (gameRunning) {
        // Initialize and generate maze
        vector<vector<char>> maze(HEIGHT, vector<char>(WIDTH, WALL));
        uniform_int_distribution<> rowDist(1, HEIGHT-2);
        uniform_int_distribution<> colDist(1, WIDTH-2);

        int startRow = rowDist(gen) | 1; // Ensure it's odd
        int startCol = colDist(gen) | 1; // Ensure it's odd

        generateMaze(maze, startRow, startCol);
        placeGameElements(maze);

        bool levelComplete = false;
        system("cls");

        // Game loop for current level
        while (gameRunning && !levelComplete) {
            printHUD();
            printMaze(maze);

            // Update bullets
            updateBullets(maze);

            if (_kbhit()) {
                char key = tolower(_getch());

                int newRow = playerRow;
                int newCol = playerCol;

                switch (key) {
                    case 'w': newRow--; break;
                    case 's': newRow++; break;
                    case 'a': newCol--; break;
                    case 'd': newCol++; break;
                    case ' ': shootBullet(); break; // Space to shoot
                    case 27: gameRunning = false; break; // ESC key
                }

                if (newRow != playerRow || newCol != playerCol) {
                    levelComplete = movePlayer(maze, newRow, newCol);
                }
            }

            static int enemyMoveCounter = 0;
            if (++enemyMoveCounter >= max(5, 10-level/2)) {
                moveEnemies(maze);
                enemyMoveCounter = 0;
            }

            Sleep(50);
        }

        // Increase difficulty
        if (levelComplete && level % 3 == 0 && WIDTH < 30) {
            WIDTH += 2;
            HEIGHT += 2;
        }
    }

    // Game over
    system("cls");
    setColor(WHITE, BLACK);
    cout << "\n\n\tGame Over!\n\n";
    cout << "\tFinal Score: " << score << "\n";
    cout << "\tLevels Completed: " << level-1 << "\n\n";
    cout << "\tPress any key to exit...";
    _getch();

    return 0;
}

