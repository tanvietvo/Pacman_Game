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
	uint8_t is_wall;
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
void game_pause_process(void);

static int remaining_dots = 0;
// 4 hướng (bước 2 ô để chừa 1 ô tường giữa các “phòng”)
static const int di[4] = {-2, 2, 0, 0};
static const int dj[4] = {0, 0, -2, 2};

static inline int in_bounds(int i, int j)
{
	return (i >= 0 && i < MAZE_ROW_N && j >= 0 && j < MAZE_COLUMN_N);
}

/* Declare Private Support Functions -----------------------------------------*/
uint8_t is_button_up(void);
uint8_t is_button_down(void);
uint8_t is_button_left(void);
uint8_t is_button_right(void);

void lcd_update_score(int);
void lcd_draw_control_button(void);
void lcd_draw_pause_button(void);
void lcd_draw_home_button(void);

void maze_generate_random(void);
static void maze_scatter_dots_and_place_spawns(void);
static inline E_DIRECTION opposite(E_DIRECTION);

void home_screen_process(void);

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

	lcd_draw_control_button();
	lcd_draw_pause_button();
	lcd_draw_home_button();

	/*
	 * TO DO
	 *
	 * 2. When the game starts, all tiles in the 10x10 maze will have one pac dot available, except Pac-man's tile.
	 * - Firstly, you have to assign suitable values to maze.cells[][].is_pac_dot.
	 * - Then, draw all pac dots on the maze.
	 */
	maze_generate_random();
	maze_scatter_dots_and_place_spawns(); // rải dot + đặt spawn + cập nhật remaining_dots
	// VẼ: tường & dot
	for (int i = 0; i < MAZE_ROW_N; ++i)
	{
		for (int j = 0; j < MAZE_COLUMN_N; ++j)
		{
			uint16_t x1 = MAZE_LEFT_BORDER + j * MAZE_CELL_WIDTH;
			uint16_t y1 = MAZE_TOP_BORDER + i * MAZE_CELL_HEIGHT;
			uint16_t x2 = x1 + MAZE_CELL_WIDTH - 1;
			uint16_t y2 = y1 + MAZE_CELL_HEIGHT - 1;

			if (maze.cells[i][j].is_wall)
			{
				lcd_fill(x1 + 1, y1 + 1, x2 - 1, y2 - 1, BLACK);
			}
			else
			{
				lcd_fill(x1 + 1, y1 + 1, x2 - 1, y2 - 1, BACKGROUND_COLOR);
				if (maze.cells[i][j].is_pac_dot)
				{
					pac_dot_draw(i, j, PAC_DOTS_COLOR);
				}
			}
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
	pacman.direction = STOP;
	pacman.score = 0;
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

	uint16_t x, y;
	if (touch_get_calibrated_xy(&x, &y))
	{
		if ((x >= BTN_HOME_X1 && x <= BTN_HOME_X2) && (y >= BTN_HOME_Y1 && y <= BTN_HOME_Y2))
		{
			lcd_clear(BACKGROUND_COLOR);
			game_set_state(STATE_HOME);
			home_screen_init();
			return;
		}

		if ((x >= BTN_PAUSE_X1 && x <= BTN_PAUSE_X2) && (y >= BTN_PAUSE_Y1 && y <= BTN_PAUSE_Y2))
		{
			lcd_dim_area(MAZE_LEFT_BORDER, MAZE_TOP_BORDER, MAZE_RIGHT_BORDER, MAZE_BOTTOM_BORDER);
			char *pause_string = "Tap to continue...";
			uint8_t pause_string_font_size = 16;
			uint8_t pause_string_font_width = pause_string_font_size / 2;
			uint16_t pause_string_width = strlen(pause_string) * pause_string_font_width;

			uint16_t maze_width = MAZE_RIGHT_BORDER - MAZE_LEFT_BORDER;
			uint16_t pause_string_x = MAZE_LEFT_BORDER + (maze_width - pause_string_width) / 2;
			uint16_t maze_height = MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER;
			uint16_t pause_string_y = MAZE_TOP_BORDER + (maze_height - pause_string_font_size) / 2;

			lcd_show_string(pause_string_x, pause_string_y, pause_string, RED, WHITE, pause_string_font_size, 0);
			game_set_state(STATE_GAME_PAUSED);
			return;
		}
	}

	if (counter_game == 0) {
		pacman_moving_process();
		ghost_direction_process();
		ghost_moving_process();

		game_handler();
		game_draw();
	}
//	// ---- DEBUG to find min/max of raw(x, y) ----
//	uint16_t raw_x, raw_y;
//	char buffer[30];
//
//	if (touch_read_raw_xy(&raw_x, &raw_y)) {
//		// Nếu đang chạm, in tọa độ GỐC
//		sprintf(buffer, "RAW X: %-5u", raw_x); // Dùng %-5u để căn lề
//		lcd_show_string(10, 50, buffer, BLACK, BACKGROUND_COLOR, 16, 0);
//
//		sprintf(buffer, "RAW Y: %-5u", raw_y); // Dùng %-5u để căn lề
//		lcd_show_string(10, 70, buffer, BLACK, BACKGROUND_COLOR, 16, 0);
//	} else {
//		// Nếu không chạm
//		// Xóa dòng X (y=50)
//		lcd_show_string(10, 50, "NOT TOUCHED ", BLACK, BACKGROUND_COLOR, 16, 0);
//
//		// **THÊM DÒNG NÀY ĐỂ XÓA DÒNG Y (y=70)**
//		// Vẽ một chuỗi rỗng (dấu cách) để đè lên
//		lcd_show_string(10, 70, "            ", BLACK, BACKGROUND_COLOR, 16, 0);
//	}
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
	int same_tile_collision = (pacman.i == ghost.i && pacman.j == ghost.j);
	int crossover_collision = (pacman.i == ghost.i_pre && pacman.j == ghost.j_pre) &&
	                              (pacman.i_pre == ghost.i && pacman.j_pre == ghost.j);
	if (same_tile_collision || crossover_collision)
	{
		char *lose = "GAME OVER!";
		uint8_t lose_font_size = 24;
		uint8_t lose_font_width = lose_font_size / 2;
		uint16_t lose_width = strlen(lose) * lose_font_width;
		uint16_t lose_x = (lcddev.width - lose_width) / 2;
		uint16_t lose_y = (MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / 2 + lose_font_size;
		lcd_show_string(lose_x, lose_y, lose, RED, WHITE, lose_font_size, 0);

		current_state = STATE_GAME_OVER;
		return;
	}

	// Check the score condition
	if (maze.cells[pacman.i][pacman.j].is_pac_dot == 1)
	{
		maze.cells[pacman.i][pacman.j].is_pac_dot = 0;
		pacman.score++;
		remaining_dots--;

		// Display score
		led_7seg_set_digit(((pacman.score / 1000) % 10), 0, 0);
		led_7seg_set_digit(((pacman.score / 100) % 10), 1, 0);
		led_7seg_set_digit(((pacman.score / 10) % 10), 2, 0);
		led_7seg_set_digit((pacman.score % 10), 3, 0);

		// Update score on lcd
		lcd_update_score(pacman.score);
	}

	// Check the win condition
	if (remaining_dots == 0)
	{
		char *win = "YOU WIN!";
		uint8_t win_font_size = 24;
		uint8_t win_font_width = win_font_size / 2;
		uint16_t win_width = strlen(win) * win_font_width;
		uint16_t win_x = (lcddev.width - win_width) / 2;
		uint16_t win_y = (MAZE_BOTTOM_BORDER - MAZE_TOP_BORDER) / 2 + win_font_size;
		lcd_show_string(win_x, win_y, win, RED, WHITE, win_font_size, 0);

		current_state = STATE_GAME_WIN;
		return;
	}
}

void game_loop_tick()
{
	static uint8_t delay_counter = 0;
	switch(current_state)
	{
		case STATE_HOME:
			home_screen_process();
			break;

		case STATE_GAME_INIT:
			game_init();
			current_state = STATE_GAME_PLAYING;
			break;

		case STATE_GAME_PLAYING:
			button_scan();
			game_process();
			break;

		case STATE_GAME_PAUSED:
			game_pause_process();
			break;

		case STATE_GAME_WIN:
			delay_counter++;
			if (delay_counter >= 40)
			{
				game_set_state(STATE_HOME);
				delay_counter = 0;
			}
			break;

		case STATE_GAME_OVER:
			delay_counter++;
			if (delay_counter >= 40)
			{
				game_set_state(STATE_GAME_INIT);
				delay_counter = 0;
			}
			break;
	}
}

void game_set_state(E_GAME_STATE new_state)
{
	current_state = new_state;
}

void game_pause_process()
{
	uint16_t x, y;
	if (touch_get_calibrated_xy(&x, &y))
	{
		lcd_fill(MAZE_LEFT_BORDER + 1, MAZE_TOP_BORDER + 1, MAZE_RIGHT_BORDER - 1, MAZE_BOTTOM_BORDER - 1, BACKGROUND_COLOR);

		for (int i = 0; i < MAZE_ROW_N; ++i)
		{
			for (int j = 0; j < MAZE_COLUMN_N; ++j)
			{
				uint16_t x1 = MAZE_LEFT_BORDER + j * MAZE_CELL_WIDTH;
				uint16_t y1 = MAZE_TOP_BORDER + i * MAZE_CELL_HEIGHT;
				uint16_t x2 = x1 + MAZE_CELL_WIDTH - 1;
				uint16_t y2 = y1 + MAZE_CELL_HEIGHT - 1;

				if (maze.cells[i][j].is_wall)
				{
					lcd_fill(x1 + 1, y1 + 1, x2 - 1, y2 - 1, BLACK);
				}
				else if (maze.cells[i][j].is_pac_dot)
				{
					pac_dot_draw(i, j, PAC_DOTS_COLOR);
				}
			}
		}

		pacman_draw(pacman.i, pacman.j, PACMAN_COLOR);
		ghost_draw(ghost.i, ghost.j, GHOST_COLOR);

		game_set_state(STATE_GAME_PLAYING);
	}
}

void pacman_direction_process(void) {
	/*
	 * TO DO
	 *
	 * Let user use button to control Pac-Man.
	 */
	uint16_t x, y;
	if (touch_get_calibrated_xy(&x, &y))
	{
		// Check whether touch inside of the btn
		if ((x >= BTN_UP_X1 && x <= BTN_UP_X2) && (y >= BTN_UP_Y1 && y <= BTN_UP_Y2))
			pacman.direction = UP;
		else if ((x >= BTN_DOWN_X1 && x <= BTN_DOWN_X2) && (y >= BTN_DOWN_Y1 && y <= BTN_DOWN_Y2))
			pacman.direction = DOWN;
		else if ((x >= BTN_LEFT_X1 && x <= BTN_LEFT_X2) && (y >= BTN_LEFT_Y1 && y <= BTN_LEFT_Y2))
			pacman.direction = LEFT;
		else if ((x >= BTN_RIGHT_X1 && x <= BTN_RIGHT_X2) && (y >= BTN_RIGHT_Y1 && y <= BTN_RIGHT_Y2))
			pacman.direction = RIGHT;
	}

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
	// Nếu đang STOP thì thôi
	if (pacman.direction == STOP)
		return;

	int ni = pacman.i;
	int nj = pacman.j;

	// 1) Tính ô đích theo hướng hiện tại
	switch (pacman.direction)
	{
	case UP:
		ni = pacman.i - 1;
		break;
	case DOWN:
		ni = pacman.i + 1;
		break;
	case LEFT:
		nj = pacman.j - 1;
		break;
	case RIGHT:
		nj = pacman.j + 1;
		break;
	default:
		pacman.direction = STOP;
		return;
	}

	// 2) Biên: nếu vượt lưới thì dừng
	if (ni < 0 || ni >= MAZE_ROW_N || nj < 0 || nj >= MAZE_COLUMN_N)
	{
		//pacman.direction = STOP;
		return;
	}

	// 3) Tường: nếu ô đích là tường thì dừng (không đổi vị trí)
	if (maze.cells[ni][nj].is_wall)
	{
		//pacman.direction = STOP;
		return;
	}

	// 4) Hợp lệ: commit di chuyển
	pacman.i_pre = pacman.i;
	pacman.j_pre = pacman.j;
	pacman.i = ni;
	pacman.j = nj;
}

void ghost_direction_process(void) {
	/*
	 * TO DO
	 *
	 * Make Ghost move randomly.
	 * Hint: Change direction randomly.
	 */
	// Nếu STOP: chọn 1 hướng hợp lệ để khởi động
	if (ghost.direction == STOP)
	{
		E_DIRECTION cand[4];
		int n = 0;
		if (ghost.i > 0 && !maze.cells[ghost.i - 1][ghost.j].is_wall)
			cand[n++] = UP;
		if (ghost.i < MAZE_ROW_N - 1 && !maze.cells[ghost.i + 1][ghost.j].is_wall)
			cand[n++] = DOWN;
		if (ghost.j > 0 && !maze.cells[ghost.i][ghost.j - 1].is_wall)
			cand[n++] = LEFT;
		if (ghost.j < MAZE_COLUMN_N - 1 && !maze.cells[ghost.i][ghost.j + 1].is_wall)
			cand[n++] = RIGHT;
		ghost.direction = (n > 0) ? cand[rand() % n] : STOP;
		return;
	}

	// Thỉnh thoảng rẽ ở ngã ba/ tư
	int branches = 0;
	if (ghost.i > 0 && !maze.cells[ghost.i - 1][ghost.j].is_wall && opposite(ghost.direction) != UP)
		branches++;
	if (ghost.i < MAZE_ROW_N - 1 && !maze.cells[ghost.i + 1][ghost.j].is_wall && opposite(ghost.direction) != DOWN)
		branches++;
	if (ghost.j > 0 && !maze.cells[ghost.i][ghost.j - 1].is_wall && opposite(ghost.direction) != LEFT)
		branches++;
	if (ghost.j < MAZE_COLUMN_N - 1 && !maze.cells[ghost.i][ghost.j + 1].is_wall && opposite(ghost.direction) != RIGHT)
		branches++;
	int want_turn = (branches >= 1 && (rand() % 100) < 20); // 20% rẽ

	// Nếu phía trước là tường/ra biên hoặc muốn rẽ → chọn hướng hợp lệ, tránh U-turn nếu có thể
	int ni = ghost.i, nj = ghost.j;
	switch (ghost.direction)
	{
	case UP:
		ni--;
		break;
	case DOWN:
		ni++;
		break;
	case LEFT:
		nj--;
		break;
	case RIGHT:
		nj++;
		break;
	default:
		break;
	}
	int forward_blocked = (ni < 0 || ni >= MAZE_ROW_N || nj < 0 || nj >= MAZE_COLUMN_N || maze.cells[ni][nj].is_wall);

	if (forward_blocked || want_turn)
	{
		E_DIRECTION cand[4];
		int n = 0, opp = opposite(ghost.direction);
		if (ghost.i > 0 && !maze.cells[ghost.i - 1][ghost.j].is_wall && UP != opp)
			cand[n++] = UP;
		if (ghost.i < MAZE_ROW_N - 1 && !maze.cells[ghost.i + 1][ghost.j].is_wall && DOWN != opp)
			cand[n++] = DOWN;
		if (ghost.j > 0 && !maze.cells[ghost.i][ghost.j - 1].is_wall && LEFT != opp)
			cand[n++] = LEFT;
		if (ghost.j < MAZE_COLUMN_N - 1 && !maze.cells[ghost.i][ghost.j + 1].is_wall && RIGHT != opp)
			cand[n++] = RIGHT;
		if (n == 0)
		{ // bí thì quay đầu
			if (ghost.i > 0 && !maze.cells[ghost.i - 1][ghost.j].is_wall)
				cand[n++] = UP;
			if (ghost.i < MAZE_ROW_N - 1 && !maze.cells[ghost.i + 1][ghost.j].is_wall)
				cand[n++] = DOWN;
			if (ghost.j > 0 && !maze.cells[ghost.i][ghost.j - 1].is_wall)
				cand[n++] = LEFT;
			if (ghost.j < MAZE_COLUMN_N - 1 && !maze.cells[ghost.i][ghost.j + 1].is_wall)
				cand[n++] = RIGHT;
		}
		ghost.direction = (n > 0) ? cand[rand() % n] : STOP;
	}
}

void ghost_moving_process(void) {
	/*
	 * TO DO
	 *
	 * Update Ghost's current and previous position based on current direction.
	 */
	ghost.i_pre = ghost.i;
	ghost.j_pre = ghost.j;

	int ni = ghost.i, nj = ghost.j;
	switch (ghost.direction)
	{
	case UP:
		ni--;
		break;
	case DOWN:
		ni++;
		break;
	case LEFT:
		nj--;
		break;
	case RIGHT:
		nj++;
		break;
	default:
		return;
	}

	if (ni < 0 || ni >= MAZE_ROW_N || nj < 0 || nj >= MAZE_COLUMN_N || maze.cells[ni][nj].is_wall)
	{
		ghost.direction = STOP; // sẽ chọn hướng mới ở tick sau
		return;
	}

	ghost.i = ni;
	ghost.j = nj;
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
		// Draw pacman in circle (r = 3)
		lcd_draw_circle(x_center, y_center, color, 3, 1);
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
	{
		button_count[1]--;
		return 1;
	}
	return 0;
}

uint8_t is_button_down(void) {
	/*
	 * TO DO
	 */
	if (button_count[5] > 0)
	{
		button_count[5]--;
		return 1;
	}
	return 0;
}

uint8_t is_button_left(void) {
	/*
	 * TO DO
	 */
	if (button_count[4] > 0)
	{
		button_count[4]--;
		return 1;
	}
	return 0;
}

uint8_t is_button_right(void) {
	/*
	 * TO DO
	 */
	if (button_count[6] > 0)
	{
		button_count[6]--;
		return 1;
	}
	return 0;
}

void lcd_update_score(int score)
{
	char text_buffer[20];
	uint8_t font_size = 16;
	uint8_t font_width = font_size / 2;
	uint16_t text_width;
	uint16_t start_x;
	uint16_t start_y = 10;

	sprintf(text_buffer, "Score: %d", score);

	text_width = strlen(text_buffer) * font_width;
	if (text_width < lcddev.width)
		start_x = (lcddev.width - text_width) / 2;
	else
		start_x = 0;

	uint16_t clear_x1 = BTN_HOME_X2 + 2;
	uint16_t clear_x2 = BTN_PAUSE_X1 - 2;

	lcd_fill(clear_x1, start_y, clear_x2, start_y + font_size, BACKGROUND_COLOR);
	lcd_show_string(start_x, start_y, text_buffer, BLACK, BACKGROUND_COLOR, font_size, 0);
}

void lcd_draw_control_button()
{
	uint8_t  font_size = 12;

	lcd_draw_button_with_text(BTN_UP_X1, BTN_UP_Y1, BTN_SIZE, BTN_SIZE, "UP", font_size, GRAY, WHITE, GRAY);
	lcd_draw_button_with_text(BTN_LEFT_X1, BTN_LEFT_Y1, BTN_SIZE, BTN_SIZE, "LEFT", font_size, GRAY, WHITE, GRAY);
	lcd_draw_button_with_text(BTN_DOWN_X1, BTN_DOWN_Y1, BTN_SIZE, BTN_SIZE, "DOWN", font_size, GRAY, WHITE, GRAY);
	lcd_draw_button_with_text(BTN_RIGHT_X1, BTN_RIGHT_Y1, BTN_SIZE, BTN_SIZE, "RIGHT", font_size, GRAY, WHITE, GRAY);
}

void lcd_draw_pause_button()
{
//	lcd_fill(BTN_PAUSE_X1, BTN_PAUSE_Y1, BTN_PAUSE_X2, BTN_PAUSE_Y2, BACKGROUND_COLOR);
//	lcd_draw_rectangle(BTN_PAUSE_X1, BTN_PAUSE_Y1, BTN_PAUSE_X2, BTN_PAUSE_Y2, BLACK);
//
//	uint16_t pad_x = 12;
//	uint16_t pad_y = 8;
//	uint16_t bar_width = 6;
//	uint16_t bar_space = 6;
//
//	uint16_t x_start = BTN_PAUSE_X1 + pad_x;
//	uint16_t y_start = BTN_PAUSE_Y1 + pad_y;
//	uint16_t y_end = BTN_PAUSE_Y2 - pad_y;
//
//	lcd_fill(x_start, y_start, x_start + bar_width, y_end, BLACK);
//	lcd_fill(x_start + bar_width + bar_space, y_start, x_start + bar_width * 2 + bar_space, y_end, BLACK);

	lcd_draw_button_with_text(BTN_PAUSE_X1, BTN_PAUSE_Y1, BTN_PAUSE_WIDTH, BTN_PAUSE_HEIGHT, "PAUSE", BTN_PAUSE_FONT_SIZE, GRAY, WHITE, GRAY);
}

void lcd_draw_home_button()
{
	lcd_draw_button_with_text(BTN_HOME_X1, BTN_HOME_Y1, BTN_HOME_WIDTH, BTN_HOME_HEIGHT, "HOME", BTN_HOME_FONT_SIZE, GRAY, WHITE, GRAY);
}

void maze_generate_random(void)
{
	// 1) fill toàn tường
	for (int i = 0; i < MAZE_ROW_N; ++i)
	{
		for (int j = 0; j < MAZE_COLUMN_N; ++j)
		{
			maze.cells[i][j].is_wall = 1;
			maze.cells[i][j].is_pac_dot = 0;
		}
	}

	// 2) chọn start ở chỉ số lẻ (ô “phòng”), để đảm bảo lưới có tường xen kẽ
	int si = (rand() % (MAZE_ROW_N / 2)) * 2 + 1;
	int sj = (rand() % (MAZE_COLUMN_N / 2)) * 2 + 1;
	if (si >= MAZE_ROW_N)
		si = MAZE_ROW_N - 2;
	if (sj >= MAZE_COLUMN_N)
		sj = MAZE_COLUMN_N - 2;

	// 3) DFS ngẫu nhiên với stack nhỏ
	typedef struct
	{
		int i, j;
	} Node;
	Node stack[MAZE_ROW_N * MAZE_COLUMN_N];
	int top = 0;

	maze.cells[si][sj].is_wall = 0;
	stack[top++] = (Node){si, sj};

	while (top > 0)
	{
		Node cur = stack[top - 1];

		// liệt kê các hướng hợp lệ (đi 2 ô, đích chưa khắc)
		int dirs[4] = {0, 1, 2, 3};
		// xáo trộn 4 hướng
		for (int k = 3; k > 0; --k)
		{
			int r = rand() % (k + 1);
			int t = dirs[k];
			dirs[k] = dirs[r];
			dirs[r] = t;
		}

		int moved = 0;
		for (int k = 0; k < 4; ++k)
		{
			int ni = cur.i + di[dirs[k]];
			int nj = cur.j + dj[dirs[k]];
			if (!in_bounds(ni, nj))
				continue;
			if (maze.cells[ni][nj].is_wall == 0)
				continue; // đã khắc rồi

			// đục bức tường ở giữa
			int wi = cur.i + (di[dirs[k]] / 2);
			int wj = cur.j + (dj[dirs[k]] / 2);
			maze.cells[wi][wj].is_wall = 0;
			// khắc ô đích
			maze.cells[ni][nj].is_wall = 0;

			// tiến tới ô đích
			stack[top++] = (Node){ni, nj};
			moved = 1;
			break;
		}

		if (!moved)
		{
			// không còn hướng đi → backtrack
			--top;
		}
	}
}

static void maze_scatter_dots_and_place_spawns(void)
{
	remaining_dots = 0;

	for (int i = 0; i < MAZE_ROW_N; ++i)
	{
		for (int j = 0; j < MAZE_COLUMN_N; ++j)
		{
			if (!maze.cells[i][j].is_wall)
			{
				maze.cells[i][j].is_pac_dot = 1;
				remaining_dots++;
			}
			else
			{
				maze.cells[i][j].is_pac_dot = 0;
			}
		}
	}

	// đặt Pac-Man ở 1 ô trống (ví dụ góc trên trái khả dụng)
	int pi = 1, pj = 1;
	if (maze.cells[pi][pj].is_wall)
	{
		// tìm ô trống gần nhất
		for (int i = 1; i < MAZE_ROW_N; i += 2)
		{
			for (int j = 1; j < MAZE_COLUMN_N; j += 2)
			{
				if (!maze.cells[i][j].is_wall)
				{
					pi = i;
					pj = j;
					goto FOUND_P;
				}
			}
		}
	}
FOUND_P:
	pacman.i = pacman.i_pre = pi;
	pacman.j = pacman.j_pre = pj;

	// không để dot dưới chân Pac-Man
	if (maze.cells[pi][pj].is_pac_dot)
	{
		maze.cells[pi][pj].is_pac_dot = 0;
		remaining_dots--;
	}

	// đặt Ghost ở một ô trống xa xa (vd. góc dưới phải khả dụng)
	int gi = MAZE_ROW_N - 2, gj = MAZE_COLUMN_N - 2;
	if (maze.cells[gi][gj].is_wall || (gi == pi && gj == pj))
	{
		// tìm ô trống khác
		for (int i = MAZE_ROW_N - 2; i >= 1; i -= 2)
		{
			for (int j = MAZE_COLUMN_N - 2; j >= 1; j -= 2)
			{
				if (!maze.cells[i][j].is_wall && !(i == pi && j == pj))
				{
					gi = i;
					gj = j;
					goto FOUND_G;
				}
			}
		}
	}
FOUND_G:
	ghost.i = ghost.i_pre = gi;
	ghost.j = ghost.j_pre = gj;
	ghost.direction = STOP; // để hàm ghost_direction_process() chọn hướng hợp lệ
}

static inline E_DIRECTION opposite(E_DIRECTION d)
{
	switch (d)
	{
	case UP:
		return DOWN;
	case DOWN:
		return UP;
	case LEFT:
		return RIGHT;
	case RIGHT:
		return LEFT;
	default:
		return STOP;
	}
}

void home_screen_process()
{
	uint16_t x, y;
	if (touch_get_calibrated_xy(&x, &y))
	{
		if ((x >= BTN_PLAY_X1 && x <= BTN_PLAY_X2) && (y >= BTN_PLAY_Y1 && y <= BTN_PLAY_Y2))
			current_state = STATE_GAME_INIT;
	}
}

void home_screen_init()
{
	led_7seg_set_digit(0, 0, 0);
	led_7seg_set_digit(0, 1, 0);
	led_7seg_set_digit(0, 2, 0);
	led_7seg_set_digit(0, 3, 0);

	lcd_clear(BACKGROUND_COLOR);
	char *title = "PAC-MAN";
	uint8_t title_font_size = 24;
	uint8_t title_font_width = title_font_size / 2;
	uint16_t title_width = strlen(title) * title_font_width;
	uint16_t title_x = (lcddev.width - title_width) / 2;
	uint16_t title_y = 100;

	lcd_show_string(title_x, title_y, title, BLACK, BACKGROUND_COLOR, title_font_size, 0);
	lcd_draw_button_with_text(BTN_PLAY_X1, BTN_PLAY_Y1, BTN_PLAY_WIDTH, BTN_PLAY_HEIGHT, "PLAY", BTN_PLAY_FONT_SIZE, GREEN, WHITE, GREEN);
}
