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
	{ 0xf238fdf7, "cdev_del" },
	{ 0x58f3a1ad, "cdev_init" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xaf87d05c, "device_destroy" },
	{ 0xfa53f3eb, "__spi_register_driver" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x28cc25db, "arm_copy_from_user" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0x5f754e5a, "memset" },
	{ 0x7c32d0f0, "printk" },
	{ 0xef34a95d, "driver_unregister" },
	{ 0x7c1318e3, "device_create" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0xe0a1cfb8, "spi_sync" },
	{ 0x8fe01c8, "cdev_add" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0x1e047854, "warn_slowpath_fmt" },
	{ 0xcab2fae0, "spi_write_then_read" },
	{ 0x3df0823f, "class_destroy" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xebdab104, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cshtl,rfid_rc522");
MODULE_ALIAS("of:N*T*Cshtl,rfid_rc522C*");

MODULE_INFO(srcversion, "201B6D9D31764652E6F05F2");
