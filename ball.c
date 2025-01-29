#include <stdio.h>
#include "ball.h"
#include "draw.h"

//Movement function, simply move ball by its velocity
void updateNextPos(ball *curBall){
	ball *currentBall = curBall;

	currentBall->xPos += currentBall->xVel;
	currentBall->yPos += currentBall->yVel;

}

//Collision function of next possible movement
collState checkNextPos(ball curBall, unsigned char game[], int sizeX, int sizeY, brick brickArr[]){ //pass reference so should be unchanged
	collState collStateToReturn;
	ball currentBall = curBall;
	int pixCheck = 0;

	//increase ball position once by velocity
	//basically looking one game tick into the future to check for overlaps
	currentBall.xPos += currentBall.xVel;
	currentBall.yPos += currentBall.yVel;

	// checks 32x32 of ball
	// checks only the edges of the ball, checking the center of the ball is redundant
	// first we check the horizontal edges, left and right of the ball
	for(int i = currentBall.yPos; i < 32+currentBall.yPos; i++){
		for(int j = 0; j < 1; j++){

			//calculate based on ball position what pixels it is now occupying
			//and check what those pixels have in terms of game objects
			pixCheck = game[i*sizeX+currentBall.xPos+(j*31)];

			//The walls and floor have hard coded tags
			if(pixCheck == 253){ //top
				collStateToReturn.collideObjNum = 253;
				collStateToReturn.collStateReturn = 0;
				return collStateToReturn;
			} else if(pixCheck == 254){ //left/right
				collStateToReturn.collideObjNum = 254;
				collStateToReturn.collStateReturn = 1;
				return collStateToReturn;
			} else if (pixCheck == 255){ //bottom
				collStateToReturn.collideObjNum = 255;
				collStateToReturn.collStateReturn = 2;
				return collStateToReturn;

			//else if the pixel isn't '0' signifying nothing
			} else if (pixCheck != 0){

				//we do a bitmask of the last bit
				//this will tell us if it was a horizontal or vertical collision
				collStateToReturn.collStateReturn = pixCheck & 0x1;

				//we then bitshift the tag removing the last bit to get the tag of the block it's colliding with
				//the tag corresponds to the index of the block in the block array
				//the paddle, walls, and floor all technically count as blocks
				collStateToReturn.collideObjNum = pixCheck >> 1;
				return collStateToReturn;
			}
		}

	}

	//this is the above function except it checks the horizontal edges of the ball, top and bottom
	for(int j = currentBall.xPos; j < 32+currentBall.xPos; j++){
		for(int i = 0; i < 1; i++){
				pixCheck = game[(currentBall.yPos+(i*31))*sizeX+j];

				//check for special tags, walls, ceiling, and floor
				if(pixCheck == 253){ //top
					collStateToReturn.collideObjNum = 253;
					collStateToReturn.collStateReturn = 0;
					return collStateToReturn;
				} else if(pixCheck == 254){ //left/right
					collStateToReturn.collideObjNum = 254;
					collStateToReturn.collStateReturn = 1;
					return collStateToReturn;
				} else if (pixCheck == 255){ //bottom
					collStateToReturn.collideObjNum = 255;
					collStateToReturn.collStateReturn = 2;
					return collStateToReturn;

				//else if the pixel isn't '0' signifying nothing
				} else if (pixCheck != 0){

					//we do a bitmask of the last bit
					//this will tell us if it was a horizontal or vertical collision
					collStateToReturn.collStateReturn = pixCheck & 0x1;

					//we then bitshift the tag removing the last bit to get the tag of the block it's colliding with
					//the tag corresponds to the index of the block in the block array
					//the paddle, walls, and floor all technically count as blocks
					collStateToReturn.collideObjNum = pixCheck >> 1;

					return collStateToReturn;
				}
		}
	}
	collStateToReturn.collideObjNum = pixCheck;// should be zero if not collided
	collStateToReturn.collStateReturn = -1; //default case in resolve collision, do nothing
	return collStateToReturn;
}

