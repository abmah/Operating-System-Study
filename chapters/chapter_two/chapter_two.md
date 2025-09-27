# Linux Kernel Module Development Guide

## What Are Kernel Modules?
Think of kernel modules as small programs that can be plugged into the Linux kernel while it's running. They're like apps for your operating system's core.

## Part 1: Basic Module Creation

### Step 1: Check Current Modules
```bash
# See what modules are currently loaded
lsmod
```
This shows: module name, size, and what's using it.

### Step 2: Create Your First Module (simple.c)
```c
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

// Called when module loads
int simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");
    return 0;  // 0 = success
}

// Called when module unloads
void simple_exit(void) {
    printk(KERN_INFO "Removing Kernel Module\n");
}

// Register entry/exit points
module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");
```

**Key Points:**
- `printk()` is like `printf()` but for kernel - output goes to kernel log
- `KERN_INFO` sets message priority level
- Return 0 from init = success, anything else = failure

### Step 3: Compile and Test
```bash
# Compile the module
make

# Load the module (creates simple.ko file)
sudo insmod simple.ko

# Check if it loaded
lsmod | grep simple

# See kernel messages
dmesg

# Remove the module
sudo rmmod simple

# Clear kernel log buffer (optional)
sudo dmesg -c
```

## Part 2: Enhanced Module Features

### Add Kernel-Only Functions
Modify your `simple_init()` and `simple_exit()`:

```c
#include <linux/hash.h>   // For GOLDEN_RATIO_PRIME
#include <linux/gcd.h>    // For gcd() function
#include <linux/jiffies.h> // For jiffies
#include <asm/param.h>    // For HZ

int simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");
    
    // Print golden ratio prime
    printk(KERN_INFO "Golden Ratio Prime: %lu\n", GOLDEN_RATIO_PRIME);
    
    // Print timer info
    printk(KERN_INFO "Jiffies: %lu, HZ: %d\n", jiffies, HZ);
    
    return 0;
}

void simple_exit(void) {
    printk(KERN_INFO "Removing Kernel Module\n");
    
    // Print GCD of 3300 and 24
    printk(KERN_INFO "GCD of 3300 and 24: %lu\n", gcd(3300, 24));
    
    // Print current jiffies
    printk(KERN_INFO "Jiffies at exit: %lu\n", jiffies);
}
```

**Understanding jiffies and HZ:**
- `HZ` = timer frequency (e.g., 100 = 100 interrupts/second)
- `jiffies` = timer interrupts since boot
- Time elapsed = (jiffies_end - jiffies_start) / HZ seconds

## Part 3: Creating /proc Files

### What is /proc?
The `/proc` filesystem is virtual - files exist only in memory and show kernel/process info.

### Example: Hello World /proc Entry (hello.c)
```c
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define BUFFER_SIZE 128
#define PROC_NAME "hello"

// Function called when /proc/hello is read
ssize_t proc_read(struct file *file, char __user *usr_buf, 
                  size_t count, loff_t *pos) {
    int rv = 0;
    char buffer[BUFFER_SIZE];
    static int completed = 0;
    
    if (completed) {
        completed = 0;
        return 0;  // EOF
    }
    
    completed = 1;
    rv = sprintf(buffer, "Hello World\n");
    
    // Copy from kernel space to user space
    copy_to_user(usr_buf, buffer, rv);
    
    return rv;
}

static struct file_operations proc_ops = {
    .owner = THIS_MODULE,
    .read = proc_read,
};

int proc_init(void) {
    // Create /proc/hello entry
    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    return 0;
}

void proc_exit(void) {
    // Remove /proc/hello entry
    remove_proc_entry(PROC_NAME, NULL);
}

module_init(proc_init);
module_exit(proc_exit);
```

### Test the /proc Entry
```bash
# Compile and load
make
sudo insmod hello.ko

# Read the /proc file
cat /proc/hello

# Should output: Hello World

# Remove module
sudo rmmod hello
```

## Part 4: Your Assignments

### Assignment 1: /proc/jiffies Module
Create a module that shows current jiffies value:

```bash
# When loaded, this should work:
cat /proc/jiffies
# Output: current jiffies value
```

**Hints:**
- Modify the `proc_read()` function to print jiffies instead of "Hello World"
- Use `sprintf(buffer, "%lu\n", jiffies);`

### Assignment 2: /proc/seconds Module  
Create a module that shows elapsed seconds since module loaded:

```bash
# When loaded, this should work:
cat /proc/seconds
# Output: number of seconds since module loaded
```

**Hints:**
- Store `jiffies` value at module load time
- Calculate elapsed time: `(current_jiffies - start_jiffies) / HZ`
- This gives you seconds elapsed

## Terminal Workflow Summary

```bash
# 1. Create your .c file
nano simple.c

# 2. Compile
make

# 3. Load module
sudo insmod simple.ko

# 4. Test functionality
lsmod | grep simple
dmesg | tail
cat /proc/your_entry  # if creating /proc files

# 5. Remove module
sudo rmmod simple

# 6. Check removal
dmesg | tail
```

## Key Concepts
- **Kernel Space vs User Space**: Modules run in kernel space with higher privileges
- **printk()**: Kernel's version of printf(), output goes to kernel log
- **jiffies**: Global counter of timer interrupts since boot
- **HZ**: Timer frequency (interrupts per second)
- **/proc filesystem**: Virtual files that show kernel/system information
- **copy_to_user()**: Required to safely copy data from kernel to user space

Remember: Always compile frequently with `make` and test with `dmesg` after loading/removing modules!