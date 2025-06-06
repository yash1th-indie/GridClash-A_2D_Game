#include <iostream>
#include <queue>
#include <vector>
#include <utility>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cmath>
using namespace std;

// Game constants
const int width = 50;
const int height = 15;
const int dashstaminacost = 20;
const int maxhealth = 70;
const int lasermanacost = 30;

// Global game state
int framecount = 0;
bool isdashing1 = false, isdashing2 = false;
int dashduration = 4;
int dashcooldown = 10;
int dashcooldowntimer1 = 0, dashcooldowntimer2 = 0;
int dashtimer1 = 0, dashtimer2 = 0;
int lastmovex1 = 0, lastmovey1 = 0, lastmovex2 = 0, lastmovey2 = 0;

// Laser system
bool laseractive1 = false, laseractive2 = false;
bool laserwarning1 = false, laserwarning2 = false;
int laserwarningtimer1 = 0, laserwarningtimer2 = 0;
const int laserduration = 8;
int lasertimer1 = 0, lasertimer2 = 0;
vector<pair<int, int>> lasertiles1, lasertiles2;

void hidecursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor;
    GetConsoleCursorInfo(console, &cursor);
    cursor.bVisible = false;
    SetConsoleCursorInfo(console, &cursor);
}

class Entity {
public:
    int x, y;
    int health;
    int stamina;
    int mana;

    Entity(int startx, int starty, int starthealth = 70, int startstamina = 100, int startmana = 100) {
        x = startx;
        y = starty;
        health = starthealth;
        stamina = startstamina;
        mana = startmana;
    }
};

Entity player1(10, 5);
Entity player2(width - 10, 5);

class Projectile {
public:
    int x, y;
    int dx, dy;
    bool isPlayer1;

    Projectile(int startx, int starty, int movex, int movey, bool isP1) {
        x = startx;
        y = starty;
        dx = movex;
        dy = movey;
        isPlayer1 = isP1;
    }

    void move() {
        x += dx;
        y += dy;
    }

    bool outofbounds() {
        return x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1;
    }
};

queue<Projectile> projectiles;

void drawbars() {
    cout << "Player 1 - HP: ";
    for (int i = 0; i < (player1.health * 20) / maxhealth; i++) cout << "#";
    cout << " (" << player1.health << ") ";
    
    cout << "Stamina: ";
    for (int i = 0; i < player1.stamina / 10; i++) cout << "+";
    cout << " (" << player1.stamina << ") ";
    
    cout << "Mana: ";
    for (int i = 0; i < player1.mana / 10; i++) cout << "*";
    cout << " (" << player1.mana << ")\n";

    cout << "Player 2 - HP: ";
    for (int i = 0; i < (player2.health * 20) / maxhealth; i++) cout << "#";
    cout << " (" << player2.health << ") ";
    
    cout << "Stamina: ";
    for (int i = 0; i < player2.stamina / 10; i++) cout << "+";
    cout << " (" << player2.stamina << ") ";
    
    cout << "Mana: ";
    for (int i = 0; i < player2.mana / 10; i++) cout << "*";
    cout << " (" << player2.mana << ")\n\n";
}

void startlaserwarning(bool isPlayer1) {
    if (isPlayer1) {
        laserwarning1 = true;
        laserwarningtimer1 = 7;
        lasertiles1.clear();
        for (int i = 1; i < width-1; i++) {
            lasertiles1.emplace_back(i, player1.y);
        }
    } else {
        laserwarning2 = true;
        laserwarningtimer2 = 7;
        lasertiles2.clear();
        for (int i = 1; i < width-1; i++) {
            lasertiles2.emplace_back(i, player2.y);
        }
    }
}

void activatelaser(bool isPlayer1) {
    if (isPlayer1) {
        laserwarning1 = false;
        laseractive1 = true;
        lasertimer1 = laserduration;
        player1.mana -= lasermanacost;
    } else {
        laserwarning2 = false;
        laseractive2 = true;
        lasertimer2 = laserduration;
        player2.mana -= lasermanacost;
    }
}

void updatelasers() {
    // Player 1 laser
    if (laserwarning1) {
        laserwarningtimer1--;
        if (laserwarningtimer1 <= 0) {
            activatelaser(true);
        }
    }
    if (laseractive1) {
        lasertimer1--;
        if (lasertimer1 <= 0) {
            laseractive1 = false;
            lasertiles1.clear();
        }
    }

    // Player 2 laser
    if (laserwarning2) {
        laserwarningtimer2--;
        if (laserwarningtimer2 <= 0) {
            activatelaser(false);
        }
    }
    if (laseractive2) {
        lasertimer2--;
        if (lasertimer2 <= 0) {
            laseractive2 = false;
            lasertiles2.clear();
        }
    }
}

