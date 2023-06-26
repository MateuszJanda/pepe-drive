#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/ioctl.h>

#include "data.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Mateusz Janda");
MODULE_DESCRIPTION("A simple Linux driver for in-memory char device.");

#define PEPE_MODULE_NAME "pepe"
#define PEPE_NUM_OF_DEVS 1

#define PEPE_IOCTL_IOC_MAGIC 'k'
#define PEPE_IOCTL_CMD_IS_WEDNESDAY _IOR(PEPE_IOCTL_IOC_MAGIC, 0, int)
#define PEPE_IOCTL_MAXNR 0

static int pepe_major = 0;
static int pepe_minor = 0;

// Device specific structure
struct pepe_dev {
	// I think currently it's not needed because driver only read data
	struct mutex mutex;
	// Char device structure
	struct cdev cdev;
};

// Array storing pepe devices
static struct pepe_dev *pepe_devs[PEPE_NUM_OF_DEVS] = { NULL };

// Handling "wednesday" parameter
static int pepe_check_day(char *buffer, const struct kernel_param *kp)
{
	struct timespec64 now;
	struct tm time;
	int len;

	ktime_get_real_ts64(&now);
	time64_to_tm(now.tv_sec, 0, &time);

	if (time.tm_wday == 3) {
		len = snprintf(buffer, PAGE_SIZE,
			       "It is Wednesday my Dudes.\n");
	} else {
		len = snprintf(buffer, PAGE_SIZE, "Not Wednesday, pepe sad.\n");
	}

	// Plus 1 for terminating null character
	return len < PAGE_SIZE ? len + 1 : -EINVAL;
}

static const struct kernel_param_ops pepe_wednesday_ops = {
	.get = pepe_check_day
};

int pepe_open(struct inode *inode, struct file *filp)
{
	// Device information
	struct pepe_dev *dev =
		container_of(inode->i_cdev, struct pepe_dev, cdev);
	filp->private_data = dev;

	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);

	// Nothing else is needed, pepe is read only device
	return 0;
}

ssize_t pepe_read(struct file *filp, char __user *buff, size_t count,
		  loff_t *f_pos)
{
	struct pepe_dev *dev = filp->private_data;
	ssize_t retval = 0;

	printk(KERN_DEBUG "pepe call: %s()\n", __FUNCTION__);
	printk(KERN_DEBUG "pepe_read() requested, f_pos = %lld, count = %lu.\n",
	       *f_pos, count);

	if (mutex_lock_interruptible(&dev->mutex)) {
		printk(KERN_DEBUG
		       "Error: mutex_lock_interruptible() unable to set.\n");
		return -ERESTARTSYS;
	}

	if (*f_pos >= sizeof(pepe_data)) {
		retval = 0;
		count = 0;
		goto end_of_file;
	}

	if (*f_pos + count > sizeof(pepe_data)) {
		count = sizeof(pepe_data) - *f_pos;
	}

	if (copy_to_user(buff, pepe_data + *f_pos, count)) {
		printk(KERN_DEBUG "Error: copy_to_user().\n");
		retval = -EFAULT;
		goto fail_copy_to_user;
	}

	retval = count;
	*f_pos += count;

end_of_file:
fail_copy_to_user:
	printk(KERN_DEBUG "pepe_read() delivered, f_pos = %lld, count = %lu.\n",
	       *f_pos, count);

	mutex_unlock(&dev->mutex);
	return retval;
}

long pepe_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long retval = 0;

	struct timespec64 now;
	struct tm time;

	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);

	if (_IOC_TYPE(cmd) != PEPE_IOCTL_IOC_MAGIC) {
		printk(KERN_DEBUG
		       "Error: command %d with incorrect magic number.\n",
		       cmd);
		return -ENOTTY;
	}

	if (_IOC_NR(cmd) > PEPE_IOCTL_MAXNR) {
		printk(KERN_DEBUG "Error: command %d above max number.\n", cmd);
		return -ENOTTY;
	}

	if (cmd == PEPE_IOCTL_CMD_IS_WEDNESDAY) {
		ktime_get_real_ts64(&now);
		time64_to_tm(now.tv_sec, 0, &time);
		if (time.tm_wday == 3) {
			return 1;
		} else {
			return 0;
		}
	}

	return retval;
}

