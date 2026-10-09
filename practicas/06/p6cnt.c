/*
 * @Autor       Ivan E. Nicolas Montesinos
 * @Fecha       08/10/2026
 * @Descripcion Cuenta aperturas; acepta reset y rechaza otras ordenes.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/uaccess.h>

static DEFINE_MUTEX(candado);
static unsigned long long aperturas;
static unsigned long long total_aperturas;

static int contador_open(struct inode *inode, struct file *archivo)
{
    mutex_lock(&candado);
    aperturas++;
    total_aperturas++;
    mutex_unlock(&candado);
    return 0;
}

static ssize_t contador_read(struct file *archivo, char __user *destino,
                             size_t cantidad, loff_t *posicion)
{
    char texto[64];
    int largo;

    mutex_lock(&candado);
    largo = scnprintf(texto, sizeof(texto), "aperturas: %llu\n", aperturas);
    mutex_unlock(&candado);
    /* Respeta la posicion y devuelve EOF al terminar la linea. */
    return simple_read_from_buffer(destino, cantidad, posicion, texto, largo);
}

static ssize_t contador_write(struct file *archivo, const char __user *origen,
                              size_t cantidad, loff_t *posicion)
{
    char orden[6];

    if (cantidad == 0)
        return 0;
    if (cantidad != 5 && cantidad != 6)
        return -EINVAL;
    if (copy_from_user(orden, origen, cantidad))
        return -EFAULT;
    if (memcmp(orden, "reset", 5) != 0 ||
        (cantidad == 6 && orden[5] != '\n'))
        return -EINVAL;

    mutex_lock(&candado);
    aperturas = 0;
    mutex_unlock(&candado);
    pr_info("contador reiniciado\n");
    return cantidad;
}

static const struct file_operations contador_fops = {
    .owner = THIS_MODULE,
    .open = contador_open,
    .read = contador_read,
    .write = contador_write,
};

static struct miscdevice contador_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "p6cnt",
    .fops = &contador_fops,
    .mode = 0666,
};

static int __init contador_init(void)
{
    int resultado = misc_register(&contador_dev);

    if (resultado)
        return resultado;
    pr_info("/dev/p6cnt creado, minor %d\n", contador_dev.minor);
    return 0;
}

static void __exit contador_exit(void)
{
    misc_deregister(&contador_dev);
    pr_info("/dev/p6cnt eliminado; total de aperturas: %llu; desde reset: %llu\n",
            total_aperturas, aperturas);
}

module_init(contador_init);
module_exit(contador_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ivan E. Nicolas Montesinos");
MODULE_DESCRIPTION("Practica 6 FSE: contador de aperturas virtual");
