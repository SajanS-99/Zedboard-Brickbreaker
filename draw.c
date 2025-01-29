#include <stdio.h>
#include "draw.h"
#include <string.h>
#include "ball.h"
#include <stdlib.h>

/*

Note : X , Y positions are given from the top left of something you wish to disaply. IE: 0,0 is the top left corner, and an specific images
you wish to draw, such as a brick or character starting drawing from the top left, and then off X to thr right, and Y downwards


*/

//takes a screen(which is just an array of bytes that correspons to the length*height*4, since each pixel is 4 bytes
//shifts rgb values, since the VGA core reads only the 4 most signifcant bits
//uses sizeX aand the x,y positions given to find the correct position in the array which corresponds to the screen value
//then updates the 4 bytes at that pixel location
void drawPixel(unsigned char *screen, int x, int y, int r, int g, int b, int sizeX, int sizeY){

	r = r << 4;
	g = g << 4;
	b = b << 4;
	//assumes x,y under sizeX, sizeY
	// converts y coordinate to row(ie how many rows*4 * current row) and adds left/right offset
	screen[y*sizeX*4+x*4] = r; //red
	screen[y*sizeX*4+x*4+1] = g; // green
    screen[y*sizeX*4+x*4+2] = b; //blue
	screen[y*sizeX*4+x*4+3] = 0x00; //nothing/not used

}

//Same as draw pixel, but loops through an entire screen filling in a single colour
void fillScreen(unsigned char *screen, int r, int g, int b, int sizeX, int sizeY){

	r = r << 4;
	g = g << 4;
	b = b << 4;


	for (int i = 0; i < sizeY; i++){
			for (int j = 0; j < sizeX*4; j+=4){
					screen[i*sizeX*4+j] = r; //r
					screen[i*sizeX*4+j+1] = g; //g
					screen[i*sizeX*4+j+2] = b; //b
					screen[i*sizeX*4+j+3] = 0x00;
			}
		}
}
//ballline is just a collection of 128 bytes, or 32 pixels which contain some colours you wish the ball to be
//it then copies that line of the ball 32 times, incremeting an entire row each time to get to the next 
//horizontal line
//this results in a 32 by 32 pixel block, which represents the ball being drawn
void drawBallFast(unsigned char *screen, unsigned char *ballLine, int x, int y, int sizeX, int sizeY){

	unsigned char *tempScreen = screen;

	int horzOffset = x*4;
	int vertOffset = y*sizeX*4;
	int nextLine = sizeX*4;

	tempScreen += horzOffset;
	tempScreen += vertOffset;

	for(int i = 0; i < 32; i++){
		memcpy(tempScreen,ballLine,128);
		tempScreen +=nextLine;
	}
}

//This is similar to drawBall, but instead of taking in a single line, it takes in a brick struct. The brick struct tells 
//the code where to start drawing, and based off the size of the brick, it first creates a single line of the brick
//and copies that over multiple rows to the screen
void drawRect(unsigned char *screen, int r, int g, int b, int sizeX, int sizeY, brick currBrick){

	unsigned char *tempScreen = screen;

	r = r << 4;
	g = g << 4;
	b = b << 4;

	int nextLine = sizeX*4;
	int horzOffset = currBrick.leftX*4;
	int vertOffset = currBrick.topY*sizeX*4;

	int brickXSize = currBrick.rightX - currBrick.leftX;
	int brickYSize = currBrick.bottomY - currBrick.topY;
	int bytesToCopy = brickXSize;
	bytesToCopy *= 4;

	unsigned char *brickDraw;
	brickDraw = (unsigned char*)malloc(bytesToCopy*sizeof(char));
	 for(int i = 0; i < bytesToCopy; i+=4){
		 brickDraw[i] = r;
		 brickDraw[i+1] = g;
		 brickDraw[i+2] = b;
		 brickDraw[i+3] = 0;
	 }


	tempScreen += horzOffset;
	tempScreen += vertOffset;

	for(int i = 0; i < brickYSize; i++){
			memcpy(tempScreen,brickDraw,bytesToCopy);
			tempScreen +=nextLine;
		}


	free(brickDraw);
}
//this is just a visual helper function to draw grey bars arround the game screen, which are the walls of the game
void setUpAndDrawGameScreen(unsigned char *screen, unsigned char *game, int sizeX, int sizeY){
	memset(game,0,1310720);


  for(int y = 0; y < sizeY; y++){
	  for(int x = 0; x < sizeX; x++){
		  if( y < 5 ){
			  game[y*sizeX+x] = 0xfd; //top
		  } else if (x >= sizeX-5 && x < sizeX ){
			  game[y*sizeX+x] = 0xfe;//right
		  } else if (x >= 0 && x < 5){
			  game[y*sizeX+x] = 0xfe; //left
		  } else if (y > 1021){
			  game[y*sizeX+x] = 0xff; //bottom
		  }
	  }
  }

  memset(screen,0,5242880);

  for(int y = 0; y < sizeY; y++){
  	  for(int x = 0; x < sizeX; x++){
  		  if( x >= 0 && x < 5 ){
  			  drawPixel(screen,x,y,6,6,6,sizeX,sizeY);
  		  } else if (x >= sizeX-5 && x < sizeX ){
  			  drawPixel(screen,x,y,6,6,6,sizeX,sizeY);
  		  } else if (y < 5){
  			  drawPixel(screen,x,y,6,6,6,sizeX,sizeY);
  		  }
  	  }
    }

}

