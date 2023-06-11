#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "killerProcess"
#define CLASS_NAME "killerProcess"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Sandoval");
MODULE_DESCRIPTION("A simple Linux char driver that kills a process");
MODULE_VERSION("0.1");

static int majorNumber;
static char message[256] = { 0 };
static short sizeOfMessage;
static int numberOpens = 0;
static struct class *killerProcessClass = NULL;
static struct device *killerProcessDevice = NULL;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);

static struct file_operations fops = { .open = dev_open, .read = dev_read, .write = dev_write, .release = dev_release };

static int __init killer_process_init(void)
{
	printk(KERN_INFO "killerProcess: Initializing the killerProcess LKM\n");

	// Try to dynamically allocate a major number for the device
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT "killerProcess failed to register a major number\n");
		return majorNumber;
	}
	printk(KERN_INFO "killerProcess: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	killerProcessClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(killerProcessClass)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(killerProcessClass);
	}
	printk(KERN_INFO "killerProcess: device class registered correctly\n");

	// Register the device
	killerProcessDevice = device_create(killerProcessClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(killerProcessDevice)) {
		class_destroy(killerProcessClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the device\n");
		return PTR_ERR(killerProcessDevice);
	}
	printk(KERN_INFO "killerProcess: device class created correctly\n");
	return 0;
}

static void __exit killer_process_exit(void)
{
	device_destroy(killerProcessClass, MKDEV(majorNumber, 0));
	class_unregister(killerProcessClass);
	class_destroy(killerProcessClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "killerProcess: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	numberOpens++;
	printk(KERN_INFO "killerProcess: Device has been opened %d time(s)\n", numberOpens);
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error_count = 0;
	error_count = copy_to_user(buffer, message, sizeOfMessage);

	if (error_count == 0) {
		printk(KERN_INFO "killerProcess: Sent %d characters to the user\n", sizeOfMessage);
		return (sizeOfMessage = 0);
	} else {
		printk(KERN_INFO "killerProcess: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	sprintf(message, "%s(%zu letters)", buffer, len);
	sizeOfMessage = strlen(message);
	printk(KERN_INFO "killerProcess: Received %zu characters from the user\n", len);
	return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "killerProcess: Device successfully closed\n");
	return 0;
}

module_init(killer_process_init);
module_exit(killer_process_exit);