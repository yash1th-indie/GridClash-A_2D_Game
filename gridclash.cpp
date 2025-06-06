#include <iostream>
#include <queue>
#include <vector>
#include <utility>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cmath>
using namespace std;

const int width = 50;
const int height = 15;
const int dashcost = 20;
const int maxhp = 70;

int frame = 0;
bool dashing = false;
int dashlength = 4;
int dashwait = 10;
int dashwaittimer = 0;
int dashtime = 0;
int lastx = 0, lasty = 0;

int bossdelay = 6;
int bossmovewait = 3;
int chase = 3;
int laserwait = 60;
int laserwaittimer = 0;
bool laseron = false;
bool laserwarn = false;
int laserwarntime = 0;
int lasertime = 8;
int lasertimer = 0;
vector<pair<int, int>> lasercells;

const int minionhp = 2;
const int miniondmg = 1;
const int minionwait = 100;
int miniontimer = 0;
int maxminions = 6;
vector<pair<int, int>> spawneffects;

struct wave {
    int x, y;
    int size;
    bool grow;
};
vector<wave> waves;
int wavewait = 0;
const int maxwavewait = 100;
const int wavedmg = 3;

void hidecursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cur;
    GetConsoleCursorInfo(console, &cur);
    cur.bVisible = false;
    SetConsoleCursorInfo(console, &cur);
}

class thing {
public:
    int x, y;
    int hp;
    int stamina;

    thing(int startx, int starty, int starthp = 10, int startstam = 100) {
        x = startx;
        y = starty;
        hp = starthp;
        stamina = startstam;
    }
};

class minion : public thing {
public:
    minion(int startx, int starty) : thing(startx, starty, minionhp) {}
};

thing player(5, 5, 70, 100);
thing boss(20, 5, maxhp, 0);
vector<minion> minions;

class bullet {
public:
    int x, y;
    int dx, dy;

    bullet(int startx, int starty, int movex, int movey) {
        x = startx;
        y = starty;
        dx = movex;
        dy = movey;
    }

    void move() {
        x += dx;
        y += dy;
    }

    bool out() {
        return x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1;
    }
};

queue<bullet> pbullets;
queue<bullet> bbullets;

void drawhp(bool level1 = false) {
    cout << "Player HP: ";
    for (int i = 0; i < (player.hp * 20) / maxhp; i++) cout << "#";
    cout << " (" << player.hp << ")\n";

    cout << "Player Stamina: ";
    for (int i = 0; i < player.stamina / 10; i++) cout << "+";
    cout << " (" << player.stamina << ")\n";

    if (level1) {
        cout << "Target HP: ";
        int bossbars = (boss.hp * 20) / 30;
        for (int i = 0; i < bossbars; i++) cout << "#";
        cout << " (" << boss.hp << "/30)\n\n\n";
    } else {
        cout << "Boss HP: ";
        int bossbars = (boss.hp * 20) / maxhp;
        for (int i = 0; i < bossbars; i++) cout << "#";
        cout << " (" << boss.hp << "/" << maxhp << ")\n\n\n";
    }
}

void startlaserwarn() {
    laserwarn = true;
    laserwarntime = 7;
    
    lasercells.clear();
    for (int i = 1; i < width-1; i++) {
        lasercells.emplace_back(i, boss.y);
        lasercells.emplace_back(i, boss.y-1);
        lasercells.emplace_back(i, boss.y+1);
    }
    for (int i = 1; i < height-1; i++) {
        lasercells.emplace_back(boss.x, i);
        lasercells.emplace_back(boss.x-1, i);
        lasercells.emplace_back(boss.x+1, i);
    }
}

void firelaser() {
    laserwarn = false;
    laseron = true;
    lasertimer = lasertime;
}

void updatelaser() {
    if (laserwaittimer > 0) {
        laserwaittimer--;
        return;
    }

    if (laserwarn) {
        laserwarntime--;
        if (laserwarntime <= 0) {
            firelaser();
        }
        return;
    }

    if (laseron) {
        lasertimer--;
        if (lasertimer <= 0) {
            laseron = false;
            laserwaittimer = laserwait;
            lasercells.clear();
        }
        return;
    }

    if (rand() % 10 == 0) {
        startlaserwarn();
    }
}

