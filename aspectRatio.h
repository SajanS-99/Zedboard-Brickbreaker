#ifndef ASPECTRATIO_H
#define ASPECTRATIO_H


//changes screen to a different aspect ratio
void changeTwoToOne(char* arrayToFill2To1, char* screenGiven, int horz, int vert);

//decides which aspect ratio to use
void currentAspectRatioOut(int *pointerToVGA, int *fullScreenPointer, int *halfScreenPointer, int horz, int vert, int aspectState);


#endif
