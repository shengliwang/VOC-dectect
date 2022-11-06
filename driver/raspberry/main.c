#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include <asm/io.h> //ioremap iounmap的头文件
#include <linux/delay.h>

#include "i2c_master.h"
#include "sgp30_driver.h"
#include "public.h"

static struct cdev *sensor_cdev;

static int major = 111;
static int minor = 0;
static unsigned char cmdbuf[sizeof(struct sgp30_cmd)];

/*
* 模块不支持 多进程/线程访问（函数不具有可重入性）
*/

static int sensor_open(struct inode *inode, struct file *fp)
{
    return 0;
}

static ssize_t sensor_read(struct file *fp, char __user *buf, size_t size, loff_t *ppos)
{
    struct sgp30_cmd * sgpcmd;
    int ret;
    if (size != sizeof(struct sgp30_cmd)){
        return -EFAULT;
    }

    if (copy_from_user(cmdbuf, buf, size))
    {
        return -EFAULT;
    }
    
    sgpcmd = (struct sgp30_cmd *)cmdbuf;
    ret = size;
    switch (sgpcmd->cmd){
        case CMD_GET_SERIAL_ID:{
            if (sgp30_get_serial_id(sgpcmd->dataout)){
                ret = -EIO;
            }
            break;
        }
        case CMD_GET_VERSION:{
            if (sgp30_get_version(sgpcmd->dataout)){
                ret = -EIO;
            }
            break;
        }
        case CMD_MEASURE_AIR_QUALITY:{
            msleep(1000);
            if (sgp30_measure_air_quality((u16 *)(sgpcmd->dataout), (u16*)(sgpcmd->dataout+2))){
                ret = -EIO;
            }
            break;
        }
    }

    if(copy_to_user(buf, cmdbuf, size)){
        return -EFAULT;
    }

    return ret;
}

static ssize_t sensor_write(struct file *fp, const char __user *buf, size_t size, loff_t *ppos)
{
    struct sgp30_cmd * sgpcmd;
    int duration;

    if (size != sizeof(struct sgp30_cmd)){
        return -EINVAL;
    }

    if (copy_from_user(cmdbuf, buf, size)){
        return -EFAULT;
    }
    
    sgpcmd = (struct sgp30_cmd *)cmdbuf;
    switch (sgpcmd->cmd){
        case CMD_INIT_AIR_QUALITY:{
            printk("recv CMD_INIT_AIR_QUALITY\n");
            if (sgp30_init_air_quality()){
                printk("sgp30_init_air_quality failed\n");
                return -EIO;
            }
            duration = 20;
            while(duration--){
                msleep(1000);
            }
            break;
        }
        default:{
            printk("unkown cmd\n");
            return -EINVAL;
        }
    }

    return size;
}

static const struct file_operations fps = {
    .owner = THIS_MODULE,
    .read = sensor_read,
    .write = sensor_write,
    .open = sensor_open,
};

static int sensor_init(void)
{
    int ret = 0;
    dev_t dev_no;

    printk("%s: start\n", __FUNCTION__);

    if (major){
        dev_no = MKDEV(major, minor);
        ret = register_chrdev_region(dev_no, 1, "sensor");
    }else{
        ret = alloc_chrdev_region(&dev_no, 0, 1, "sensor");
    }

    if (0 != ret)
    {
        printk("alloc dev no failed\n");
        return ret;
    }

    printk("%s: alloc char dev ok(%d, %d)\n", __FUNCTION__, MAJOR(dev_no), MINOR(dev_no));

    if (0 != i2c_master_init())
    {
        printk("%s: i2c_master_init failed\n", __FUNCTION__);
        ret = -EFAULT;
        goto i2c_master_error;
    }

    sensor_cdev = kmalloc(sizeof(struct cdev), GFP_KERNEL);
    if (!sensor_cdev)
    {
        printk(KERN_ERR "malloc failed\n");
        ret = -ENOMEM;
        goto malloc_fail;
    }

    cdev_init(sensor_cdev, &fps);
    sensor_cdev->owner = THIS_MODULE;
    ret = cdev_add(sensor_cdev, dev_no, 1);

    if (ret)
    {
        printk(KERN_ERR "cdev_add failed\n");
        goto cdev_add_err;
    }

    printk(KERN_INFO "disk module ininted major=%d, minor=%d\n", MAJOR(dev_no), MINOR(dev_no));

    return 0;

cdev_add_err:
    kfree(sensor_cdev);

malloc_fail:
    i2c_master_deinit();
    
i2c_master_error:
    unregister_chrdev_region(dev_no, 1);
    
    return ret;
}

static void sensor_exit(void)
{
    dev_t dev = sensor_cdev->dev;
    int count = sensor_cdev->count;

    cdev_del(sensor_cdev);

    kfree(sensor_cdev);

    i2c_master_deinit();

    unregister_chrdev_region(dev, count);
    printk(KERN_INFO "disk modue exited\n");
}

module_init(sensor_init);
module_exit(sensor_exit);

module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);

MODULE_LICENSE("GPL");
