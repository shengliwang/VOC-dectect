#include <stddef.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include "i2c_master.h"



/* ==== 移植适配区==== 树莓派找不到相关的接口设置gpio为open-drain模式，所以这里只能用push-pull 模式*/
// #define SCL	(23)	// 使用 gpio23 作为 scl
// #define SDA	(24)	// 使用 gpio24 作为 sda
#define SCL	(20)	// 使用 gpio23 作为 scl
#define SDA	(21)	// 使用 gpio24 作为 sda


static inline unsigned char GPIORead(unsigned gpio){
	// 由于在linux接口中，找不到设置gpio为open-drain模式，所以只能先把gpio改为输入，然后去读。
	gpio_direction_input(gpio);
	return gpio_get_value(gpio);
}

#define SCL_1		\
do 	\
{	\
	gpio_direction_output(SCL, 1);\
	gpio_set_value(SCL, 1);\
}while(0)

#define SCL_0		\
do 	\
{	\
	gpio_direction_output(SCL, 0);\
	gpio_set_value(SCL, 0);\
}while(0)

#define SDA_1		\
do 	\
{	\
	gpio_direction_output(SDA, 1);\
	gpio_set_value(SDA, 1);\
}while(0)

#define SDA_0		\
do 	\
{	\
	gpio_direction_output(SDA, 0);\
	gpio_set_value(SDA, 0);\
}while(0)


#define INTERVAL	(100)	// 单位： us

/* delay 函数，单位 微秒 us */
static inline void Delay(unsigned int usecs){
	udelay(usecs);
}
/* ==== 移植适配区 end==== */


/*note: when return, SCL in low level.*/
void i2c_start(void){
	SCL_1;
	SDA_1;
	Delay(50);

	SDA_0;			/* sda low 表示 起始信号 */
	Delay(INTERVAL);

	SCL_0;
	Delay(INTERVAL);
}


void i2c_stop(void){
	SCL_0;
	SDA_0;
	Delay(INTERVAL);

	SCL_1;
	Delay(INTERVAL);
	SDA_1;
	Delay(INTERVAL);
}

 /*
 发送顺序 MSB->LSB, 比如 11110000b， 在总线上的发送顺序为 1 1 1 1 0 0 0 0
 返回0代表成功，其他，代表没有收到ACK
 注意： delay设置 10us的时候，I2c的速度为 50Kbps
 note: when return, SCL in low level.
 */
int i2c_write_byte(unsigned char data){
	int i;

	for (i = 7; i >= 0; --i){
		
		if (data & (1 << i)){
			SDA_1;

		} else {
			SDA_0;

		}
		Delay(INTERVAL);
		SCL_1;
		Delay(INTERVAL);
		SCL_0;
	}

	GPIORead(SDA);

	Delay(INTERVAL/2);

	if (0 == GPIORead(SDA)){ /* 读到ack了*/
		i = 0;
	} else {
		i = 1;
	}
	Delay(INTERVAL/2);
	SCL_1;

	Delay(INTERVAL); 

	SCL_0;
	Delay(2*INTERVAL); /*delay some time for wating slave to handle interrupt*/

	return i;
}

/*写：rw=0，
读： rw=1*/
int i2c_send_addr(unsigned char addr, bool rw){
	return i2c_write_byte( (addr<< 1) + rw);
}

#define U8BIT(n) ((unsigned char)( 1 << n))
/* ack=1, 发送ack，ack=0，nack*/
unsigned char i2c_read_byte(bool ack){
	unsigned char data = 0;
	int i;

	for (i = 7; i >= 0; --i)
	{
		/* 这里在下降沿 稳定后，先读sda*/
		if (GPIORead(SDA)){
			data |= U8BIT(i);
		}

		SCL_1;
		Delay(INTERVAL);
		
		SCL_0;
		Delay(INTERVAL);
	}


	if (ack){
		SDA_0;
	}else{
		SDA_1;
	}

	SCL_1;
	Delay(INTERVAL);

	SCL_0;
	Delay(2*INTERVAL); /*delay some time for wating slave to handle interrupt*/
	return data;
}


int i2c_master_init(void)
{
	int ret;
	ret = gpio_request(SCL, "vicking's gpio for i2c");
	if (0!=ret){
		printk("%s: gpio %d request failed \n",__FUNCTION__, SCL);
		return -EFAULT;
	}

	ret = gpio_request(SDA, "vicking's gpio for sda");
	if (0!=ret){
		printk("%s: gpio %d request failed \n",__FUNCTION__, SDA);
		goto sda_err;
	}

	return 0;

sda_err:
	gpio_free(SCL);
	return -EFAULT;
}

void i2c_master_deinit(void)
{
	gpio_free(SDA);
	gpio_free(SCL);
}