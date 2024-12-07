#include <linux/module.h>  // Для макросов модулей (MODULE_*)
#include <linux/kernel.h>  // Для printk()

// Заголовки для информации о модуле
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomsk State University");
MODULE_DESCRIPTION("A simple Linux kernel module for TSU");

// Функция, вызываемая при загрузке модуля
static int __init tsu_init(void) {
    printk(KERN_INFO "Welcome to the Tomsk State University\n");
    return 0;  // Возврат 0 означает успешную загрузку
}

// Функция, вызываемая при выгрузке модуля
static void __exit tsu_exit(void) {
    printk(KERN_INFO "Tomsk State University forever!\n");
}

// Указываем функции для загрузки и выгрузки модуля
module_init(tsu_init);
module_exit(tsu_exit);
