/*
 * pacman.h
 */

#ifndef INC_PACMAN_H_
#define INC_PACMAN_H_

/* Includes */
#include <stdint.h>

/* Constants */
#define MAZE_COLUMN_N      		10
#define MAZE_ROW_N         		9
#define MAZE_CELL_WIDTH    		20
#define MAZE_CELL_HEIGHT        20

#define MAZE_TOP_BORDER			35
#define MAZE_BOTTOM_BORDER 		215 // 20*9+35
#define MAZE_LEFT_BORDER		20
#define MAZE_RIGHT_BORDER		220

#define BACKGROUND_COLOR		WHITE
#define PACMAN_COLOR			YELLOW
#define GHOST_COLOR				RED
#define PAC_DOTS_COLOR			BROWN

#define BTN_SIZE                40

#define BTN_UP_X1               100
#define BTN_UP_Y1               225
#define BTN_LEFT_X1             50
#define BTN_LEFT_Y1             275
#define BTN_DOWN_X1             100
#define BTN_DOWN_Y1             275
#define BTN_RIGHT_X1            150
#define BTN_RIGHT_Y1            275

#define BTN_UP_X2               (BTN_UP_X1 + BTN_SIZE)
#define BTN_UP_Y2               (BTN_UP_Y1 + BTN_SIZE)
#define BTN_LEFT_X2             (BTN_LEFT_X1 + BTN_SIZE)
#define BTN_LEFT_Y2             (BTN_LEFT_Y1 + BTN_SIZE)
#define BTN_DOWN_X2             (BTN_DOWN_X1 + BTN_SIZE)
#define BTN_DOWN_Y2             (BTN_DOWN_Y1 + BTN_SIZE)
#define BTN_RIGHT_X2            (BTN_RIGHT_X1 + BTN_SIZE)
#define BTN_RIGHT_Y2            (BTN_RIGHT_Y1 + BTN_SIZE)

/* Functions */
void game_init(void);
void game_process(void);

#endif /* INC_BUTTON_H_ */
