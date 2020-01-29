/*
   Application template for Amazfit Bip BipOS
   (C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>

   Game 2048 for Amazfit Bip BipOS
   (C) Claudio Benvenuti  2019 <claudio.benvenuti@gmail.com>

*/

#include <libbip.h>
#include "2048.h"

#define SIZE 4

enum {
  DOWN,
  UP,
  LEFT,
  RIGHT,
  MOVES
};

unsigned long randseed;
unsigned short randint( short max )
{
  randseed = randseed * get_tick_count();
  randseed++;
  return ( ( randseed >> 16 ) * max ) >> 16;
}

void begin(struct game* g, int index_listed)
{
  set_display_state_value (8, 1);
  set_display_state_value (4, 1);
  set_display_state_value (2, 1);
  _srand(get_tick_count());

  struct game placeholder;
  ElfReadSettings(index_listed, g,  0, sizeof(placeholder));

  if (g->record == 0){
    for (short i = 0; i < SIZE; i++)
      for (short y = 0; y < SIZE; y++){
        g->tiles[i][y] = NULL;
        g->undo[i][y] = NULL;
      }
    short i,j;
    i = random_spot(g);
    j = random_spot(g);
    g->tiles[i][j] = random_value(g);
    g->undo[i][j] = g->tiles[i][j];
    g->score = 0;
    g->moves = 0;
    g->record = 0;
    do{
      i=random_spot(g);
      j=random_spot(g);
    }while (g->tiles[i][j]);
    g->tiles[i][j] = random_value(g);
    g->undo[i][j] = g->tiles[i][j];
  }
}

short random_spot()
{
  return randint(SIZE);
}

short random_value()
{
  return (randint(10)) ? 2 : 4;
}

void twist(struct game* g)
{
  struct game g2;
  _memcpy (&g2, g, sizeof(g2));
  for (short i = 0; i < SIZE; ++i)
    for (short j = 0; j < SIZE; ++j)
      g2.tiles[i][j] = g->tiles[j][i];
  _memcpy (g, &g2, sizeof(g2));
}

void flip(struct game* g)
{
  struct game g2;
  _memcpy (&g2, g, sizeof(g2));
  for (short i = 0; i < SIZE; ++i)
    for (short j = 0; j < SIZE; ++j)
      g2.tiles[i][j] = g->tiles[i][SIZE - j - 1];
  _memcpy (g, &g2, sizeof(g2));
}

unsigned int fall_column(short* a, short* b)
{
  short i,j;
  short prev = 0;
  unsigned int score = 0;
  j = 0;
  for (i = 0; i < SIZE; ++i)
    if (a[i]) {
      if (a[i] == prev) {
        b[j - 1] *= 2;
        prev = 0;
        score += b[j - 1];
      } else {
        b[j++] = a[i];
        prev = a[i];
      }
    }
  return score;
}

void fall(struct game* g)
{
  struct game g2;
  _memcpy (&g2, g, sizeof(g2));
  unsigned int score = g->score;
  for (int i = 0; i < SIZE; ++i)
    for (short j = 0; j < SIZE; ++j)
      g2.tiles[i][j] = NULL;
  for (short i = 0; i < SIZE; ++i){
    score += fall_column(g->tiles[i], g2.tiles[i]);
  }
  g2.score = score;
  _memcpy (g, &g2, sizeof(g2));
}

int same(struct game* a, struct game* b)
{
  short i,j;
  for (i = 0; i < SIZE; ++i)
    for (j = 0; j < SIZE; ++j)
      if (a->tiles[i][j] != b->tiles[i][j])
        return 0;
  return 1;
}

int tryfalling(struct game* g)
{
  struct game g2;
  _memcpy (&g2, g, sizeof(g2));
  fall(g);
  if (same(g, &g2))
    return 0;
  return 1;
}

void popup(struct game* g)
{
  short i,j;
  while (1) {
    i = random_spot(g);
    j = random_spot(g);
    if (!g->tiles[i][j]) {
      g->tiles[i][j] = random_value(g);
      return;
    }
  }
}

