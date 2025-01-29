/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "draw.h"
#include "ball.h"
#include <string.h>

#include "xtime_l.h"
#include <stdio.h>
#include <sleep.h>
#include "xil_io.h"
#include "xil_mmu.h"
#include "platform.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xpseudo_asm.h"
#include "xil_exception.h"
#include <stdbool.h>
#include "hw_rng.h"
#include "audio.h"


//shared memory with core0 using on chip memory
#define COMM_VAL (*(volatile unsigned long *)(0xFFFF0000))
#define currentState (*(volatile int*) (0xFFFF0024))
#define vga_val (*(volatile int *)(0x00900000))
#define testPointer (*(volatile int**)(0xFFFF0048))
#define testState (*(volatile int*) (0xFFFF072))
#define cursorGameSettings (*(volatile int*) (0xFFFF096))
#define cursorScreenMenu (*(volatile int*) (0xFFFF0BA))
#define cursorScreenSettings (*(volatile int*) (0xFFFF0DE))
#define flagDraw (*(volatile bool*) (0xFFFF0102))
#define gameActiveDraw  (*(volatile bool*) (0xFFFF0126))
#define resetGame  (*(volatile bool*) (0xFFFF0150))
#define gameUnPause (*(volatile bool*) (0xFFFF0174))
#define currentScreenPointer (*(volatile int**)(0xFFFF0198))
#define ball_launched (*(volatile int*)(0xFFFF01BC))
#define paddle_x (*(volatile int*)(0xFFFF01E0))
#define paddle_y (*(volatile int*)(0xFFFF0204))
#define paddle_width (*(volatile int*)(0xFFFF0228))
#define paddle_update (*(volatile int*)(0xFFFF024C))
#define reset_call (*(volatile int*)(0xFFFF0270))
#define current_vol (*(volatile int*)(0xFFFF0294))
#define menubeep (*(volatile int*)(0xFFFF02B8))

//defines base address for getting numbers from random number generator
#define RNG_BASE XPAR_HW_RNG_0_S00_AXI_RNG_BASEADDR

extern u32 MMUTable;
//size of screen
int horz = 1280;
int vert = 1024;

XTime gbl_time_before_test;
XTime *p_gbl_time_before_test = &gbl_time_before_test;
XTime gbl_time_after_test;
XTime *p_gbl_time_after_test = &gbl_time_after_test;

XTime gbl_time_before_test2;
XTime *p_gbl_time_before_test2 = &gbl_time_before_test2;
XTime gbl_time_after_test2;
XTime *p_gbl_time_after_test2 = &gbl_time_after_test2;

XTime frameBefore;
XTime *pframeBefore = &frameBefore;
XTime frameAfter;
XTime *pframeAfter = &frameAfter;

