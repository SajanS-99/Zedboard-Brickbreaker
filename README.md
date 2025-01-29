# Zedboard-Brickbreaker

A simplified version of brickbreaker implented on a Zedboard Development Board by Digilent. Source code authors are split between myself and a fellow student, so I can't claim authorship of the audio code nor parts of the collision and game loop code. 

The project used a VGA IP core to display graphics. There were four main frames, which consisted of a title screen, a board screen, and 2 menu screens depending on if the menu was entered from the title or board state. 

Each frame is stored in DRAM, and is written at the start of the program. One of the ARM cores is dedicated to updating the current frame whenever the game state changes. The other is dedicated to audio and the core game loop.

A pseudo random number generator made using VHDL is used to randomize the block positions for the game. 

Interupts are used to read and resolve push button inputs. 
