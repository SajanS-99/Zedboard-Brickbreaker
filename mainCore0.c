#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "draw.h"
#include "ball.h"
#include "aspectRatio.h"
#include "xil_io.h"
#include "xil_mmu.h"
#include "xpseudo_asm.h"
#include <sleep.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


// Parameter definitions
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TMR_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID
#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define INTC_TMR_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR

#define BTN_INT 			XGPIO_IR_CH1_MASK
#define TMR_LOAD			0xFFF40000

XGpio BTNInst;
XScuGic INTCInst;
XTmrCtr TMRInst;



//Variable declaration for later
static int btn_value;

volatile bool TIMER_INTR_FLG;

int previousState;
int buttonMove = 0;
int aspectRat = 0;

int buttonX = 0;
int buttonY = 0;

long long int diffTime;
int horz = 1280;
int vert = 1024;

int * image_buffer_pointer = (int *)0x00900000;
int NUM_BYTES_BUFFER = 5242880;


 int state = 0;
 char strToDisp[16];
 int buttonCount;
 int* currentScreenPointer;
 unsigned char TwoByOneScreen[5242880];
 int * pointerTo2ndScreen = (int*)TwoByOneScreen;


//from AMD forums and AMD docs, used for time functions later
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

#define sev() __asm__("sev")
#define ARM1_STARTADR 0xFFFFFFF0
#define ARM1_BASEADDR 0x10080000
//shared values using on chip memory, each allocated 24 32-bit words, for extra space safety
#define COMM_VAL (*(volatile unsigned long *)(0xFFFF0000))
#define currentState (*(volatile int*) (0xFFFF0024))
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

//

//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
static void BTN_Intr_Handler(void *baseaddr_p);
static void TMR_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr);

//----------------------------------------------------
// INTERRUPT HANDLER FUNCTIONS
// - called by the timer, button interrupt, performs
// - LED flashing
//----------------------------------------------------



/*
Button handler mostly consistes of changing cursor values, which are used to select options. It simply 
checks current screen state(0 = start screen, 1 = start menu, 2 = game screen, 3 = game menu)
based off the cursor position, it can go up or down, and pressing center activates the selected option
for left/right, it changes volume or paddle position based if its in a menu or in the game screen

*/

void BTN_Intr_Handler(void *InstancePtr)
{

	XTime_GetTime(p_gbl_time_after_test); //gets current time
	//value to get difference between time last pushed a button and now
	diffTime = (u64) gbl_time_after_test - (u64) gbl_time_before_test;
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) !=
			BTN_INT) {
			return;
		}
	btn_value = XGpio_DiscreteRead(&BTNInst, 1);
	//if enough time has passed between button presses, 
	if (diffTime > 1250000){
	// Increment counter based on button value
	// Reset if centre button pressed
		if(btn_value == 2){ //down
			buttonCount += 1;
			state = 1;
			flagDraw = true;
			buttonMove = 2;


			
			if(currentState == 0){ //down
				menubeep = 1;
					//if on a menu, move cursor down
							if(cursorScreenMenu < 1){
								cursorScreenMenu++;
							}
						} else if (currentState == 1){
							menubeep = 1;
							if(cursorScreenSettings < 1){
								cursorScreenSettings++;
							}
						} else if (currentState == 2){ //exit out of game, so pause if one game screen
							currentState = 3;
							gameUnPause = false;
							gameActiveDraw = false;
						} else if (currentState == 3){
							menubeep = 1;
							if(cursorGameSettings < 4){
								cursorGameSettings++;
							}
						}



		}else if (btn_value == 4) { //left
			buttonCount += 4;
			state = 2;
			flagDraw = true;
			buttonMove = 3;

			//update paddle position if on game screen, else change volume if on volume setting and correct menu screen for volume

			if(currentState == 2){
							if(paddle_x - 32 >= 0){
								paddle_x = paddle_x - 32;
								paddle_update = 1;
							}
			} else if (currentState == 1){
				menubeep = 1;
				if(cursorScreenSettings == 0){
					if(current_vol > 1){
						current_vol--;
					}
				}
			} else if (currentState == 3){
				menubeep = 1;
				if(cursorGameSettings == 1){
					if(current_vol > 1){
						current_vol--;
					}
				}
			}

		} else if (btn_value == 8) { //right
			buttonCount = 15;
			state = 3;
			flagDraw = true;
			buttonMove = 4;
			//update paddle position if on game screen, else change volume if on volume setting and correct menu screen for volume

			if(currentState == 2){
							if(paddle_x + paddle_width + 32 <= horz){
								paddle_x = paddle_x + 32;
								paddle_update = 1;
							}
			} else if (currentState == 1){
				menubeep = 1;
				if(cursorScreenSettings == 0){
					if(current_vol < 10){
						current_vol++;
					}
				}
			} else if (currentState == 3){
				menubeep = 1;
				if(cursorGameSettings == 1){
					if(current_vol < 10){
						current_vol++;
					}
				}
			}

		} else if (btn_value == 16) { //top

			buttonCount = 1000;
			state = 4;
			flagDraw = true;
			//ignore gameState


				//move cursor up if on one of the 3 menus
			buttonMove = 1;
			if(currentState == 0){
				menubeep = 1;
						if(cursorScreenMenu > 0){
							cursorScreenMenu--;
							}
				} else if (currentState == 1){
					menubeep = 1;
						if(cursorScreenSettings > 0){
							cursorScreenSettings--;
						}
				} else if (currentState == 3){
					menubeep = 1;
						if(cursorGameSettings > 0){
							cursorGameSettings--;
						}
				}



		} else if (btn_value == 1) { //centre
			buttonCount = 0;

			state = 0;
			flagDraw = true;

			if(testState == 0){
					testState = 1;
			} else {
					testState = 0;
			}
			//on start menu
			if(currentState == 0){
				menubeep = 1;
				if(cursorScreenMenu == 0){ //start menu to game
					currentState = 2;
					gameActiveDraw = true;
				} else if (cursorScreenMenu == 1){ //start menu to settings
					currentState = 1;
				}
			} else if (currentState == 1){ //on settings
				menubeep = 1;
				if(cursorScreenSettings == 1){ //goto menu from start settings
					currentState = 0;
				}
			} else if (currentState == 3){ // on game mennu
				menubeep = 1;
				if(cursorGameSettings == 2){ // go to main menu
					currentState = 0;
				} else if (cursorGameSettings == 0){ //game pause to game
					currentState = 2;
					gameActiveDraw = true;
					gameUnPause = true;// for actually game, should resume after unpause
				} else if (cursorGameSettings == 3){// reset game
					//maybe do nothing and put this in game loop
					resetGame = true;
					reset_call = 1;
					ball_launched = 0;
					paddle_x = 512;
					paddle_y = 960;
				} else if (cursorGameSettings == 4){ //change aspect ratio to other aspect ratio
					if(aspectRat == 0){
						aspectRat = 1;
					} else if (aspectRat == 1){
						aspectRat = 0;
					}
				}
			} else if (currentState == 2){ //start game
				gameUnPause = true;
				ball_launched = 1;
			}





		}

		 XTime_GetTime(p_gbl_time_before_test);

	}
  
    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);




}

