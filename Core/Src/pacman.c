/*
 * pacman.c
 */

/* Includes ------------------------------------------------------------------*/
#include "pacman.h"
#include "button.h"
#include "lcd.h"
#include "led_7seg.h"
#include "touch.h"

#include <stdlib.h>
#include <time.h>
#include "string.h"
#include <stdio.h>

/* Enums ---------------------------------------------------------------------*/

typedef enum DIRECTION {
	UP, DOWN, LEFT, RIGHT, STOP
} E_DIRECTION;

/* Struct --------------------------------------------------------------------*/
typedef struct CELL {
	uint8_t is_pac_dot;
} S_CELL;

typedef struct MAZE {
	S_CELL cells[MAZE_ROW_N][MAZE_COLUMN_N];
} S_MAZE;

typedef struct GHOST {
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	E_DIRECTION direction;
} S_GHOST;

typedef struct PACMAN {
	uint8_t i, j;
	uint8_t i_pre, j_pre;
	E_DIRECTION direction;
	int score;
} S_PACMAN;

/* Private Objects -----------------------------------------------------------*/
// Pac-Man object
S_PACMAN pacman;
void pacman_draw(uint8_t i, uint8_t j, uint16_t color);
void pacman_direction_process(void);
void pacman_moving_process(void);

// Ghost object
S_GHOST ghost;
void ghost_draw(uint8_t i, uint8_t j, uint16_t color);
void ghost_direction_process(void);
void ghost_moving_process(void);

// Maze object
S_MAZE maze;
void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color);

// Game Engine object
void game_draw(void);
void game_handler(void);

/* Declare Private Support Functions -----------------------------------------*/
uint8_t is_button_up(void);
uint8_t is_button_down(void);
uint8_t is_button_left(void);
uint8_t is_button_right(void);
void lcd_update_score(int);
void lcd_draw_control_button();

/* Public Functions ----------------------------------------------------------*/
/**
 * @brief  	Init Pac-Man game
 * @param  	None
 * @note  	Call when you want to init game
 * @retval 	None
 */
void game_init(void) {
	/*
	 * DONE (can be modified)
	 *
	 * 1. Draw a frame for the maze
	 */
	lcd_clear(BACKGROUND_COLOR);
	lcd_draw_rectangle(MAZE_LEFT_BORDER, MAZE_TOP_BORDER, MAZE_RIGHT_BORDER, MAZE_BOTTOM_BORDER, BLACK);
	//lcd_show_string(20, 20, "PAC-MAN", BLACK, BACKGROUND_COLOR, 16, 0);
	//lcd_show_string(20, 250, "Score: ", BLACK, BACKGROUND_COLOR, 16, 0);
	lcd_draw_control_button();

	/*
	 * TO DO
	 *
	 * 2. When the game starts, all tiles in the 10x10 maze will have one pac dot available, except Pac-man's tile.
	 * - Firstly, you have to assign suitable values to maze.cells[][].is_pac_dot.
	 * - Then, draw all pac dots on the maze.
	 */
	for (int i = 0; i < MAZE_ROW_N; i++)
	{
		for (int j = 0; j < MAZE_COLUMN_N; j++)
		{
			maze.cells[i][j].is_pac_dot = 1;
			pac_dot_draw(i, j, PAC_DOTS_COLOR);
		}
	}

	/*
	 * TO DO
	 *
	 * 3. Init Pac-Man object.
	 * - Firstly, you have to initialize default values for the pacman object.
	 * - Then, draw Pac-Man in the first position.
	 * - Remember that reset maze.cells[][] at pacman's location.
	 */
	 pacman.i = 0;
	 pacman.j = 0;
	 pacman.i_pre = 0;
	 pacman.j_pre = 0;
	 pacman.direction = STOP;
	 pacman.score = 0;

	 // Clear pac_dot at cell[0][0];
	 maze.cells[pacman.i][pacman.j].is_pac_dot = 0;
	 pac_dot_draw(pacman.i, pacman.j, BACKGROUND_COLOR);

	 // Draw pacman
	 pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);

	 // Reset score
	 led_7seg_set_digit(0, 0, 0);
	 led_7seg_set_digit(0, 1, 0);
	 led_7seg_set_digit(0, 2, 0);
	 led_7seg_set_digit(0, 3, 0);
	 lcd_update_score(pacman.score);

	/*
	 * TO DO
	 *
	 * 4. Init Ghost object.
	 * - Firstly, you have to initialize default values for the ghost object.
	 * - Then, draw ghost in the first position.
	 */
	 ghost.i = MAZE_ROW_N - 1;
	 ghost.j = MAZE_COLUMN_N - 1;
	 ghost.i_pre = ghost.i;
	 ghost.j_pre = ghost.j;
	 ghost.direction = STOP;
	 ghost_draw(ghost.i, ghost.j, GHOST_COLOR);
}