int pepe_release(struct inode *inode, struct file *filp)
{
	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);

	// No hardware to shut down
	return 0;
}

static struct file_operations pepe_fops = {
	.owner = THIS_MODULE,
	.open = pepe_open,
	.read = pepe_read,
	.unlocked_ioctl = pepe_ioctl,
	.release = pepe_release,
};

static void __init peep_fail_cleanup(void)
{
	dev_t dev_num = 0;
	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);

	// Deallocated device resources.
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		if (pepe_devs[i] != NULL) {
			cdev_del(&pepe_devs[i]->cdev);
			kfree(pepe_devs[i]);
		}
	}

	// Unregister char device number
	if (pepe_major != 0) {
		dev_num = MKDEV(pepe_major, pepe_minor);
		unregister_chrdev_region(dev_num, PEPE_NUM_OF_DEVS);
	}
}

static void __init pepe_setup_dev(struct pepe_dev *dev)
{
	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);
	mutex_init(&dev->mutex);

	// Setup char dev
	cdev_init(&dev->cdev, &pepe_fops);
	dev->cdev.owner = THIS_MODULE;
}

static int __init pepe_init(void)
{
	int err = -ENOMEM;
	bool fail_kmalloc = false;
	bool fail_cdev_add = false;
	dev_t dev_num = 0;

	printk(KERN_WARNING PEPE_MODULE_NAME " loaded.\n");
	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);

	// Dynamically allocate device number
	err = alloc_chrdev_region(&dev_num, pepe_minor, PEPE_NUM_OF_DEVS,
				  PEPE_MODULE_NAME);

	if (err < 0) {
		printk(KERN_DEBUG
		       "Error(%d): alloc_chrdev_region() failed on pepe.\n",
		       err);
		goto fail;
	}
	pepe_major = MAJOR(dev_num);

	// Allocate devices resources
	err = -ENOMEM;
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		pepe_devs[i] = kmalloc(sizeof(struct pepe_dev), GFP_KERNEL);
		if (pepe_devs[i] == NULL) {
			printk(KERN_DEBUG
			       "Error(%d): kmalloc() failed on pepe%d.\n",
			       err, i);
			fail_kmalloc = true;
			break;
		}

		pepe_setup_dev(pepe_devs[i]);

		// Make this char device usable in user-space
		dev_num = MKDEV(pepe_major, pepe_minor + i);
		err = cdev_add(&pepe_devs[i]->cdev, dev_num, 1);
		if (err < 0) {
			printk(KERN_DEBUG "Error(%d): Adding %s%d error.\n",
			       err, PEPE_MODULE_NAME, i);
			kfree(pepe_devs[i]);
			pepe_devs[i] = NULL;
			fail_cdev_add = true;
			break;
		}
	}

	// All or nothing. In case of fail make cleanup
	if (fail_kmalloc || fail_cdev_add) {
		goto fail;
	}

	return 0;
fail:
	peep_fail_cleanup();
	return err;
}

static void __exit pepe_exit(void)
{
	dev_t dev_num = MKDEV(pepe_major, pepe_minor);

	printk(KERN_DEBUG "pepe call: %s().\n", __FUNCTION__);

	// Deallocated device resources
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		cdev_del(&pepe_devs[i]->cdev);
		kfree(pepe_devs[i]);
	}

	// Unregister char device number
	unregister_chrdev_region(dev_num, PEPE_NUM_OF_DEVS);
	printk(KERN_WARNING PEPE_MODULE_NAME " unloaded.\n");
}

// Parameter (/sys/module/pepe/parameters/wednesday)
module_param_cb(wednesday, &pepe_wednesday_ops, NULL, 0444);
module_init(pepe_init);
module_exit(pepe_exit);