void move(struct game* g, short way)
{
  short tmp[SIZE][SIZE];
  for (short i = 0; i < SIZE; ++i)
    for (short j = 0; j < SIZE; ++j)
      tmp[i][j] = g->tiles[i][j];

  if (way / 2)
    twist(g);
  if (way % 2)
    flip(g);
  if (tryfalling(g)){
    //create undo
    for (short i = 0; i < SIZE; i++)
      for (short y = 0; y < SIZE; y++)
        g->undo[i][y] = tmp[i][y];
    g->moves = g->moves + 1;
    popup(g);
  }
  if (way % 2)
    flip(g);
  if (way / 2)
    twist(g);
  if (g->score > g->record) {
    g->record = g->score;
  }
}


struct regmenu_ screen_data = {
  55,
  1,
  0,
  dispatch_screen,
  key_press_screen,
  screen_job,
  0,
  show_screen,
  0,
  0
};

int main(int param0, char** argv){
  show_screen((void*) param0);
}

void show_screen (void *param0) {
  struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	pointer to a pointer to screen data
  struct app_data_ *	app_data;					//	pointer to screen data
  if ( (param0 == *app_data_p) && get_var_menu_overlay()){
    app_data = *app_data_p;
    *app_data_p = NULL;
    reg_menu(&screen_data, 0);
    *app_data_p = app_data;
  } else {
    reg_menu(&screen_data, 0);
    *app_data_p = (struct app_data_ *)pvPortMalloc(sizeof(struct app_data_));
    app_data = *app_data_p;
    _memclr(app_data, sizeof(struct app_data_));
    app_data->proc = param0;
    if ( param0 && app_data->proc->ret_f )
      app_data->ret_f = app_data->proc->elf_finish;
    else
      app_data->ret_f = show_watchface;
    struct game g;
    begin(&g, app_data->proc->index_listed);
    app_data->screen = 1;
    _memcpy (&app_data->game, &g, sizeof(g));
  }
  draw_screen(&app_data->game);
}

void key_press_screen(){
  struct app_data_** 	app_data_p = get_ptr_temp_buf_2();
  struct app_data_ *	app_data = *app_data_p;
  struct game *g = &app_data->game;
  struct game placeholder;

  ElfWriteSettings(app_data->proc->index_listed, g,  0, sizeof(placeholder));
  show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);
};

void screen_job(){
}

