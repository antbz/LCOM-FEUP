#include <lcom/lcf.h>

#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>
#include "keyboard.h"
#include "defs.h"

#define ESC     0x81

extern uint8_t data[2];
#ifdef LAB3
extern int cnt;
#endif
extern bool kbd_valid;
extern bool two_byte_code;
extern uint32_t timer_counter;

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(kbd_test_scan)() {
  // Variables for handling interrupt subscription
  uint8_t bitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;

  // Subscribes to keyboard interrupts and checks for errors in the process
  if (subscribe_kbc(&bitno)) {
    printf("Error subscribing to KBC!");
    return 1;
  }

  // Builds bit-mask
  uint8_t irq_set = BIT(bitno);

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
            if (kbd_valid && !two_byte_code) { // If the IH "returned" a valid state, ie no errors and not reading the first byte of a to byte code 
              // Print the scancode for two and single byte codes
              // printf("save the turtles"); DEBUG
              if (data[0] == TWO_BYTE) {
                kbd_print_scancode(!(data[1] & BIT(7)) /*make or break code*/, 2, data);
              }
              else
              {
                kbd_print_scancode(!(data[0] & BIT(7)), 1, data);
              }
              
            }
          }
        default:
          break;
      }
    }
  }  

  // Unsubscribes keyboard interrupts and checks for errors in the process
  if (unsubscribe_kbc()) {
    printf("Error unsubscribing from KBC!");
    return 1;
  };

  /* Supostamente limpa o buffer no final mas está pendente
  uint8_t flush;
  util_sys_inb(OUT_BUF, &flush);
  */

  // Prints the number of sys_inb calls
  #ifdef LAB3
  kbd_print_no_sysinb(cnt);
  #endif

  return 0;
}

int(kbd_test_poll)() {
  while (data[0] != ESC) { // While the scancode is not the ESC breakcode
    // Check if we want to read from the buffer and read from the buffer
    kbc_ih(); //TODO separar verificação de se há algo para ler do handler!
    
    // Prints the read scancodes
    if (kbd_valid && !two_byte_code) {
      if (data[0] == TWO_BYTE) {
        kbd_print_scancode(!(data[1] & BIT(7)), 2, data);
      }
      else
      {
        kbd_print_scancode(!(data[0] & BIT(7)), 1, data);
      }
      
    }
    // Adds a delay so as to not overload the controller
    tickdelay(micros_to_ticks(20000));
  }

  // Resets the command byte of the controller so MINIX handler can go back to controlling KBI
  uint8_t command;
  kbc_cmd_read(&command);
  command |= BIT(0);
  kbc_cmd_write(command);

  /* Supostamente cenas mas na verdade nope
  uint8_t flush;
  util_sys_inb(OUT_BUF, &flush);
  */

  // Prints the number of sys_inb calls
  #ifdef LAB3
  kbd_print_no_sysinb(cnt);
  #endif

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  // Variables for handling interrupt subscription
  uint8_t tbitno, kbitno;
  int ipc_status = 0;
  message msg;
  uint8_t r;

  // Subscribes timer and keyboard interrupts
  if (timer_subscribe_int(&tbitno)) {
    printf("Error subscribing to timer!");
    return 1;
  }
  if (subscribe_kbc(&kbitno)) {
    printf("Error subscribing to KBC!");
    return 1;
  }

  // Builds masks
  uint8_t k_irq_set = BIT(kbitno);
  uint8_t t_irq_set = BIT(tbitno);

  while(data[0] != ESC && (timer_counter / 60) < n) { // While the scancode is not ESC breakcode AND the timer isn't over the recieved limit
    r = driver_receive(ANY, &msg, &ipc_status);
    if ( r != 0 ) { 
        printf("driver_receive failed with: %d", r);
        continue;
        }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: 	
          if (msg.m_notify.interrupts & t_irq_set) { // If it is a timer notification, call T_IH
            timer_int_handler();
            // printf("%d\n", timer_counter); DEBUG
          }		
          if (msg.m_notify.interrupts & k_irq_set) { // If it is a keyboard notification, call K_IH
            kbc_ih();
            if (kbd_valid && !two_byte_code) {
              if (data[0] == TWO_BYTE) {
                bool make = !(data[1] & BIT(7));
                kbd_print_scancode(make, 2, data);
              }
              else
              {
                bool make = !(data[0] & BIT(7));
                kbd_print_scancode(make, 1, data);
              }
              timer_counter = 0;
            }
          }
        default:
          break;
      }
    }
  }  

  /* Supostamente limpa o buffer, mas provoca erros nos testes
  uint8_t flush;
  util_sys_inb(OUT_BUF, &flush);
  */

  // Unsubscribes keyboard and timer interrupts
  if (unsubscribe_kbc()) {
    printf("Error unsubscribing from KBC!");
    return 1;
  };
  if (timer_unsubscribe_int()) {
    printf("Error unsubscribing from TIMER!");
  }
  
  // Prints the number of sys_inb calls
  #ifdef LAB3
  kbd_print_no_sysinb(cnt);
  #endif

  return 0;
}
