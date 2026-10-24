#include "iGraphics.h"
#include "iSound.h"
#include <cmath>
#include <fstream>
#include <cstring>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800

bool showEnterButton = false;
bool enterButtonVisible = false;
bool levelInitialized = false;
bool showNameWarning = false;
int enterButtonX = SCREEN_WIDTH - 400;
int enterButtonY = (SCREEN_HEIGHT - 80) / 2;

bool ballBlast = false;
float ballBlastX = 0, ballBlastY = 0;

Mix_Chunk *pointTakingSound = nullptr;
Mix_Chunk *boomSound = nullptr;
Mix_Chunk *pointSound = nullptr;
Mix_Chunk *typeSound = nullptr;
Mix_Chunk *finishSound = nullptr;
Mix_Chunk *checkpointSound = nullptr;

int lastGameState = -2;

Image menuBG, buttonImg, bluePrintImg, background, background1, ballBlastImg, aboutBG, arrowImg, gameoverImg, congratulationImg, prevImg, afterImg, checkpointLitImg, pointTextImg, brickImg, flagImg, loadingBallSmallImg, loadingBallBigImg, leaderboardImg, pauseImg, pointImg, spikeImg, spikeImg2, spikeImg3, livesButtonImg, scoreButtonImg, soundImg, offSoundImg, volumeIncImg, volumeDecImg;
Image ballSprites[18];

int flagX = 2850, flagY = 43;
int flagWidth = 64, flagHeight = 64;

int gameState = -1, previousGameState = -1;
float ballX = 60, ballY = 109;
int radius = 25, groundY = 50, jump = 0, Vy = 0, gravity = -1;
float window = 0;
int level_width = 3000, currentLevel = 1;

#define leftMargin 250
#define rightMargin 950

int buttonWidth = 300, buttonHeight = 80;
int leaderboardViewLevel = 1;

bool soundEnabled = true;
int soundButtonX = 50, soundButtonY = 720;
int soundButtonWidth = 60, soundButtonHeight = 60;
int musicChannel = -1;

int musicVolume = 64;
int volumeButtonImgSize = 22;
int volumeIncX = soundButtonX + soundButtonWidth + 5;
int volumeIncY = soundButtonY + soundButtonHeight - volumeButtonImgSize + 2;
int volumeDecX = soundButtonX + soundButtonWidth + 5;
int volumeDecY = soundButtonY - 2;

int baseScore = 0;

int arrowX = 20, arrowY = 720, arrowWidth = 60, arrowHeight = 60, arrowHoverOffset = 0;
int hoveredButton = -1;
int ballFrame = 0, ballDirection = 1;
float ballVelocityX = 0;
float playerScore = 0, maxScore = 0;

bool showPointText = false;
int pointTextTimer = 0;
int pointTextValue = 0;

const int menuButtonGap = 20;
const int menuButtonWidth = 300;
const int menuButtonHeight = 80;
const int menuButtonX = 850;
const int newGameY = 500;
const int leaderboardY = newGameY - (menuButtonHeight + menuButtonGap);
const int aboutY = leaderboardY - (menuButtonHeight + menuButtonGap);
const int blueprintY = aboutY - (menuButtonHeight + menuButtonGap);
const int quitY = blueprintY - (menuButtonHeight + menuButtonGap);

const int newGameX = menuButtonX;
const int leaderboardX = menuButtonX;
const int aboutX = menuButtonX;
const int quitX = menuButtonX;
const int blueprintX = menuButtonX;

float furthestX = 67;
float maxFurthestX = 67;
int lives = 3;
bool checkpointSet = false;
float checkpointX = 67, checkpointY = groundY;

int ballBlastTimer = 0;

char playerName[30] = "";
int nameIndex = 0, inputActive = 0;

int blueprintScrollY = 0;
const int blueprintScrollStep = 40;
const int blueprintBoxX = 150;
const int blueprintBoxY = 100;
const int blueprintBoxW = 1020;
const int blueprintBoxH = 600;
const int blueprintContentHeight = 900;

const char *blueprintLines[] = {
    "HOW TO PLAY",
    "",
    "MOVE THE BALL WITH ARROW KEYS!",
    "UP,DOWN,LEFT,RIGHT",
    "",
    "Reach the flag to finish the level. Collect points and avoid spikes!",
    "",
    "Spikes: Touching them will cost you a life!",
    "Spikes are deadly obstacles. Avoid them at all costs. If you touch a spike, you lose a life and respawn at the last checkpoint.",
    "",
    "Points: Collect for extra score!",
    "Points are scattered throughout the level. Collect them to increase your score. Try to collect as many as possible!",
    "",
    "Flag: Reach here to complete the level.",
    "The flag marks the end of the level. Reach it to finish and submit your score. Plan your path to get there safely!",
    "",
    "Checkpoint: Touch to save your progress.",
    "This icon indicates a checkpoint. Touching it will save your progress.",
    "",
    "Checkpoint Achieved:",
    "When the checkpoint lights up, it means your progress is saved at this spot.",
    "",
    "Restart Point:",
    "After losing a life, you will restart from this marked point.",
};
const int blueprintLineCount = sizeof(blueprintLines) / sizeof(blueprintLines[0]);
int blueprintScrollIndex = 0;

int volumePopupTimer = 0;
const int volumePopupDuration = 180;
bool volumeOnSoundHover = false;

void drawButtonText(int x, int y, const char *text, int isHovered);
void drawArrow();

void startBackgroundMusic()
{
    if (soundEnabled && musicChannel == -1)
    {
        musicChannel = iPlaySound("music.wav", true, musicVolume * 100 / 128);
    }
}

void stopBackgroundMusic()
{
    if (musicChannel != -1)
    {
        iStopSound(musicChannel);
        musicChannel = -1;
    }
}

void toggleBackgroundMusic()
{
    if (soundEnabled)
    {
        startBackgroundMusic();
    }
    else
    {
        stopBackgroundMusic();
    }
}

void increaseVolume()
{
    if (musicVolume < 128)
    {
        musicVolume += 16;
        if (musicVolume > 128)
            musicVolume = 128;
        if (musicChannel != -1)
            iSetVolume(musicChannel, musicVolume * 100 / 128);
    }
}

void decreaseVolume()
{
    if (musicVolume > 0)
    {
        musicVolume -= 16;
        if (musicVolume < 0)
            musicVolume = 0;
        if (musicChannel != -1)
            iSetVolume(musicChannel, musicVolume * 100 / 128);
    }
}

#define MAX_SPIKES2 50
#define MAX_POINTS 50
#define MAX_AFTER 50
#define MAX_PREV 50

struct PointTextEffect
{
    bool active;
    float x, y;
    float startY;
    float endY;
    int timer;
    int type;
};
#define MAX_POINT_TEXTS 50
PointTextEffect pointTextEffects[MAX_POINT_TEXTS];

typedef struct
{
    int x, y, width, height;
    Image image;
} Spike2;

typedef struct
{
    int x, y, width, height;
    Image image;
    bool collected;
} Point;

typedef struct
{
    int x, y, width, height;
    Image image;
    bool collected;
} Prev;

typedef struct
{
    int x, y, width, height;
    Image image;
    bool show;
} After;

Spike2 spikes2[MAX_SPIKES2];

Point points[MAX_POINTS];
After afters[MAX_AFTER];
Prev prevs[MAX_PREV];

int spikeCount2 = 0;

int pointCount = 0;
int prevCount = 0;
int afterCount = 0;

typedef struct
{
    int x, y, width, height;
    Image *image;
    bool topCollision;
    bool bottomCollision;
    bool leftCollision;
    bool rightCollision;
} Brick;
#define MAX_BRICKS 1000
Brick bricks[MAX_BRICKS];
int brickCount = 0;

typedef struct
{
    char name[30];
    int score, level;
} LeaderboardEntry;
LeaderboardEntry leaderboard1[10], leaderboard2[10], leaderboard3[10];
int leaderboardCount1 = 0, leaderboardCount2 = 0, leaderboardCount3 = 0;

int blinkCounter = 0, timeSinceStart = 0;
int loadingBalls = 8, loadingCircleOrbitRadius = 30, activeLoadingBall = 0, loadingBlinkCounter = 0;

