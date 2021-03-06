/*
 *  netchardev.c: First go at client-side net char device
 */

/* 
 * With help and code from:
 * http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html
 * http://lwn.net/Kernel/LDD3/
 */

/* System Includes */
#include <linux/init.h>
#include <linux/module.h>	/* Needed by all modules */
#include <linux/moduleparam.h>

#include <linux/kernel.h>	/* Needed for printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>           /* Everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/cdev.h>
#include <linux/file.h>

#include <asm/uaccess.h>	/* for copy/put_user */

#include <linux/net.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/socket.h>
#include <net/sock.h>


/* Module Info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("A Sayler <andy.sayler@gmail.com>");
MODULE_DESCRIPTION("A sample kernel module");
MODULE_SUPPORTED_DEVICE("testdevice");

/* TODO: Move a lot of this to a proper header file */

/* Global Defines */
#define SUCCESS 0
#define DEV_NAME "ncddev" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80	  /* Max length of the message from the device */
#define NUM_DEVS 1
#define PORT 5000
#define SRVADDR 0x7F000001 /* 127.0.0.1 */

#ifndef NCD_MAJOR
#define NCD_MAJOR 0   /* dynamic major by default */
#endif

#ifndef NCD_MINOR
#define NCD_MINOR 0   /* first minor by default */
#endif

/* Structures */
struct ncd_dev {
	struct cdev cdev;	  /* Char device structure		*/
};

/* Local Global Variables */
static int ncd_major = NCD_MAJOR;
static int ncd_minor = NCD_MINOR;
static int Device_Open = 0;   /* Used to prevent multiple access to device */
static char msg[BUF_LEN];     /* The msg the device will give when asked */
static char* msg_Ptr;
struct ncd_dev* ncd_devices;	/* allocated in scull_init_module */

