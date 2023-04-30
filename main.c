#include <linux/init.h>
#include <linux/module.h>


MODULE_LICENSE("Dual BSD/GPL");


static int m_init(void)
{
	printk(KERN_ALERT "Hello, world\n");
	return 0;
}
static void m_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_exit(m_exit);
module_init(m_init);
