#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cop4610t");
MODULE_DESCRIPTION("Example of kernel module for timer");

#define ENTRY_NAME "timer"
#define PERMS 0644
#define PARENT NULL

static struct proc_dir_entry* proc_timer;
static struct timespec64 prev_time = {0};
char buf[256];
int len = 0;
int thread_proc_open(struct inode *sp_inode, struct file *sp_file) {
    struct timespec64 ts_now;

    ktime_get_real_ts64(&ts_now);
    
    if(prev_time.tv_sec != 0 || prev_time.tv_nsec != 0){
        struct timespec64 elapsed;
        elapsed.tv_sec = ts_now.tv_sec - prev_time.tv_sec;
        elapsed.tv_nsec = ts_now.tv_nsec - prev_time.tv_nsec;
        if(elapsed.tv_nsec < 0){
         elapsed.tv_sec--;
         elapsed.tv_nsec += 1000000000;
        } 
        len = snprintf(buf, sizeof(buf), "current time: %lld.%09ld\n elapsed time: %lld.%09ld\n", 
        (long long)ts_now.tv_sec, ts_now.tv_nsec, elapsed.tv_sec, elapsed.tv_nsec);
     }
    
     if(prev_time.tv_sec == 0 && prev_time.tv_nsec == 0){
        len = snprintf(buf, sizeof(buf), "current time: %lld.%09ld\n", (long long)ts_now.tv_sec, ts_now.tv_nsec);
    }
    prev_time = ts_now;
    return 0;
}
static ssize_t timer_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    return simple_read_from_buffer(ubuf, count, ppos, buf, len); // better than copy_from_user
}

static const struct proc_ops timer_fops = {
    .proc_open = thread_proc_open,
    .proc_read = timer_read,
};

static int __init timer_init(void)
{
    proc_timer = proc_create(ENTRY_NAME, PERMS, PARENT, &timer_fops);
    prev_time.tv_sec = 0;
    prev_time.tv_nsec = 0;
    if (!proc_timer) {
        return -ENOMEM;
    }
    return 0;
}

static void __exit timer_exit(void)
{
    proc_remove(proc_timer);
}

module_init(timer_init);
module_exit(timer_exit);
