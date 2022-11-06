#ifndef __MY_PUBLIC_H___
#define __MY_PUBLIC_H___

#define MAX_DATA_IN_LEN     16
#define MAX_DATA_OUT_LEN    16

enum sgp30_cmd_type{
    CMD_GET_SERIAL_ID,
    CMD_GET_VERSION,
    CMD_INIT_AIR_QUALITY,
    CMD_MEASURE_AIR_QUALITY,
};

struct sgp30_cmd{
    unsigned int cmd;
    unsigned char datain[MAX_DATA_IN_LEN];
    unsigned char dataout[MAX_DATA_OUT_LEN];
};

#endif