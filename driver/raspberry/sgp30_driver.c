

#include <stddef.h>
#include <asm/io.h>
#include <linux/delay.h>




#include "sgp30_driver.h"
#include "i2c_master.h"


enum sgp30_cmd_tag {
    SGP30_CMD_Init_air_quality      = 0x2003,
    SGP30_CMD_Measure_air_quality   = 0x2008,
    SGP30_CMD_Set_humidity          = 0x2061,
    SGP30_CMD_Get_feature_set_version = 0x202f,
    SGP30_CMD_Measure_test          = 0x2032,
    SGP30_CMD_Get_Serial_ID         = 0x3682,
 
};

/*单位：ms*/
enum sgp30_cmd_wait_time_tag {
    SGP30_CMD_Init_air_quality_WAIT_TIME    = 10,
    SGP30_CMD_Measure_air_quality_WAIT_TIME = 12,
    SGP30_CMD_Set_humidity_WAIT_TIME        = 10,
    SGP30_CMD_Get_feature_set_version_WAIT_TIME = 2,
    SGP30_CMD_Measure_test_WAIT_TIME        = 220,
    SGP30_CMD_Get_Serial_ID_WAIT_TIME       = 1,
};


#define SGP30_ADDR  (0x58)

#define ERROR_CHECK(fn) \
do {    \
    int ret = fn;   \
    if (0 != ret){\
        printk("%s:%d: error", __FUNCTION__, __LINE__);\
        return ret;\
    }\
}while(0)

/*
 * p, pointer to an array, len, array length
 * return, crc of array
*/
unsigned char sgp30_crc(unsigned char *p, int len)
{
  int i;
  unsigned char crc = 0xff;

  while(len--){
    crc ^= *p++;
    for (i = 0; i < 8; ++i){
      if (crc & 0x80){
        crc = (crc << 1) ^ 0x31;
      }else{
        crc <<= 1;
      }
    }
  }

  return crc;
}


/* arr参数是否为空，在上层做 */
static int sgp30_cmd(enum sgp30_cmd_tag cmd, int duration, unsigned char * dataout, int readlen){
    unsigned char data;
    int i;
    i2c_start();

    ERROR_CHECK(i2c_send_addr(SGP30_ADDR, I2C_WRITE));

    /*write command, MSB先发，LSB后发*/
    data = (u16)cmd >> 8;
    ERROR_CHECK(i2c_write_byte(data));
    data = (unsigned char)(cmd & 0xff);
    ERROR_CHECK(i2c_write_byte(data));

    /*waiting spg30 prepare data*/
    msleep(duration+10);

    if (NULL == dataout){
        i2c_stop();
        return 0;
    }

    /* read data */
    i2c_start();
    ERROR_CHECK(i2c_send_addr(SGP30_ADDR, I2C_READ));
    for (i = 0; i< readlen; ++i) {
        dataout[i] = i2c_read_byte(true);
    }

    i2c_stop();
    return 0;
}


int sgp30_get_version(unsigned char * arr){      
    enum sgp30_cmd_tag cmd = SGP30_CMD_Get_feature_set_version;
    int duration = SGP30_CMD_Get_feature_set_version_WAIT_TIME;
    unsigned char result[3];

    if (NULL == arr){
        return 1;
    }

    ERROR_CHECK(sgp30_cmd(cmd, duration, result, 3));

    /*check crc*/
    if (result[2] == sgp30_crc(result, 2)){
        arr[0] = result[0]; arr[1] = result[1];
        return 0;
    }

    return 1;
}

int sgp30_self_test(void){      
    enum sgp30_cmd_tag cmd = SGP30_CMD_Measure_test;
    int duration = SGP30_CMD_Measure_test_WAIT_TIME;
    unsigned char result[4];

    ERROR_CHECK(sgp30_cmd(cmd, duration, result, sizeof(result)));
    
    // printk("data: 0x%x, 0x%x, crc: 0x%x, should be 0x%x\n", result[0], result[1],result[2],sgp30_crc(result, 2));
    /*check crc*/
    if (result[2] != sgp30_crc(result, 2)){
        return 1;
    }

    /*check test result.*/
    if (0xd4 != result[0] || 0x0 != result[1])
    {
        return 1;
    }

    return 0;
}


int sgp30_init_air_quality(void){
    enum sgp30_cmd_tag cmd = SGP30_CMD_Init_air_quality;
    int duration = SGP30_CMD_Init_air_quality_WAIT_TIME;
    
    ERROR_CHECK(sgp30_cmd(cmd, duration, NULL, 0));
    
    return 0;
}

int sgp30_measure_air_quality(u16 *co2, u16 *tvoc){
    enum sgp30_cmd_tag cmd = SGP30_CMD_Measure_air_quality;
    int duration = SGP30_CMD_Measure_air_quality_WAIT_TIME;
    unsigned char result[6];

    ERROR_CHECK(sgp30_cmd(cmd, duration, result, sizeof(result)));
    
    // printk("%s: data: [0x%x, 0x%x, 0x%x], [0x%x, 0x%x, 0x%x], crc should be: 0x%x, 0x%x",__FUNCTION__,
    // result[0], result[1], result[2], result[3], result[4], result[5],
    // sgp30_crc(result, 2), sgp30_crc(result+3, 2));

    /*check crc*/
    if (result[2] != sgp30_crc(result, 2)){
        return 1;
    }
    if (result[5] != sgp30_crc(result+3, 2)){
        return 1;
    }

    /* convert reuslt to cpu endian, current data is big endian(MSB first) */
    *co2 = be16_to_cpu(*(u16 *)result);
    *tvoc = be16_to_cpu(*(u16 *)(result+3));

    printk("%s: co2: %dppm, tvoc: %dppb\n", __FUNCTION__, *co2, *tvoc);

    return 0;
}



int sgp30_get_serial_id(unsigned char * sid){
    enum sgp30_cmd_tag cmd = SGP30_CMD_Get_Serial_ID;
    int duration = SGP30_CMD_Get_Serial_ID_WAIT_TIME;
    unsigned char result[9];

    if (NULL == sid){
        return 1;
    }

    ERROR_CHECK(sgp30_cmd(cmd, duration, result, 9));

    // printk("serial id: [0x%x, 0x%x, 0x%x], [0x%x, 0x%x, 0x%x], [0x%x, 0x%x, 0x%x], crc should be 0x%x, 0x%x, 0x%x\n", 
    //         result[0], result[1], result[2],
    //         result[3], result[4], result[5],
    //         result[6], result[7], result[8], 
    //         sgp30_crc(result, 2), sgp30_crc(result+3, 2), sgp30_crc(result+6, 2));

    /*check crc*/
    if (sgp30_crc(result, 2) != result[2]){
        return 1;
    }

    if (sgp30_crc(result+3, 2) != result[5]){
        return 1;
    }

    if (sgp30_crc(result+6, 2) != result[8]){
        return 1;
    }

    /*copy out*/
    sid[0] = result[0]; sid[1] = result[1]; 
    sid[2] = result[3]; sid[3] = result[4]; 
    sid[4] = result[6]; sid[5] = result[7]; 
    return 0;
}