int getTextWidth(const char *text, void *font)
{
    int width = 0;
    for (int i = 0; text[i]; i++)
        width += glutBitmapWidth(font, text[i]);
    return width;
}
void loadLeaderboardFromFile(int level)
{
    std::ifstream fin;
    if (level == 1)
        fin.open("leaderboard1.txt");
    else if (level == 2)
        fin.open("leaderboard2.txt");
    else
        fin.open("leaderboard3.txt");
    if (!fin)
        return;
    int &count = (level == 1) ? leaderboardCount1 : (level == 2) ? leaderboardCount2
                                                                 : leaderboardCount3;
    LeaderboardEntry *lb = (level == 1) ? leaderboard1 : (level == 2) ? leaderboard2
                                                                      : leaderboard3;
    count = 0;
    std::string line;
    while (std::getline(fin, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        int rank, score;
        char name[30];
        if (sscanf(line.c_str(), "%d. %29[^-]- %d", &rank, name, &score) == 3)
        {
            int len = strlen(name);
            while (len > 0 && (name[len - 1] == ' ' || name[len - 1] == '-'))
                name[--len] = '\0';
            strcpy(lb[count].name, name);
            lb[count].score = score;
            lb[count].level = level;
            count++;
            if (count >= 10)
                break;
        }
    }
    fin.close();
}

void saveLeaderboardToFile(int level)
{
    std::ofstream fout;
    int count;
    LeaderboardEntry *lb;
    if (level == 1)
    {
        fout.open("leaderboard1.txt");
        count = leaderboardCount1;
        lb = leaderboard1;
    }
    else if (level == 2)
    {
        fout.open("leaderboard2.txt");
        count = leaderboardCount2;
        lb = leaderboard2;
    }
    else
    {
        fout.open("leaderboard3.txt");
        count = leaderboardCount3;
        lb = leaderboard3;
    }
    fout << "# Leaderboard - Top 10 Scores\n";
    fout << "# Format: Rank. PlayerName - Score\n\n";
    for (int i = 0; i < 10; i++)
    {
        if (i < count)
            fout << (i + 1) << ". " << lb[i].name << " - " << lb[i].score << "\n";
        else
            fout << (i + 1) << ". --- - 0\n";
    }
    fout.close();
}

void iTextWrapped(int x, int y, const char *text, void *font, int maxWidth, int lineHeight)
{
    char buf[1024];
    strcpy(buf, text);
    char *token = strtok(buf, " ");
    char line[1024] = "";
    int cx = x, cy = y;
    while (token)
    {
        char temp[1024];
        strcpy(temp, line);
        if (strlen(temp) > 0)
            strcat(temp, " ");
        strcat(temp, token);
        if (getTextWidth(temp, font) > maxWidth)
        {
            iText(cx, cy, line, font);
            cy -= lineHeight;
            strcpy(line, token);
        }
        else
        {
            if (strlen(line) > 0)
                strcat(line, " ");
            strcat(line, token);
        }
        token = strtok(NULL, " ");
    }
    if (strlen(line) > 0)
        iText(cx, cy, line, font);
}

void drawLeaderboardSelect()
{
    iShowLoadedImage(0, 0, &background1);
    int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
    iSetColor(255, 255, 255);
    int titleWidth = getTextWidth("SELECT LEADERBOARD LEVEL", GLUT_BITMAP_HELVETICA_18);
    iText((SCREEN_WIDTH - titleWidth) / 2, 650, "SELECT LEADERBOARD LEVEL", GLUT_BITMAP_HELVETICA_18);

    iShowLoadedImage(centerX, 500, &buttonImg);
    drawButtonText(centerX, 500, "LEVEL 1", hoveredButton == 101);

    iShowLoadedImage(centerX, 400, &buttonImg);
    drawButtonText(centerX, 400, "LEVEL 2", hoveredButton == 102);

    iShowLoadedImage(centerX, 300, &buttonImg);
    drawButtonText(centerX, 300, "LEVEL 3", hoveredButton == 103);

    drawArrow();
}

bool pointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy)
{
    float dX = px - cx;
    float dY = py - cy;
    float dX21 = bx - ax;
    float dY12 = ay - by;
    float D = dY12 * (cx - ax) + dX21 * (cy - ay);
    float s = dY12 * dX + dX21 * dY;
    float t = (ay - cy) * dX + (ax - cx) * dY;
    if (D < 0)
        return (s <= 0) && (t <= 0) && (s + t >= D);
    return (s >= 0) && (t >= 0) && (s + t <= D);
}

bool ballCollidesWithSpikePolygon(float ballX, float ballY, int radius, int spikeX, int spikeY, int spikeW, int spikeH)
{
    int numSpikes = 5;
    float margin = spikeW * 0.025f;
    float spikeWidth = (spikeW - 2 * margin) / (float)numSpikes;
    float baseY = spikeY;
    float spikeHeight = spikeH * 0.55f;

    for (int i = 0; i < numSpikes; i++)
    {
        float left = spikeX + margin + i * spikeWidth;
        float right = left + spikeWidth;
        float tipX = left + spikeWidth / 2.0f;
        float tipY = baseY + spikeHeight;

        for (int a = 0; a < 8; a++)
        {
            float angle = a * 3.1415926f / 4.0f;
            float px = ballX + radius + cos(angle) * radius;
            float py = ballY + radius + sin(angle) * radius;
            if (pointInTriangle(px, py, left, baseY, tipX, tipY, right, baseY))
                return true;
        }
        float dx = (tipX - (ballX + radius));
        float dy = (tipY - (ballY + radius));
        if (dx * dx + dy * dy <= radius * radius)
            return true;
    }
    return false;
}

