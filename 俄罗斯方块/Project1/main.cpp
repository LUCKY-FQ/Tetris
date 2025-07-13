#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <tchar.h>

const int ROWS = 20;
const int COLS = 10;
const int BLOCK_SIZE = 30;
const int WIDTH = COLS * BLOCK_SIZE;
const int HEIGHT = ROWS * BLOCK_SIZE;
const int GRID_LINE = 1;

int score = 0;
bool paused = false;

// 初始形状
const int baseShapes[7][4][4] = {
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, // I
    {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}}, // O
    {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}}, // S
    {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}}, // Z
    {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}}, // L
    {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}}, // J
    {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}}  // T
};
int shapes[7][4][4][4];

struct Block {
    int type, rotation, x, y;
    COLORREF color;
} current, next;

int grid[ROWS][COLS] = { 0 };

void rotateShape(const int src[4][4], int dst[4][4]) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            dst[j][3 - i] = src[i][j];
}

void initShapes() {
    for (int t = 0; t < 7; t++) {
        memcpy(shapes[t][0], baseShapes[t], sizeof(baseShapes[t]));
        for (int r = 1; r < 4; r++)
            rotateShape(shapes[t][r - 1], shapes[t][r]);
    }
}

bool canMove(int x, int y, int r) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (shapes[current.type][r][i][j]) {
                int nx = x + j;
                int ny = y + i;
                if (nx < 0 || nx >= COLS || ny >= ROWS)
                    return false;
                if (ny >= 0 && grid[ny][nx])
                    return false;
            }
    return true;
}

void fixBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (shapes[current.type][current.rotation][i][j]) {
                int nx = current.x + j;
                int ny = current.y + i;
                if (ny >= 0)
                    grid[ny][nx] = current.color;
            }
}

void clearLines() {
    for (int i = ROWS - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < COLS; j++)
            if (!grid[i][j])
                full = false;
        if (full) {
            score += 100;
            for (int k = i; k > 0; k--)
                for (int j = 0; j < COLS; j++)
                    grid[k][j] = grid[k - 1][j];
            for (int j = 0; j < COLS; j++)
                grid[0][j] = 0;
            i++;
        }
    }
}

Block randomBlock() {
    Block b;
    b.type = rand() % 7;
    b.rotation = 0;
    b.x = COLS / 2 - 2;
    b.y = -1;
    b.color = RGB(rand() % 256, rand() % 256, rand() % 256);
    return b;
}

void newBlock() {
    current = next;
    next = randomBlock();
}

void drawNextBlock() {
    int startX = WIDTH + 20;
    int startY = 120;
    outtextxy(startX, startY - 30, _T("下一个:"));
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (shapes[next.type][next.rotation][i][j]) {
                setfillcolor(next.color);
                solidrectangle(startX + j * 20, startY + i * 20,
                    startX + (j + 1) * 20 - 2, startY + (i + 1) * 20 - 2);
            }
}

void draw() {
    setbkcolor(WHITE);
    cleardevice();

    // 固定方块
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            if (grid[i][j]) {
                setfillcolor(grid[i][j]);
                solidrectangle(j * BLOCK_SIZE + GRID_LINE, i * BLOCK_SIZE + GRID_LINE,
                    (j + 1) * BLOCK_SIZE - GRID_LINE, (i + 1) * BLOCK_SIZE - GRID_LINE);
            }

    // 当前方块
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (shapes[current.type][current.rotation][i][j]) {
                int nx = current.x + j;
                int ny = current.y + i;
                if (ny >= 0) {
                    setfillcolor(current.color);
                    solidrectangle(nx * BLOCK_SIZE + GRID_LINE, ny * BLOCK_SIZE + GRID_LINE,
                        (nx + 1) * BLOCK_SIZE - GRID_LINE, (ny + 1) * BLOCK_SIZE - GRID_LINE);
                }
            }

    // 网格线
    setlinecolor(RGB(200, 200, 200));
    for (int i = 0; i <= ROWS; i++)
        line(0, i * BLOCK_SIZE, WIDTH, i * BLOCK_SIZE);
    for (int j = 0; j <= COLS; j++)
        line(j * BLOCK_SIZE, 0, j * BLOCK_SIZE, HEIGHT);

    // 分数
    settextcolor(BLACK);
    settextstyle(20, 0, _T("黑体"));
    TCHAR s[50];
    _stprintf_s(s, _T("得分: %d"), score);
    outtextxy(WIDTH + 20, 50, s);

    drawNextBlock();

    if (paused) {
        settextcolor(RED);
        settextstyle(30, 0, _T("黑体"));
        outtextxy(WIDTH / 2 - 50, HEIGHT / 2, _T("暂停"));
    }

    FlushBatchDraw();
}