//function to resolve the collision of the ball
//figure out if the ball should be reset, or bounce
void resolveCollision(ball *curBall, collState collStateArg, unsigned char game[], brick brickToFill,int sizeX, int sizeY){

	ball *currentBall = curBall;
	collState currentCollState = collStateArg;

	//switch case using the bitmasked edge part of the block tag
	switch (currentCollState.collStateReturn){

	case 0: //hit on a horizontal edge, flip vertical speed

		currentBall->yVel = -currentBall->yVel;

		break;

	case 1: //hit from a vertical edge, continue vertical speed but flip horizontal speed

		currentBall->xVel = -currentBall->xVel;

		break;

	case 2: //hit the floor, stop moving

		currentBall->yVel = 0;
		currentBall->xVel = 0;

		break;

	default:
		break;
	}

	/*if (currentCollState.collideObjNum == 255){
		//currentBall->xPos = 640;
		//currentBall->yPos = 780;
	} else */

	//if the collision wasn't with a persistent object (walls, ceiling, paddle), delete the visual display of the object
	//as it would be a block or the floor, but the floor has no visuals so that would cause no issues
	if(currentCollState.collideObjNum != 253 && currentCollState.collideObjNum != 254 && currentCollState.collideObjNum != 1){
		fillGameReplaceRect0(game, brickToFill, sizeX, sizeY);
	}


}
//uses a bricks position, it updates the game array with its value stored in the brick
//this indicates that on the game board, a brick is at that position
void fillGameWithRect(unsigned char game[], brick brickToFill, int sizeX, int sizeY){

	//int xLen = brickToFill.rightX - brickToFill.leftX;
	//int yLen = brickToFill.bottomY - brickToFill.topY;
	brickToFill.val = brickToFill.val << 1;

	for(int i = brickToFill.topY+1; i < brickToFill.bottomY-1; i++){
		for(int j = brickToFill.leftX+1; j < brickToFill.rightX-1; j++){
			game[i*sizeX+j] = brickToFill.val;
		}
	}
	for(int i = brickToFill.topY; i < brickToFill.bottomY; i++){
		game[i*sizeX+brickToFill.leftX] = ((brickToFill.val)) | (0x1);
		game[i*sizeX+brickToFill.rightX-1] = ((brickToFill.val)) | (0x1);
	}
	for(int j = brickToFill.leftX; j < brickToFill.rightX; j++){
		game[brickToFill.topY*sizeX+j] = ((brickToFill.val)) | (0x0);
		game[(brickToFill.bottomY-1)*sizeX+j] = ((brickToFill.val)) | (0x0);
	}
}

//takes a bricks position, which has an x and y position. Looping through the position on the screen at that x,y, it sets those positions to 0
// 0 essentially acts as an empty space, so this is used for removing a brick and telling the game the spot is empty
void fillGameReplaceRect0(unsigned char game[], brick brickToFill, int sizeX, int sizeY){

//	int xLen = brickToFill.rightX - brickToFill.leftX;
//	int yLen = brickToFill.bottomY - brickToFill.topY;

	for(int i = brickToFill.topY; i < brickToFill.bottomY; i++){
				for(int j = brickToFill.leftX; j < brickToFill.rightX; j++){
					game[i*sizeX+j] = 0;
				}
			}

}