/* Local Prototypes */
static int __init ncd_start(void);
static void ncd_cleanup(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

/* Module Parameters */
module_param(ncd_major, int, S_IRUGO);
module_param(ncd_minor, int, S_IRUGO);

/* Assign Function Handlers */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/* Assign Init and Exit */
module_init(ncd_start);
module_exit(ncd_cleanup);

/*
 * Set up the char_dev structure for this device.
 */
static void ncd_setup_cdev(struct ncd_dev *dev, int index)
{
	int err;
	int devno = MKDEV(ncd_major, ncd_minor + index);
    
	cdev_init(&(dev->cdev), &fops);
	(dev->cdev).owner = THIS_MODULE;
	(dev->cdev).ops = &fops;
	err = cdev_add (&(dev->cdev), devno, 1);
	/* Fail gracefully if need be */
	if (err){
		printk(KERN_NOTICE "Error %d adding scull%d", err, index);
	}
}

/* This function is called when the module is loaded */
static int __init ncd_start(void)
{

	int result;
	int i;
	dev_t dev = 0;

        /*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	if (ncd_major) {
		dev = MKDEV(ncd_major, ncd_minor);
		result = register_chrdev_region(dev, NUM_DEVS, DEV_NAME);
	} else {
		result = alloc_chrdev_region(&dev, ncd_minor, NUM_DEVS,
				DEV_NAME);
		ncd_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "%s: can't get major %d\n", DEV_NAME,
			ncd_major);
		return result;
	}

        /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	ncd_devices = kmalloc(NUM_DEVS * sizeof(*ncd_devices), GFP_KERNEL);
	if (!ncd_devices) {
		result = -ENOMEM;
		printk(KERN_WARNING "%s: can't allocate devices.\n",
			DEV_NAME);
		return result;
	}
	memset(ncd_devices, 0, NUM_DEVS * sizeof(*ncd_devices));

        /* Initialize each device. */
	for (i = 0; i < NUM_DEVS; i++) {
		ncd_setup_cdev(&(ncd_devices[i]), i);
	}

	printk(KERN_INFO "I was assigned major number %d.\n",
		ncd_major);
	printk(KERN_INFO "To talk to the driver, create a dev file\n");
	printk(KERN_INFO "with 'mknod /dev/%s c %d %d'.\n",
		DEV_NAME, ncd_major, ncd_minor);
	printk(KERN_INFO "Try various minor numbers from %d to %d.\n",
		ncd_minor, (ncd_minor + NUM_DEVS - 1));
	printk(KERN_INFO "Try to cat and echo to the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

	return SUCCESS;
}

/* This function is called when the module is unloaded */
static void ncd_cleanup(void)
{
	int i;
	dev_t devno = MKDEV(ncd_major, ncd_minor);

	/* Get rid of our char dev entries */
	if (ncd_devices) {
		for (i = 0; i < NUM_DEVS; i++) {
			cdev_del(&((ncd_devices[i]).cdev));
		}
		kfree(ncd_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, NUM_DEVS);
	printk(KERN_INFO "Removed %s module.\n", DEV_NAME);
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{

	struct sockaddr_in servaddr;
	struct socket* sock = NULL;
	
	static int counter = 0;
	int length = 0;
	int retval = 0;

	struct msghdr sndmsg;
	struct iovec iov;
	char buf[255];

	mm_segment_t oldfs;

	if (Device_Open) {
		return -EBUSY;
	}

	sprintf(buf, "open");

	Device_Open++;
	sprintf(msg, "I already told you %d times: Hello world!\n",
		counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	if((retval = sock_create(PF_INET, SOCK_STREAM,
						IPPROTO_TCP, &sock)) < 0){
		printk(KERN_WARNING "%s: Error creating socket.\n", DEV_NAME);
		Device_Open--;
		module_put(THIS_MODULE);
		return retval;
	}
	else {
		printk(KERN_INFO "%s: Socket created.\n", DEV_NAME);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr =
		htonl(INADDR_LOOPBACK);

	if((retval = sock->ops->connect(sock, (struct sockaddr *) &servaddr,
							sizeof(servaddr),
							O_RDWR))){
		printk(KERN_WARNING "%s: Error connecting to 0x%X.\n",
			DEV_NAME, SRVADDR);
		Device_Open--;
		module_put(THIS_MODULE);
		return retval;
	}
	else {
		printk(KERN_INFO "%s: Socket connected.\n", DEV_NAME);
	}

	sndmsg.msg_flags = 0;
	sndmsg.msg_name = NULL;
	sndmsg.msg_namelen = 0;
	sndmsg.msg_control = NULL;
	sndmsg.msg_controllen = 0;
	sndmsg.msg_iov = &iov;
	sndmsg.msg_iovlen = 1;
	
	sndmsg.msg_iov->iov_len = 255;
	sndmsg.msg_iov->iov_base = buf;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	length = sock_sendmsg(sock, &sndmsg, 255);
	set_fs(oldfs);
	if(length < 0){
		printk(KERN_WARNING "%s: Error sending. Length = %d.\n",
			DEV_NAME, length);
		Device_Open--;
		module_put(THIS_MODULE);
		return length;
	}
	else{
		printk(KERN_INFO "%s: Message sent: %s.\n", DEV_NAME, buf);
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	length = sock_recvmsg(sock, &sndmsg, 255, 0);
	set_fs(oldfs);
	if(length < 0){
		printk(KERN_WARNING "%s: Error receiving. Length = %d.\n",
			DEV_NAME, length);
		Device_Open--;
		module_put(THIS_MODULE);
		return length;
	}
	else{
		printk(KERN_INFO "%s: Received: %s\n",
			DEV_NAME, (char*) sndmsg.msg_iov->iov_base);
		printk(KERN_INFO "%s: Length: %d\n",
			DEV_NAME, length);
		printk(KERN_INFO "%s: Message received: %s.\n", DEV_NAME, buf);
	}

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;	/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*msg_Ptr == '\0') {
		return 0;
	}

	/* 
	 * Actually put the data into the buffer 
	 */
	while (length && *msg_Ptr) {

		/* 
		 * The buffer is in the user data segment, not the kernel 
		 * segment so "*" assignment won't work.  We have to use 
		 * put_user which copies data from the kernel data segment to
		 * the user data segment. 
		 */
		put_user(*(msg_Ptr++), buffer++);

		length--;
		bytes_read++;
	}

	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}
