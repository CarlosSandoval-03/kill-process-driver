#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>

#define DEVICE_NAME "killerProcess"
#define CLASS_NAME "killerProcess"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Sandoval");
MODULE_DESCRIPTION("A simple Linux char driver that kills a process");
MODULE_VERSION("0.1");

static int majorNumber;
static int numberOpens = 0;
static char message[256] = { 0 };
static struct class *killerProcessClass = NULL;
static struct device *killerProcessDevice = NULL;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);

static struct file_operations fops = { .open = dev_open, .write = dev_write, .release = dev_release };

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

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	printk(KERN_INFO "killerProcess: Received %zu characters from the user\n", len);

	// Get string from user
	if (copy_from_user(message, buffer, len) != 0) {
		printk(KERN_INFO "killerProcess: Failed to copy message from user\n");
		return -EFAULT;
	}

	// Get pid from string
	int pid = -1;
	if (kstrtoint(message, 10, &pid) < 0) {
		printk(KERN_INFO "killerProcess: Failed to convert string to int\n");
		return -EFAULT;
	}

	if (pid == -1) {
		printk(KERN_INFO "killerProcess: Invalid pid\n");
		return len;
	}

	// Kill the process
	printk(KERN_INFO "killerProcess: Killing process %d\n", pid);

	if (pid < 0) {
		printk(KERN_INFO "killerProcess: Invalid pid\n");
		return len;
	}

	struct task_struct *task;
	rcu_read_lock(); // Lock the task list
	task = pid_task(find_vpid(pid), PIDTYPE_PID); // Find the task with the given pid and store it in task
	if (task == NULL) {
		printk(KERN_INFO "killerProcess: Process %d not found\n", pid);
		rcu_read_unlock();
		return len;
	}
	send_sig_info(SIGKILL, SEND_SIG_FORCED, task); // Send the SIGKILL signal to the task
	rcu_read_unlock(); // Unlock the task list
	printk(KERN_INFO "killerProcess: Process %d killed\n", pid);
	return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "killerProcess: Device successfully closed\n");
	return 0;
}

module_init(killer_process_init);
module_exit(killer_process_exit);