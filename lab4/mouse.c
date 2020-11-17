#include <lcom/lcf.h>
#include <lcom/lab4.h>
#include <lcom/utils.h>
#include "mouse.h"
#include "defs.h"

static int hookid = 2; // timer - 0, kb - 1, mouse - 2
struct packet pp;
unsigned counter = 0;

int (subscribe_mouse)(uint8_t *bit_no) {
  *bit_no = hookid;
  if (sys_irqsetpolicy(IRQ12 , IRQ_REENABLE | IRQ_EXCLUSIVE, &hookid)) {
    return 1;
  }
  return 0;
}

int enable_mouse() {
  return sys_irqenable(&hookid);
}

int disable_mouse() {
  return sys_irqdisable(&hookid);
}

int (unsubscribe_mouse)() {
  uint8_t status;

  if (sys_irqrmpolicy(&hookid)) {
    return 1;
  }

  mouse_get_status_byte(&status);

  if (status & OBF) {
    util_sys_inb(OUT_BUF, &status);
  }

  return 0;
}

int mouse_get_status_byte(uint8_t *status) {
  uint8_t mouse_status;
  if (util_sys_inb(KBD_STAT_REG, &mouse_status)) {
    return 1;
  }
  *status = mouse_status;
  return 0;
}

void (mouse_ih)(void) {
  uint8_t status, byte;
  
  if (mouse_get_status_byte(&status)) {
    return;
  }

  if (status & OBF) {
    if(util_sys_inb(OUT_BUF, &byte)) {
      return;
    }
    //printf("%d\n",counter);
    if (counter % 3 == 0) { // Synchronize
      if (byte & BIT(3)) {
        pp.bytes[counter % 3] = byte;
        counter++;
      } else {
        return;
      }
    } else {
      pp.bytes[counter % 3] = byte;
      counter++;
    }
  }
}

void assemble_packet(struct packet *pp) {
  pp->lb = pp->bytes[0] & LB;
  pp->rb = pp->bytes[0] & RB;
  pp->mb = pp->bytes[0] & MB;
  pp->x_ov = pp->bytes[0] & X_OVF;
  pp->y_ov = pp->bytes[0] & Y_OVF;
  if (pp->bytes[0] & MSB_X_DELTA) {
    pp->delta_x = pp->bytes[1] - 256;
  } else {
    pp->delta_x = pp->bytes[1];
  }
  if (pp->bytes[0] & MSB_Y_DELTA) {
    pp->delta_y = pp->bytes[2] - 256;
  } else {
    pp->delta_y = pp->bytes[2];
  }
}

int mouse_cmd_write(uint8_t command) {
  uint8_t status, ackn;

  // Read status byte to see if IBF is full
  // Send command to mouse
  // Re-send command if failed in one byte
  // Halt execution if error in two consecutive bytes

  do {
    if (mouse_get_status_byte(&status)) {
      return 1;
    }
    if (status & IBF) {
      continue;
    }
    if (sys_outb(KBC_CMD_REG, WRITE_TO_MOUSE)) {
      return 1;
    }

    if (mouse_get_status_byte(&status)) {
      return 1;
    }
    if (status & IBF) {
      continue;
    }
    if (sys_outb(KBC_ARG_REG, command)) {
      return 1;
    }

    tickdelay(micros_to_ticks(20000));

    if (util_sys_inb(OUT_BUF, &ackn)) {
      return 1;
    }

    if (ackn == ERROR) {
      return -1;
    }
  } while (ackn != ACK);
  
  return 0;
}

int kbc_cmd_write(uint8_t command) {
  sys_outb(KBC_CMD_REG, WRITE_CMD);
  sys_outb(KBC_ARG_REG, command);
  return 0;
}

enum state_t state = INIT;

extern uint8_t x_drawn;

void check_symbol(struct mouse_ev *evt, uint8_t x_len) {
  switch (state) {
    case INIT:
      if(evt->type == LB_PRESSED ) {
        state = DRAW_UP;
      }
      else
      {
        state = INIT;
      }
      break;
    case DRAW_UP:
      if( evt->type == LB_RELEASED && x_drawn >= x_len ) {
        x_drawn = 0;
        state = COMP_UP;
      } 
      else if ( evt->type == MOUSE_MOV ){
        state = DRAW_UP;
      }
      else
      {
        state = INIT;
      }
      break;
    case COMP_UP:
      if(evt->type == RB_PRESSED) {
        state = DRAW_DOWN;
      } else if (evt->type == MOUSE_MOV) {
        state = COMP_UP;
      } else if (evt->type == LB_PRESSED) {
        state = DRAW_UP;
      } else
      {
        state = INIT;
      }
      break;
    case DRAW_DOWN:
      if(evt->type == RB_RELEASED && x_drawn >= x_len) {
        state = COMP;
      } 
      else if ( evt->type == MOUSE_MOV ){
        state = DRAW_DOWN;
      }
      else
      {
        state = INIT;
      }
      break;
    default:
      break;
  }
}

uint16_t absolute_value(int num) {
    if (num < 0) {
        return -num;
    }
    else {
        return num;
    }
}

struct mouse_ev* detect_mouse_ev (struct packet *pp, uint8_t x_len, uint8_t tolerance) {
  struct mouse_ev *ret = malloc(sizeof(struct mouse_ev));
  ret->delta_x = pp->delta_x;
  ret->delta_y = pp->delta_y;

  if (pp->lb && !pp->mb && !pp->rb && pp->delta_x == 0 && pp->delta_y == 0) {
    ret->type = LB_PRESSED;
    return ret;
  }
  else if (state == DRAW_UP && !pp->lb && !pp->mb && !pp->rb)
  {
    ret->type = LB_RELEASED;
    return ret;
  }
  else if (!pp->lb && !pp->mb && pp->rb && pp->delta_x == 0 && pp->delta_y == 0)
  {
    ret->type = RB_PRESSED;
    return ret;
  }
  else if (state == DRAW_DOWN && !pp->lb && !pp->mb && !pp->rb)
  {
    ret->type = RB_RELEASED;
    return ret;
  }
  else if ((pp->lb && !pp->mb && !pp->rb && pp->delta_y >= pp->delta_x) || (!pp->lb && !pp->mb && pp->rb && pp->delta_y  <= -(pp->delta_x)) || absolute_value(pp->delta_x) <= tolerance)
  {
    ret->type = MOUSE_MOV;
    printf("[%d,%d]", pp->delta_x,x_drawn);

    x_drawn += pp->delta_x;
    return ret;
  }
  else
  {
    ret->type = BUTTON_EV;
    return ret;
  }
  
}
