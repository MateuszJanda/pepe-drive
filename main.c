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

struct pepe_dev {
	// record how many blocks now in the list
	int block_counter;
	struct mutex mutex;
	struct cdev cdev;
	// list of storage blocks
	struct list_head block_list;
};

static struct pepe_dev *pepe_devs[PEPE_NUM_OF_DEVS] = { 0 };

static struct file_operations pepe_fops = {
	.owner = THIS_MODULE,
	.open = pepe_open,
	.read = pepe_read,
	.write = pepe_write,
	.release = pepe_release,
};

static void __init fail_init_cleanup(void)
{
	dev_t dev_num = 0;

	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		if (!pepe_devs[i]) {
			kfree(pepe_devs[i]);
		}
	}

	if (pepe_major == 0 && pepe_minor == 0) {
		dev_num = MKDEV(pepe_major, pepe_minor);
		unregister_chrdev_region(dev_num, PEPE_NUM_OF_DEVS);
	}
}

static void __init init_pepe_dev(struct pepe_dev *dev)
{
	dev->block_counter = 0;

	INIT_LIST_HEAD(&dev->block_list);
	mutex_init(&dev->mutex);

	cdev_init(&dev->cdev, &pepe_fops);
	dev->cdev.owner = THIS_MODULE;
}

static int __init m_init(void)
{
	int err = -ENOMEM;
	bool fail_kmalloc = false;
	dev_t dev_num = 0;

	printk(KERN_WARNING PEPE_MODULE_NAME " is loaded\n");

	// Dynamically allocate device number
	err = alloc_chrdev_region(&dev_num, pepe_minor, PEPE_NUM_OF_DEVS,
				  PEPE_MODULE_NAME);

	if (!err) {
		pr_debug("Error(%d): alloc_chrdev_region() failed on pepe\n",
			 err);
		goto fail;
	}
	pepe_major = MAJOR(dev_num);

	// Allocate devices
	err = -ENOMEM;
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		pepe_devs[i] = kmalloc(sizeof(struct pepe_dev), GFP_KERNEL);
		if (!pepe_devs[i]) {
			pr_debug("Error(%d): kmalloc() failed on pepe%d\n", err,
				 i);
			fail_kmalloc = true;
			break;
		}

		init_pepe_dev(pepe_devs[i]);
	}

	if (!fail_kmalloc) {
		goto fail;
	}

	return 0;
fail:
	fail_init_cleanup();
	return err;
}

static void __exit m_exit(void)
{
	dev_t dev_num = MKDEV(pepe_major, pepe_minor);

	printk(KERN_WARNING PEPE_MODULE_NAME " unloaded\n");

	// Deallocated resources
	for (int i = 0; i < PEPE_NUM_OF_DEVS; i++) {
		kfree(pepe_devs[i]);
	}

	unregister_chrdev_region(dev_num, PEPE_NUM_OF_DEVS);
}

module_init(m_init);
module_exit(m_exit);