void makewave() {
    waves.push_back({boss.x, boss.y, 1, true});
}

void updatewaves() {
    for (size_t i = 0; i < waves.size(); ) {
        if (frame % 3 == 0) {
            if (waves[i].grow) {
                waves[i].size++;
                if (waves[i].size > 13) {
                    waves[i].grow = false;
                }
            } else {
                waves[i].size--;
                if (waves[i].size <= 0) {
                    waves.erase(waves.begin() + i);
                    continue;
                }
            }
        }

        int dx = abs(player.x - waves[i].x);
        int dy = abs(player.y - waves[i].y);
        double dist = sqrt(dx*dx + dy*dy);
        
        if (dist <= waves[i].size + 0.5 && 
            dist >= waves[i].size - 0.5 && 
            !dashing) {
            player.hp -= wavedmg;
            cout << '\a';
        }

        i++;
    }
}

void updateminions() {
    if (!spawneffects.empty()) {
        spawneffects.clear();
    }

    if (miniontimer <= 0 && minions.size() < maxminions && boss.hp < maxhp*0.4) {
        for (int i = 0; i < 3; i++) {
            int offx = (rand() % 5) - 2;
            int offy = (rand() % 5) - 2;
        
            int spawnx = boss.x + offx;
            int spawny = boss.y + offy;
        
            if (spawnx > 0 && spawnx < width-1 && spawny > 0 && spawny < height-1) {
                spawneffects.emplace_back(spawnx, spawny);
                minions.emplace_back(spawnx, spawny);
            }
        }
        miniontimer = minionwait;
    } else if (miniontimer > 0) {
        miniontimer--;
    }

    for (auto& m : minions) {
        if (frame % 4 == 0) { 
            if (m.x < player.x && m.x < width-2) m.x++;
            else if (m.x > player.x && m.x > 1) m.x--;
        
            if (m.y < player.y && m.y < height-2) m.y++;
            else if (m.y > player.y && m.y > 1) m.y--;
        }
    }

    for (size_t i = 0; i < minions.size(); ) {
        if (minions[i].x == player.x && minions[i].y == player.y && !dashing) {
            player.hp -= miniondmg;
            cout << '\a';
            minions.erase(minions.begin() + i);
        } else {
            i++;
        }
    }
}

void drawgame(bool level1 = false) {
    system("cls");
    if (level1) {
        cout << "LEVEL 1 - TRAINING\n";
    } else {
        cout << "LEVEL 2 - BOSS FIGHT\n";
    }
    drawhp(level1);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool drew = false;

            if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                cout << "#";
                continue;
            }

            if (x == player.x && y == player.y) {
                cout << "P";
                continue;
            }

            if (x == boss.x && y == boss.y) {
                cout << (level1 ? "T" : "B");
                continue;
            }

            if (!level1) {
                for (auto& eff : spawneffects) {
                    if (x == eff.first && y == eff.second) {
                        cout << "*";
                        drew = true;
                        break;
                    }
                }
                if (drew) continue;

                for (auto& m : minions) {
                    if (x == m.x && y == m.y) {
                        cout << "m";
                        drew = true;
                        break;
                    }
                }
                if (drew) continue;

                if (laserwarn) {
                    for (auto& cell : lasercells) {
                        if (x == cell.first && y == cell.second) {
                            cout << ".";
                            drew = true;
                            break;
                        }
                    }
                    if (drew) continue;
                }

                if (laseron) {
                    for (auto& cell : lasercells) {
                        if (x == cell.first && y == cell.second) {
                            cout << "#";
                            drew = true;
                            break;
                        }
                    }
                    if (drew) continue;
                }

                for (auto& w : waves) {
                    int dx = abs(x - w.x);
                    int dy = abs(y - w.y);
                    double dist = sqrt(dx*dx + dy*dy);
                    
                    if (dist >= w.size - 0.5 && dist <= w.size + 0.5) {
                        cout << "~";
                        drew = true;
                        break;
                    }
                }
                if (drew) continue;
            }

            queue<bullet> temp = pbullets;
            while (!temp.empty()) {
                bullet b = temp.front(); temp.pop();
                if (b.x == x && b.y == y) {
                    cout << "*";
                    drew = true;
                    break;
                }
            }
            if (drew) continue;

            if (!level1) {
                temp = bbullets;
                while (!temp.empty()) {
                    bullet b = temp.front(); temp.pop();
                    if (b.x == x && b.y == y) {
                        cout << "o";
                        drew = true;
                        break;
                    }
                }
            }
            if (drew) continue;

            if (!drew) cout << " ";
        }
        cout << endl;
    }
}

