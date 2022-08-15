#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x516e49f9, "module_layout" },
	{ 0x848cee4, "gpiod_unexport" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xf238fdf7, "cdev_del" },
	{ 0x3df0823f, "class_destroy" },
	{ 0xaf87d05c, "device_destroy" },
	{ 0xfe990052, "gpio_free" },
	{ 0xd6b8e852, "request_threaded_irq" },
	{ 0xaf09f062, "gpiod_to_irq" },
	{ 0xda575cc, "gpiod_export" },
	{ 0x3e8ff20c, "gpiod_direction_input" },
	{ 0x4e8b35ac, "gpiod_direction_output_raw" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x7c1318e3, "device_create" },
	{ 0xebdab104, "__class_create" },
	{ 0x8fe01c8, "cdev_add" },
	{ 0x58f3a1ad, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x1e047854, "warn_slowpath_fmt" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0x97255bdf, "strlen" },
	{ 0x91715312, "sprintf" },
	{ 0xddc1fca4, "gpiod_set_raw_value" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x3555ff59, "gpiod_get_raw_value" },
	{ 0x395c5ae9, "gpio_to_desc" },
	{ 0xb43f9365, "ktime_get" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

