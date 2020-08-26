## 练习0：填写已有实验

以下操作都是将**内容**复制到 lab6 里（都可以从 lab4 复制）**不要复制整个文件！！**

将 lab5 的 `kern/debug/kdebug.c`、`kern/init/init.c` 以及  `kern/trap/trap.c` 复制到 lab6 里

将 lab5 的 `kern/mm/pmm.c` 和 `kern/mm/default_pmm.c` 复制到 lab6 里

将 lab5 的 `kern/mm/vmm.c` 和 `kern/mm/swap_fifo.c` 复制到 lab6 里

最后将 lab5 的 `kern/process/proc.c` 复制到 lab6 里，之后还要改进代码

### trap_dispatch 函数源码

写于：**kern/trap/trap.c**

```c
static void
trap_dispatch(struct trapframe* tf){
    ......
    case IRQ_OFFSET + IRQ_TIMER:
#if 0
        LAB3 : If some page replacement algorithm(such as CLOCK PRA) need tick to change the priority of pages,
            then you can add code here.
#endif
            /* LAB1 YOUR CODE : STEP 3 */
            /* handle the timer interrupt */
            /* (1) After a timer interrupt, you should record this event using a global variable (increase it), such as ticks in kern/driver/clock.c
             * (2) Every TICK_NUM cycle, you can print some info using a funciton, such as print_ticks().
             * (3) Too Simple? Yes, I think so!
             */
             /* LAB5 YOUR CODE */
             /* you should upate you lab1 code (just add ONE or TWO lines of code):
              *    Every TICK_NUM cycle, you should set current process's current->need_resched = 1
              */
              /* LAB6 YOUR CODE */
              /* you should upate you lab5 code
               * IMPORTANT FUNCTIONS:
               * sched_class_proc_tick
               */
            break;
    ......
}
```

### trap_dispatch 函数答案

写于：**kern/trap/trap.c**

```c
static void
trap_dispatch(struct trapframe *tf) {
    ......
    case IRQ_OFFSET + IRQ_TIMER:
        ticks++;
        assert(current != NULL);
        sched_class_proc_tick(current);
        break;
    ......
}
```

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
     //LAB5 YOUR CODE : (update LAB4 steps)
    /*
     * below fields(add in LAB5) in proc_struct need to be initialized	
     *       uint32_t wait_state;                        // waiting state
     *       struct proc_struct *cptr, *yptr, *optr;     // relations between processes
	 */
     //LAB6 YOUR CODE : (update LAB5 steps)
    /*
     * below fields(add in LAB6) in proc_struct need to be initialized
     *     struct run_queue *rq;                       // running queue contains Process
     *     list_entry_t run_link;                      // the entry linked in run queue
     *     int time_slice;                             // time slice for occupying the CPU
     *     skew_heap_entry_t lab6_run_pool;            // FOR LAB6 ONLY: the entry in the run pool
     *     uint32_t lab6_stride;                       // FOR LAB6 ONLY: the current stride of the process
     *     uint32_t lab6_priority;                     // FOR LAB6 ONLY: the priority of process, set by lab6_set_priority(uint32_t)
     */
    }
    return proc;
}
```

### alloc_proc 函数答案

写于：**kern/process/proc.c**

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
        proc->parent = NULL;                                  // 该进程的父进程
        proc->wait_state = 0;                                 // 等待状态的标志位
        proc->cptr = NULL;                                    // 该进程的子进程
        proc->yptr = NULL;                                    // 该进程的弟进程
        proc->optr = NULL;                                    // 该进程的兄进程
        proc->rq = NULL;                                      // Lab6：当前进程在运行队列中的指针
        list_init(&(proc->run_link));                         // Lab6：运行队列的指针
        proc->time_slice = 0;                                 // Lab6：占用 CPU 的时间片
        proc->lab6_run_pool.left = NULL;                      // Lab6：运行池中的条目
        proc->lab6_run_pool.right = NULL;                     // Lab6：运行池中的条目
        proc->lab6_run_pool.parent = NULL;                    // Lab6：运行池中的条目
        proc->lab6_stride = 0;                                // Lab6：步进值
        proc->lab6_priority = 0;                              // Lab6：优先级 (和步进值成反比)
    }
    return proc;
}
```

## 练习1: 使用 Round Robin 调度算法

大致流程如下：

`copy_mm → lock_mm → lock → schedule`

之后再调用关于 RR 算法来进行调度，先了解一下有关的结构体

### run_queue 结构体

写于：**kern/sched/sched.h**

```c
struct run_queue {
    list_entry_t run_list;  // 双向链表的头节点
    unsigned int proc_num;  // 就绪态进程的个数
    int max_time_slice;     // 最大时间片
    skew_heap_entry_t *lab6_run_pool;
};
```

### copy_mm 函数

写于：**kern/process/proc.c**

