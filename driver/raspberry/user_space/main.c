#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../public.h"


#define sensor_fname    "/dev/sensor"

void usage(char * fname){
    printf(
        "usage: %s <option>\n"
        "support option:\n"
        "-i: Init sgp30 air measurement algorithm.\n"
        "    This action should be executed after every power up or soft reset\n"
        "    of spg30 chip.\n"
        "-s: get serial id in 6 bytes.\n"
        "-m: measure air quality which will return like this:\n"
        "    co2: 450ppm, tvoc: 70ppb\n"
        "-v: show sgp30 feature version.\n"
        "-h: show this help.\n"
        , fname);
}

int  arg_valid(char c){
    switch (c){
        case 'i':
        case 's':
        case 'm':
        case 'v':
        case 'h':{
            return 0;
        }
        default:{
            return 1;
        }     
    }
}

int main(int argc, char * argv[]){
    int fd;
    int ret;
    int i;

    /* 参数检查 */
    if (argc != 2){
        usage(argv[0]);
        exit(1);
    }

    if (argv[1][0] != '-'){
        usage(argv[0]);
        exit(1);
    }

    if (0 != arg_valid(argv[1][1])){
        usage(argv[0]);
        exit(1);
    }

    if ('h' == argv[1][1]){
        usage(argv[0]);
        exit(0);
    }

    /* 打开设备文件 */
    fd = open(sensor_fname, O_RDWR);
    if (fd < 0){
        perror("Open " sensor_fname);
        exit(1);
    }

    /* 操作设备文件（读写）*/
    struct sgp30_cmd cmd;
    memset(&cmd, 0, sizeof(cmd));
    switch(argv[1][1]){
        case 'i':{
            cmd.cmd = CMD_INIT_AIR_QUALITY;
            ret = write(fd, &cmd, sizeof(cmd));
            if (ret >= 0){
                ret = 0;
            }
            break;
        }
        case 's':{
            cmd.cmd = CMD_GET_SERIAL_ID;
            ret = read(fd, &cmd, sizeof(cmd));
            if (ret >= 0){
                printf("sgp30 serial id: ");
                for (i = 0; i < 6; ++i){
                    printf("0x%x ",cmd.dataout[i]);
                }
                printf("\n");
                ret = 0;
            }
            break;
        }
        case 'm':{
            cmd.cmd = CMD_MEASURE_AIR_QUALITY;
            ret = read(fd, &cmd, sizeof(cmd));
            if (ret >= 0){
                printf("co2: %uppm, ", *(unsigned short*)cmd.dataout);
                printf("tvoc: %uppm\n", *(unsigned short*)(cmd.dataout+2));
                ret = 0;
            }
            break;
        }
        case 'v':{
            cmd.cmd = CMD_GET_VERSION;
            ret = read(fd, &cmd, sizeof(cmd));
            if (ret >=0 ){
                printf("sgp30 feature version: 0x%x 0x%x\n", cmd.dataout[0], cmd.dataout[1]);
                ret = 0;
            }
            break;
        }
        default:{
            /* never executed */
            break;
        }
    }

    if (ret < 0){
        perror(sensor_fname);
    }

    close(fd);
    return ret;
}