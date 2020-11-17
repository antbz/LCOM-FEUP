// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>
#include <lcom/xpm.h>

#include <lcom/timer.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "keyboard.h"
#include "defs.h"
#include "videocard.h"
#include "keyboard.h"

extern uint8_t data[2];
extern bool kbd_valid;
extern bool two_byte_code;

extern uint32_t timer_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(video_test_init)(uint16_t mode, uint8_t delay) {
  if (set_vbe_mode(mode)) {
    return 1;
  }

  tickdelay(micros_to_ticks(delay * 1000000));

  vg_exit();

  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y, uint16_t width, uint16_t height,uint32_t color) {

  map_video_mem(mode);

  if (set_vbe_mode(mode)) {
    printf("Error setting VBE mode!");
    return 1;
  }
  
  draw_rectangle(color, x, y, width, height);
 
  
  uint8_t kbd_bitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;
  bool preserve = false;

  if (subscribe_kbc(&kbd_bitno)) {
    printf("Error subscribing to KBC!");
    return 1;
  }

  uint8_t irq_set = BIT(kbd_bitno);

  while(data[0] != ESC) { // While the scancode is not the ESC key breakcode
    // Gets a request message
    r = driver_receive(ANY, &msg, &ipc_status);
    if ( r != 0 ) { 
        printf("Error driver_receive failed with: %d", r);
        continue;
        }
    if (is_ipc_notify(ipc_status)) { // When a notification is recieved
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 			// In case it is an HW interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // AND is the subscribed one
            kbc_ih(); // Call the IH 
            if (data[0] == ESC) {
              break;
            }
          }
        default:
          break;
      }
    }
  }

  vg_exit();

  if (unsubscribe_kbc()) {
    printf("Error unsubscribing from KBC!");
    return 1;
  }
  if (!kbd_get_status_byte(&preserve)) {
    uint8_t flush;
    util_sys_inb(OUT_BUF, &flush);
  }

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  map_video_mem(mode);

  if (set_vbe_mode(mode)) {
    printf("Error setting VBE mode!");
    return 1;
  }
  
  
  draw_pattern(no_rectangles, first, step);

  
  uint8_t kbd_bitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;
  bool preserve = false;

  if (subscribe_kbc(&kbd_bitno)) {
    printf("Error subscribing to KBC!");
    return 1;
  }

  uint8_t irq_set = BIT(kbd_bitno);

  while(data[0] != ESC) { // While the scancode is not the ESC key breakcode
    // Gets a request message
    r = driver_receive(ANY, &msg, &ipc_status);
    if ( r != 0 ) { 
        printf("Error driver_receive failed with: %d", r);
        continue;
        }
    if (is_ipc_notify(ipc_status)) { // When a notification is recieved
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 			// In case it is an HW interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // AND is the subscribed one
            kbc_ih(); // Call the IH 
            if (data[0] == ESC) {
              break;
            }
          }
        default:
          break;
      }
    }
  }

  vg_exit();

  if (unsubscribe_kbc()) {
    printf("Error unsubscribing from KBC!");
    return 1;
  }
  if (!kbd_get_status_byte(&preserve)) {
    uint8_t flush;
    util_sys_inb(OUT_BUF, &flush);
  }

  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  //change to video mode 0x105
  map_video_mem(VIDEO_MODE);

  if (set_vbe_mode(VIDEO_MODE)) {
    return 1;
  }
  //display pixmap at (xi, yi)
  xpm_image_t img;
  uint8_t *map;
  map = xpm_load(xpm, XPM_INDEXED, &img);

  draw_xpm(x, y, img, map);

  //release of ESC - reset video mode to default text mode
  uint8_t bitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;

  if (subscribe_kbc(&bitno)) {
    printf("Error subscribing to KBC!");
    return 1;
  }

  uint8_t irq_set = BIT(bitno);

  while(data[0] != ESC) { 
    r = driver_receive(ANY, &msg, &ipc_status);
    if ( r != 0 ) { 
        printf("Error driver_receive failed with: %d", r);
        continue;
        }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();  
            }
          
        default:
          break;
      }
    }
  }
  
  if (unsubscribe_kbc()) {
    printf("Error unsubscribing from KBC!");
    return 1;
  };
  vg_exit();
  //return
  return 0;

}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  
  //change to video mode
  map_video_mem(VIDEO_MODE);

  if (set_vbe_mode(VIDEO_MODE)) {
    return 1;
  }
  //load the pixmap at (xi, yi)
  xpm_image_t img;
  uint8_t *map;
  map = xpm_load(xpm, XPM_INDEXED, &img);

  // move_xpm(img, map, xi, yi, xf, yf, speed, fr_rate);
  //move the pixmap to (xf, yf) at speed with fr_rate frames per second
      //NOTES: 
      //either xf is equal to xi or yf is equal to yi
      //+speed - displacement in pixels between consecutive frames
      //-speed - number of frames required for a displacement of one pixel
      //if speed is positive and the length of the movement is not a multiple of speed, the last displacement of the pixmap in its movement will be smaller than speed

  uint8_t kb_bitno, timer_bitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;

  if (subscribe_kbc(&kb_bitno)) {
    printf("Error subscribing to KBC!");
    return 1;
  }

  if (timer_subscribe_int(&timer_bitno)) {
    return 1;
  }

  uint8_t kb_irq_set = BIT(kb_bitno);
  uint8_t timer_irq_set = BIT(timer_bitno);

  uint16_t displacement, d_frame;
  char axis;
  if (xi == xf && yi != yf) {
    displacement = yf - yi;
    axis = 'y';
  } else if (xi != xf && yi == yf) {
    displacement = xf - xi;
    axis = 'x';
  } else {
    return 1;
  }
  if (speed > 0) {
    d_frame = speed;
  } else if (speed < 0) {
    d_frame = 1;
  } else {
    return 1;
  }
  uint16_t moved = 0;

  draw_xpm(xi, yi, img, map);

  while(data[0] != ESC) { 
    r = driver_receive(ANY, &msg, &ipc_status);
    if ( r != 0 ) { 
        printf("Error driver_receive failed with: %d", r);
        continue;
    }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & kb_irq_set) {
            kbc_ih();  
          } else if (msg.m_notify.interrupts & timer_irq_set) {
            timer_int_handler();
            if (timer_counter % (60 / fr_rate) == 0) {
              if (moved < displacement) {
                uint16_t last = moved;
                if (displacement - moved >= d_frame) {
                  moved += d_frame;
                } else {
                  moved += displacement - moved;
                }
                switch (axis) {
                case 'x': {
                  clear_xpm(xi + last, yi, img);
                  draw_xpm(xi + moved, yi, img, map);
                  break;
                }
                case 'y': {
                  clear_xpm(xi, yi + last, img);
                  draw_xpm(xi, yi + moved, img, map);
                  break;
                }
                default:
                  break;
                }
              }
            }
          }
          break;
        default:
          break;
      }
    }  
  }
  
  if (unsubscribe_kbc()) {
    printf("Error unsubscribing from KBC!");
    return 1;
  }

  if (timer_unsubscribe_int()) {
    return 1;
  }

  vg_exit();

  //return
  return 0;
}

/*
int(video_test_controller)() {
  //call VBE function 0x0
  vg_vbe_contr_info_t info;
  //parse VBE controller information returned
  vg_display_vbe_contr_info(info);
  return 0;
}*/