//takes in a character(0-9, A-Z) and draws specific pixels at a given location. It offsets the position X to the right, and Y downwards,
//and based of the character given, draws a preset arrangement of pixels in a 32 by 32 pixel grid
void drawChar(unsigned char *screen, int x, int y, int r, int g, int b, int sizeX, int sizeY, char character){

	switch(character){
	case '0':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 10 && j < 21 && i > 2 && i < 8){ // - top line
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 11 && i > 7 && i < 23 ){ // left bar |
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 7 && i < 23){ // right bar |
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 22 && i < 28){ // bottom line _
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}// 0
				}
			break;

	case '1':
			for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 10 && j < 16 && i > 2 && i < 23){ // |
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 21 && i > 22 && i < 28 ){ // -
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 11 && i > 7 && i < 13){ // /
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} // 1
				}
			}
			break;
	case '2':
			for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 15 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 16 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 26 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '3':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 15 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 11 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '4':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 15 && j < 21 && i > 2 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 26 && i > 17 && i < 23 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 11 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 16 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '5':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 26 && i > 2 && i < 8){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 11 && i > 7 && i < 18 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 13 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 21 && i > 23 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '6':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}  else if (j > 10 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '7':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 21 && i > 2 && i < 8){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 18 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 15 && j < 21 && i > 17 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '8':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 11 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case '9':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 15 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case 'A':
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 28 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case 'B':
			for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 2 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case 'C':
			for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 7 && i < 22){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 13){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 17 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case 'D':
			for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 2 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 23){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
				break;
	case 'E':
			for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 5 && j < 11 && i > 2 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 26 && i > 2 && i < 8 ){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 12 && i < 18){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 26 && i > 22 && i < 28){
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					}
				}
			}
			break;
	case 'F':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 26 && i > 2 && i < 8 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'G':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 7 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 26 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if(j > 20 && j < 26 && i > 17 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'H':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'I':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 8){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 16 && i > 2 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 2 && i < 8){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 11 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'J':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 20 && j < 26 && i > 2 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 22 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 11 && i > 17 && i < 22){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'K':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 8 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 17 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 16 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'L':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 26 && i > 22 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'M':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 14 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 17 && j < 21 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 13 && j < 18 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'N':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 16 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'O':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 7 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 7 && i < 23 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 2 && i < 8){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'P':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'Q':
					for(int i = 0; i < 32; i++){
						for (int j = 0; j < 32; j++){
							if(j > 5 && j < 11 && i > 7 && i < 23){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 20 && j < 26 && i > 7 && i < 18){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 15 && j < 21 && i > 17 && i < 23){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 10 && j < 16 && i > 22 && i < 28){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 20 && j < 26 && i > 22 && i < 28){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							}
						}
					}
					break;
	case 'R':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 2 && i < 8 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 17 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'S':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 21 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 17 && i < 23 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 11 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 26 && i > 2 && i < 8){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
			break;
	case 'T':
					for(int i = 0; i < 32; i++){
						for (int j = 0; j < 32; j++){
							if(j > 5 && j < 26 && i > 2 && i < 8){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 13 && j < 18 && i > 7 && i < 28 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							}
						}
					}
					break;
	case 'U':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 22){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 22 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 22 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'V':
					for(int i = 0; i < 32; i++){
						for (int j = 0; j < 32; j++){
							if(j > 5 && j < 10 && i > 2 && i < 13){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 9 && j < 14 && i > 12 && i < 23 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 13 && j < 18 && i > 22 && i < 28 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 17 && j < 22 && i > 12 && i < 23 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 21 && j < 26 && i > 2 && i < 13 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							}
						}
					}
				break;
	case 'W':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 14 && i > 17 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 17 && j < 21 && i > 17 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 13 && j < 18 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'X':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 13 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 11 && i > 17 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 17 && i < 28){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'Y':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 11 && i > 2 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 20 && j < 26 && i > 2 && i < 13 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}  else if (j > 10 && j < 21 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 13 && j < 18 && i > 17  && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case 'Z':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
						if(j > 5 && j < 26 && i > 2 && i < 8){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 26 && i > 22 && i < 28 ){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 5 && j < 11 && i > 17 && i < 23){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 10 && j < 16 && i > 12 && i < 18){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						} else if (j > 15 && j < 21 && i > 7 && i < 13){
							drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}
				}
				break;
	case ' ': // don't draw so background remains same
		//for(int i = 0; i < 32; i++){
		//	for(int j = 0; j < 32; j++){
		//		drawPixel(screen,x+j,y+i,r,g,b,sizeX,sizeY);
		//	}
		//}
			break;
	case ':':
				for(int i = 0; i < 32; i++){
					for (int j = 0; j < 32; j++){
							if(j > 10 && j < 16 && i > 7 && i < 13){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							} else if (j > 10 && j < 16 && i > 22 && i < 28 ){
								drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
							}
						}
					}
				break;
	case '>':
		for(int i = 0; i < 32; i++){
			for(int j = 0; j < 32; j++){
				if (j > 10 && j < 16 && i > 7 && i < 13 ){
					drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
				} else if (j > 15 && j < 21 && i > 12 && i < 18){
					drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
				} else if (j > 10 && j < 16 && i > 17 && i < 23){
					drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
				}
			}
		}
				break;

	case '<': //fill completely
		for(int i = 0; i < 32; i++){
			for(int j = 0; j < 32; j++){
					drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
				}
			}
				break;
	default:
			//draw 0 for unknown char, or any char not accounted for in list
		for(int i = 0; i < 32; i++){
				for (int j = 0; j < 32; j++){
					if(j > 10 && j < 21 && i > 2 && i < 8){ // - top line
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 5 && j < 11 && i > 7 && i < 23 ){ // left bar |
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 20 && j < 26 && i > 7 && i < 23){ // right bar |
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
					} else if (j > 10 && j < 21 && i > 22 && i < 28){ // bottom line _
						drawPixel(screen, x+j,y+i,r,g,b,sizeX,sizeY);
						}
					}// 0
				}
			break;
	}


}
//loops through a char array drawing a char, until it reaches end of string
//it also checks for overflow from the screen, such as checking if the xPos is not too far right(ie, less than the max horizontal row size)
//and checks if the yPos is still above the bottom of the screen
void drawString(unsigned char *screen, int x, int y, int r, int g, int b, int sizeX, int sizeY, char *sentence){
	int currentChar = 0;
	int xPos = x;
	int yPos = y;
	while(sentence[currentChar] != '\0'){
		drawChar(screen,xPos,yPos,r,g,b,sizeX,sizeY,sentence[currentChar]);
		xPos +=32;
		if(xPos >= sizeX){
			yPos +=32;
			xPos = 0;
		}
		if(yPos >= sizeY){
			break;
		}
		currentChar++;
	}
}


