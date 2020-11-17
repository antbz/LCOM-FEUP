#include <lcom/lcf.h>
#include <lcom/utils.h>
#include <stdint.h>
#include <stdio.h>

int (subscribe_mouse)(uint8_t *bit_no);
int (unsubscribe_mouse)();
int enable_mouse();
int disable_mouse();
int mouse_enable_data_rep();
int mouse_get_status_byte(uint8_t *status);
void (mouse_ih)();
void assemble_packet(struct packet *pp);
int mouse_cmd_write(uint8_t command);
int kbc_cmd_write(uint8_t command);

enum state_t {
  INIT,
  DRAW_UP,
  COMP_UP,
  DRAW_DOWN,
  COMP
};
void check_symbol(struct mouse_ev *evt, uint8_t x_len);
struct mouse_ev* detect_mouse_ev (struct packet *pp, uint8_t x_len, uint8_t tolerance);