/**
 * @brief  	Process game
 * @param  	None
 * @note  	Call in loop (main)
 * @retval 	None
 */
void game_process(void) {
	static uint8_t counter_game = 0;
	counter_game = (counter_game + 1) % 5; // Game tick = 5 * 50ms = 250ms

	pacman_direction_process(); // Put this function here to read buttons. (every 50ms)

	if (counter_game == 0) {
		pacman_moving_process();
		ghost_direction_process();
		ghost_moving_process();

		game_handler();
		game_draw();
	}
}

/* Private Functions ---------------------------------------------------------*/
void game_draw(void) {
	/*
	 * TO DO
	 *
	 * Draw Pac-Man, Ghost, and Pac Dots.
	 *
	 * Hint: Remember to delete the object in the previous position, before drawing the new one.
	 */
	pacman_draw(pacman.i_pre, pacman.j_pre, BACKGROUND_COLOR);
	ghost_draw(ghost.i_pre, ghost.j_pre, BACKGROUND_COLOR);

	// Re-draw pac_dot if ghost goes through
	if (maze.cells[ghost.i_pre][ghost.j_pre].is_pac_dot == 1)
	{
		pac_dot_draw(ghost.i_pre, ghost.j_pre, PAC_DOTS_COLOR);
	}

	pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);
	ghost_draw(ghost.i, ghost.j, GHOST_COLOR);
}

void game_handler(void) {
	/*
	 * TO DO
	 *
	 * 1. Check the loss condition, show something, and restart the game.
	 * 2. Check the win condition, show something, and restart the game.
	 * 3. Check if Pac-Man has won any dots or not, then update the score.
	 */
	// Check the loss condition
	if (pacman.i == ghost.i && pacman.j == ghost.j)
	{
		lcd_show_string(70, 100, "GAME OVER", RED, BACKGROUND_COLOR, 24, 0);
		HAL_Delay(2000);
		game_init(); // Start over
		return;
	}

	// Check the score condition
	if (maze.cells[pacman.i][pacman.j].is_pac_dot == 1)
	{
		maze.cells[pacman.i][pacman.j].is_pac_dot = 0;
		pacman.score++;

		// Display score
		led_7seg_set_digit(((pacman.score / 1000) % 10), 0, 0);
		led_7seg_set_digit(((pacman.score / 100) % 10), 1, 0);
		led_7seg_set_digit(((pacman.score / 10) % 10), 2, 0);
		led_7seg_set_digit((pacman.score % 10), 3, 0);

		// Update score on lcd
		lcd_update_score(pacman.score);
	}

	// Check the win condition
	uint8_t dots_remaining = 0;
	for (int i = 0; i < MAZE_ROW_N; i++)
	{
		for (int j = 0; j < MAZE_COLUMN_N; j++)
		{
			if (maze.cells[i][j].is_pac_dot == 1)
			{
				dots_remaining = 1;
				break;
			}
		}
		if (dots_remaining == 1)
			break;
	}

	if (dots_remaining == 0)
	{
		lcd_show_string(80, 100, "YOU WIN!", GREEN, BACKGROUND_COLOR, 24, 0);
		HAL_Delay(2000);
		game_init(); // Start over
		return;
	}
}

