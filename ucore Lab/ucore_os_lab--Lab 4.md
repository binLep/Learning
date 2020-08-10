## 练习0：填写已有实验

以下操作都是将**内容**复制到 lab4 里，**不要复制整个文件！！**

将 lab1 的 `kern/debug/kdebug.c`、`kern/init/init.c` 以及  `kern/trap/trap.c` 复制到 lab4 里

再将 lab2 的 `kern/mm/pmm.c` 和 `kern/mm/default_pmm.c` 复制到 lab4 里

最后将 lab3 的 `kern/mm/vmm.c` 和 `kern/mm/swap_fifo.c` 复制到 lab4 里

## 练习1：分配并初始化一个进程控制块

### alloc_proc 函数源码

写于：**kern/process/proc.c**

```c
// alloc_proc - alloc a proc_struct and init all fields of proc_struct
static struct proc_struct *
alloc_proc(void) {
    struct proc_struct *proc = kmalloc(sizeof(struct proc_struct));
    if (proc != NULL) {
    //LAB4:EXERCISE1 YOUR CODE
    /*
     * below fields in proc_struct need to be initialized
     *       enum proc_state state;                      // Process state
     *       int pid;                                    // Process ID
     *       int runs;                                   // the running times of Proces
     *       uintptr_t kstack;                           // Process kernel stack
     *       volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU?
     *       struct proc_struct *parent;                 // the parent process
     *       struct mm_struct *mm;                       // Process's memory management field
     *       struct context context;                     // Switch here to run process
     *       struct trapframe *tf;                       // Trap frame for current interrupt
     *       uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)
     *       uint32_t flags;                             // Process flag
     *       char name[PROC_NAME_LEN + 1];               // Process name
     */
    }
    return proc;
}
```

### alloc_proc 函数答案

照着注释里给的元素一个个填就行

相比于视频里给的元素来讲，这个函数的注释里没有 `list_link` 和 `hash_link` 元素，也就是这俩不需要初始化

```c
static struct proc_struct *
alloc_proc(void) {
    struct proc_struct *proc = kmalloc(sizeof(struct proc_struct));
    if (proc != NULL) {
        proc->pid = -1;                                       // 进程ID
        memset(&(proc->name), 0, PROC_NAME_LEN);              // 进程名
        proc->state = PROC_UNINIT;                            // 进程状态
        proc->runs = 0;                                       // 进程时间片
        proc->need_resched = 0;                               // 进程是否能被调度
        proc->flags = 0;                                      // 标志位
        proc->kstack = 0;                                     // 进程所使用的内存栈地址
        proc->cr3 = boot_cr3;                                 // 将页目录表地址设为内核页目录表基址
        proc->mm = NULL;                                      // 进程所用的虚拟内存
        memset(&(proc->context), 0, sizeof(struct context));  // 进程的上下文
        proc->tf = NULL;                                      // 中断帧指针
        proc->parent = NULL;                                  // 父进程
    }
    return proc;
}
```

## 练习2：为新创建的内核线程分配资源

### do_fork 函数源码

写于：**kern/process/proc.c**

