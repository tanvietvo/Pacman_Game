/*
 * pacman.h
 */

#ifndef INC_PACMAN_H_
#define INC_PACMAN_H_

/* Includes */
#include <stdint.h>

/* Constants */
#define MAZE_COLUMN_N      		20
#define MAZE_ROW_N         		18
#define MAZE_CELL_WIDTH    		10
#define MAZE_CELL_HEIGHT        10

#define MAZE_TOP_BORDER			35
#define MAZE_BOTTOM_BORDER 		215 // 20*9+35
#define MAZE_LEFT_BORDER		20
#define MAZE_RIGHT_BORDER		220

#define BACKGROUND_COLOR		WHITE
#define PACMAN_COLOR			BLUE
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

#define BTN_PAUSE_WIDTH         35
#define BTN_PAUSE_HEIGHT        25
#define BTN_PAUSE_X2            (lcddev.width - 5)
#define BTN_PAUSE_Y1 			5
#define BTN_PAUSE_X1 			(BTN_PAUSE_X2 - BTN_PAUSE_WIDTH)
#define BTN_PAUSE_Y2 			(BTN_PAUSE_Y1 + BTN_PAUSE_HEIGHT)
#define BTN_PAUSE_FONT_SIZE     12

#define BTN_HOME_WIDTH          35
#define BTN_HOME_HEIGHT         25
#define BTN_HOME_X1             5
#define BTN_HOME_Y1             5
#define BTN_HOME_X2             (BTN_HOME_X1 + BTN_HOME_WIDTH)
#define BTN_HOME_Y2             (BTN_HOME_Y1 + BTN_HOME_HEIGHT)
#define BTN_HOME_FONT_SIZE      12

#define BTN_PLAY_X1             70
#define BTN_PLAY_Y1             180
#define BTN_PLAY_WIDTH          100
#define BTN_PLAY_HEIGHT         50
#define BTN_PLAY_X2             (BTN_PLAY_X1 + BTN_PLAY_WIDTH)
#define BTN_PLAY_Y2             (BTN_PLAY_Y1 + BTN_PLAY_HEIGHT)
#define BTN_PLAY_FONT_SIZE      24

typedef enum
{
	STATE_HOME,
	STATE_GAME_INIT,
	STATE_GAME_PLAYING,
	STATE_GAME_PAUSED,
	STATE_GAME_WIN,
	STATE_GAME_OVER
} E_GAME_STATE;

extern E_GAME_STATE current_state;

/* Functions */
void game_init(void);
void game_process(void);
void game_loop_tick();
void game_set_state(E_GAME_STATE);

void home_screen_init();

#endif /* INC_PACMAN_H_ */
