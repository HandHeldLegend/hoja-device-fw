// ProGCC 4 main board (progcc-4-hw PGS_Main_Board), RP2354B

#define PGPIO_BTN_POWER     9
#define PGPIO_BTN_GL        10
#define PGPIO_EN_DPAD       12
#define PGPIO_BTN_LS        13
#define PGPIO_EN_TRIGGER_L  14
#define PGPIO_BTN_CAPTURE   15
#define PGPIO_BTN_MINUS     16
#define PGPIO_EN_TRIGGER_R  28
#define PGPIO_BTN_PLUS      29
#define PGPIO_BTN_HOME      30
#define PGPIO_BTN_GR        32
#define PGPIO_BTN_RS        33
#define PGPIO_EN_SEWN       34

// The hall sensors' four shared outputs (ADC_0..ADC_3 on the schematic). Each enable line, active
// low, puts one bank of four sensors on them.
#define PGPIO_HOVER_ADC_0   42
#define PGPIO_HOVER_ADC_1   43
#define PGPIO_HOVER_ADC_2   44
#define PGPIO_HOVER_ADC_3   45

#define PGPIO_STICK_LY      40
#define PGPIO_STICK_LX      41
#define PGPIO_STICK_RX      46
#define PGPIO_STICK_RY      47
