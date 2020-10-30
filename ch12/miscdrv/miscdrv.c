/*
 * ch12/miscdrv/miscdrv.c
 ***************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch 12 : Writing a Simple Misc Device Driver
 ****************************************************************
 * Brief Description:
 * A simple 'skeleton' Linux character device driver, belonging to the
 * 'misc' class (major #10). Here, we simply build something of a 'template'
 * or skeleton for a misc driver.
 *
 * For details, please refer the book, Ch 12.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>            /* the fops, file data structures */
#include "../../convenient.h"

#define OURMODNAME   "miscdrv"

MODULE_AUTHOR("Kaiwan N Billimoria");
MODULE_DESCRIPTION("LLKD book:ch12/miscdrv: simple 'skeleton' misc char driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

/*
 * open_miscdrv()
 * The driver's open 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is opened. Here, we simply print out some relevant info.
 * The POSIX standard requires open() to return the file descriptor in success;
 * note, though, that this is done within the kernel VFS (when we return). So,
 * all we do here is return 0 indicating success.
 */
static int open_miscdrv(struct inode *inode, struct file *filp)
{
	PRINT_CTX(); // displays process (or intr) context info

	pr_info("%s:%s():\n"
		" filename: \"%s\"\n"
		" wrt open file: f_flags = 0x%x\n",
	       OURMODNAME, __func__,
	       filp->f_path.dentry->d_iname, filp->f_flags);

	return 0;
}

/*
 * read_miscdrv()
 * The driver's read 'method'; it has effectively 'taken over' the read syscall
 * functionality! Here, we simply print out some info.
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; we simply return 'count', pretending that we 'always succeed'.
 */
static ssize_t read_miscdrv(struct file *filp, char __user *ubuf,
				size_t count, loff_t *off)
{
	pr_info("%s:%s():\n", OURMODNAME, __func__);
	return count;
}

/*
 * write_miscdrv()
 * The driver's write 'method'; it has effectively 'taken over' the write syscall
 * functionality! Here, we simply print out some info.
 * The POSIX standard requires that the read() and write() system calls return
 * the number of bytes read or written on success, 0 on EOF and -1 (-ve errno)
 * on failure; we simply return 'count', pretending that we 'always succeed'.
 */
static ssize_t write_miscdrv(struct file *filp, const char __user *ubuf,
				size_t count, loff_t *off)
{
	pr_info("%s:%s():\n", OURMODNAME, __func__);
	return count;
}

/*
 * close_miscdrv()
 * The driver's close 'method'; this 'hook' will get invoked by the kernel VFS
 * when the device file is closed (technically, when the file ref count drops
 * to 0). Here, we simply print out some relevant info.
 * We simply return 0 indicating success.
 */
static int close_miscdrv(struct inode *inode, struct file *filp)
{
	pr_info("%s:%s(): filename: \"%s\"\n",
	       OURMODNAME, __func__, filp->f_path.dentry->d_iname);
	return 0;
}

static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv,
	.read = read_miscdrv,
	.write = write_miscdrv,
	.release = close_miscdrv,
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,	/* kernel dynamically assigns a free minor# */
	.name = "llkd_miscdrv",	/* when misc_register() is invoked, the kernel
							 * will auto-create device file as /dev/llkd_miscdrv ;
							 * also populated within /sys/class/misc/ and /sys/devices/virtual/misc/ */
	.mode = 0666,			/* ... dev node perms set as specified here */
	.fops = &llkd_misc_fops,	/* connect to this driver's 'functionality' */
};

static int __init miscdrv_init(void)
{
	int ret;
	struct device *dev;

	if ((ret = misc_register(&llkd_miscdev))) {
		pr_notice("%s: misc device registration failed, aborting\n",
			OURMODNAME);
		return ret;
	}

	/* Retrieve the device pointer for this device */
	dev = llkd_miscdev.this_device;
	pr_info("%s: LLKD misc driver (major # 10) registered, minor# = %d,"
		" dev node is /dev/llkd_miscdrv\n",
		OURMODNAME, llkd_miscdev.minor);

	dev_info(dev, "sample dev_info(): minor# = %d\n", llkd_miscdev.minor);

	return 0;		/* success */
}

static void __exit miscdrv_exit(void)
{
	misc_deregister(&llkd_miscdev);
	pr_info("%s: LLKD misc driver deregistered, bye\n", OURMODNAME);
}

module_init(miscdrv_init);
module_exit(miscdrv_exit);