#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

int i2c_master_init(void);
void i2c_master_deinit(void);
void i2c_test(void);


#define I2C_READ	(1)
#define I2C_WRITE	(0)

void i2c_start(void);
int i2c_send_addr(unsigned char addr, bool rw);
int i2c_write_byte(unsigned char data);
unsigned char i2c_read_byte(bool ack);
void i2c_stop(void);


#endif
