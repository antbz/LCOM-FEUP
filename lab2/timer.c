#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int hookid = TIMER0_IRQ;
uint32_t counter = 0;

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  //verifica se o valor da frequência é válido
  if (freq < 19 || freq > TIMER_FREQ) { //se for <19 new_freq sai fora da gama de 16bit, se for >TIMER_FREQ newfreq=0 
    printf("Invalid frequency");
    return 1;
  }
  
  //ler configuracao do timer 0
  uint8_t st;
  timer_get_conf(timer, &st);
  
  //separar o valor da nova frequencia em dois bytes
  uint32_t newfreq = TIMER_FREQ / freq;
  uint8_t lsb, msb;

  util_get_LSB(newfreq, &lsb);
  util_get_MSB(newfreq, &msb);

  //printf("freq = %d, msb = %x, lsb = %x", newfreq, msb, lsb);

  //alterar a frequencia do timer pretendido
  switch (timer)
  {
  case 0:
    sys_outb(TIMER_CTRL, TIMER_SEL0 | TIMER_LSB_MSB | (st&0x0f));
    sys_outb(TIMER_0, lsb);
    sys_outb(TIMER_0, msb);
    break;
  case 1:
    sys_outb(TIMER_CTRL, TIMER_SEL1 | TIMER_LSB_MSB | (st&0x0f));
    sys_outb(TIMER_1, lsb);
    sys_outb(TIMER_1, msb);
    break;
  case 2:
    sys_outb(TIMER_CTRL, TIMER_SEL2 | TIMER_LSB_MSB | (st&0x0f));
    sys_outb(TIMER_2, lsb);
    sys_outb(TIMER_2, msb);
    break;
  default:
    return 1;
    break;
  }
  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = hookid;
  sys_irqsetpolicy(TIMER0_IRQ , IRQ_REENABLE, &hookid);
  return 0;
}

int (timer_unsubscribe_int)() {
  sys_irqrmpolicy(&hookid);
  return 0;
}

void (timer_int_handler)() {
  counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  sys_outb(TIMER_CTRL, TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));
  switch(timer)
  {
    case 0:
      return util_sys_inb(TIMER_0, st);
    case 1:
      return util_sys_inb(TIMER_1, st);
    case 2:
      return util_sys_inb(TIMER_2, st);
    default:
      return 1;
  }
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
  union timer_status_field_val val;
  switch (field)
  {
  case tsf_all:
    val.byte = st;
    break;
  case tsf_initial:
    switch (st & (BIT(5) | BIT(4))) {
      case TIMER_LSB_MSB: 
        val.in_mode = MSB_after_LSB;
        break;
      case TIMER_MSB:
        val.in_mode = MSB_only;
        break;
      case TIMER_LSB:
        val.in_mode = LSB_only;
        break;
      default:
        val.in_mode = INVAL_val;
        break;
    }
    break;
  case tsf_mode:
    val.count_mode = (st&(BIT(3)|BIT(2)|BIT(1)))>>1;
    break;
  case tsf_base:
    val.bcd = st&TIMER_BCD;
    break;
  default:
    return 1;
  }
  return timer_print_config(timer, field, val);
}
