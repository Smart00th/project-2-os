#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/ktime.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Morgan W.");
MODULE_DESCRIPTION("Timer kernel module");

#define ENTRY_NAME "my_timer"
#define PERMS 0644
#define PARENT NULL
#define BUF_LEN 100

static struct proc_dir_entry* timer_entry;

static char msg[BUF_LEN];
static int procfs_buf_len;
struct timespec64 time;

//calculates the elapsed time (time1-time0)
struct timespec64 diff_timespec(const struct timespec64 *time1, const struct timespec64 *time0)
{
	
	struct timespec64 diff;
	diff.tv_sec = time1->tv_sec - time0->tv_sec;
	diff.tv_nsec = time1->tv_nsec - time0->tv_nsec;
	
	//if the nanoseconds are negative
	if(diff.tv_nsec < 0) {
		diff.tv_nsec += 1000000000;
		diff.tv_sec--;
	}
	return diff;
}


static ssize_t timer_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	struct timespec64 tcurrent;
	struct timespec64 tdiff;

	//current time
	ktime_get_real_ts64(&tcurrent);

	//if stored time is zero/no elapsed time, just print current time
	if(time.tv_sec == 0 && time.tv_nsec == 0)
	{
		time = tcurrent;
		sprintf(msg, "current time: %llu.%lu\n", tcurrent.tv_sec, tcurrent.tv_nsec);
	}
	//if stored time exists, print elapsed time too
	else {
		tdiff = diff_timespec(&tcurrent, &time);
		sprintf(msg, "current time: %llu.%lu\nelapsed time: %llu.%lu\n", tcurrent.tv_sec, tcurrent.tv_nsec, tdiff.tv_sec, tdiff.tv_nsec);
	
	}

	//buffer message length
	procfs_buf_len = strlen(msg);

	if(*ppos > 0 || count < procfs_buf_len)
		return 0;
	if(copy_to_user(ubuf, msg, procfs_buf_len))
		return -EFAULT;

	time = tcurrent;
	*ppos = procfs_buf_len;

	return procfs_buf_len;
}

static const struct proc_ops timer_fops = {
	.proc_read = timer_read
};

static int __init timer_init(void)
{
	//creates /proc
	timer_entry = proc_create(ENTRY_NAME, PERMS, PARENT, &timer_fops);
	if(!timer_entry) {
		printk(KERN_ERR "Failed to create /proc/%s entry\n", ENTRY_NAME);
		return -ENOMEM;
	}

	//initialize stored time
	time.tv_sec = 0;
	time.tv_nsec = 0;

	return 0;
}


static void __exit timer_exit(void)
{
	proc_remove(timer_entry);
	return;
}

module_init(timer_init);
module_exit(timer_exit);