void TMR_Intr_Handler(void *data)
{
	//stop time, display the image currently pointed to and change the aspect ratio if the aspect ratio flag is set true
	XTmrCtr_Stop(&TMRInst,0);


	currentAspectRatioOut(image_buffer_pointer, currentScreenPointer,pointerTo2ndScreen,horz,vert,aspectRat);

	//reset and start up timer again
	XTmrCtr_Reset(&TMRInst,0);
	XTmrCtr_Start(&TMRInst,0);

}



//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{
  int status;
  //----------------------------------------------------
  // INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
  //----------------------------------------------------
  // Initialise LEDs
  //status = XGpio_Initialize(&LEDInst, LEDS_DEVICE_ID);
  //if(status != XST_SUCCESS) return XST_FAILURE;
  // Initialise Push Buttons
  status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  // Set LEDs direction to outputs
  //XGpio_SetDataDirection(&LEDInst, 1, 0x00);
  // Set all buttons direction to inputs
  XGpio_SetDataDirection(&BTNInst, 1, 0xFF);


  //----------------------------------------------------
  // SETUP THE TIMER
  //----------------------------------------------------
  status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  XTmrCtr_SetHandler(&TMRInst, TMR_Intr_Handler, &TMRInst);
  XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
  XTmrCtr_SetOptions(&TMRInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
 

  // Initialize interrupt controller
  status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst, &BTNInst);
  if(status != XST_SUCCESS) return XST_FAILURE;

 // XTmrCtr_Start(&TMRInst, 0);

  //Disable cache on OCM
  // S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
  Xil_SetTlbAttributes(0xFFFF0000,0x14de2);

  //print("ARM0: writing startaddress for ARM1\n\r");
  Xil_Out32(ARM1_STARTADR, ARM1_BASEADDR);
  dmb(); //waits until write has finished

  //print("ARM0: sending the SEV to wake up ARM1\n\r");
  sev();


	//settings up a large amount of variables used later in the program
  buttonCount = 0;
  testState = 0;



  XTime_GetTime(p_gbl_time_before_test);
  XTime_GetTime(p_gbl_time_before_test2);
  XTime_GetTime(p_gbl_time_after_test2);
  XTime_GetTime(pframeBefore);
  XTime_GetTime(pframeAfter);
  long long int frameTime = 0;


  int loop = 0;
  int counter = 0;
  long long int diffTime2;
  cursorGameSettings = 0;
  cursorScreenMenu = 0;
  currentState = 0;
  previousState = 0;
 cursorScreenSettings = 0;
 flagDraw = false;
 gameActiveDraw = false;
 gameUnPause = false;
 resetGame = false;
 current_vol = 1;

 ball_launched = 0;
 paddle_x = 512;
 paddle_y = 960;
 paddle_width = 256; //32*8
 paddle_update = 0;
 reset_call = 0;
 menubeep = 0;


sleep(5); //sleep for 5 seconds, to let core 1 set up the memory spaces that are accessed from core 0, else it results in a memory access error without this line
//since core 0 is attempting to access memory which hasn't been allocated yet



 XTmrCtr_Start(&TMRInst, 0);

  while(1){
		// loop forever, since game always running 
	  	  	  }




  return 0;
}

//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Enable interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();


	return XST_SUCCESS;

}

int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;
	
	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;


	// Connect timer interrupt to handler
	status = XScuGic_Connect(&INTCInst,
							 INTC_TMR_INTERRUPT_ID,
							 (Xil_ExceptionHandler)TMR_Intr_Handler,
							 (void *)TmrInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);
	
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID);
	

	return XST_SUCCESS;
}