//sets up the the bricks and gives it a position in the brick array, which is later used to quickly access and remove bricks from the game board
void setUpBrickStates(int states, int totBricks, brick brickArr[states][totBricks],int brickArrEnd[]){

	int curVal = 2;
	int curState = 0;

	for(int i = 0; i < 4; i++){
			for(int j = 0; j < 5; j++ ){
				brickArr[curState][i*5+j].bottomY = 100 * (i+1) + 50;
				brickArr[curState][i*5+j].topY = 100 * (i+1);
				brickArr[curState][i*5+j].leftX = 250+j*150;
				brickArr[curState][i*5+j].rightX = 350+j*150;
				brickArr[curState][i*5+j].val = curVal;
				curVal++;
			}
		}


	brickArrEnd[curState] = 20;
	curState++;
	curVal = 2;

	for(int i = 0; i < 4; i++){
			for(int j = 0; j < 2; j++ ){
				brickArr[curState][i*2+j].bottomY = 100 * (i+1) + 50;
				brickArr[curState][i*2+j].topY = 100 * (i+1);
				brickArr[curState][i*2+j].leftX = 100+j*150;
				brickArr[curState][i*2+j].rightX = 200+j*150;
				brickArr[curState][i*2+j].val = curVal;
				curVal++;
			}
		}

	for(int i = 0; i < 4; i++){
				for(int j = 0; j < 2; j++ ){
					brickArr[curState][i*2+j+8].bottomY = 100 * (i+1) + 50;
					brickArr[curState][i*2+j+8].topY = 100 * (i+1);
					brickArr[curState][i*2+j+8].leftX = 750+j*150;
					brickArr[curState][i*2+j+8].rightX = 850+j*150;
					brickArr[curState][i*2+j+8].val = curVal;
					curVal++;
				}
			}

	brickArrEnd[curState] = 16;
	curState++;
	curVal = 2;

	for(int i = 0; i < 6; i++){
		brickArr[curState][i].bottomY = 150;
		brickArr[curState][i].topY = 100;
		brickArr[curState][i].leftX = 100+i*150;
		brickArr[curState][i].rightX = 200+i*150;
		brickArr[curState][i].val = curVal;
		curVal++;
	}

	for(int i = 0; i < 6; i++){
			brickArr[curState][i+6].bottomY = 500;
			brickArr[curState][i+6].topY = 450;
			brickArr[curState][i+6].leftX = 100+i*150;
			brickArr[curState][i+6].rightX = 200+i*150;
			brickArr[curState][i+6].val = curVal;
			curVal++;
		}

	brickArrEnd[curState] = 12;
	curState++;
	curVal = 2;

	for(int i = 0; i < 4; i++){
		brickArr[curState][i].bottomY = 250 + i*100;
		brickArr[curState][i].topY = 200 + i*100;
		brickArr[curState][i].leftX = 580;
		brickArr[curState][i].rightX = 680;
		brickArr[curState][i].val = curVal;
		curVal++;
		}

	for(int i = 0; i < 2; i++){
			brickArr[curState][i+4].bottomY = 350 + i*100;
			brickArr[curState][i+4].topY = 300 + i*100;
			brickArr[curState][i+4].leftX = 280;
			brickArr[curState][i+4].rightX = 380;
			brickArr[curState][i+4].val = curVal;
			curVal++;
			}

	for(int i = 0; i < 2; i++){
				brickArr[curState][i+6].bottomY = 350 + i*100;
				brickArr[curState][i+6].topY = 300 + i*100;
				brickArr[curState][i+6].leftX = 880;
				brickArr[curState][i+6].rightX = 980;
				brickArr[curState][i+6].val = curVal;
				curVal++;
				}

	brickArrEnd[curState] = 8;
		curState++;
		curVal = 2;


		for(int i = 0; i < 2; i++){
				for(int j = 0; j < 7; j++ ){
					brickArr[curState][i*7+j].bottomY = 100 * (i+1) + 50;
					brickArr[curState][i*7+j].topY = 100 * (i+1);
					brickArr[curState][i*7+j].leftX = 100+j*150;
					brickArr[curState][i*7+j].rightX = 200+j*150;
					brickArr[curState][i*7+j].val = curVal;
					curVal++;
				}
			}

		brickArrEnd[curState] = 14;
		curState++;
		curVal = 2;

			for(int i = 0; i < 4; i++){
					for(int j = 0; j < 2; j++ ){
						brickArr[curState][i*2+j].bottomY = 100 * (i+1) + 50;
						brickArr[curState][i*2+j].topY = 100 * (i+1);
						brickArr[curState][i*2+j].leftX = 100+j*150;
						brickArr[curState][i*2+j].rightX = 200+j*150;
						brickArr[curState][i*2+j].val = curVal;
						curVal++;
					}
				}

			for(int i = 0; i < 4; i++){
						for(int j = 0; j < 2; j++ ){
							brickArr[curState][i*2+j+8].bottomY = 100 * (i+1) + 50;
							brickArr[curState][i*2+j+8].topY = 100 * (i+1);
							brickArr[curState][i*2+j+8].leftX = 750+j*150;
							brickArr[curState][i*2+j+8].rightX = 850+j*150;
							brickArr[curState][i*2+j+8].val = curVal;
							curVal++;
						}
					}

			brickArrEnd[curState] = 20;
			curState++;
			curVal = 2;



}