void drawarena() {
    system("cls");
    cout << "DUEL MODE\n";
    drawbars();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool drawn = false;

            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                cout << "#";
                continue;
            }

            if (x == player1.x && y == player1.y) {
                cout << "1";
                continue;
            }

            if (x == player2.x && y == player2.y) {
                cout << "2";
                continue;
            }

            // Draw laser warnings
            if (laserwarning1) {
                for (auto& tile : lasertiles1) {
                    if (x == tile.first && y == tile.second) {
                        cout << ".";
                        drawn = true;
                        break;
                    }
                }
                if (drawn) continue;
            }

            if (laserwarning2) {
                for (auto& tile : lasertiles2) {
                    if (x == tile.first && y == tile.second) {
                        cout << ".";
                        drawn = true;
                        break;
                    }
                }
                if (drawn) continue;
            }

            // Draw active lasers
            if (laseractive1) {
                for (auto& tile : lasertiles1) {
                    if (x == tile.first && y == tile.second) {
                        cout << "|";
                        drawn = true;
                        break;
                    }
                }
                if (drawn) continue;
            }

            if (laseractive2) {
                for (auto& tile : lasertiles2) {
                    if (x == tile.first && y == tile.second) {
                        cout << "|";
                        drawn = true;
                        break;
                    }
                }
                if (drawn) continue;
            }

            // Draw projectiles
            queue<Projectile> temp = projectiles;
            while (!temp.empty()) {
                Projectile p = temp.front(); temp.pop();
                if (p.x == x && p.y == y) {
                    cout << (p.isPlayer1 ? "*" : "o");
                    drawn = true;
                    break;
                }
            }
            if (drawn) continue;

            if (!drawn) cout << " ";
        }
        cout << endl;
    }
}

void updateshots() {
    int count = projectiles.size();
    for (int i = 0; i < count; i++) {
        Projectile p = projectiles.front();
        projectiles.pop();
        p.move();

        if (p.isPlayer1) {
            if (p.x == player2.x && p.y == player2.y) {
                if (!isdashing2) player2.health -= 2;
                continue;
            }
        } else {
            if (p.x == player1.x && p.y == player1.y) {
                if (!isdashing1) player1.health -= 2;
                continue;
            }
        }

        if (!p.outofbounds()) projectiles.push(p);
    }
}