void pacman_direction_process(void) {
	/*
	 * TO DO
	 *
	 * Let user use button to control Pac-Man.
	 */
	if (is_button_up())
		pacman.direction = UP;
	else if (is_button_down())
		pacman.direction = DOWN;
	else if (is_button_left())
		pacman.direction = LEFT;
	else if (is_button_right())
		pacman.direction = RIGHT;
}

void pacman_moving_process(void) {
	/*
	 * TO DO
	 *
	 * Update Pac-Man's current and previous position based on current direction.
	 *
	 */
	// Update previous position
	pacman.i_pre = pacman.i;
	pacman.j_pre = pacman.j;

	switch (pacman.direction) {
	case UP:
		if (pacman.i <= 0)
			pacman.direction = STOP;
		else
			pacman.i--;
		break;

	case DOWN:
		if (pacman.i >= MAZE_ROW_N - 1)
			pacman.direction = STOP;
		else
			pacman.i++;
		break;

	case LEFT:
		if (pacman.j <= 0)
			pacman.direction = STOP;
		else
			pacman.j--;
		break;

	case RIGHT:
		if (pacman.j >= MAZE_COLUMN_N - 1)
			pacman.direction = STOP;
		else
			pacman.j++;
		break;

	case STOP:
		// pacman.direction = STOP;
		break;

	default:
		pacman.direction = STOP;
		break;
	}
}

void ghost_direction_process(void) {
	/*
	 * TO DO
	 *
	 * Make Ghost move randomly.
	 * Hint: Change direction randomly.
	 */
	static E_DIRECTION new_direction;
	new_direction = (E_DIRECTION) (rand() % 4); // Random 0-3 in enum

	switch (ghost.direction) {
	case UP:

		break;

	case DOWN:

		break;

	case LEFT:

		break;

	case RIGHT:

		break;

	default:
		break;
	}

	ghost.direction = new_direction;
}

void ghost_moving_process(void) {
	/*
	 * TO DO
	 *
	 * Update Ghost's current and previous position based on current direction.
	 */
	ghost.i_pre = ghost.i;
	ghost.j_pre = ghost.j;

	switch (ghost.direction) {
	case UP:
		if (ghost.i <= 0)
			ghost.direction = STOP;
		else
			ghost.i--;
		break;

	case DOWN:
		if (ghost.i >= MAZE_ROW_N - 1)
			ghost.direction = STOP;
		else
			ghost.i++;
		break;

	case LEFT:
		if (ghost.j <= 0)
			ghost.direction = STOP;
		else
			ghost.j--;
		break;

	case RIGHT:
		if (ghost.j >= MAZE_COLUMN_N - 1)
			ghost.direction = STOP;
		else
			ghost.j++;
		break;

	case STOP:
		ghost.direction = (E_DIRECTION) (rand() % 4);
		break;

	default:
		ghost.direction = STOP;
		break;
	}
}

void pac_dot_draw(uint8_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	// Center coordinates
	uint16_t x_center = MAZE_LEFT_BORDER + (j * MAZE_CELL_WIDTH) + (MAZE_CELL_WIDTH / 2);
	uint16_t y_center = MAZE_TOP_BORDER + (i * MAZE_CELL_HEIGHT) + (MAZE_CELL_HEIGHT / 2);

	if (color == BACKGROUND_COLOR)
	{
		// Draw square on
		uint16_t x1 = MAZE_LEFT_BORDER + j * MAZE_CELL_WIDTH + 1;
		uint16_t y1 = MAZE_TOP_BORDER + i * MAZE_CELL_HEIGHT + 1;
		lcd_fill(x1, y1, x1 + MAZE_CELL_WIDTH - 2, y1 + MAZE_CELL_HEIGHT - 2, BACKGROUND_COLOR);
	}
	else
	{
		// Draw dot (r = 3)
		lcd_draw_circle(x_center, y_center, color, 3, 1);
	}
}

