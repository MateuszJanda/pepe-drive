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


struct scull_dev {
	// record how many blocks now in the list
	int block_counter;
	struct mutex mutex;
	struct cdev cdev;
	// list of storage blocks
	struct list_head block_list;
};


static int __init m_init(void)
{
	int err = 0;
	dev_t devno;

	printk(KERN_WARNING PEPE_MODULE_NAME " is loaded\n");

	// Alloc device number
	err = alloc_chrdev_region(&devno, pepe_minor, PEPE_NUM_OF_DEVS,
				  PEPE_MODULE_NAME);

	// pr_debug("%d %d\n", devno, pepe_major);

	if (err < 0) {
		return err;
	}

	return 0;
}


static void __exit m_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(m_init);
module_exit(m_exit);
