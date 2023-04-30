#include <linux/init.h>
#include <linux/module.h>


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Mateusz Janda");
MODULE_DESCRIPTION("A simple char device driver that provides ANSI image.");


static int __init m_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}
static void __exit m_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(m_init);
module_exit(m_exit);