```c
/* do_fork -     parent process for a new child process
 * @clone_flags: used to guide how to clone the child process
 * @stack:       the parent's user stack pointer. if stack==0, It means to fork a kernel thread.
 * @tf:          the trapframe info, which will be copied to child process's proc->tf
 */
int
do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    int ret = -E_NO_FREE_PROC;
    struct proc_struct *proc;
    if (nr_process >= MAX_PROCESS) {
        goto fork_out;
    }
    ret = -E_NO_MEM;
    //LAB4:EXERCISE2 YOUR CODE
    /*
     * Some Useful MACROs, Functions and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   alloc_proc:   create a proc struct and init fields (lab4:exercise1)
     *   setup_kstack: alloc pages with size KSTACKPAGE as process kernel stack
     *   copy_mm:      process "proc" duplicate OR share process "current"'s mm according clone_flags
     *                 if clone_flags & CLONE_VM, then "share" ; else "duplicate"
     *   copy_thread:  setup the trapframe on the  process's kernel stack top and
     *                 setup the kernel entry point and stack of process
     *   hash_proc:    add proc into proc hash_list
     *   get_pid:      alloc a unique pid for process
     *   wakeup_proc:  set proc->state = PROC_RUNNABLE
     * VARIABLES:
     *   proc_list:    the process set's list
     *   nr_process:   the number of process set
     */

    //    1. call alloc_proc to allocate a proc_struct
    //    2. call setup_kstack to allocate a kernel stack for child process
    //    3. call copy_mm to dup OR share mm according clone_flag
    //    4. call copy_thread to setup tf & context in proc_struct
    //    5. insert proc_struct into hash_list && proc_list
    //    6. call wakeup_proc to make the new child process RUNNABLE
    //    7. set ret vaule using child proc's pid
fork_out:
    return ret;

bad_fork_cleanup_kstack:
    put_kstack(proc);
bad_fork_cleanup_proc:
    kfree(proc);
    goto fork_out;
}
```

### E_NO_FREE_PROC 宏

写于：**libs/error.h**

```c
#define E_NO_FREE_PROC      5   // Attempt to create a new process beyond
```

这个意思是再申请一个新进程的话就会超过规定的进程总数

### MAX_PROCESS 宏

写于：**kern/process/proc.h**

```c
#define MAX_PROCESS                 4096
```

规定最大的进程总数为 0x1000

### E_NO_MEM 宏

写于：**libs/error.h**

```c
#define E_NO_MEM            4   // Request failed due to memory shortage
```

由于内存不足，请求失败

### setup_kstack 函数

写于：**kern/process/proc.c**

```c
// setup_kstack - alloc pages with size KSTACKPAGE as process kernel stack
static int
setup_kstack(struct proc_struct *proc) {
    struct Page *page = alloc_pages(KSTACKPAGE);   // 申请 2 个连续的物理页
    if (page != NULL) {                            // 如果能获取到物理页地址
        proc->kstack = (uintptr_t)page2kva(page);  // 用 kstack 记录物理页的虚拟地址
        return 0;
    }
    return -E_NO_MEM;                              // 否则返回 -4，表示内存不足，申请失败
}
```

该函数的作用就是创建内核堆栈，会申请两个连续的物理页

#### KSTACKPAGE 宏

写于：**kern/mm/memlayout.h**

```c
#define KSTACKPAGE          2                           // # of pages in kernel stack
```

规定内核堆栈中页的数目

### kfree 函数

写于：**kern/mm/kmalloc.c**

```c
void kfree(void *block)
{
	bigblock_t *bb, **last = &bigblocks;
	unsigned long flags;

	if (!block)
		return;

	if (!((unsigned long)block & (PAGE_SIZE-1))) {
		/* might be on the big block list */
		spin_lock_irqsave(&block_lock, flags);
		for (bb = bigblocks; bb; last = &bb->next, bb = bb->next) {
			if (bb->pages == block) {
				*last = bb->next;
				spin_unlock_irqrestore(&block_lock, flags);
				__slob_free_pages((unsigned long)block, bb->order);
				slob_free(bb, sizeof(bigblock_t));
				return;
			}
		}
		spin_unlock_irqrestore(&block_lock, flags);
	}

	slob_free((slob_t *)block - 1, 0);
	return;
}
```

具体内容以后分析，看函数名可知该函数的作用是释放内核堆块

### copy_mm 函数

写于：**kern/process/proc.c**

```c
// copy_mm - process "proc" duplicate OR share process "current"'s mm according clone_flags
//         - if clone_flags & CLONE_VM, then "share" ; else "duplicate"
static int
copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
    assert(current->mm == NULL);
    /* do nothing in this project */
    return 0;
}
```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



### 

写于：

```c

```



sd



