#ifndef BALL_H
#define BALL_H

struct ball_s{
	int xPos;
	int yPos;
	int xVel; // 0 is -1, easier for code later
	int yVel;
};

typedef struct ball_s ball;

struct brick_s{ //from x = leftX to x <= rightX
	//from y = topY, to y<= bottomY, topY is lower than bottomY since low Y is higher up
	int leftX;
	int rightX;
	int topY;
	int bottomY;
	int val;
};

typedef struct brick_s brick;

struct collState_s {
	int collideObjNum;
	int collStateReturn;
};

typedef struct collState_s collState;

collState checkNextPos(ball curBall,unsigned char game[], int sizeX, int sizeY, brick brickArr[]);

void fillGameWithRect(unsigned char game[], brick brickToFill, int sizeX, int sizeY);

void fillGameReplaceRect0(unsigned char game[], brick brickToFill, int sizeX, int sizeY);

void updateNextPos(ball *curBall);

void resolveCollision(ball *curBall, collState collStateArg, unsigned char game[], brick brickToFill, int sizeX, int sizeY);

void setUpBrickStates(int states, int totBricks, brick brickArr[states][totBricks], int brickArrEnd[]);



#endif
