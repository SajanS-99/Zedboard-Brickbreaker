#include "aspectRatio.h"
#include <stdio.h>
#include <string.h>


//copies every other line of array 1(full screen) to the first half of the 2nd array, and sets the 2nd half of the 2nd array to black
// gives an effect of shrinking the image by half, which in effect is a change of aspect ratio
void changeTwoToOne(char* arrayToFill2To1, char* screenGiven, int horz, int vert){

	for(int i = 0; i < vert/2; i++){	//8 = 4*2, skip over 1 line
		memcpy(&arrayToFill2To1[i*horz*4],&screenGiven[i*horz*8], horz*4);
	}						//start halfway through screen
	memset(&arrayToFill2To1[horz*vert*2], 0, 2621440); //horz*vert*2

}

//using aspect state as a flag, copies either the full image to where the VGA is reading, or only a changed image with a different aspect ratio
//to where the VGA is reading
void currentAspectRatioOut(int *pointerToVGA, int *fullScreenPointer, int *halfScreenPointer, int horz, int vert, int aspectState){

	switch(aspectState){

	case 0:
		memcpy(pointerToVGA, fullScreenPointer, 5242880); //vert*horz*4, hardcode in to make faster
		break;
	case 1:
		changeTwoToOne(halfScreenPointer, fullScreenPointer,horz,vert); //changes current screen image to different aspect ratio
		memcpy(pointerToVGA, halfScreenPointer,5242880);
		break;

	default:
		break;

	}

}