void updateplayers() {
    if (_kbhit()) {
        char key = _getch();
        
        // Player 1 movement
        int newx1 = player1.x;
        int newy1 = player1.y;
        if (key == 'w' && player1.y > 1) { newy1--; lastmovex1 = 0; lastmovey1 = -1; }
        if (key == 's' && player1.y < height - 2) { newy1++; lastmovex1 = 0; lastmovey1 = 1; }
        if (key == 'a' && player1.x > 1) { newx1--; lastmovex1 = -1; lastmovey1 = 0; }
        if (key == 'd' && player1.x < width - 2) { newx1++; lastmovex1 = 1; lastmovey1 = 0; }

        if (newx1 != player1.x || newy1 != player1.y) {
            player1.x = newx1;
            player1.y = newy1;
        }

        // Player 1 shooting
        if (key == 'y') projectiles.push(Projectile(player1.x, player1.y - 1, 0, -1, true));
        if (key == 'h') projectiles.push(Projectile(player1.x, player1.y + 1, 0, 1, true));
        if (key == 'g') projectiles.push(Projectile(player1.x - 1, player1.y, -1, 0, true));
        if (key == 'j') projectiles.push(Projectile(player1.x + 1, player1.y, 1, 0, true));

        // Player 1 dash
        if (key == ' ' && dashcooldowntimer1 == 0 && player1.stamina >= dashstaminacost && (lastmovex1 != 0 || lastmovey1 != 0)) {
            isdashing1 = true;
            dashtimer1 = dashduration;
            dashcooldowntimer1 = dashcooldown;
            player1.stamina -= dashstaminacost;

            for (int i = 0; i < 3; i++) {
                int newx = player1.x + lastmovex1;
                int newy = player1.y + lastmovey1;

                if (newx > 0 && newx < width - 1) player1.x = newx;
                if (newy > 0 && newy < height - 1) player1.y = newy;
            }
        }

        // Player 1 laser
        if (key == 'u' && !laserwarning1 && !laseractive1 && player1.mana >= lasermanacost) {
            startlaserwarning(true);
        }

        // Player 2 movement
        int newx2 = player2.x;
        int newy2 = player2.y;
        if (key == 'p' && player2.y > 1) { newy2--; lastmovex2 = 0; lastmovey2 = -1; }
        if (key == ';' && player2.y < height - 2) { newy2++; lastmovex2 = 0; lastmovey2 = 1; }
        if (key == 'l' && player2.x > 1) { newx2--; lastmovex2 = -1; lastmovey2 = 0; }
        if (key == '\'' && player2.x < width - 2) { newx2++; lastmovex2 = 1; lastmovey2 = 0; }

        if (newx2 != player2.x || newy2 != player2.y) {
            player2.x = newx2;
            player2.y = newy2;
        }

        // Player 2 shooting
        if (key == '5') projectiles.push(Projectile(player2.x, player2.y - 1, 0, -1, false));
        if (key == '2') projectiles.push(Projectile(player2.x, player2.y + 1, 0, 1, false));
        if (key == '1') projectiles.push(Projectile(player2.x - 1, player2.y, -1, 0, false));
        if (key == '3') projectiles.push(Projectile(player2.x + 1, player2.y, 1, 0, false));

        // Player 2 dash
        if (key == '0' && dashcooldowntimer2 == 0 && player2.stamina >= dashstaminacost && (lastmovex2 != 0 || lastmovey2 != 0)) {
            isdashing2 = true;
            dashtimer2 = dashduration;
            dashcooldowntimer2 = dashcooldown;
            player2.stamina -= dashstaminacost;

            for (int i = 0; i < 3; i++) {
                int newx = player2.x + lastmovex2;
                int newy = player2.y + lastmovey2;

                if (newx > 0 && newx < width - 1) player2.x = newx;
                if (newy > 0 && newy < height - 1) player2.y = newy;
            }
        }

        // Player 2 laser
        if (key == '9' && !laserwarning2 && !laseractive2 && player2.mana >= lasermanacost) {
            startlaserwarning(false);
        }

        if (key == 'q') exit(0);
    }

    // Regenerate stamina and mana
    if (framecount % 5 == 0) {
        if (player1.stamina < 100) player1.stamina += 3;
        if (player2.stamina < 100) player2.stamina += 3;
        if (player1.mana < 100) player1.mana += 2;
        if (player2.mana < 100) player2.mana += 2;
    }

    // Update dash cooldowns
    if (dashcooldowntimer1 > 0) dashcooldowntimer1--;
    if (dashcooldowntimer2 > 0) dashcooldowntimer2--;
    if (dashtimer1 > 0) {
        dashtimer1--;
        if (dashtimer1 == 0) isdashing1 = false;
    }
    if (dashtimer2 > 0) {
        dashtimer2--;
        if (dashtimer2 == 0) isdashing2 = false;
    }
}

void checklaserdamage() {
    if (laseractive1) {
        for (auto& tile : lasertiles1) {
            if (player2.x == tile.first && player2.y == tile.second && !isdashing2) {
                player2.health -= 1;
                break;
            }
        }
    }
    if (laseractive2) {
        for (auto& tile : lasertiles2) {
            if (player1.x == tile.first && player1.y == tile.second && !isdashing1) {
                player1.health -= 1;
                break;
            }
        }
    }
}

void duel() {
    player1 = Entity(10, 5);
    player2 = Entity(width - 10, 5);
    projectiles = queue<Projectile>();
    laseractive1 = laseractive2 = false;
    laserwarning1 = laserwarning2 = false;
    
    while (true) {
        drawarena();
        updateplayers();
        updateshots();
        updatelasers();
        checklaserdamage();

        if (player1.health <= 0) {
            system("cls");
            cout << "PLAYER 2 WINS!\n";
            break;
        }

        if (player2.health <= 0) {
            system("cls");
            cout << "PLAYER 1 WINS!\n";
            break;
        }

        framecount++;
    }
}

int main() {
    hidecursor();
    srand(time(0));

    cout << "GRID CLASH - DUEL MODE\n\n";
    cout << "PLAYER 1 CONTROLS:\n";
    cout << "  MOVEMENT: WASD\n";
    cout << "  SHOOTING: YGHJ (Up/Left/Down/Right)\n";
    cout << "  DASH: SPACEBAR\n";
    cout << "  LASER: U\n\n";
    
    cout << "PLAYER 2 CONTROLS:\n";
    cout << "  MOVEMENT: PL;'\n";
    cout << "  SHOOTING: 5123 (Up/Left/Down/Right)\n";
    cout << "  DASH: 0\n";
    cout << "  LASER: 9\n\n";
    
    cout << "Press any key to start...";
    _getch();

    duel();

    cout << "\nThanks for playing! Press any key to exit...";
    _getch();
    return 0;
}