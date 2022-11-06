#ifndef __MYSGP30_DRIVER_H__
#define __MYSGP30_DRIVER_H__

int sgp30_get_version(unsigned char * arr);
int sgp30_self_test(void);
int sgp30_init_air_quality(void);
int sgp30_measure_air_quality(u16 *co2, u16 *tvoc);
int sgp30_get_serial_id(unsigned char * sid);


#endif