#include <lcom/lcf.h>

// Register defines
#define KBD_STAT_REG  0x64
#define KBC_CMD_REG   0x64
#define OUT_BUF       0x60
#define KBC_ARG_REG   0x60
#define IRQ12         12

#define LB            BIT(0)
#define RB            BIT(1)
#define MB            BIT(2)
#define MSB_X_DELTA   BIT(4)
#define MSB_Y_DELTA   BIT(5)
#define X_OVF         BIT(6)
#define Y_OVF         BIT(7)

// Mouse related KBC Command defines
#define READ_CMD        0x20
#define WRITE_CMD       0x60
#define DISABLE_MOUSE   0xA7
#define ENABLE_MOUSE    0xA8
#define CHECK_INTERFACE 0xA9
#define WRITE_TO_MOUSE  0xD4

// Status byte defines
#define PARITY    BIT(7)
#define TIMEOUT   BIT(6)
#define AUX       BIT(5)
#define INH       BIT(4)
#define A2        BIT(3)
#define SYS       BIT(2)
#define IBF       BIT(1)
#define OBF       BIT(0)

//Arguments of command 0xD4
#define RESET           0xFF
#define RESEND          0xFE
#define SET_DEF         0xF6
#define DISABLE_DR      0xF5
#define ENABLE_DR       0xF4
#define SET_SAMPLE_RATE 0xF3
#define SET_RMT_MODE    0xF0
#define READ_DATA       0xEB
#define SET_STREAM_MODE 0xEA
#define STATUS_RQST     0xE9
#define SET_RESOLUTION  0xE8
#define SET_SCALE_2_1   0xE7
#define SET_SCALE_1_1   0xE6

//Mouse responses
#define ACK 0xFA
#define NACK  0xFE
#define ERROR 0xFC
