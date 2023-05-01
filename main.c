#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Mateusz Janda");
MODULE_DESCRIPTION("A simple char device driver that provides ANSI image.");

#define PEPE_MODULE_NAME "pepe"
#define PEPE_NUM_OF_DEVS 1

static int pepe_major = 0;
static int pepe_minor = 0;

// Internal device specific structure.
struct pepe_dev {
	// record how many blocks now in the list
	int block_counter;
	struct mutex mutex;
	// Char device structure.
	struct cdev cdev;
	// list of storage blocks
	struct list_head block_list;
};

static struct pepe_dev *pepe_devs[PEPE_NUM_OF_DEVS] = { NULL };

int pepe_open(struct inode *inode, struct file *filp)
{
	// Device information.
	struct pepe_dev *dev =
		container_of(inode->i_cdev, struct pepe_dev, cdev);
	filp->private_data = dev;

	// Nothing else is needed, pepe is read only device.
	return 0;
}

int pepe_release(struct inode *inode, struct file *filp)
{
	// No hardware to shut down.
	return 0;
}

static struct file_operations pepe_fops = {
	.owner = THIS_MODULE,
	.open = pepe_open,
	.read = pepe_read,
	.release = pepe_release,
};

static void __init peep_fail_cleanup(void)
{
	dev_t dev_num = 0;

	// Deallocated device resources.
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		if (pepe_devs[i] != NULL) {
			cdev_del(&pepe_devs[i]->cdev);
			kfree(pepe_devs[i]);
		}
	}

	// Unregister char device number.
	if (pepe_major != 0) {
		dev_num = MKDEV(pepe_major, pepe_minor);
		unregister_chrdev_region(dev_num, PEPE_NUM_OF_DEVS);
	}
}

static void __init pepe_setup_dev(struct pepe_dev *dev)
{
	// Setup pepe_dev.
	dev->block_counter = 0;

	INIT_LIST_HEAD(&dev->block_list);
	mutex_init(&dev->mutex);

	// Setup char dev.
	cdev_init(&dev->cdev, &pepe_fops);
	dev->cdev.owner = THIS_MODULE;
}

static int __init pepe_init(void)
{
	int err = -ENOMEM;
	bool fail_kmalloc = false;
	bool fail_cdev_add = false;
	dev_t dev_num = 0;

	printk(KERN_WARNING PEPE_MODULE_NAME " is loaded\n");

	// Dynamically allocate device number.
	err = alloc_chrdev_region(&dev_num, pepe_minor, PEPE_NUM_OF_DEVS,
				  PEPE_MODULE_NAME);

	if (!err) {
		pr_debug("Error(%d): alloc_chrdev_region() failed on pepe\n",
			 err);
		goto fail;
	}
	pepe_major = MAJOR(dev_num);

	// Allocate devices.
	err = -ENOMEM;
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		pepe_devs[i] = kmalloc(sizeof(struct pepe_dev), GFP_KERNEL);
		if (pepe_devs[i] != NULL) {
			pr_debug("Error(%d): kmalloc() failed on pepe%d\n", err,
				 i);
			fail_kmalloc = true;
			break;
		}

		pepe_setup_dev(pepe_devs[i]);

		// Make this char device usable in userspace
		dev_num = MKDEV(pepe_major, pepe_minor + i);
		err = cdev_add(&pepe_devs[i]->cdev, dev_num, 1);
		if (err) {
			pr_debug("Error(%d): Adding %s%d error\n", err,
				 PEPE_MODULE_NAME, i);
			kfree(pepe_devs[i]);
			pepe_devs[i] = NULL;
			fail_cdev_add = true;
			break;
		}
	}

	// All or nothing. In case of fail make cleanup.
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

	printk(KERN_WARNING PEPE_MODULE_NAME " unloaded\n");

	// Deallocated device resources.
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		cdev_del(&pepe_devs[i]->cdev);
		kfree(pepe_devs[i]);
	}

	// Unregister char device number.
	unregister_chrdev_region(dev_num, PEPE_NUM_OF_DEVS);
}

module_init(pepe_init);
module_exit(pepe_exit);
