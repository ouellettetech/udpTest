#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/netpoll.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bradley Ouellette");
MODULE_DESCRIPTION("Example UDP Module");
MODULE_VERSION("0.02");

#define DEVICE_NAME "lkm_example"
#define EXAMPLE_MSG "Hello, World3!\n"
#define BUF_LEN 80
#define INADDR_SEND ((unsigned long int)0xC0A89710) //10.0.2.15  192.168.151.16


/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int major_num;
static int device_open_count = 0;
static char msg_buffer[BUF_LEN + 1];
static char *msg_ptr;
static struct socket *sock;
static bool sock_init;
static struct sockaddr_in sin;
static struct msghdr msg;
static struct iovec iov;


/* This structure points to all of the device functions */
static struct file_operations file_ops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
	int bytes_read = 0;
	/* If we’re at the end, loop back to the beginning */
	if (*msg_ptr == 0) {
		msg_ptr = msg_buffer;
	}
	
	/* Put data in the buffer */
	while (len && *msg_ptr) {
		/* Buffer is in user data, not kernel, so you can’t just reference
		* with a pointer. The function put_user handles this for us */
		put_user(*(msg_ptr++), buffer++);
		len--;
		bytes_read++;
 	}
	return bytes_read;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) { 
    int i; 
	int error, len;
    pr_info("device_write(%p,%p,%ld)", file, buffer, len); 
 
    for (i = 0; i < len && i < BUF_LEN; i++){ 
        get_user(msg_buffer[i], buffer + i);
	}
	msg_buffer[i] = '\0';
    /* Again, return the number of input characters used. */ 
	printk(KERN_INFO "Got Message %s\n", msg_buffer);
	
	iov.iov_base = msg_buffer;
	len = strlen(msg_buffer);
	iov.iov_len = len;
	msg.msg_iovlen = len;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	error = sock_sendmsg(sock,&msg,len);
	set_fs(old_fs);

	printk(KERN_INFO "Got responce on sending the socket %d\n", error);
    return i; 
} 

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
	/* If device is open, return busy */
	if (device_open_count) {
		return -EBUSY;
	}
	device_open_count++;
	try_module_get(THIS_MODULE);
	return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
	/* Decrement the open counter and usage count. Without this, the module would not unload. */
	device_open_count--;
	module_put(THIS_MODULE);
	return 0;
}

static int __init lkm_example_init(void) {
	/* Fill buffer with our message */
	strncpy(msg_buffer, EXAMPLE_MSG, BUF_LEN);
	/* Set the msg_ptr to the buffer */
	msg_ptr = msg_buffer;
	/* Try to register character device */
	major_num = register_chrdev(0, "lkm_example", &file_ops);
	if (major_num < 0) {
		printk(KERN_ALERT "Could not register device: %d\n", major_num);
		return major_num;
	} else {
		printk(KERN_INFO "lkm_example module loaded with device major number %d\n", major_num);
		return 0;
	}

	if (sock_init == false) {
		/* Creating socket */
		error = sock_create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);
  		if (error<0) {
    		printk(KERN_DEBUG "Can't create socket. Error %d\n",error);
		}

  		/* Connecting the socket */
  		sin.sin_family = AF_INET;
  		sin.sin_port = htons(1764);
  		sin.sin_addr.s_addr = htonl(INADDR_SEND);
  		error = sock->ops->connect(sock, (struct sockaddr *)&sin, sizeof(struct sockaddr), 0);
  		if (error<0)
    		printk(KERN_DEBUG "Can't connect socket. Error %d\n",error);

  		/* Preparing message header */
  		msg.msg_flags = 0;
  		msg.msg_name = &sin;
  		msg.msg_namelen  = sizeof(struct sockaddr_in);
  		msg.msg_control = NULL;
  		msg.msg_controllen = 0;
  		msg.msg_iov = &iov;
  		msg.msg_control = NULL;
		sock_init = true;
	}
}

static void __exit lkm_example_exit(void) {
	/* Remember — we have to clean up after ourselves. Unregister the character device. */
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "Goodbye, World!\n");
}

/* Register module functions */
module_init(lkm_example_init);
module_exit(lkm_example_exit);