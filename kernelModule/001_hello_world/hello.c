#include <linux/module.h>
#include <linux/kernel.h>

int hello_init(void)
{
	pr_alert("Hello World :)\n");
	return 0;
}
void hello_exit(void)
{
	pr_alert("Goodbye World!\n");
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trieu Huynh <vikingtc4@gmail.com>");
MODULE_DESCRIPTION("A Hello World driver.");