int dispatch_screen (void *param){
  struct app_data_** 	app_data_p = get_ptr_temp_buf_2();
  struct app_data_ *	app_data = *app_data_p;
  struct gesture_ *gest = param;
  int result = 0;
  struct game *g = &app_data->game;
  switch (gest->gesture){
    case GESTURE_CLICK: {
                          vibrate(1,50,0);
                          switch (app_data->screen){
                            case 1: {
                                      app_data->screen = 2;
                                      draw_score_screen(g->moves, g->score, g->record);
                                      break;
                                    }
                            case 2: {
                                      if ( ( gest->touch_pos_y >143) && ( gest->touch_pos_y <= 176)  && ( gest->touch_pos_x >= 1) &&  ( gest->touch_pos_x <= 176) ){
                                        app_data->screen = 3;
                                        set_bg_color(COLOR_WHITE);
                                        fill_screen_bg();
                                        show_elf_res_by_id(app_data->proc->index_listed, 0, 24, 10);
                                        set_bg_color(COLOR_WHITE);
                                        set_fg_color(COLOR_BLACK);
                                        text_out_center("by Claudio Benvenuti", 88, 145);

                                        repaint_screen_lines(0, 176);
                                      } else if ( ( gest->touch_pos_y >90) && ( gest->touch_pos_y < 133)  && ( gest->touch_pos_x >= 1) &&  ( gest->touch_pos_x <= 100) ){
                                        app_data->screen = 4;
                                        ask_confirmation();
                                      } else if ( ( gest->touch_pos_y >90) && ( gest->touch_pos_y < 133)  && ( gest->touch_pos_x >= 110) &&  ( gest->touch_pos_x <= 176) ){
                                        short tmp;
                                        for (short i = 0; i < SIZE; i++) {
                                          for (short y = 0; y < SIZE; y++) {
                                            tmp = g->tiles[i][y];
                                            g->tiles[i][y] = g->undo[i][y];
                                            g->undo[i][y] = tmp;
                                          }
                                        }
                                        app_data->screen = 1;
                                        draw_board(g);
                                      } else {
                                        app_data->screen = 1;
                                        draw_board(g);
                                      }
                                      break;
                                    }
                            case 3: {
                                      app_data->screen = 1;
                                      draw_board(g);
                                      break;
                                    }
                            case 4: {
                                      if ( ( gest->touch_pos_y > 84) && ( gest->touch_pos_y <= 113)  && ( gest->touch_pos_x >= 20) &&  ( gest->touch_pos_x <= 78) ){
                                        for (short i = 0; i < SIZE; i++)
                                          for (short y = 0; y < SIZE; y++){
                                            g->tiles[i][y] = NULL;
                                            g->undo[i][y] = NULL;
                                          }
                                        short i,j;
                                        i = random_spot(g);
                                        j = random_spot(g);
                                        g->tiles[i][j] = random_value(g);
                                        g->undo[i][j] = g->tiles[i][j];
                                        do{
                                          i=random_spot(g);
                                          j=random_spot(g);
                                        }while (g->tiles[i][j]);
                                        g->tiles[i][j] = random_value(g);
                                        g->undo[i][j] = g->tiles[i][j];
                                        g->moves = 0;
                                        g->score = 0;
                                      }
                                      app_data->screen = 1;
                                      draw_board(g);
                                      break;
                                    }
                          }

                          break;
                        };
    case GESTURE_SWIPE_DOWN: {
                               if (app_data->screen == 1){
                                 move(g, 0);
                               }
                               app_data->screen = 1;
                               draw_board(g);
                               break;
                             };
    case GESTURE_SWIPE_UP: {
                             if (app_data->screen == 1){
                               move(g, 1);
                             }
                             app_data->screen = 1;
                             draw_board(g);
                             break;
                           };
    case GESTURE_SWIPE_LEFT: {
                               if (app_data->screen == 1){
                                 move(g, 2);
                               }
                               app_data->screen = 1;
                               draw_board(g);
                               break;
                             };
    case GESTURE_SWIPE_RIGHT: {
                                if (app_data->screen == 1){
                                  move(g, 3);
                                }
                                app_data->screen = 1;
                                draw_board(g);
                                break;
                              };
    default:{
              vibrate(1,70,0);
              break;
            };

  }
  return result;
};


void draw_board(struct game* g){
  set_bg_color(COLOR_BLACK);
  fill_screen_bg();
  char str[sizeof(int) * 4 + 1];
  short n, offset;
  for (short i = 0; i < SIZE; i++) {
    for (short y = 0; y < SIZE; y++) {
      n = g->tiles[i][SIZE-1-y];
      if (!n) {
        set_bg_color(COLOR_WHITE);
      } else if(n < 4) {
        set_fg_color(COLOR_BLACK);
        set_bg_color(COLOR_YELLOW);
      } else if (n < 32) {
        set_fg_color(COLOR_BLACK);
        set_bg_color(COLOR_AQUA);
      } else if (n < 128) {
        set_fg_color(COLOR_BLACK);
        set_bg_color(COLOR_PURPLE);
      } else if (n < 512) {
        set_fg_color(COLOR_BLACK);
        set_bg_color(COLOR_GREEN);
      } else if (n < 512) {
        set_fg_color(COLOR_BLACK);
        set_bg_color(COLOR_BLUE);
      } else if (n < 1024) {
        set_fg_color(COLOR_WHITE);
        set_bg_color(COLOR_RED);
      } else if (n < 2048) {
        set_fg_color(COLOR_WHITE);
        set_bg_color(COLOR_BLACK);
      } else if (n >= 2048 ) {
        set_fg_color(COLOR_RED);
        set_bg_color(COLOR_WHITE);
      }
      draw_filled_rect_bg(3+(40*i)+(3*i),3+(40*y)+(3*y), 43+(40*i)+(3*i),43+(40*y)+(3*y));
      if (n) {
        _sprintf (str, "%4d", n);
        offset = (n<1000) ? 0 : 5;
        text_out_center(str, 19 + offset +(40*i)+(3*i), 13+(40*y)+(3*y));
      }
    }
  }
  repaint_screen_lines(0, 176);
};

