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
// These are kernel headers - different from regular C programs
#include <linux/init.h>     // For __init and __exit macros
#include <linux/kernel.h>   // For kernel functions like printk()
#include <linux/module.h>   // Core module functionality

// This function runs when we load the module into kernel
// Think of it as the "main()" function for kernel modules
int simple_init(void) {
    // printk() is the kernel version of printf()
    // KERN_INFO sets the message priority level (like severity)
    // Output goes to kernel log buffer, not regular terminal
    printk(KERN_INFO "Loading Kernel Module\n");
    
    // Return 0 means "success, module loaded properly"
    // Any other number means "error, don't load this module"
    return 0;
}

// This function runs when we remove the module from kernel
// It's like a cleanup function - runs when we do "rmmod"
void simple_exit(void) {
    // Print message to kernel log saying we're leaving
    printk(KERN_INFO "Removing Kernel Module\n");
    // void means we don't return anything - can't fail to unload
}

// These macros tell the kernel which functions to call
// when loading/unloading the module
module_init(simple_init);   // "When loading, call simple_init()"
module_exit(simple_exit);   // "When unloading, call simple_exit()"

// Module metadata - not required but good practice
MODULE_LICENSE("GPL");                    // Software license type
MODULE_DESCRIPTION("Simple Module");      // What this module does
MODULE_AUTHOR("SGG");                    // Who wrote it
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
// Additional headers for special kernel functions
#include <linux/hash.h>    // Contains GOLDEN_RATIO_PRIME constant
#include <linux/gcd.h>     // Contains gcd() function (greatest common divisor)
#include <linux/jiffies.h> // Contains jiffies variable (timer counter)
#include <asm/param.h>     // Contains HZ constant (timer frequency)

int simple_init(void) {
    printk(KERN_INFO "Loading Kernel Module\n");
    
    // GOLDEN_RATIO_PRIME is a special constant only available in kernel
    // %lu means "print as unsigned long integer"
    printk(KERN_INFO "Golden Ratio Prime: %lu\n", GOLDEN_RATIO_PRIME);
    
    // jiffies = number of timer interrupts since system booted
    // HZ = how many timer interrupts happen per second
    // For example: if HZ=100, timer interrupts 100 times per second
    printk(KERN_INFO "Jiffies: %lu, HZ: %d\n", jiffies, HZ);
    
    return 0;
}

void simple_exit(void) {
    printk(KERN_INFO "Removing Kernel Module\n");
    
    // gcd() function finds greatest common divisor of two numbers
    // This function only exists in kernel space, not user space
    printk(KERN_INFO "GCD of 3300 and 24: %lu\n", gcd(3300, 24));
    
    // Print jiffies again - compare with init value to see time passed
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
// Standard kernel module headers
#include <linux/init.h>     // For init/exit macros
#include <linux/kernel.h>   // For kernel functions
#include <linux/module.h>   // For module functionality
#include <linux/proc_fs.h>  // For /proc filesystem functions
#include <asm/uaccess.h>    // For copy_to_user() function

#define BUFFER_SIZE 128     // Size of our text buffer
#define PROC_NAME "hello"   // Name of our /proc file (/proc/hello)

// This function gets called every time someone reads /proc/hello
// Like when you type "cat /proc/hello"
ssize_t proc_read(struct file *file, char __user *usr_buf, 
                  size_t count, loff_t *pos) {
    
    int rv = 0;                    // Return value (bytes written)
    char buffer[BUFFER_SIZE];      // Buffer to hold our text (in kernel memory)
    static int completed = 0;      // Flag to prevent infinite reading
    
    // This logic prevents the function from running forever
    // Without this, "cat /proc/hello" would never stop!
    if (completed) {
        completed = 0;    // Reset for next read
        return 0;         // Return 0 = "end of file"
    }
    
    completed = 1;        // Mark as completed for this read
    
    // Write our message into the kernel buffer
    // sprintf works just like regular sprintf
    rv = sprintf(buffer, "Hello World\n");
    
    // CRITICAL: Must copy data from kernel space to user space
    // buffer = kernel memory, usr_buf = user program memory
    // User programs can't directly access kernel memory!
    copy_to_user(usr_buf, buffer, rv);
    
    // Return number of bytes we wrote
    return rv;
}

// This structure tells kernel what functions to call for file operations
// We only care about reading, so we only set .read
static struct file_operations proc_ops = {
    .owner = THIS_MODULE,    // Who owns this (our module)
    .read = proc_read,       // Function to call when file is read
};

// Called when module loads - create the /proc file
int proc_init(void) {
    // proc_create() creates a new file in /proc filesystem
    // Parameters: name, permissions, parent_directory, operations_struct
    proc_create(PROC_NAME, 0666, NULL, &proc_ops);
    return 0;
}

// Called when module unloads - clean up the /proc file
void proc_exit(void) {
    // Remove our /proc file when module is removed
    // Otherwise it would be a "zombie" file
    remove_proc_entry(PROC_NAME, NULL);
}

// Register our init/exit functions
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

**Solution Framework:**
```c
// You'll need these headers
#include <linux/jiffies.h>  // For jiffies variable

ssize_t proc_read(struct file *file, char __user *usr_buf, 
                  size_t count, loff_t *pos) {
    // ... same completion logic as hello.c ...
    
    // Instead of "Hello World", print current jiffies
    // jiffies changes constantly, so you get current value each time
    rv = sprintf(buffer, "%lu\n", jiffies);
    
    // ... rest same as hello.c ...
}
```

**Hints:**
- Copy the hello.c structure exactly
- Only change the sprintf line to print jiffies instead of "Hello World"
- Change PROC_NAME to "jiffies"

### Assignment 2: /proc/seconds Module  
Create a module that shows elapsed seconds since module loaded:

```bash
# When loaded, this should work:
cat /proc/seconds
# Output: number of seconds since module loaded
```

**Solution Framework:**
```c
#include <linux/jiffies.h>  // For jiffies variable
#include <asm/param.h>      // For HZ constant

// Global variable to store jiffies when module first loaded
static unsigned long start_jiffies;

int proc_init(void) {
    // Save current jiffies when module loads
    start_jiffies = jiffies;
    
    // Create /proc/seconds file
    proc_create("seconds", 0666, NULL, &proc_ops);
    return 0;
}

ssize_t proc_read(struct file *file, char __user *usr_buf, 
                  size_t count, loff_t *pos) {
    // ... same completion logic ...
    
    // Calculate elapsed time:
    // (current_jiffies - start_jiffies) = jiffies elapsed
    // jiffies_elapsed / HZ = seconds elapsed
    unsigned long elapsed_jiffies = jiffies - start_jiffies;
    unsigned long elapsed_seconds = elapsed_jiffies / HZ;
    
    rv = sprintf(buffer, "%lu\n", elapsed_seconds);
    
    // ... rest same ...
}
```

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