void pacman_draw(uint8_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	uint16_t x_center = MAZE_LEFT_BORDER + (j * MAZE_CELL_WIDTH) + (MAZE_CELL_WIDTH / 2);
	uint16_t y_center = MAZE_TOP_BORDER + (i * MAZE_CELL_HEIGHT) + (MAZE_CELL_HEIGHT / 2);

	if (color == BACKGROUND_COLOR)
	{
		uint16_t x1 = MAZE_LEFT_BORDER + j * MAZE_CELL_WIDTH + 1;
		uint16_t y1 = MAZE_TOP_BORDER + i * MAZE_CELL_HEIGHT + 1;
		lcd_fill(x1, y1, x1 + MAZE_CELL_WIDTH - 2, y1 + MAZE_CELL_HEIGHT - 2, BACKGROUND_COLOR);
	}
	else
	{
		// Draw pacman in circle (r = 8)
		lcd_draw_circle(x_center, y_center, color, 8, 1);
	}
}

void ghost_draw(uint8_t i, uint8_t j, uint16_t color) {
	/*
	 * TO DO
	 *
	 * Draw whatever you like
	 */
	uint16_t x1 = MAZE_LEFT_BORDER + j * MAZE_CELL_WIDTH + 2;
	uint16_t y1 = MAZE_TOP_BORDER + i * MAZE_CELL_HEIGHT + 2;
	uint16_t x2 = x1 + MAZE_CELL_WIDTH - 4;
	uint16_t y2 = y1 + MAZE_CELL_HEIGHT - 4;

	// Draw ghost in square
	lcd_fill(x1, y1, x2, y2, color);
}

uint8_t is_button_up(void) {
	/*
	 * TO DO
	 */
	if (button_count[1] > 0)
		return 1;
	return 0;
}

uint8_t is_button_down(void) {
	/*
	 * TO DO
	 */
	if (button_count[5] > 0)
		return 1;
	return 0;
}

uint8_t is_button_left(void) {
	/*
	 * TO DO
	 */
	if (button_count[4] > 0)
		return 1;
	return 0;
}

uint8_t is_button_right(void) {
	/*
	 * TO DO
	 */
	if (button_count[6] > 0)
		return 1;
	return 0;
}

void lcd_update_score(int score)
{
	char text_buffer[20];
	uint8_t font_height = 16;
	uint8_t font_width = font_height / 2;
	uint16_t text_width;
	uint16_t start_x;
	uint16_t start_y = 10;

	sprintf(text_buffer, "Score: %d", score);

	text_width = strlen(text_buffer) * font_width;
	if (text_width < lcddev.width)
		start_x = (lcddev.width - text_width) / 2;
	else
		start_x = 0;

	lcd_fill(0, start_y, lcddev.width - 1, start_y + font_height, BACKGROUND_COLOR);
	lcd_show_string(start_x, start_y, text_buffer, BLACK, BACKGROUND_COLOR, font_height, 0);
}

void lcd_draw_control_button()
{
	lcd_draw_rectangle(75, lcddev.height - 10 - 30, 105, lcddev.height - 10, BLACK); // LEFT
	lcd_draw_rectangle(105, lcddev.height - 10 - 30, 135, lcddev.height - 10, BLACK); // BOTTOM
	lcd_draw_rectangle(135, lcddev.height - 10 - 30, 165, lcddev.height - 10, BLACK); // RIGHT
	lcd_draw_rectangle(105, lcddev.height - 10 - 30 - 30, 135, lcddev.height - 10 - 30, BLACK); // UP
	//lcd_show_string_center(0, 25, "^", BLACK, BACKGROUND_COLOR, 24, 0);
}
