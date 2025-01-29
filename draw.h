#ifndef DRAW_H
#define DRAW_H
#include "ball.h"

//assumes 32/4 byte per pixel
void drawPixel(unsigned char *screen, int x, int y, int r, int g, int b, int sizeX, int sizeY); // future draw

void drawBallFast(unsigned char *screen, unsigned char *ballLine, int x, int y, int sizeX, int sizeY);

void drawRect(unsigned char *screen, int r, int g, int b, int sizeX, int sizeY, brick currBrick);

void setUpAndDrawGameScreen(unsigned char *screen, unsigned char *game, int sizeX, int sizeY);

void fillScreen(unsigned char *screen, int r, int g, int b, int sizeX, int sizeY); // fills screen with single color

void drawChar(unsigned char *screen, int x, int y, int r, int g, int b, int sizeX, int sizeY, char character); // draws a char, assumes 32x32 grid

void drawString(unsigned char *screen, int x, int y, int r, int g, int b, int sizeX, int sizeY, char *sentence); //calls drawChar repeatedly for string



#endif
