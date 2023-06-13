# Miniature_tetris
Tetris game based on PIC16F887

This is a port of the Tetris game using a PIC16F887, a LCD 16x2 and 4 buttons.

This little game have a start button, fast fall and move (right or left) buttons.

# How it works
Using the auto screen print of my driver, the program create a figure making set some bits of the array of the screeen, then, until no one bit is falling for gravity the program will create another figure. The selection of what figure to spawn is selected with the current time, using the 2 less significant bits of the TMR0

## Gravity logic
The program use an AND mask between the screen line and 1 line below this. Then whith and OR mask the pixels get back 1 line, and if there some pixels in the 2 lines the bits r restored using an OR mask whith the previous AND mask result.