void updatebullets(queue<bullet>& bullets, bool playerbullets, bool level1 = false) {
    int count = bullets.size();
    for (int i = 0; i < count; i++) {
        bullet b = bullets.front();
        bullets.pop();
        b.move();

        if (!playerbullets) {
            if (b.x == player.x && b.y == player.y) {
                if (!dashing) player.hp -= 2;
                continue;
            }
        } else {
            bool hit = false;
            
            if (b.x == boss.x && b.y == boss.y) {
                boss.hp -= 2;
                hit = true;
            }
            
            if (!level1) {
                for (size_t j = 0; j < minions.size(); j++) {
                    if (b.x == minions[j].x && b.y == minions[j].y) {
                        minions[j].hp -= 2;
                        if (minions[j].hp <= 0) {
                            minions.erase(minions.begin() + j);
                        }
                        hit = true;
                        break;
                    }
                }
            }
            
            if (hit) continue;
        }

        if (!b.out()) bullets.push(b);
    }
}

void updateplayer() {
    if (_kbhit()) {
        char key = _getch();
        int newx = player.x;
        int newy = player.y;

        if (key == 'w' && player.y > 1) { newy--; lastx = 0; lasty = -1; }
        if (key == 's' && player.y < height - 2) { newy++; lastx = 0; lasty = 1; }
        if (key == 'a' && player.x > 1) { newx--; lastx = -1; lasty = 0; }
        if (key == 'd' && player.x < width - 2) { newx++; lastx = 1; lasty = 0; }

        if (newx != player.x || newy != player.y) {
            player.x = newx;
            player.y = newy;
        }

        if (key == 'i') pbullets.push(bullet(player.x, player.y - 1, 0, -1));
        if (key == 'k') pbullets.push(bullet(player.x, player.y + 1, 0, 1));
        if (key == 'j') pbullets.push(bullet(player.x - 1, player.y, -1, 0));
        if (key == 'l') pbullets.push(bullet(player.x + 1, player.y, 1, 0));

        if (key == ' ' && dashwaittimer == 0 && player.stamina >= dashcost && (lastx != 0 || lasty != 0)) {
            dashing = true;
            dashtime = dashlength;
            dashwaittimer = dashwait;
            player.stamina -= dashcost;

            for (int i = 0; i < 3; i++) {
                int newx = player.x + lastx;
                int newy = player.y + lasty;

                if (newx > 0 && newx < width - 1) player.x = newx;
                if (newy > 0 && newy < height - 1) player.y = newy;
            }
        }

        if (key == 'q') exit(0);
    }

    if (frame % 5 == 0 && player.stamina < 100) {
        player.stamina += 3;
    }

    if (dashwaittimer > 0) dashwaittimer--;
    if (dashtime > 0) {
        dashtime--;
        if (dashtime == 0) dashing = false;
    }
}

