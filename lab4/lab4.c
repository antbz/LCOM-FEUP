
// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you

#include "mouse.h"
#include "defs.h"
#include "i8254.h"
#include <lcom/timer.h>

extern struct packet pp;
extern unsigned counter;
extern uint32_t timer_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}


int (mouse_test_packet)(uint32_t cnt) {
    uint8_t bitno;
    int ipc_status = 0;
    message msg;
    uint8_t r;

    

    if (subscribe_mouse(&bitno)) {
      return 1;
    }

    disable_mouse();

    if (mouse_cmd_write(ENABLE_DR)) {
      return 1;
    }
    
    enable_mouse();
    
    uint8_t irq_set = BIT(bitno); // bit-mask

    while (cnt > 0) { // repeats process for specified number of packets
      r = driver_receive(ANY, &msg, &ipc_status); // gets request message
      if ( r != 0 ) { 
          printf("Error driver_receive failed with: %d", r);
          continue;
      }
      if (is_ipc_notify(ipc_status)) { // When a notification is received
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE:                         // In case it is an HW interrupt notification
            if (msg.m_notify.interrupts & irq_set) { // AND is the subscribed one
              mouse_ih(); // Call the IH
              if (counter % 3 == 0 && counter != 0) {
                assemble_packet(&pp);
                mouse_print_packet(&pp);
                cnt--;
              }
            }
          default:
            break;
        }
      }
    }  

    disable_mouse();

    if (mouse_cmd_write(DISABLE_DR)) {
      return 1;
    }
    
    enable_mouse();

    if (unsubscribe_mouse() != 0 ){
        return 1;
    }

    return 0;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    while(cnt > 0) {
      mouse_cmd_write(READ_DATA);
      for (unsigned i = 0; i < 3; i++) {
        mouse_ih();
    }
    assemble_packet(&pp);  
    mouse_print_packet(&pp);

    cnt--;

    tickdelay(micros_to_ticks(period*1000)); 
  }
  
  if (mouse_cmd_write(SET_STREAM_MODE)) {
      return 1;
    }

  if (mouse_cmd_write(DISABLE_DR)) {
    return 1;
  }

  if (kbc_cmd_write(minix_get_dflt_kbc_cmd_byte())) {
    return 1;
  }

  return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
  uint8_t mouse_bitno;
  uint8_t timer_bitno;

  int ipc_status = 0;
  message msg;
  uint8_t r;

  if (subscribe_mouse(&mouse_bitno)) {
    return 1;
  }
  if (timer_subscribe_int(&timer_bitno)) {
    return 1;
  }

  disable_mouse();
  if (mouse_cmd_write(ENABLE_DR)) {
      return 1;
  }
  enable_mouse();

  uint8_t mouse_irq_set = BIT(mouse_bitno);
  uint8_t timer_irq_set = BIT(timer_bitno);

  while (timer_counter/60 < idle_time) {
      r = driver_receive(ANY, &msg, &ipc_status);
      if ( r != 0 ) { 
          printf("Error driver_receive failed with: %d", r);
          continue;
      }
      if (is_ipc_notify(ipc_status)) { // When a notification is received
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE:                         // In case it is an HW interrupt notification
            if (msg.m_notify.interrupts & mouse_irq_set) { // AND is the subscribed one
              mouse_ih(); // Call the IH 
              if (counter % 3 == 0) {
                assemble_packet(&pp);  
                mouse_print_packet(&pp);
                timer_counter = 0;
              }
              break;
            }
            else if (msg.m_notify.interrupts & timer_irq_set) {
              timer_int_handler();
              break;
            }
          default:
            break;
        }
      }
    }  
  
  disable_mouse();

  if (mouse_cmd_write(DISABLE_DR)) {
    return 1;
  }
    
  enable_mouse();

  if (unsubscribe_mouse() != 0 ){
    return 1;
  }

  if (timer_unsubscribe_int() != 0 ){
    return 1;
  }

  return 0;
}

extern enum state_t state;
uint8_t x_drawn = 0;

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
  uint8_t bitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;

  if (subscribe_mouse(&bitno)) {
    return 1;
  }

  disable_mouse();

  if (mouse_cmd_write(ENABLE_DR)) {
    return 1;
  }
    
  enable_mouse();
    
  uint8_t irq_set = BIT(bitno); // bit-mask


  while ( state != COMP || x_drawn < x_len/*x_len1 < x_len && x_len2 < x_len */ ) {
    r = driver_receive(ANY, &msg, &ipc_status); // gets request message
    if ( r != 0 ) { 
      printf("Error driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // When a notification is received
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                         // In case it is an HW interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // AND is the subscribed one
            mouse_ih(); // Call the IH
            if (counter % 3 == 0 && counter != 0) {
              assemble_packet(&pp);
              struct mouse_ev *event = detect_mouse_ev(&pp, x_len, tolerance);
              printf("%d : ",event->type);
              printf("%d -> ", state);
              check_symbol(event, x_len);
              printf("%d (%d) - ", state, x_drawn);
              mouse_print_packet(&pp);
            }
          }
        default:
          break;
      }
    }
  }  

  disable_mouse();

  if (mouse_cmd_write(DISABLE_DR)) {
    return 1;
  }
    
  enable_mouse();

  if (unsubscribe_mouse() != 0 ){
    return 1;
  }

  return 0;
}