int main()
{
    init_platform();
    print("CPU1: init_platform\n\r");

    init_audio();

    //Disable cache on OCM
    // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
    Xil_SetTlbAttributes(0xFFFF0000,0x14de2);

   //creates game array
//sets empty
    unsigned char game[1310720];
     memset(game,0,1310720);

     u32 slv_reg_read = 0;
     HW_RNG_mWriteReg(RNG_BASE, 0, 170);

	//sets up walls for game
     for(int y = 0; y < vert; y++){
   	  for(int x = 0; x < horz; x++){
   		  if( y < 5 ){
   			  game[y*horz+x] = 0xfd; //top
   		  } else if (x >= horz-5 && x < horz ){
   			  game[y*horz+x] = 0xfe;//right
   		  } else if (x >= 0 && x < 5){
   			  game[y*horz+x] = 0xfe; //left
   		  } else if (y > 1021){
   			  game[y*horz+x] = 0xff; //bottom
   		  }
   	  }
     }

	//sets up game screen walls
     unsigned char gameScreen[5242880];
     int* gameScreenPointer = (int*)gameScreen;
     fillScreen(gameScreen,0,0,0,horz,vert);
     for(int y = 0; y < vert; y++){
   	  for(int x = 0; x < horz; x++){
   		  if( x >= 0 && x < 5 ){
   			  drawPixel(gameScreen,x,y,6,6,6,horz,vert);
   		  } else if (x >= horz-5 && x < horz ){
   			  drawPixel(gameScreen,x,y,6,6,6,horz,vert);
   		  } else if (y < 5){
   			  drawPixel(gameScreen,x,y,6,6,6,horz,vert);
   		  }
   	  }
     }

	//variables used later
     unsigned char gameMenu[5242880];
     char lifeBuf[5];
     char scoreBuf[5];
     char highScoreBuf[5];
     char volumeBuf[5];
     current_vol = 5;
     int checkVolDraw = current_vol;
     int screenMenuVol = checkVolDraw;
     int gameMenuVol = checkVolDraw;
	 //used to store, high score, score, lives, etc
     int highScore = 0;
     int score = 0;
     int lives = 20;
     sprintf(highScoreBuf,"%d",highScore);
     sprintf(scoreBuf,"%d",score);
     sprintf(lifeBuf,"%d",lives);
     sprintf(volumeBuf,"%d",current_vol);
	 //sets up game menu
     int* gameMenuPointer = (int*)gameMenu;
     fillScreen(gameMenu,6,6,6,horz,vert);
     drawString(gameMenu,512,500,15,15,15,horz,vert,"GAME");
     drawString(gameMenu,512,532,15,15,15,horz,vert,"SOUND:");
     drawString(gameMenu,704,532,15,15,15,horz,vert,volumeBuf);
     drawString(gameMenu,512,564,15,15,15,horz,vert,"MAIN MENU");
     drawString(gameMenu,512,592,15,15,15,horz,vert,"RESET");
     drawString(gameMenu,512,624,15,15,15,horz,vert,"ASPECT RATIO");
     drawString(gameMenu,400,100,15,15,15,horz,vert,"SCORE:");
     drawString(gameMenu,592,100,15,15,15,horz,vert,scoreBuf);
     drawString(gameMenu,400,132,15,15,15,horz,vert,"LIVES:");
     drawString(gameMenu,592,132,15,15,15,horz,vert,lifeBuf);

	//sets up start screen
     unsigned char startScreen[5242880];
     int * startScreenPointer = (int*)startScreen;
     fillScreen(startScreen,6,6,6,horz,vert);
      drawString(startScreen,400,450,15,15,15,horz,vert,"BRICK BREAKER");
      drawString(startScreen,512,500,15,15,15,horz,vert,"GAME");
      drawString(startScreen,512,532,15,15,15,horz,vert,"SETTINGS");
      drawString(startScreen,400,600,15,15,15,horz,vert,"HIGH SCORE:");
      drawString(startScreen,400,600,15,15,15,horz,vert,"HIGH SCORE:");
      drawString(startScreen,784,600,15,15,15,horz,vert,highScoreBuf);


	//sets up start menu
     unsigned char startMenu[5242880];
     int * startMenuPointer = (int*)startMenu;
     fillScreen(startMenu,6,6,6,horz,vert);
     drawString(startMenu,512,500,15,15,15,horz,vert,"SOUND:");
     drawString(startMenu,704,500,15,15,15,horz,vert,volumeBuf);
     drawString(startMenu,512,532,15,15,15,horz,vert,"MAIN MENU");





     XTime_GetTime(p_gbl_time_before_test);
     XTime_GetTime(p_gbl_time_before_test2);
     XTime_GetTime(p_gbl_time_after_test2);
     XTime_GetTime(pframeBefore);
     XTime_GetTime(pframeAfter);
     long long int frameTime = 0;

       currentScreenPointer = startScreenPointer;

       cursorGameSettings = 0;
       cursorScreenMenu = 0;
       currentState = 0;
      cursorScreenSettings = 0;
     // char word[50] = "a b c e g h i k l m n o r s t u v";
	 //this used to set up the 2 balls for the game, although only 1 i used, both kept for testing
      int rb = 15;
      int gb = 15;
      int bb = 15;
      rb = rb << 4;
      gb = gb << 4;
      bb = bb << 4;
      int rb2 = 6;
      int gb2 = 6;
      int bb2 = 6;
      rb2 = rb2 << 4;
      gb2 = gb2 << 4;
      bb2 = bb2 << 4;
      unsigned char balldraw[128];
      for(int i = 0; i < 128; i+=4){
     	 balldraw[i] = rb;
     	 balldraw[i+1] = gb;
     	 balldraw[i+2] = bb;
     	 balldraw[i+3] = 0;
      }

      unsigned char balldraw2[128];
      for(int i = 0; i < 128; i+=4){
     	 balldraw2[i] = 0;
     	 balldraw2[i+1] = 0;
     	 balldraw2[i+2] = 0;
     	 balldraw2[i+3] = 0;
      }






	//loop forever for game
    while(1){
		//this portion resets game, and draws bricks for across newly reset game screens
    	memset(gameScreen,0,5242880);
    	memset(game,0,1310720);

	//draws walls for game
    	for(int y = 0; y < vert; y++){
    	   	  for(int x = 0; x < horz; x++){
    	   		  if( y < 5 ){
    	   			  game[y*horz+x] = 0xfd; //top
    	   		  } else if (x >= horz-5 && x < horz ){
    	   			  game[y*horz+x] = 0xfe;//right
    	   		  } else if (x >= 0 && x < 5){
    	   			  game[y*horz+x] = 0xfe; //left
    	   		  } else if (y > 1021){
    	   			  game[y*horz+x] = 0xff; //bottom
    	   		  }
    	   	  }
    	     }

    	for(int y = 0; y < vert; y++){
    	   	  for(int x = 0; x < horz; x++){
    	   		  if( x >= 0 && x < 5 ){
    	   			  drawPixel(gameScreen,x,y,6,6,6,horz,vert);
    	   		  } else if (x >= horz-5 && x < horz ){
    	   			  drawPixel(gameScreen,x,y,6,6,6,horz,vert);
    	   		  } else if (y < 5){
    	   			  drawPixel(gameScreen,x,y,6,6,6,horz,vert);
    	   		  }
    	   	  }
    	     }

	//resets ball and paddle
    	 ball gameBall = {paddle_x + 112, paddle_y - 32, 0, 0};

      collState gameCollStateCheck;
      gameCollStateCheck.collStateReturn = -1;
      gameCollStateCheck.collideObjNum = 0;


      brick paddle = {paddle_x, paddle_x+paddle_width,paddle_y, paddle_y + 32, 1};

      int brick_count = 46;

      brick bricksArr[brick_count];
      bricksArr[0] = paddle;


      int brick_row = 12;
      int brick_col = 12;
      int brick_width = 128;
      int brick_height = 64;
      int brick_gap = 12;
      brick temp_brick = {0,0,0,0,0};

	//draws bricks based of the value of the rng value
      for(int i = 1; i < brick_count; i++){
    	 slv_reg_read = HW_RNG_mReadReg(RNG_BASE, 0);

     	 if(((int)slv_reg_read % 2) == 0){
     		temp_brick.leftX = brick_col;
     		temp_brick.rightX = brick_col + brick_width;
     		temp_brick.topY = brick_row;
     		temp_brick.bottomY = brick_row + brick_height;
     		temp_brick.val = i+1;
     		bricksArr[i] = temp_brick;
     	 }
     	 else{
     		temp_brick.leftX = 0;
     		temp_brick.rightX = 0;
     		temp_brick.topY = 0;
     		temp_brick.bottomY = 0;
     		temp_brick.val = i+1;
     		bricksArr[i] = temp_brick;
     	 }


     	 if(brick_col + brick_width + brick_gap + brick_width <= horz){
     		 brick_col = brick_col + brick_width + brick_gap;
     	 }
     	 else if(brick_row + brick_height + brick_gap + brick_height <= vert - 128){
     		 brick_col = brick_gap;
     		 brick_row = brick_row + brick_height + brick_gap;
     	 }
      }

      for (int i = 0; i < brick_count; i++){
     	 drawRect(gameScreen, 5,5,5, horz,vert,bricksArr[i]);
     	 fillGameWithRect(game,bricksArr[i],horz,vert);
      }

	//draw ball
     drawBallFast(gameScreen, &balldraw2[0],gameBall.xPos,gameBall.yPos,horz,vert);
     gameUnPause = false;
     lives = 20;
     score = 0;


	//if not reseting, loop in game loop forever
      while(reset_call == 0){

		//sound for menus
    	  if(menubeep == 1){
    		  remotePlay(1,current_vol);
    		  menubeep = 0;
    	  }


		//if on start screen, update cursor based off given cursor position
    	 if(currentState == 0){
    		  currentScreenPointer = startScreenPointer;


    		  if(cursorScreenMenu == 0){
    			  drawChar(startScreen,480,532,6,6,6,horz,vert,'>');
    			  drawChar(startScreen,480,500,15,15,15,horz,vert,'>');
    		  } else  if(cursorScreenMenu == 1){
    			  drawChar(startScreen,480,500,6,6,6,horz,vert,'>');
    			  drawChar(startScreen,480,532,15,15,15,horz,vert,'>');
    		  }
    		  //remotePlay(1,current_vol);
    	 }
    	 else if (currentState == 1){ //else if on menu screen, again update cursor
    		  currentScreenPointer = startMenuPointer;
    		  if(current_vol != checkVolDraw){

    		    		    drawString(startMenu,704,500,6,6,6,horz,vert,volumeBuf);
    	 		   		    sprintf(volumeBuf,"%d",screenMenuVol);
    	 		   		    drawString(startMenu,704,500,6,6,6,horz,vert,volumeBuf);
    	 		   		    sprintf(volumeBuf,"%d",current_vol);
    		       		    drawString(startMenu,704,500,15,15,15,horz,vert,volumeBuf);
    		      		    checkVolDraw = current_vol;
    		      		    screenMenuVol = current_vol;
    		      		    //remotePlay(1,current_vol);
    	   		     }


    		  if(cursorScreenSettings == 0){
    			  drawChar(startMenu,480,532,6,6,6,horz,vert,'>');
    		  		drawChar(startMenu,480,500,15,15,15,horz,vert,'>');
    		  	 } else if(cursorScreenSettings == 1){
    		  		drawChar(startMenu,480,500,6,6,6,horz,vert,'>');
    		  		drawChar(startMenu,480,532,15,15,15,horz,vert,'>');
    		  	 }
    		  //remotePlay(1,current_vol);
    	 } //if on game screen, do game loop
    	 else if (currentState == 2){
    		  currentScreenPointer = gameScreenPointer;
    		  //remotePlay(1,current_vol);
			  //unpause the game
    		  switch (gameUnPause){
    		  case 0:

    			  break;

    		  case 1:
				//if enough time has passed, update the ball position
    			  if(frameTime > 120000){

						//check next position for a collision
    				  gameCollStateCheck = checkNextPos(gameBall, game, horz, vert, bricksArr);
    				  
    				  if(gameCollStateCheck.collideObjNum != 0){ //non zero == colliskon occures
					  //go through different states of collision based off what happening
    					  remotePlay(0, current_vol);
    					  resolveCollision(&gameBall,gameCollStateCheck,game,bricksArr[gameCollStateCheck.collideObjNum-1], horz,vert);
    					  if(gameCollStateCheck.collideObjNum == 255){ //bottom of screen
    						  drawBallFast(gameScreen, &balldraw2[0],gameBall.xPos,gameBall.yPos,horz,vert);
    						  //gamePause = 0;
    						  ball_launched = 0;
    						  drawString(gameMenu,592,132,6,6,6,horz,vert,lifeBuf);
    						  lives--;
    						  remotePlay(2,current_vol);
    						  sprintf(lifeBuf,"%d",lives);
    						  drawString(gameMenu,592,132,15,15,15,horz,vert,lifeBuf);
    					  } else if (gameCollStateCheck.collideObjNum != 254 && gameCollStateCheck.collideObjNum != 253 && gameCollStateCheck.collideObjNum != 1){ //walls
    						  drawRect(gameScreen,0,0,0, horz,vert, bricksArr[gameCollStateCheck.collideObjNum-1]);
    					  } else { //hit a brick
    						  if(gameBall.yPos < 900){
    						  drawString(gameMenu,592,100,6,6,6,horz,vert,scoreBuf);
    						  score++;
    						  sprintf(scoreBuf,"%d",score);
    						  drawString(gameMenu,592,100,15,15,15,horz,vert,scoreBuf);
    						  if(score > highScore){ //update highscore if needed
    							  drawString(startScreen,784,600,6,6,6,horz,vert,highScoreBuf);
    							  highScore = score;
    							  sprintf(highScoreBuf,"%d",highScore);
    							  drawString(startScreen,784,600,15,15,15,horz,vert,highScoreBuf);
    						  	  }
    						  }
    					  }
    				  }

    				  if(paddle_update == 1){
    					  drawRect(gameScreen, 0,0,0, horz,vert,bricksArr[0]);
    					  fillGameReplaceRect0(game,bricksArr[0],horz,vert);

    					  bricksArr[0].leftX = paddle_x;
    					  bricksArr[0].rightX = paddle_x + paddle_width;

    					  drawRect(gameScreen, 5,5,5, horz,vert,bricksArr[0]);
    					  fillGameWithRect(game,bricksArr[0],horz,vert);
    					  paddle_update = 0;
    				  }
    				  drawBallFast(gameScreen, &balldraw2[0],gameBall.xPos,gameBall.yPos,horz,vert);

    				  if(ball_launched == 0){
    					  gameBall.xPos = paddle_x + 112;
    					  gameBall.yPos = paddle_y - 32;
    					  gameBall.xVel = 0;
    					  gameBall.yVel = 0;
    				  }
    				  else if(ball_launched == 1 && (gameBall.xVel == 0) && (gameBall.yVel == 0)){
    					  gameBall.xVel = 1;
    					  gameBall.yVel = -1;
    				  }
					  //update next position of ball and draw it
    				  updateNextPos(&gameBall);
    				  drawBallFast(gameScreen, &balldraw[0],gameBall.xPos,gameBall.yPos,horz,vert);

    				  XTime_GetTime(pframeBefore);
    			  }
					
    			  XTime_GetTime(pframeAfter);
					//gets time between updates of the ball, not really a frame rate, more of a tick rate
    			  frameTime = frameAfter - frameBefore;

    		  	  break;
    		  }
    		  if(lives == 0){
    			  reset_call = 1;

    		  }

    	  } else if (currentState == 3){
			//if current state is game menu, update cursors based of where cursor should be drawn
    		  currentScreenPointer = gameMenuPointer;

    		  if(current_vol != checkVolDraw){
    		      		  		drawString(gameMenu,704,532,6,6,6,horz,vert,volumeBuf);
    		      		  		sprintf(volumeBuf,"%d",gameMenuVol);
    		      		  		drawString(gameMenu,704,532,6,6,6,horz,vert,volumeBuf);
    		      		  	    sprintf(volumeBuf,"%d",current_vol);
    		      		  		drawString(gameMenu,704,532,15,15,15,horz,vert,volumeBuf);
    		      		  		checkVolDraw = current_vol;
    		      		  		gameMenuVol = current_vol;
    		      		  		}

    		     		  if(cursorGameSettings == 0){
    		     		  		 drawChar(gameMenu,480,500,15,15,15,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,532,6,6,6,horz,vert,'>');
    		     		  	} else if(cursorGameSettings == 1){
    		     		  		drawChar(gameMenu,480,500,6,6,6,horz,vert,'>');
    		     		  		 drawChar(gameMenu,480,532,15,15,15,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,564,6,6,6,horz,vert,'>');
    		     		  	} else if(cursorGameSettings == 2){
    		     		  		drawChar(gameMenu,480,532,6,6,6,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,564,15,15,15,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,596,6,6,6,horz,vert,'>');
    		     		  	} else if (cursorGameSettings == 3){
    		     		  		drawChar(gameMenu,480,564,6,6,6,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,596,15,15,15,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,624,6,6,6,horz,vert,'>');
    		     		  	//	if(reset_call){
    		     		  			reset_call = 1;


    		     		  	} else if (cursorGameSettings == 4){
    		     		  		drawChar(gameMenu,480,596,6,6,6,horz,vert,'>');
    		     		  		drawChar(gameMenu,480,624,15,15,15,horz,vert,'>');
    		     		  	}
    	  }


      }
      reset_call = 0;
    }



    cleanup_platform();
    return 0;
}