void updateboss(bool level1 = false) {
    if (level1) return;

    if (laserwarn || laseron) return;

    if (frame % bossmovewait == 0) {
        int dx = abs(boss.x - player.x);
        int dy = abs(boss.y - player.y);

        bool chase = rand() % 4 != 0;

        if ((dx > chase || dy > chase) && chase) {
            if (boss.x < player.x && boss.x < width - 2) boss.x++;
            else if (boss.x > player.x && boss.x > 1) boss.x--;
            else if (boss.y < player.y && boss.y < height - 2) boss.y++;
            else if (boss.y > player.y && boss.y > 1) boss.y--;
        } else {
            int dir = rand() % 4;
            if (dir == 0 && boss.y > 1 && boss.y < player.y) boss.y--;
            else if (dir == 1 && boss.y < height - 2 && boss.y > player.y) boss.y++;
            else if (dir == 2 && boss.x > 1 && boss.x < player.x) boss.x--;
            else if (dir == 3 && boss.x < width - 2 && boss.x > player.x) boss.x++;
            else {
                if (rand() % 2 == 0) boss.y += (rand() % 2 == 0) ? -1 : 1;
                else boss.x += (rand() % 2 == 0) ? -1 : 1;
            }
        }

        if (boss.x < 1) boss.x = 1;
        if (boss.x >= width - 1) boss.x = width - 2;
        if (boss.y < 1) boss.y = 1;
        if (boss.y >= height - 1) boss.y = height - 2;
    }

    if (frame % bossdelay == 0 && !laserwarn && !laseron) {
        int dx = 0, dy = 0;
        if (abs(boss.x - player.x) > abs(boss.y - player.y)) {
            dx = (player.x > boss.x) ? 1 : -1;
        } else if (boss.y != player.y) {
            dy = (player.y > boss.y) ? 1 : -1;
        } else {
            dx = (player.x > boss.x) ? 1 : -1;
        }
        bbullets.push(bullet(boss.x + dx, boss.y + dy, dx, dy));
    }
}

void firstlevel() {
    player = thing(5, 5, 70, 100);
    boss = thing(20, 5, 30, 0);
    pbullets = queue<bullet>();
    
    while (true) {
        drawgame(true);
        updateplayer();
        updatebullets(pbullets, true, true);
        
        if (boss.hp <= 0) {
            system("cls");
            cout << "TARGET DESTROYED!\n";
            cout << "You're ready for the real fight!\n";
            cout << "Press any key to continue to Level 2...";
            Sleep(2000);
            _getch();
            break;
        }

        frame++;
        Sleep(20);
    }
}

void secondlevel() {
    player = thing(5, 5, 100, 100);
    boss = thing(20, 5, maxhp, 0);
    minions.clear();
    pbullets = queue<bullet>();
    bbullets = queue<bullet>();
    waves.clear();
    laseron = false;
    laserwarn = false;
    
    while (true) {
        drawgame();
        updateplayer();
        updateboss();
        updateminions();
        
        if (boss.hp <= 55) updatelaser();
        
        if (boss.hp <= 30) {
            if (wavewait <= 0) {
                makewave();
                wavewait = maxwavewait;
            } else {
                wavewait--;
            }
        }
        
        updatewaves();
        updatebullets(pbullets, true);
        updatebullets(bbullets, false);

        if (laseron) {
            for (auto& cell : lasercells) {
                if (player.x == cell.first && player.y == cell.second && !dashing) {
                    player.hp-=1;
                    break;
                }
            }
        }

        if (player.hp <= 0) {
            system("cls");
            cout << "YOU DIED!\n";
            break;
        }

        if (boss.hp <= 0) {
            system("cls");
            cout << "YOU DEFEATED THE BOSS!\n";
            break;
        }

        frame++;
        Sleep(20);
    }
}

int main() {
    hidecursor();
    srand(time(0));

    cout << "GRID CLASH\n\n";
    cout << "CONTROLS:\n";
    cout << "  MOVEMENT: \n";
    cout << "    MOVE UP:    W \n";
    cout << "    MOVE LEFT:  A \n";
    cout << "    MOVE DOWN:  S \n";
    cout << "    MOVE RIGHT: D \n";
    cout << "  SHOOTING: \n";
    cout << "    SHOOT UP:    I \n";
    cout << "    SHOOT LEFT:  J \n";
    cout << "    SHOOT DOWN:  K \n";
    cout << "    SHOOT RIGHT: L \n";
    cout << "  DASH: SPACEBAR\n";
    cout << "  QUIT: Q\n\n";
    cout << "LEVEL 1: Static target practice\n";
    cout << "LEVEL 2: Full boss fight with minions and special attacks\n\n";
    cout << "Press any key to start...";
    _getch();

    firstlevel();
    secondlevel();

    cout << "\nThanks for playing! Press any key to exit...";
    _getch();
    return 0;
}