void drawSpikePolygons(int x, int y, int width, int height)
{
    int numSpikes = 5;
    float margin = width * 0.025f;
    float spikeW = (width - 2 * margin) / (float)numSpikes;
    float baseY = y;
    float spikeH = height * 0.55f;

    iSetColor(255, 0, 0);
    for (int i = 0; i < numSpikes; i++)
    {
        float left = x + margin + i * spikeW;
        float right = left + spikeW;
        float tipX = left + spikeW / 2.0f;
        float tipY = baseY + spikeH;

        double px[3] = {left, tipX, right};
        double py[3] = {baseY, tipY, baseY};
        iPolygon(px, py, 3);
    }
}
void drawBlueprintScreen()
{
    iShowLoadedImage(0, 0, &bluePrintImg);

    const int blueprintLeftMargin = blueprintBoxX + 30;
    const int iconMargin = 60;
    const int maxTextWidth = blueprintBoxW - 120 - iconMargin;
    const int lineHeight = 60;
    const int linesPerScreen = 10;

    int totalLines = 0;
    for (int i = 0; i < linesPerScreen; i++)
    {
        int idx = blueprintScrollIndex + i;
        if (idx >= blueprintLineCount)
            break;
        totalLines++;
    }
    int totalHeight = totalLines * lineHeight;
    int y = blueprintBoxY + blueprintBoxH / 2 + totalHeight / 2 - lineHeight + 30;
    if (y < 100)
        y = 100;
    for (int i = 0; i < linesPerScreen; i++)
    {
        int idx = blueprintScrollIndex + i;
        if (idx >= blueprintLineCount)
            break;

        int textX = blueprintLeftMargin + iconMargin;
        int iconX = blueprintLeftMargin;
        int iconY = y - 10;

        if (strcmp(blueprintLines[idx], "Checkpoint: Touch to save your progress.") == 0)
        {
            iShowLoadedImage(iconX, iconY, &prevImg);
        }
        else if (strcmp(blueprintLines[idx], "Checkpoint Achieved:") == 0)
        {
            iShowLoadedImage(iconX, iconY, &checkpointLitImg);
        }
        else if (strcmp(blueprintLines[idx], "Restart Point:") == 0)
        {
            iShowLoadedImage(iconX, iconY, &afterImg);
        }
        else if (strcmp(blueprintLines[idx], "Spikes: Touching them will cost you a life!") == 0)
        {
            iShowLoadedImage(iconX, iconY, &spikeImg2);
        }
        else if (strcmp(blueprintLines[idx], "Points: Collect for extra score!") == 0)
        {
            iShowLoadedImage(iconX, iconY, &pointImg);
        }
        else if (strcmp(blueprintLines[idx], "Flag: Reach here to complete the level.") == 0)
        {
            iShowLoadedImage(iconX, iconY, &flagImg);
        }

        if (strcmp(blueprintLines[idx], "HOW TO PLAY") == 0)
        {
            iSetColor(0, 0, 0);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (strstr(blueprintLines[idx], "Spikes:") == blueprintLines[idx])
        {
            iSetColor(255, 80, 80);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (strstr(blueprintLines[idx], "Points:") == blueprintLines[idx])
        {
            iSetColor(255, 180, 0);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (strstr(blueprintLines[idx], "Flag:") == blueprintLines[idx])
        {
            iSetColor(0, 180, 0);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (strstr(blueprintLines[idx], "Checkpoint:") == blueprintLines[idx])
        {
            iSetColor(0, 150, 255);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (strstr(blueprintLines[idx], "Checkpoint Achieved:") == blueprintLines[idx])
        {
            iSetColor(0, 200, 255);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (strstr(blueprintLines[idx], "Restart Point:") == blueprintLines[idx])
        {
            iSetColor(0, 180, 180);
            iText(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24);
        }
        else if (blueprintLines[idx][0] == '\0')
        {
        }
        else
        {
            iSetColor(60, 60, 60);
            iTextWrapped(textX, y, blueprintLines[idx], GLUT_BITMAP_TIMES_ROMAN_24, maxTextWidth, lineHeight);
        }
        y -= lineHeight;
    }

    drawArrow();
}

void addPoint(int x, int y)
{
    if (pointCount < MAX_POINTS)
    {
        points[pointCount].x = x;
        points[pointCount].y = y;
        points[pointCount].width = 50;
        points[pointCount].height = 50;
        points[pointCount].image = pointImg;
        points[pointCount].collected = false;
        pointCount++;
    }
}

void addPrev(int x, int y)
{
    if (prevCount < MAX_PREV)
    {
        prevs[prevCount].x = x;
        prevs[prevCount].y = y;
        prevs[prevCount].width = 50;
        prevs[prevCount].height = 50;
        prevs[prevCount].image = prevImg;
        prevs[prevCount].collected = false;
        prevCount++;
    }
}

void addAfter(int x, int y)
{
    if (afterCount < MAX_AFTER)
    {
        afters[afterCount].x = x;
        afters[afterCount].y = y;
        afters[afterCount].width = 50;
        afters[afterCount].height = 50;
        afters[afterCount].image = afterImg;
        afters[afterCount].show = false;
        afterCount++;
    }
}

void addSpike2(int x, int y)
{
    if (spikeCount2 < MAX_SPIKES2)
    {
        spikes2[spikeCount2].x = x;
        spikes2[spikeCount2].y = y;
        spikes2[spikeCount2].width = 50;
        spikes2[spikeCount2].height = 50;
        spikes2[spikeCount2].image = spikeImg2;
        spikeCount2++;
    }
}

void addLeftToRightWall(int x, int yStart, int heightBricks)
{
    int brickSize = 50;
    for (int i = 0; i < heightBricks; i++)
    {
        if (brickCount < MAX_BRICKS)
        {
            bricks[brickCount].x = x + i * brickSize;
            bricks[brickCount].y = yStart;
            bricks[brickCount].width = brickSize;
            bricks[brickCount].height = brickSize;
            bricks[brickCount].image = &brickImg;
            bricks[brickCount].topCollision = false;
            bricks[brickCount].bottomCollision = false;
            bricks[brickCount].leftCollision = false;
            bricks[brickCount].rightCollision = false;
            brickCount++;
        }
    }
}

void addVerticalWall(int x, int yStart, int heightBricks)
{
    int brickSize = 50;
    for (int i = 0; i < heightBricks; i++)
    {
        if (brickCount < MAX_BRICKS)
        {
            bricks[brickCount].x = x;
            bricks[brickCount].y = yStart + i * brickSize;
            bricks[brickCount].width = brickSize;
            bricks[brickCount].height = brickSize;
            bricks[brickCount].image = &brickImg;
            bricks[brickCount].topCollision = false;
            bricks[brickCount].bottomCollision = false;
            bricks[brickCount].leftCollision = false;
            bricks[brickCount].rightCollision = false;
            brickCount++;
        }
    }
}
void addUpToDownWall(int x, int yStart, int heightBricks)
{
    int brickSize = 50;
    for (int i = 0; i < heightBricks; i++)
    {
        if (brickCount < MAX_BRICKS)
        {
            bricks[brickCount].x = x;
            bricks[brickCount].y = yStart - i * brickSize;
            bricks[brickCount].width = brickSize;
            bricks[brickCount].height = brickSize;
            bricks[brickCount].image = &brickImg;
            bricks[brickCount].topCollision = false;
            bricks[brickCount].bottomCollision = false;
            bricks[brickCount].leftCollision = false;
            bricks[brickCount].rightCollision = false;
            brickCount++;
        }
    }
}
void singleBrick(int x, int y)
{
    int brickSize = 50;
    if (brickCount < MAX_BRICKS)
    {
        bricks[brickCount].x = x;
        bricks[brickCount].y = y;
        bricks[brickCount].width = brickSize;
        bricks[brickCount].height = brickSize;
        bricks[brickCount].image = &brickImg;
        bricks[brickCount].topCollision = false;
        bricks[brickCount].bottomCollision = false;
        bricks[brickCount].leftCollision = false;
        bricks[brickCount].rightCollision = false;
        brickCount++;
    }
}

void setupLevel(int level)
{
    brickCount = 0;
    spikeCount2 = 0;
    pointCount = 0;
    prevCount = 0;
    afterCount = 0;
    int brickSize = 50;
    if (level == 1)
    {

        addUpToDownWall(300, SCREEN_HEIGHT - brickSize, 13);
        addUpToDownWall(350, SCREEN_HEIGHT - brickSize, 13);
        addUpToDownWall(400, SCREEN_HEIGHT - brickSize, 13);
        addUpToDownWall(450, SCREEN_HEIGHT - brickSize, 13);
        addUpToDownWall(500, SCREEN_HEIGHT - brickSize, 13);
        addUpToDownWall(550, SCREEN_HEIGHT - brickSize, 13);
        singleBrick(600, SCREEN_HEIGHT - brickSize - 11 * brickSize);
        addVerticalWall(750, brickSize, 5);
        addVerticalWall(800, brickSize, 5);
        addLeftToRightWall(850, 250, 5);
        addSpike2(850, 300);
        addSpike2(900, 300);
        addPoint(850, 200);
        addPrev(850, 50);
        addAfter(850, 50);

        addLeftToRightWall(1050, 450, 4);
        singleBrick(1200, 200);
        addVerticalWall(1250, brickSize, 10);
        addSpike2(1300, 50);
        addSpike2(1350, 50);

        singleBrick(1400, 150);
        addVerticalWall(1450, brickSize, 4);
        addLeftToRightWall(1500, 200, 2);
        addPoint(1500, 150);
        addSpike2(1850, 50);
        addSpike2(1900, 50);
        addLeftToRightWall(1700, 200, 3);
        singleBrick(1750, 150);
        addLeftToRightWall(1950, 200, 2);
        addVerticalWall(2050, brickSize, 4);
        addPrev(2200, 50);
        addAfter(2200, 50);
        addUpToDownWall(2250, SCREEN_HEIGHT - brickSize, 11);
        addLeftToRightWall(2250, 200, 4);
        singleBrick(2550, 100);
        addPoint(2550, 150);
        addVerticalWall(2600, brickSize, 7);
        addVerticalWall(2650, brickSize, 6);
        addSpike2(2650, 350);
        addVerticalWall(2700, brickSize, 9);
    }
    else if (level == 2)
    {
        addVerticalWall(350, brickSize, 4);
        singleBrick(300, 150);
        addLeftToRightWall(400, 200, 6);
        addLeftToRightWall(650, 250, 2);
        addLeftToRightWall(700, 300, 2);
        addLeftToRightWall(750, 350, 2);
        addLeftToRightWall(800, 400, 2);
        addLeftToRightWall(850, 450, 2);
        addPoint(400, 150);

        addUpToDownWall(400, SCREEN_HEIGHT - brickSize, 7);
        addLeftToRightWall(450, 500, 5);
        addPoint(450, 450);
        addPoint(450, SCREEN_HEIGHT - brickSize - 50);
        addPrev(450, 550);
        addAfter(450, 550);
        addSpike2(600, 550);
        addLeftToRightWall(1100, 300, 2);
        addSpike2(900, 50);
        addSpike2(950, 50);
        addLeftToRightWall(1000, 350, 4);
        addLeftToRightWall(1100, 400, 2);
        addLeftToRightWall(1100, 450, 4);
        addLeftToRightWall(1100, 500, 4);
        addPoint(1250, 550);
        addPrev(1248, 50);
        addAfter(1248, 50);
        addVerticalWall(1300, brickSize, 3);
        addSpike2(1350, brickSize);
        addSpike2(1400, brickSize);
        addVerticalWall(1500, brickSize, 1);
        addVerticalWall(1550, brickSize, 2);
        addVerticalWall(1600, brickSize, 3);
        addVerticalWall(1650, brickSize, 4);
        addVerticalWall(1700, brickSize, 5);
        addSpike2(1750, brickSize);
        addSpike2(1800, brickSize);
        addSpike2(1850, brickSize);
        addVerticalWall(1900, brickSize, 5);
        addVerticalWall(1950, brickSize, 4);
        addVerticalWall(2000, brickSize, 3);
        addVerticalWall(2050, brickSize, 2);
        addVerticalWall(2100, brickSize, 1);
        addVerticalWall(2250, brickSize, 7);
        addVerticalWall(2300, brickSize, 4);
        addLeftToRightWall(2150, 250, 2);
        addPoint(2200, 200);
        addUpToDownWall(2250, SCREEN_HEIGHT - brickSize, 5);
        singleBrick(2350, 200);
        addPoint(2350, 150);
        addPrev(2350, 50);
        addAfter(2350, 50);
        addSpike2(2450, 50);
        addVerticalWall(2500, brickSize, 2);
        addVerticalWall(2550, brickSize, 2);
        addVerticalWall(2600, brickSize, 7);
        addSpike2(2650, brickSize);
        addSpike2(2700, brickSize);

        addVerticalWall(2750, brickSize, 7);
        addLeftToRightWall(2500, 300, 2);
    }
    else if (level == 3)
    {
        addVerticalWall(300, brickSize, 3);
        addSpike2(350, 50);
        addUpToDownWall(450, SCREEN_HEIGHT - brickSize, 11);
        addLeftToRightWall(500, 350, 2);
        addLeftToRightWall(450, 200, 4);
        addLeftToRightWall(600, 150, 4);
        addSpike2(650, 200);
        addPoint(500, 300);
        addVerticalWall(900, brickSize, 11);
        addLeftToRightWall(750, 400, 3);
        addPoint(850, 350);
        addPrev(750, 450);
        addAfter(750, 450);

        addSpike2(800, 450);
        addSpike2(850, 450);
        addSpike2(950, 50);
        addUpToDownWall(1050, SCREEN_HEIGHT - brickSize, 13);
        addPoint(1100, 400);
        addSpike2(1150, 500);
        addSpike2(1200, 500);
        addPoint(1100, 700);
        addLeftToRightWall(1100, 450, 4);
        singleBrick(1100, 150);
        singleBrick(1250, 150);
        singleBrick(1250, 200);
        addVerticalWall(1300, brickSize, 4);
        addVerticalWall(1350, brickSize, 4);
        addSpike2(1400, 50);

        addVerticalWall(1550, brickSize, 11);
        addPoint(1500, 100);
        singleBrick(1500, 150);
        addLeftToRightWall(1400, 350, 3);

        addLeftToRightWall(1450, 550, 2);
        addPoint(1500, 500);
        addSpike2(1450, 400);
        addSpike2(1600, 50);
        addSpike2(1650, 50);
        addVerticalWall(1700, brickSize, 5);
        addLeftToRightWall(1750, 250, 4);
        addLeftToRightWall(1800, 300, 3);
        addLeftToRightWall(1850, 350, 2);
        singleBrick(1900, 400);
        addPoint(1750, 200);
        addSpike2(1850, 50);
        addUpToDownWall(2050, SCREEN_HEIGHT - brickSize, 13);
        addPoint(2000, 500);

        addLeftToRightWall(2100, 250, 3);
        addLeftToRightWall(2100, 550, 4);
        addSpike2(2100, 300);
        addPoint(2100, 700);

        addLeftToRightWall(2300, 150, 3);
        addLeftToRightWall(2300, 400, 3);
        addVerticalWall(2450, brickSize, 8);
        addLeftToRightWall(2450, 550, 3);
        addLeftToRightWall(2450, 600, 3);
        addPoint(2650, 700);
        addSpike2(2500, brickSize);
        addVerticalWall(2550, brickSize, 6);
        addVerticalWall(2600, brickSize, 4);
        addVerticalWall(2650, brickSize, 4);
        addSpike2(2600, 250);
        addSpike2(2650, 250);
        addVerticalWall(2700, brickSize, 6);
    }
    for (int y = groundY + brickSize; y <= SCREEN_HEIGHT - brickSize; y += brickSize)
        addVerticalWall(level_width - brickSize, y - 10 - 40, 1);
}

void applyGravity()
{
    bool onBrick = false;
    float ballBottom = ballY;
    float ballLeft = ballX;
    float ballRight = ballX + radius * 2;

    for (int i = 0; i < brickCount; i++)
    {
        if (ballBottom == bricks[i].y + bricks[i].height &&
            ballLeft < bricks[i].x + bricks[i].width &&
            ballRight > bricks[i].x)
        {
            onBrick = true;
            break;
        }
    }

    if (jump)
    {
        ballY += Vy;
        Vy += gravity;
        if (ballY <= groundY)
        {
            ballY = groundY;
            jump = 0;
            Vy = 0;
        }
    }
    else if (ballY > groundY && !onBrick)
    {
        Vy = -5;
        ballY += Vy;

        if (ballY < groundY)
            ballY = groundY;
    }
}

void updateBallVelocity()
{
    float prevBallX = ballX;
    float prevBallY = ballY;

    ballX += ballVelocityX;
    ballVelocityX *= 0.85f;
    if (ballVelocityX > -0.1f && ballVelocityX < 0.1f)
        ballVelocityX = 0;
    if (ballX < 67)
        ballX = 67;
    if (ballX > level_width - radius)
        ballX = level_width - radius;
    if (ballX >= 2800)
        ballVelocityX = 0;

    applyGravity();

    int brickSize = 50;
    if (ballY + radius * 2 > SCREEN_HEIGHT - brickSize)
    {
        ballY = SCREEN_HEIGHT - brickSize - radius * 2;
        Vy = 0;
        jump = 1;
    }

    for (int i = 0; i < brickCount; i++)
    {
        bricks[i].topCollision = false;
        bricks[i].bottomCollision = false;
        bricks[i].leftCollision = false;
        bricks[i].rightCollision = false;
    }

    float ballRight = ballX + radius * 2;
    float ballLeft = ballX;
    float ballTop = ballY + radius * 2;
    float ballBottom = ballY;

    float prevBallRight = prevBallX + radius * 2;
    float prevBallLeft = prevBallX;
    float prevBallTop = prevBallY + radius * 2;
    float prevBallBottom = prevBallY;

    for (int i = 0; i < brickCount; i++)
    {
        Brick &b = bricks[i];

        if (ballRight > b.x &&
            prevBallRight <= b.x &&
            ballBottom < b.y + b.height &&
            ballTop > b.y)
        {
            ballX = b.x - radius * 2;
            ballVelocityX = 0;
            b.leftCollision = true;
        }

        if (ballLeft < b.x + b.width &&
            prevBallLeft >= b.x + b.width &&
            ballBottom < b.y + b.height &&
            ballTop > b.y)
        {
            ballX = b.x + b.width;
            ballVelocityX = 0;
            b.rightCollision = true;
        }

        if (ballBottom < b.y + b.height &&
            ballBottom > b.y &&
            prevBallBottom >= b.y + b.height &&
            ballLeft < b.x + b.width &&
            ballRight > b.x)
        {
            ballY = b.y + b.height;
            Vy = 0;
            jump = 0;
            b.topCollision = true;
        }

        if (prevBallBottom == b.y + b.height &&
            ((ballLeft >= b.x + b.width && prevBallLeft < b.x + b.width) ||
             (ballRight <= b.x && prevBallRight > b.x)))
        {
            jump = 1;
            Vy = 0;
        }

        if (ballTop > b.y &&
            ballTop < b.y + b.height &&
            prevBallTop <= b.y &&
            ballLeft < b.x + b.width &&
            ballRight > b.x)
        {
            ballY = b.y - radius * 2;
            Vy = 0;
            jump = 1;
            b.bottomCollision = true;
        }
    }

    for (int i = 0; i < prevCount; i++)
    {
        if (!prevs[i].collected)
        {
            if (
                ballX + radius * 2 > prevs[i].x &&
                ballX < prevs[i].x + prevs[i].width &&
                ballY + radius * 2 > prevs[i].y &&
                ballY < prevs[i].y + prevs[i].height)
            {
                prevs[i].collected = true;
                if (checkpointSound && soundEnabled)
                    Mix_PlayChannel(-1, checkpointSound, 0);
                for (int j = 0; j < afterCount; j++)
                {
                    if (afters[j].x == prevs[i].x && afters[j].y == prevs[i].y)
                    {
                        afters[j].show = true;
                        checkpointSet = true;
                        checkpointX = prevs[i].x;
                        checkpointY = prevs[i].y;

                        for (int t = 0; t < MAX_POINT_TEXTS; t++)
                        {
                            if (!pointTextEffects[t].active)
                            {
                                pointTextEffects[t].active = true;
                                pointTextEffects[t].x = afters[j].x + afters[j].width / 2 - checkpointLitImg.width / 2;
                                pointTextEffects[t].y = afters[j].y + afters[j].height + 10;
                                pointTextEffects[t].startY = pointTextEffects[t].y;
                                pointTextEffects[t].endY = pointTextEffects[t].y + 50;
                                pointTextEffects[t].timer = 50;
                                pointTextEffects[t].type = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < pointCount; i++)
    {
        if (!points[i].collected)
        {
            if (
                ballX + radius * 2 > points[i].x &&
                ballX < points[i].x + points[i].width &&
                ballY + radius * 2 > points[i].y &&
                ballY < points[i].y + points[i].height)
            {
                points[i].collected = true;
                baseScore += 150;
                if (pointSound && soundEnabled)
                    Mix_PlayChannel(-1, pointSound, 0);

                for (int t = 0; t < MAX_POINT_TEXTS; t++)
                {
                    if (!pointTextEffects[t].active)
                    {
                        pointTextEffects[t].active = true;
                        pointTextEffects[t].x = points[i].x + points[i].width / 2 - pointTextImg.width / 2;
                        pointTextEffects[t].y = points[i].y + points[i].height + 10;
                        pointTextEffects[t].startY = pointTextEffects[t].y;
                        pointTextEffects[t].endY = pointTextEffects[t].y + 50;
                        pointTextEffects[t].timer = 50;
                        pointTextEffects[t].type = 0;
                        break;
                    }
                }
            }
        }
    }

    for (int i = 0; i < spikeCount2; i++)
    {
        if (ballCollidesWithSpikePolygon(ballX, ballY, radius, spikes2[i].x, spikes2[i].y, spikes2[i].width, spikes2[i].height))
        {
            ballBlast = true;
            ballBlastX = ballX;
            ballBlastY = ballY;
            ballBlastTimer = 2000;
            gameState = 99;
            lives--;
            if (boomSound && soundEnabled)
                Mix_PlayChannel(-1, boomSound, 0);
            break;
        }
    }
}

void loadImages()
{
    iLoadImage(&menuBG, "menu.png");
    iResizeImage(&menuBG, SCREEN_WIDTH, SCREEN_HEIGHT);
    iLoadImage(&buttonImg, "button.png");
    iResizeImage(&buttonImg, buttonWidth, buttonHeight);
    iLoadImage(&background, "background.jpg");
    iResizeImage(&background, level_width, SCREEN_HEIGHT);
    iLoadImage(&background1, "background1.jpg");
    iResizeImage(&background1, SCREEN_WIDTH, SCREEN_HEIGHT);
    iLoadImage(&leaderboardImg, "leaderboard.jpg");
    iResizeImage(&leaderboardImg, SCREEN_WIDTH, SCREEN_HEIGHT);
    iLoadImage(&bluePrintImg, "blueprint.jpeg");
    iResizeImage(&bluePrintImg, SCREEN_WIDTH, SCREEN_HEIGHT);
    iLoadImage(&aboutBG, "credit.jpg");
    iResizeImage(&aboutBG, SCREEN_WIDTH, SCREEN_HEIGHT);
    iLoadImage(&arrowImg, "arrow.png");
    iResizeImage(&arrowImg, arrowWidth, arrowHeight);
    iLoadImage(&pauseImg, "pause.png");
    iResizeImage(&pauseImg, arrowWidth, arrowHeight);
    iLoadImage(&brickImg, "brick.png");
    iResizeImage(&brickImg, 50, 50);
    iLoadImage(&flagImg, "flag.png");
    iResizeImage(&flagImg, flagWidth, flagHeight);
    iLoadImage(&loadingBallSmallImg, "Ball.png");
    iResizeImage(&loadingBallSmallImg, 20, 20);
    iLoadImage(&loadingBallBigImg, "Ball.png");
    iResizeImage(&loadingBallBigImg, 30, 30);
    iLoadImage(&spikeImg, "spike.png");
    iResizeImage(&spikeImg, 100, 100);
    iLoadImage(&spikeImg2, "spike.png");
    iResizeImage(&spikeImg2, 50, 50);
    iLoadImage(&spikeImg3, "spike.png");
    iResizeImage(&spikeImg3, 150, 150);
    iLoadImage(&pointImg, "point.png");
    iResizeImage(&pointImg, 50, 50);
    iLoadImage(&afterImg, "after.png");
    iResizeImage(&afterImg, 50, 50);
    iLoadImage(&prevImg, "prev.png");
    iResizeImage(&prevImg, 75, 50);
    iLoadImage(&ballBlastImg, "ball_blast.png");
    iResizeImage(&ballBlastImg, 100, 80);
    iLoadImage(&livesButtonImg, "button.png");
    iResizeImage(&livesButtonImg, 180, 60);
    iLoadImage(&checkpointLitImg, "checkpoint_lit.png");
    iResizeImage(&checkpointLitImg, 170, 170);
    iLoadImage(&pointTextImg, "pointText.png");
    iResizeImage(&pointTextImg, 70, 70);
    iLoadImage(&scoreButtonImg, "button.png");
    iResizeImage(&scoreButtonImg, 200, 60);
    iLoadImage(&gameoverImg, "gameover.png");
    iResizeImage(&gameoverImg, 322, 189);
    iLoadImage(&congratulationImg, "congratulation.png");
    iResizeImage(&congratulationImg, 700, 294);
    iLoadImage(&soundImg, "sound.png");
    iResizeImage(&soundImg, soundButtonWidth, soundButtonHeight);
    iLoadImage(&offSoundImg, "offsound.png");
    iResizeImage(&offSoundImg, soundButtonWidth, soundButtonHeight);
    iLoadImage(&volumeIncImg, "volumeInc.png");
    iResizeImage(&volumeIncImg, volumeButtonImgSize, volumeButtonImgSize);
    iLoadImage(&volumeDecImg, "volumeDec.png");
    iResizeImage(&volumeDecImg, volumeButtonImgSize, volumeButtonImgSize);
    pointTakingSound = Mix_LoadWAV("assets/sound/pointTaking.wav");
    boomSound = Mix_LoadWAV("boom.wav");
    pointSound = Mix_LoadWAV("point.wav");
    typeSound = Mix_LoadWAV("type.wav");
    finishSound = Mix_LoadWAV("finish.wav");
    checkpointSound = Mix_LoadWAV("checkpoint.wav");

    for (int i = 0; i < 18; i++)
    {
        char filename[100];
        sprintf(filename, "assets/images/Ball_rotate/Ball_%d.png", i);
        iLoadImage(&ballSprites[i], filename);
        iResizeImage(&ballSprites[i], radius * 2, radius * 2);
    }
}
void reloadMenuBG()
{
    iLoadImage(&menuBG, "menu.png");
    iResizeImage(&menuBG, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void drawArrow()
{
    iShowLoadedImage(arrowX, arrowY + arrowHoverOffset, &arrowImg);
}

void drawPause()
{
    iShowLoadedImage(arrowX, arrowY + arrowHoverOffset, &pauseImg);
}

void drawButtonText(int x, int y, const char *text, int isHovered)
{
    int textWidth = getTextWidth(text, GLUT_BITMAP_HELVETICA_18);
    int textX = x + (buttonWidth - textWidth) / 2;
    int textY = y + (buttonHeight - 18) / 2;
    if (isHovered)
    {
        iSetColor(255, 80, 80);
        iText(textX, textY, text, GLUT_BITMAP_HELVETICA_18);
    }
    else
    {
        iSetColor(180, 180, 180);
        iText(textX + 2, textY - 2, text, GLUT_BITMAP_HELVETICA_18);
        iSetColor(0, 0, 0);
        iText(textX, textY, text, GLUT_BITMAP_HELVETICA_18);
    }
}
void drawLoadingScreen()
{
    iShowLoadedImage(0, 0, &menuBG);
    if (!showEnterButton)
    {
        int centerX = SCREEN_WIDTH / 2, centerY = 45;
        for (int i = 0; i < loadingBalls; i++)
        {
            float angle = -(2 * 3.1416f * i) / loadingBalls;
            int ballX = centerX + loadingCircleOrbitRadius * cos(angle);
            int ballY = centerY + loadingCircleOrbitRadius * sin(angle);
            Image *ballToShow = (i == activeLoadingBall) ? &loadingBallBigImg : &loadingBallSmallImg;
            iShowLoadedImage(ballX - ballToShow->width / 2, ballY - ballToShow->height / 2, ballToShow);
        }
    }
    else if (showEnterButton && enterButtonVisible)
    {
        int buttonX = enterButtonX, buttonY = enterButtonY;
        iShowLoadedImage(buttonX, buttonY, &buttonImg);
        const char *text = "PRESS ENTER";
        int textWidth = getTextWidth(text, GLUT_BITMAP_HELVETICA_18);
        int textX = buttonX + (buttonWidth - textWidth) / 2;
        int textY = buttonY + (buttonHeight - 18) / 2;
        iSetColor(0, 0, 0);
        iText(textX, textY, text, GLUT_BITMAP_HELVETICA_18);
    }
}
void drawMenu()
{
    iShowLoadedImage(0, 0, &menuBG);
    iShowLoadedImage(newGameX, newGameY, &buttonImg);
    drawButtonText(newGameX, newGameY, "NEW GAME", hoveredButton == 0);

    iShowLoadedImage(leaderboardX, leaderboardY, &buttonImg);
    drawButtonText(leaderboardX, leaderboardY, "LEADERBOARD", hoveredButton == 1);

    iShowLoadedImage(aboutX, aboutY, &buttonImg);
    drawButtonText(aboutX, aboutY, "ABOUT", hoveredButton == 2);

    iShowLoadedImage(quitX, quitY, &buttonImg);
    drawButtonText(quitX, quitY, "QUIT GAME", hoveredButton == 3);

    iShowLoadedImage(blueprintX, blueprintY, &buttonImg);
    drawButtonText(blueprintX, blueprintY, "BLUEPRINT", hoveredButton == 4);

    if (soundEnabled)
    {
        iShowLoadedImage(soundButtonX, soundButtonY, &soundImg);
        if (volumePopupTimer > 0)
        {
            iShowLoadedImage(volumeIncX, volumeIncY, &volumeIncImg);
            iShowLoadedImage(volumeDecX, volumeDecY, &volumeDecImg);
        }
    }
    else
    {
        iShowLoadedImage(soundButtonX, soundButtonY, &offSoundImg);
    }
    if (!volumeOnSoundHover && volumePopupTimer > 0)
    {
        volumePopupTimer--;
    }
}
void drawLevelSelection()
{
    iShowLoadedImage(0, 0, &background1);
    int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
    iShowLoadedImage(centerX, 520, &buttonImg);
    drawButtonText(centerX, 520, "SELECT LEVEL", 0);
    iShowLoadedImage(centerX, 400, &buttonImg);
    drawButtonText(centerX, 400, "LEVEL 1", hoveredButton == 11);
    iShowLoadedImage(centerX, 300, &buttonImg);
    drawButtonText(centerX, 300, "LEVEL 2", hoveredButton == 12);
    iShowLoadedImage(centerX, 200, &buttonImg);
    drawButtonText(centerX, 200, "LEVEL 3", hoveredButton == 13);
    drawArrow();
}
void drawGameLevel()
{
    iShowLoadedImage(-(int)window, 0, &background);
    int brickSize = 50;
    for (int x = 0; x < level_width; x += brickSize)
        iShowLoadedImage(x - (int)window, SCREEN_HEIGHT - brickSize, &brickImg);
    for (int x = 0; x < level_width; x += brickSize)
        iShowLoadedImage(x - (int)window, 0, &brickImg);
    for (int y = 0; y < SCREEN_HEIGHT; y += brickSize)
        iShowLoadedImage(-(int)window, y - 1, &brickImg);

    for (int i = 0; i < spikeCount2; i++)
    {
        int sx = spikes2[i].x - (int)window;
        int sy = spikes2[i].y;
        int sw = spikes2[i].width;
        int sh = spikes2[i].height;
        iShowLoadedImage(sx, sy, &spikes2[i].image);
        drawSpikePolygons(sx, sy, sw, sh);
    }

    for (int i = 0; i < spikeCount2; i++)
        iShowLoadedImage(spikes2[i].x - (int)window, spikes2[i].y, &spikes2[i].image);
    for (int i = 0; i < pointCount; i++)
        if (!points[i].collected)
            iShowLoadedImage(points[i].x - (int)window, points[i].y, &points[i].image);
    for (int i = 0; i < prevCount; i++)
        if (!prevs[i].collected)
            iShowLoadedImage(prevs[i].x - (int)window, prevs[i].y, &prevs[i].image);

    for (int i = 0; i < afterCount; i++)
        if (afters[i].show)
            iShowLoadedImage(afters[i].x - (int)window, afters[i].y, &afters[i].image);
    for (int i = 0; i < brickCount; i++)
    {
        Brick b = bricks[i];
        iShowLoadedImage(b.x - (int)window, b.y, b.image);

        if (b.topCollision || b.bottomCollision || b.leftCollision || b.rightCollision)
        {
            iSetColor(255, 0, 0);
            if (b.topCollision)
                iLine(b.x - (int)window, b.y + b.height, b.x + b.width - (int)window, b.y + b.height);
            if (b.bottomCollision)
                iLine(b.x - (int)window, b.y, b.x + b.width - (int)window, b.y);
            if (b.leftCollision)
                iLine(b.x - (int)window, b.y, b.x - (int)window, b.y + b.height);
            if (b.rightCollision)
                iLine(b.x + b.width - (int)window, b.y, b.x + b.width - (int)window, b.y + b.height);
        }
    }
    for (int i = 0; i < MAX_POINT_TEXTS; i++)
    {
        if (pointTextEffects[i].active)
        {
            if (pointTextEffects[i].type == 0)
                iShowLoadedImage((int)(pointTextEffects[i].x - window), (int)pointTextEffects[i].y, &pointTextImg);
            else if (pointTextEffects[i].type == 1)
                iShowLoadedImage((int)(pointTextEffects[i].x - window), (int)pointTextEffects[i].y, &checkpointLitImg);
        }
    }

    iShowLoadedImage(flagX - (int)window, flagY, &flagImg);
    iShowLoadedImage((int)(ballX - window), ballY, &ballSprites[ballFrame]);

    if (ballBlast)
        iShowLoadedImage((int)(ballBlastX - window), ballBlastY, &ballBlastImg);
    else
        iShowLoadedImage((int)(ballX - window), ballY, &ballSprites[ballFrame]);

    char scoreText[50];
    sprintf(scoreText, "Score: %d", (int)playerScore);

    int scoreButtonX = SCREEN_WIDTH - 220;
    int scoreButtonY = SCREEN_HEIGHT - 120;
    iShowLoadedImage(scoreButtonX, scoreButtonY, &scoreButtonImg);

    iSetColor(0, 0, 0);
    int textWidth = getTextWidth(scoreText, GLUT_BITMAP_HELVETICA_18);
    int textX = scoreButtonX + (200 - textWidth) / 2;
    int textY = scoreButtonY + (60 - 18) / 2 + 5;
    iText(textX, textY, scoreText, GLUT_BITMAP_HELVETICA_18);
    drawPause();

    int livesStartX = 100;
    int livesStartY = SCREEN_HEIGHT - brickSize - 70;

    iShowLoadedImage(livesStartX, livesStartY, &livesButtonImg);

    int buttonWidth = 150;
    int buttonHeight = 50;
    int ballWidth = 30;
    int ballHeight = 30;

    int totalBallsWidth = lives * ballWidth;
    int spacing = (buttonWidth - totalBallsWidth) / (lives + 1);

    int ballStartY = livesStartY + (buttonHeight - ballHeight) / 2 + 9;

    for (int i = 0; i < lives; i++)
    {
        int ballX = livesStartX + spacing * (i + 1) + ballWidth * i + 10;
        iShowLoadedImage(ballX, ballStartY, &loadingBallBigImg);
    }

    if (showPointText)
    {
        char buf[16];
        sprintf(buf, "+%d", pointTextValue);
        int w = getTextWidth(buf, GLUT_BITMAP_HELVETICA_18);
        int x = (SCREEN_WIDTH - w) / 2;
        int y = SCREEN_HEIGHT / 2;
        iSetColor(0, 200, 0);
        iText(x, y, buf, GLUT_BITMAP_HELVETICA_18);
    }
}
void drawLeaderboardLevel(int level)
{
    iShowLoadedImage(0, 0, &leaderboardImg);
    iSetColor(255, 255, 255);
    char title[64];
    sprintf(title, "LEADERBOARD - Level %d", level);
    int titleWidth = getTextWidth(title, GLUT_BITMAP_HELVETICA_18);
    iText((SCREEN_WIDTH - titleWidth) / 2, 620, title, GLUT_BITMAP_HELVETICA_18);
    LeaderboardEntry *lb;
    int count;
    if (level == 1)
    {
        lb = leaderboard1;
        count = leaderboardCount1;
    }
    else if (level == 2)
    {
        lb = leaderboard2;
        count = leaderboardCount2;
    }
    else
    {
        lb = leaderboard3;
        count = leaderboardCount3;
    }
    for (int i = 0; i < count; i++)
    {
        char entry[100];
        sprintf(entry, "%d. %s  |  Score: %d", i + 1, lb[i].name, lb[i].score);
        int w = getTextWidth(entry, GLUT_BITMAP_HELVETICA_18);
        iText((SCREEN_WIDTH - w) / 2, 550 - i * 40, entry, GLUT_BITMAP_HELVETICA_18);
    }

    drawArrow();
}
void drawAboutScreen()
{
    iShowLoadedImage(0, 0, &aboutBG);
    drawArrow();
}
void drawNameInput()
{
    iShowLoadedImage(0, 0, &background1);
    iSetColor(255, 255, 255);
    iText(350, 530, "Enter Your Name :", GLUT_BITMAP_HELVETICA_18);
    int boxX = 350, boxY = 480, boxW = 500, boxH = 40;
    iSetColor(0, 0, 0);
    iRectangle(boxX, boxY, boxW, boxH);
    iSetColor(255, 255, 255);
    iFilledRectangle(boxX + 1, boxY + 1, boxW - 2, boxH - 2);
    if (nameIndex == 0)
    {
        iSetColor(180, 180, 180);
        iText(boxX + 10, boxY + 12, "please enter name", GLUT_BITMAP_HELVETICA_18);
    }
    else
    {
        iSetColor(0, 0, 0);
        iText(boxX + 10, boxY + 12, playerName, GLUT_BITMAP_HELVETICA_18);
    }
    iSetColor(255, 255, 255);
    iText(350, 445, "Press ENTER when done.", GLUT_BITMAP_HELVETICA_12);
    char scoreText[50];
    sprintf(scoreText, "Current Score: %d", (int)playerScore);
    iSetColor(0, 0, 0);
    iText(350, 460, scoreText, GLUT_BITMAP_HELVETICA_18);
    drawArrow();
    if (showNameWarning)
    {
        iSetColor(255, 0, 0);
        iText((SCREEN_WIDTH - getTextWidth("Please enter a name!", GLUT_BITMAP_HELVETICA_18)) / 2, 420, "Please enter a name!", GLUT_BITMAP_HELVETICA_18);
    }
}

void drawScoreScreen()
{
    iShowLoadedImage(0, 0, &background1);

    int imageGOX = (SCREEN_WIDTH - 320) / 2;
    int imageGOY = 480;
    int imageCONX = (SCREEN_WIDTH - congratulationImg.width) / 2;
    int imageCONY = 430;
    if (lives > 0)
        iShowLoadedImage(imageCONX, imageCONY, &congratulationImg);
    else
        iShowLoadedImage(imageGOX, imageGOY, &gameoverImg);

    char scoreText[50];
    sprintf(scoreText, "Your Score: %d", (int)playerScore);
    iSetColor(255, 255, 255);
    int textWidth = getTextWidth(scoreText, GLUT_BITMAP_HELVETICA_18);
    iText((SCREEN_WIDTH - textWidth) / 2, 450, scoreText, GLUT_BITMAP_HELVETICA_18);

    int menuButtonX = (SCREEN_WIDTH / 2) - buttonWidth - 10;
    int levelButtonX = (SCREEN_WIDTH / 2) + 10;
    int buttonY = 300;
    iShowLoadedImage(menuButtonX, buttonY, &buttonImg);
    drawButtonText(menuButtonX, buttonY, "MAIN MENU", hoveredButton == 20);
    iShowLoadedImage(levelButtonX, buttonY, &buttonImg);
    drawButtonText(levelButtonX, buttonY, "LEVELS", hoveredButton == 21);
}

void drawPauseMenu()
{
    iShowLoadedImage(0, 0, &background1);
    int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
    iShowLoadedImage(centerX, 500, &buttonImg);
    drawButtonText(centerX, 500, "RESUME", hoveredButton == 30);
    iShowLoadedImage(centerX, 400, &buttonImg);
    drawButtonText(centerX, 400, "NEXT LEVEL", hoveredButton == 31);
    iShowLoadedImage(centerX, 300, &buttonImg);
    drawButtonText(centerX, 300, "LEVELS", hoveredButton == 32);
    iShowLoadedImage(centerX, 200, &buttonImg);
    drawButtonText(centerX, 200, "MAIN MENU", hoveredButton == 33);
}

void sortLeaderboard(LeaderboardEntry *lb, int &count)
{
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (lb[j].score > lb[i].score)
            {
                LeaderboardEntry temp = lb[i];
                lb[i] = lb[j];
                lb[j] = temp;
            }
    if (count > 10)
        count = 10;
}

void addLeaderboardEntry(const char *name, int score, int level)
{
    LeaderboardEntry *lb;
    int *count;
    if (level == 1)
    {
        lb = leaderboard1;
        count = &leaderboardCount1;
    }
    else if (level == 2)
    {
        lb = leaderboard2;
        count = &leaderboardCount2;
    }
    else
    {
        lb = leaderboard3;
        count = &leaderboardCount3;
    }
    if (*count < 10)
    {
        strcpy(lb[*count].name, name);
        lb[*count].score = score;
        lb[*count].level = level;
        (*count)++;
    }
    else
    {
        if (score > lb[9].score)
        {
            strcpy(lb[9].name, name);
            lb[9].score = score;
            lb[9].level = level;
        }
    }
    sortLeaderboard(lb, *count);
    saveLeaderboardToFile(level);
}

void iDraw()
{
    iClear();
    switch (gameState)
    {
    case -1:
        drawLoadingScreen();
        break;
    case 0:
        drawMenu();
        break;
    case 5:
        drawLevelSelection();
        break;
    case 7:
        drawLeaderboardSelect();
        break;
    case 1:
    case 2:
    case 3:
        drawGameLevel();
        break;
    case 6:
        drawLeaderboardLevel(leaderboardViewLevel);
        break;
    case 4:

        drawAboutScreen();
        break;
    case 10:
        drawNameInput();
        break;
    case 20:
        drawScoreScreen();
        break;
    case 30:
        drawPauseMenu();
        break;
    case 50:
        drawBlueprintScreen();
        break;
    case 99:
        drawGameLevel();
        break;
    default:
        break;
    }
}

void iUpdate()
{
    if (gameState != lastGameState)
    {
        if ((gameState == 1 || gameState == 2 || gameState == 3) && !levelInitialized)
        {
            lives = 3;
        }
        lastGameState = gameState;

        if (gameState == 20)
        {
            if (finishSound && soundEnabled)
                Mix_PlayChannel(-1, finishSound, 0);

            if (playerName[0] != '\0')
                addLeaderboardEntry(playerName, (int)playerScore, currentLevel);
        }
        lastGameState = gameState;
    }

    timeSinceStart += 20;
    if (gameState == -1)
    {
        loadingBlinkCounter += 20;
        if (loadingBlinkCounter >= 200)
        {
            activeLoadingBall = (activeLoadingBall + 1) % loadingBalls;
            loadingBlinkCounter = 0;
        }
        if (timeSinceStart >= 2000)
        {
            showEnterButton = true;
            blinkCounter += 20;
            if (blinkCounter >= 500)
            {
                enterButtonVisible = !enterButtonVisible;
                blinkCounter = 0;
            }
        }
    }
    if (gameState == 99)
    {
        ballBlastTimer -= 20;
        if (ballBlastTimer <= 0)
        {
            if (lives > 0)
            {
                ballBlast = false;
                if (checkpointSet)
                {
                    ballX = checkpointX;
                    ballY = checkpointY;
                    furthestX = checkpointX;
                }
                else
                {
                    ballX = 67;
                    ballY = groundY;
                }
                ballVelocityX = 0;
                jump = 0;
                Vy = 0;
                window = 0;
                gameState = currentLevel;
            }
            else
            {
                gameState = 20;
                ballBlast = false;
            }
        }
    }
    if ((gameState == 1 || gameState == 2 || gameState == 3) && !levelInitialized)
    {
        setupLevel(currentLevel);
        levelInitialized = true;
    }
    if (gameState == 1 || gameState == 2 || gameState == 3)
    {
        updateBallVelocity();
        if (ballX > furthestX)
        {
            furthestX = ballX;
            if (furthestX > maxFurthestX)
                maxFurthestX = furthestX;
        }
        int distanceScore = (int)((maxFurthestX - 67) * 0.5f);
        if (distanceScore < 0)
            distanceScore = 0;
        playerScore = baseScore + distanceScore;

        if (ballX - window > rightMargin)
        {
            float target = ballX - rightMargin;
            window += (target - window) * 0.1f;
            if (window > level_width - SCREEN_WIDTH)
                window = level_width - SCREEN_WIDTH;
        }
        else if (ballX - window < leftMargin)
        {
            float target = ballX - leftMargin;
            window += (target - window) * 0.1f;
            if (window < 0)
                window = 0;
        }
        if (ballVelocityX != 0)
        {
            if (ballX > maxScore)
            {
            }
        }
        if (ballX + radius > flagX && ballX < flagX + flagWidth)
        {
            int finalScore = (int)playerScore + (lives * 50);
            playerScore = finalScore;
            gameState = 20;
        }
    }
    for (int i = 0; i < MAX_POINT_TEXTS; i++)
    {
        if (pointTextEffects[i].active)
        {
            float progress = 1.0f - (pointTextEffects[i].timer / 50.0f);
            pointTextEffects[i].y = pointTextEffects[i].startY + (pointTextEffects[i].endY - pointTextEffects[i].startY) * progress;
            pointTextEffects[i].timer--;
            if (pointTextEffects[i].timer <= 0)
                pointTextEffects[i].active = false;
        }
    }

    if (showPointText)
    {
        pointTextTimer--;
        if (pointTextTimer <= 0)
            showPointText = false;
    }

    if (hoveredButton == 4 && arrowHoverOffset < 10)
        arrowHoverOffset++;
    else if (hoveredButton != 4 && arrowHoverOffset > 0)
        arrowHoverOffset--;
}

void iMouseMove(int mx, int my)
{
    if (mx >= arrowX && mx <= arrowX + arrowWidth && my >= arrowY && my <= arrowY + arrowHeight)
        hoveredButton = 4;
    else
        hoveredButton = -1;
    if (gameState == 0)
    {
        if (mx >= newGameX && mx <= newGameX + buttonWidth && my >= newGameY && my <= newGameY + buttonHeight)
            hoveredButton = 0;
        else if (mx >= leaderboardX && mx <= leaderboardX + buttonWidth && my >= leaderboardY && my <= leaderboardY + buttonHeight)
            hoveredButton = 1;
        else if (mx >= aboutX && mx <= aboutX + buttonWidth && my >= aboutY && my <= aboutY + buttonHeight)
            hoveredButton = 2;
        else if (mx >= quitX && mx <= quitX + buttonWidth && my >= quitY && my <= quitY + buttonHeight)
            hoveredButton = 3;
        else if (mx >= blueprintX && mx <= blueprintX + menuButtonWidth && my >= blueprintY && my <= blueprintY + menuButtonHeight)
            hoveredButton = 4;
        bool prevHover = volumeOnSoundHover;
        if (mx >= soundButtonX && mx <= soundButtonX + soundButtonWidth && my >= soundButtonY && my <= soundButtonY + soundButtonHeight)
        {
            volumeOnSoundHover = true;
            volumePopupTimer = volumePopupDuration;
        }
        else
        {
            volumeOnSoundHover = false;
            if (prevHover)
            {
                volumePopupTimer = volumePopupDuration;
            }
        }
    }
    else if (gameState == 7)
    {
        int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
        if (mx >= centerX && mx <= centerX + buttonWidth && my >= 500 && my <= 500 + buttonHeight)
            hoveredButton = 101;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 400 && my <= 400 + buttonHeight)
            hoveredButton = 102;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 300 && my <= 300 + buttonHeight)
            hoveredButton = 103;
        else if (mx >= arrowX && mx <= arrowX + arrowWidth && my >= arrowY && my <= arrowY + arrowHeight)
            hoveredButton = 4;
        else
            hoveredButton = -1;
    }
    else if (gameState == 5)
    {
        int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
        if (mx >= centerX && mx <= centerX + buttonWidth && my >= 400 && my <= 480)
            hoveredButton = 11;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 300 && my <= 380)
            hoveredButton = 12;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 200 && my <= 280)

            hoveredButton = 13;
    }
    else if (gameState == 10)
    {
        if (mx >= arrowX && mx <= arrowX + arrowWidth && my >= arrowY && my <= arrowY + arrowHeight)
            hoveredButton = 4;
    }
    else if (gameState == 20)
    {
        int menuButtonX = (SCREEN_WIDTH / 2) - buttonWidth - 10;
        int levelButtonX = (SCREEN_WIDTH / 2) + 10;
        int buttonY = 300;
        if (mx >= menuButtonX && mx <= menuButtonX + buttonWidth && my >= buttonY && my <= buttonY + buttonHeight)
            hoveredButton = 20;
        else if (mx >= levelButtonX && mx <= levelButtonX + buttonWidth && my >= buttonY && my <= buttonY + buttonHeight)
            hoveredButton = 21;
        else
            hoveredButton = -1;
    }
    else if (gameState == 30)
    {
        int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
        if (mx >= centerX && mx <= centerX + buttonWidth && my >= 500 && my <= 500 + buttonHeight)
            hoveredButton = 30;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 400 && my <= 400 + buttonHeight)
            hoveredButton = 31;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 300 && my <= 300 + buttonHeight)
            hoveredButton = 32;
        else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 200 && my <= 200 + buttonHeight)
            hoveredButton = 33;
        else
            hoveredButton = -1;
    }
}
void iMouse(int button, int state, int mx, int my)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (mx >= arrowX && mx <= arrowX + arrowWidth && my >= arrowY && my <= arrowY + arrowHeight)
        {
            if (gameState == 1 || gameState == 2 || gameState == 3)
            {
                previousGameState = gameState;
                gameState = 30;
            }
            else if (gameState == 10 || gameState == 6 || gameState == 4 || gameState == 5)
            {
                reloadMenuBG();
                gameState = 0;
                arrowHoverOffset = 0;
            }
        }
        if (gameState == 0)
        {
            if (mx >= newGameX && mx <= newGameX + buttonWidth && my >= newGameY && my <= newGameY + buttonHeight)
            {
                playerName[0] = '\0';
                nameIndex = 0;
                inputActive = 1;
                gameState = 10;
            }
            else if (mx >= leaderboardX && mx <= leaderboardX + buttonWidth && my >= leaderboardY && my <= leaderboardY + buttonHeight)
                gameState = 7;
            else if (mx >= aboutX && mx <= aboutX + buttonWidth && my >= aboutY && my <= aboutY + buttonHeight)
                gameState = 4;
            else if (mx >= blueprintX && mx <= blueprintX + menuButtonWidth && my >= blueprintY && my <= blueprintY + menuButtonHeight)
                gameState = 50;
            else if (mx >= quitX && mx <= quitX + buttonWidth && my >= quitY && my <= quitY + buttonHeight)
            {
                if (pointTakingSound)
                {
                    Mix_FreeChunk(pointTakingSound);
                    pointTakingSound = nullptr;
                }
                exit(0);
            }
            else if (mx >= soundButtonX && mx <= soundButtonX + soundButtonWidth && my >= soundButtonY && my <= soundButtonY + soundButtonHeight)
            {
                soundEnabled = !soundEnabled;
                toggleBackgroundMusic();
            }
            else if (mx >= volumeIncX && mx <= volumeIncX + volumeButtonImgSize && my >= volumeIncY && my <= volumeIncY + volumeButtonImgSize)
            {
                if (soundEnabled)
                {
                    increaseVolume();
                }
            }
            else if (mx >= volumeDecX && mx <= volumeDecX + volumeButtonImgSize && my >= volumeDecY && my <= volumeDecY + volumeButtonImgSize)
            {
                if (soundEnabled)
                {
                    decreaseVolume();
                }
            }
        }
        else if (gameState == 7)
        {
            int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
            if (mx >= centerX && mx <= centerX + buttonWidth && my >= 500 && my <= 500 + buttonHeight)
            {
                leaderboardViewLevel = 1;
                gameState = 6;
            }
            else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 400 && my <= 400 + buttonHeight)
            {
                leaderboardViewLevel = 2;
                gameState = 6;
            }
            else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 300 && my <= 300 + buttonHeight)
            {
                leaderboardViewLevel = 3;
                gameState = 6;
            }
            else if (mx >= arrowX && mx <= arrowX + arrowWidth && my >= arrowY && my <= arrowY + arrowHeight)
            {
                reloadMenuBG();
                gameState = 0;
                arrowHoverOffset = 0;
            }
        }
        else if (gameState == 5)
        {
            int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
            brickCount = 0;
            levelInitialized = false;
            if ((mx >= centerX && mx <= centerX + buttonWidth) &&
                ((my >= 400 && my <= 480) || (my >= 300 && my <= 380) || (my >= 200 && my <= 280)))
            {
                if (my >= 400 && my <= 480)
                {
                    currentLevel = 1;
                    gameState = 1;
                }
                else if (my >= 300 && my <= 380)
                {
                    currentLevel = 2;
                    gameState = 2;
                }
                else if (my >= 200 && my <= 280)
                {
                    currentLevel = 3;
                    gameState = 3;
                }
                playerScore = 0;
                baseScore = 0;
                maxScore = 200;
                ballX = 67;
                ballY = groundY;
                ballVelocityX = 0;
                jump = 0;
                Vy = 0;
                window = 0;
                arrowHoverOffset = 0;
                furthestX = 67;
                maxFurthestX = 67;
                checkpointSet = false;
                checkpointX = 67;
                checkpointY = groundY;
            }
        }
        else if (gameState == 20)
        {
            int menuButtonX = (SCREEN_WIDTH / 2) - buttonWidth - 10;
            int levelButtonX = (SCREEN_WIDTH / 2) + 10;
            int buttonY = 300;
            if (mx >= menuButtonX && mx <= menuButtonX + buttonWidth && my >= buttonY && my <= buttonY + buttonHeight)
            {
                reloadMenuBG();
                gameState = 0;
            }
            else if (mx >= levelButtonX && mx <= levelButtonX + buttonWidth && my >= buttonY && my <= buttonY + buttonHeight)
            {
                gameState = 5;
            }
        }
        else if (gameState == 50)
        {
            if (mx >= arrowX && mx <= arrowX + arrowWidth && my >= arrowY && my <= arrowY + arrowHeight)
            {
                reloadMenuBG();
                gameState = 0;
                arrowHoverOffset = 0;
            }
        }
        else if (gameState == 30)
        {
            int centerX = (SCREEN_WIDTH - buttonWidth) / 2;
            if (mx >= centerX && mx <= centerX + buttonWidth && my >= 500 && my <= 500 + buttonHeight)
                gameState = previousGameState;
            else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 400 && my <= 400 + buttonHeight)
            {
                currentLevel++;
                if (currentLevel > 3)
                    currentLevel = 1;
                playerScore = 0;
                maxScore = 200;
                ballX = 67;
                ballY = groundY;
                ballVelocityX = 0;
                jump = 0;
                Vy = 0;
                window = 0;
                levelInitialized = false;
                gameState = currentLevel;
                checkpointSet = false;
                checkpointX = 67;
                checkpointY = groundY;
            }
            else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 300 && my <= 300 + buttonHeight)
                gameState = 5;
            else if (mx >= centerX && mx <= centerX + buttonWidth && my >= 200 && my <= 200 + buttonHeight)
            {
                if (playerName[0] != '\0')
                    addLeaderboardEntry(playerName, (int)playerScore, currentLevel);
                reloadMenuBG();
                gameState = 0;
            }
        }
    }
}
void iMouseDrag(int mx, int my) {}
void iMouseWheel(int dir, int mx, int my)
{
    if (gameState == 50)
    {
        if (mx >= blueprintBoxX && mx <= blueprintBoxX + blueprintBoxW &&
            my >= blueprintBoxY && my <= blueprintBoxY + blueprintBoxH)
        {
            blueprintScrollIndex -= dir;
            int maxScroll = blueprintLineCount - 10;
            if (blueprintScrollIndex < 0)
                blueprintScrollIndex = 0;
            if (blueprintScrollIndex > maxScroll)
                blueprintScrollIndex = maxScroll;
        }
    }
}
void iKeyboard(unsigned char key)
{
    if (gameState == -1 && showEnterButton)
        gameState = 0;
    if (gameState == 10 && inputActive)
    {
        if (key == '\r' || key == '\n')
        {
            if (nameIndex == 0)
                showNameWarning = true;
            else
            {
                inputActive = 0;
                gameState = 5;
                showNameWarning = false;
            }
        }
        else if (key == '\b')
        {
            if (nameIndex > 0)
            {
                nameIndex--;
                playerName[nameIndex] = '\0';
            }
        }
        else if (nameIndex < 29 && key >= 32 && key <= 126)
        {
            playerName[nameIndex++] = key;
            playerName[nameIndex] = '\0';
            showNameWarning = false;
            if (typeSound && soundEnabled)
                Mix_PlayChannel(-1, typeSound, 0);
        }
    }
}
void iSpecialKeyboard(unsigned char key)
{
    if (gameState != 1 && gameState != 2 && gameState != 3)
        return;

    bool onSurface = (ballY == groundY);

    float ballBottom = ballY;
    float ballLeft = ballX;
    float ballRight = ballX + radius * 2;

    for (int i = 0; i < brickCount; i++)
    {
        if (ballBottom == bricks[i].y + bricks[i].height &&
            ballLeft < bricks[i].x + bricks[i].width &&
            ballRight > bricks[i].x)
        {
            onSurface = true;
            break;
        }
    }

    if (key == GLUT_KEY_UP && !jump && onSurface)
    {
        jump = 1;
        Vy = 20;
    }

    else if (key == GLUT_KEY_LEFT)
    {
        ballDirection = -1;
        ballVelocityX = -10.0f;
        ballFrame = (ballFrame - 2 + 18) % 18;
    }
    else if (key == GLUT_KEY_RIGHT)
    {
        ballDirection = 1;
        ballVelocityX = 10.0f;
        ballFrame = (ballFrame + 2) % 18;
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    iInitializeSound();
    loadLeaderboardFromFile(1);
    loadLeaderboardFromFile(2);
    loadLeaderboardFromFile(3);
    loadImages();
    startBackgroundMusic();
    iSetTimer(20, iUpdate);
    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "BOUNCIFY");
    return 0;
}