#ifndef KBD_H
#define KBD_H

#include <stdio.h>
#include <lcom/lcf.h>
#include <stdint.h>
#include <lcom/lab3.h>


int (subscribe_kbc)(uint8_t *bit_no);
int (unsubscribe_kbc)();
int kbc_cmd_read(uint8_t *out);
int kbc_cmd_write(uint8_t command);

#endif