```c
// copy_mm - process "proc" duplicate OR share process "current"'s mm according clone_flags
//         - if clone_flags & CLONE_VM, then "share" ; else "duplicate"
static int
copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
    struct mm_struct *mm, *oldmm = current->mm;

    /* current is a kernel thread */
    if (oldmm == NULL) {
        return 0;
    }
    if (clone_flags & CLONE_VM) {
        mm = oldmm;
        goto good_mm;
    }

    int ret = -E_NO_MEM;
    if ((mm = mm_create()) == NULL) {
        goto bad_mm;
    }
    if (setup_pgdir(mm) != 0) {
        goto bad_pgdir_cleanup_mm;
    }

    lock_mm(oldmm);
    {
        ret = dup_mmap(mm, oldmm);
    }
    unlock_mm(oldmm);

    if (ret != 0) {
        goto bad_dup_cleanup_mmap;
    }

good_mm:
    mm_count_inc(mm);
    proc->mm = mm;
    proc->cr3 = PADDR(mm->pgdir);
    return 0;
bad_dup_cleanup_mmap:
    exit_mmap(mm);
    put_pgdir(mm);
bad_pgdir_cleanup_mm:
    mm_destroy(mm);
bad_mm:
    return ret;
}
```

### lock_mm 函数

写于：**kern/mm/vmm.h**

```c
static inline void
lock_mm(struct mm_struct *mm) {
    if (mm != NULL) {
        lock(&(mm->mm_lock));
    }
}
```

### lock 函数

写于：**kern/sync/sync.c**

```c
static inline void
lock(lock_t *lock) {
    while (!try_lock(lock)) {
        schedule();
    }
}
```

### schedule 函数

写于：**kern/schedule/sched.c**

```c
void
schedule(void) {
    bool intr_flag;
    struct proc_struct *next;
    local_intr_save(intr_flag);
    {
        current->need_resched = 0;
        if (current->state == PROC_RUNNABLE) {
            sched_class_enqueue(current);
        }
        if ((next = sched_class_pick_next()) != NULL) {
            sched_class_dequeue(next);
        }
        if (next == NULL) {
            next = idleproc;
        }
        next->runs ++;
        if (next != current) {
            proc_run(next);
        }
    }
    local_intr_restore(intr_flag);
}
```

### sched_class_enqueue 函数

写于：**kern/schedule/sched.c**

```c
static inline void
sched_class_enqueue(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->enqueue(rq, proc);
    }
}
```

### sched_class_dequeue 函数

写于：**kern/schedule/sched.c**

```c
static inline void
sched_class_dequeue(struct proc_struct *proc) {
    sched_class->dequeue(rq, proc);
}
```

### sched_class_pick_next 函数

写于：**kern/schedule/sched.c**

```c
static inline struct proc_struct *
sched_class_pick_next(void) {
    return sched_class->pick_next(rq);
}
```

### sched_class_proc_tick 函数

写于：**kern/schedule/sched.c**

```c
void
sched_class_proc_tick(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->proc_tick(rq, proc);  // 进入 RR 调度
    }
    else {
        proc->need_resched = 1;  // 代表该进程应该被换出去
    }
}
```

在时钟中断时会触发这个函数的调用，在 RR 调度中，每次调用都会减少当前进程时间片

### RR_init 函数

写于：**kern/schedule/default_sched.c**

```c
static void
RR_init(struct run_queue *rq) {
    list_init(&(rq->run_list));  // 创建 RR 算法的双向链表
    rq->proc_num = 0;            // 初始化就绪态进程的个数
}
```

### RR_enqueue 函数

写于：**kern/schedule/default_sched.c**

```c
static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    assert(list_empty(&(proc->run_link)));  // 确认双向链表是否为空
    list_add_before(&(rq->run_list), &(proc->run_link));  // 将元素放入队列中
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;  // 初始化进程的时间片
    }
    proc->rq = rq;  // 更新
    rq->proc_num ++;
}
```

### RR_dequeue 函数

写于：**kern/schedule/default_sched.c**

```c
static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
    list_del_init(&(proc->run_link));
    rq->proc_num --;
}
```

### RR_pick_next 函数

写于：**kern/schedule/default_sched.c**

```c
static struct proc_struct *
RR_pick_next(struct run_queue *rq) {
    list_entry_t *le = list_next(&(rq->run_list));
    if (le != &(rq->run_list)) {
        return le2proc(le, run_link);
    }
    return NULL;
}
```

### RR_proc_tick 函数

写于：**kern/schedule/default_sched.c**

```c
static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;  // 代表该进程应该被换出去
    }
}
```

时间片没用完就减小时间片的大小，用完了就标记出该进程是需要被调度的进程

### default_sched_class 结构体变量

写于：**kern/schedule/default_sched.c**

```c
struct sched_class default_sched_class = {
    .name = "RR_scheduler",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .pick_next = RR_pick_next,
    .proc_tick = RR_proc_tick,
};
```