void showMenu() {
    bool inMenu = true;
    while (inMenu) {
        // 先显示菜单
        setbkcolor(WHITE);
        cleardevice();
        settextcolor(BLUE);
        settextstyle(40, 0, _T("黑体"));
        outtextxy(WIDTH / 2 - 100, 100, _T("俄罗斯方块"));
        settextstyle(30, 0, _T("Consolas"));
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 60, _T("1.开始游戏"));
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 - 10, _T("2.游戏规则"));
        outtextxy(WIDTH / 2 - 100, HEIGHT / 2 + 40, _T("ESC.返回"));
        FlushBatchDraw();

        bool waitForInput = true;
        while (waitForInput) {
            ExMessage msg;
            if (peekmessage(&msg, EX_KEY)) {
                if (msg.message == WM_KEYDOWN) {
                    if (msg.vkcode == '1') {
                        // 开始游戏
                        inMenu = false;
                        waitForInput = false;
                    }
                    else if (msg.vkcode == '2') {
                        // 显示规则
                        setbkcolor(WHITE);
                        cleardevice();
                        settextcolor(BLACK);
                        settextstyle(30, 0, _T("Consolas"));
                        outtextxy(WIDTH / 2 - 100, 100, _T("游戏规则"));
                        settextstyle(20, 0, _T("Consolas"));
                        outtextxy(50, 160, _T("← → 移动  ↑ 旋转"));
                        outtextxy(50, 200, _T("↓ 加速下落"));
                        outtextxy(50, 240, _T("Space 直接下落"));
                        outtextxy(50, 280, _T("P 暂停/继续"));
                        outtextxy(50, 320, _T("按 ESC 返回菜单"));
                        FlushBatchDraw();

                        // 等待按 ESC 返回菜单
                        bool waitInRules = true;
                        while (waitInRules) {
                            ExMessage ruleMsg;
                            if (peekmessage(&ruleMsg, EX_KEY)) {
                                if (ruleMsg.message == WM_KEYDOWN && ruleMsg.vkcode == VK_ESCAPE) {
                                    waitInRules = false; // 返回菜单
                                }
                            }
                            Sleep(10);
                        }

                        // 返回菜单：break 内层循环重新显示菜单
                        waitForInput = false;
                    }
                    else if (msg.vkcode == VK_ESCAPE) {
                        closegraph();
                        exit(0);
                    }
                }
            }
            Sleep(10);
        }
    }
}


int main() {
    initgraph(WIDTH + 150, HEIGHT);
    BeginBatchDraw();
    srand((unsigned)time(NULL));
    initShapes();

    showMenu();
    next = randomBlock();
    newBlock();

    DWORD lastTime = GetTickCount();
    int speed = 500;

    while (true) {
        ExMessage msg;
        while (peekmessage(&msg, EX_KEY)) {
            if (msg.message == WM_KEYDOWN) {
                switch (msg.vkcode) {
                case VK_UP:
                case 'W': case 'w':
                    if (!paused && canMove(current.x, current.y + 1, current.rotation)) {
                        int r = (current.rotation + 1) % 4;
                        if (canMove(current.x, current.y, r))
                            current.rotation = r;
                    }
                    break;
                case VK_DOWN:
                case 'S': case 's':
                    if (!paused && canMove(current.x, current.y + 1, current.rotation))
                        current.y++;
                    break;
                case VK_LEFT:
                case 'A': case 'a':
                    if (!paused && canMove(current.x, current.y + 1, current.rotation) &&
                        canMove(current.x - 1, current.y, current.rotation))
                        current.x--;
                    break;
                case VK_RIGHT:
                case 'D': case 'd':
                    if (!paused && canMove(current.x, current.y + 1, current.rotation) &&
                        canMove(current.x + 1, current.y, current.rotation))
                        current.x++;
                    break;
                case VK_SPACE:
                    if (!paused) {
                        while (canMove(current.x, current.y + 1, current.rotation))
                            current.y++;
                    }
                    break;
                case 'P': case 'p':
                    paused = !paused;
                    break;
                }
            }
        }



        DWORD now = GetTickCount();
        if (!paused && now - lastTime > speed) {
            if (canMove(current.x, current.y + 1, current.rotation))
                current.y++;
            else {
                fixBlock();
                clearLines();
                newBlock();
                if (!canMove(current.x, current.y, current.rotation)) {
                    settextcolor(RED);
                    settextstyle(30, 0, _T("Consolas"));
                    outtextxy(WIDTH / 2 - 80, HEIGHT / 2, _T("Game Over!"));
                    FlushBatchDraw();
                    _getch();
                    EndBatchDraw();
                    closegraph();
                    return 0;
                }
            }
            lastTime = now;
        }

        draw();
        Sleep(10);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}