void draw_score_screen(short moves, unsigned int score, unsigned int record){
  char str[sizeof(int) * 4 + 1];
  set_fg_color(COLOR_WHITE);
  set_bg_color(COLOR_BLACK);
  fill_screen_bg();

  set_bg_color(COLOR_BLUE);
  draw_filled_rect_bg(4, 4, 86, 50);
  text_out_center("Score", 45, 6);
  _sprintf (str, "%4d", score);
  text_out_center(str, 45, 28);

  set_bg_color(COLOR_RED);
  draw_filled_rect_bg(90, 4, 172, 50);
  text_out_center("Moves", 131, 6);
  _sprintf (str, "%4d", moves);
  text_out_center(str, 131, 28);

  set_bg_color(COLOR_GREEN);
  set_fg_color(COLOR_BLACK);
  draw_filled_rect_bg(4, 54, 172, 80);
  text_out("RECORD:", 12, 58);
  _sprintf (str, "%4d", record);
  text_out(str, 85, 58);

  set_bg_color(COLOR_YELLOW);
  set_fg_color(COLOR_BLACK);
  draw_button(8, 90, 104, 133);
  set_bg_color(COLOR_YELLOW);
  text_out_center("NEW GAME", 56, 104);

  set_bg_color(COLOR_AQUA);
  set_fg_color(COLOR_BLACK);
  draw_button(114, 90, 168, 133);
  set_bg_color(COLOR_AQUA);
  text_out_center("UNDO", 141, 104);

  set_bg_color(COLOR_WHITE);
  draw_button(30, 143, 146, 169);
  set_bg_color(COLOR_BLACK);
  draw_button(32, 145, 144, 167);
  set_fg_color(COLOR_WHITE);
  text_out_center("CREDITS", 88, 147);

  repaint_screen_lines(0, 176);
}

void ask_confirmation(){
  set_fg_color(COLOR_WHITE);
  set_bg_color(COLOR_BLACK);
  fill_screen_bg();
  text_out_center("Are you sure?", 88, 40);

  set_bg_color(COLOR_GREEN);
  set_fg_color(COLOR_BLACK);
  draw_button(20, 84, 78, 113);
  set_bg_color(COLOR_GREEN);
  text_out_center("Yes" , 49, 89);

  set_bg_color(COLOR_RED);
  set_fg_color(COLOR_BLACK);
  draw_button(98, 84, 156, 113);
  set_bg_color(COLOR_RED);
  text_out_center("No", 127, 89);

  repaint_screen_lines(0, 176);
}

void draw_button(int from_x, int from_y, int to_x, int to_y){
  draw_filled_rect_bg(from_x, from_y, to_x, to_y);

  set_bg_color(COLOR_BLACK);

  draw_horizontal_line(from_y, from_x, from_x+1);
  draw_vertical_line(from_x, from_y, from_y+1);

  draw_horizontal_line(from_y, to_x-1, to_x);
  draw_vertical_line(to_x, from_y, from_y+1);

  draw_horizontal_line(to_y, from_x, from_x+1);
  draw_vertical_line(from_x, to_y-1, to_y);

  draw_horizontal_line(to_y, to_x-1, to_x);
  draw_vertical_line(to_x, to_y-1, to_y);
}

void draw_screen(struct game* g){
  set_bg_color(COLOR_BLACK);
  fill_screen_bg();
  set_graph_callback_to_ram_1();
  load_font();
  draw_board(g);
}


