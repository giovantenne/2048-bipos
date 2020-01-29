/*
   Application template for Amazfit Bip BipOS
   (C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>

   Game 2048 for Amazfit Bip BipOS
   (C) Claudio Benvenuti  2019 <claudio.benvenuti@gmail.com>

*/

#ifndef __APP_2048_H__
#define __APP_2048_H__

#define SIZE 4

struct game {
  short tiles[SIZE][SIZE];
  short undo[SIZE][SIZE];
  short moves;
  unsigned int score;
  unsigned int record;
};

struct app_data_ {
  void* 	ret_f;
  struct game game;
  short screen;
  Elf_proc_* proc;
};

short random_spot();
short random_value();
void print(struct game* g);
void twist(struct game* g);
void flip(struct game* g);
void begin(struct game* g, int index_listed);
void fall(struct game* g);
unsigned int fall_column(short* a, short* b);
int same(struct game* a, struct game* b);
int tryfalling(struct game* g);
void popup(struct game* g);
void move(struct game* g, short way);
int read_move(void);

unsigned short randint( short max );
void draw_button(int from_x, int from_y, int to_x, int to_y);
void ask_confirmation();

void 	show_screen (void *return_screen);
void 	key_press_screen();
int 	dispatch_screen (void *param);
void 	screen_job();
void draw_board(struct game* g);
void draw_screen(struct game* g);
void draw_score_screen(short moves, unsigned int score, unsigned int record);

#endif
