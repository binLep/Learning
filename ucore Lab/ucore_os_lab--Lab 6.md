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
        proc->lab6_stride = 0;                                // Lab6：步长
        proc->lab6_priority = 0;                              // Lab6：优先级 (和步长成反比)
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
    skew_heap_entry_t *lab6_run_pool;  // 初始化该进程在优先队列中的节点
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

    lock_mm(oldmm);    // 加锁
    {
        ret = dup_mmap(mm, oldmm);
    }
    unlock_mm(oldmm);  // 解锁

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

写于：**kern/sync/sync.h**

```c
static inline void
lock(lock_t *lock) {
    while (!try_lock(lock)) {
        schedule();
    }
}
```

如果该位是加锁的话，那么就进入调度算法

否则就继续执行

### try_lock 函数

写于：**kern/sync/sync.h**

```c
static inline bool
try_lock(lock_t *lock) {
    return !test_and_set_bit(0, lock);
}
```

测试该位是否为 1，为 1 说明该位已经加锁

### test_and_set_bit 函数

写于：**libs/atomic.h**

```c
/* *
 * test_and_set_bit - Atomically set a bit and return its old value
 * @nr:     the bit to set
 * @addr:   the address to count from
 * */
static inline bool
test_and_set_bit(int nr, volatile void *addr) {
    int oldbit;
    asm volatile ("btsl %2, %1; sbbl %0, %0" : "=r" (oldbit), "=m" (*(volatile long *)addr) : "Ir" (nr) : "memory");
    return oldbit != 0;
}
```

将 nr 位设置为 1，然后测试改位是否为 1 ，为 1 则说明该位已经加锁

### unlock_mm 函数

写于：**kern/mm/vmm.h**

```c
static inline void
unlock_mm(struct mm_struct *mm) {
    if (mm != NULL) {
        unlock(&(mm->mm_lock));
    }
}
```

### unlock 函数

写于：**kern/sync/sync.h**

```c
static inline void
unlock(lock_t *lock) {
    if (!test_and_clear_bit(0, lock)) {
        panic("Unlock failed.\n");
    }
}
```

测试该位是否为 0，为 0 说明该位已经解锁

### test_and_clear_bit 函数

写于：**libs/atomic.h**

```c
/* *
 * test_and_clear_bit - Atomically clear a bit and return its old value
 * @nr:     the bit to clear
 * @addr:   the address to count from
 * */
static inline bool
test_and_clear_bit(int nr, volatile void *addr) {
    int oldbit;
    asm volatile ("btrl %2, %1; sbbl %0, %0" : "=r" (oldbit), "=m" (*(volatile long *)addr) : "Ir" (nr) : "memory");
    return oldbit != 0;
}
```

bt 的作用是把 \*addr 的第 nr 位复制到 cf，btrl 在执行 bt 命令的同时，把操作数的指定位置设为 0

所以这个函数是用来将 \*addr 的第 nr 位的值设置为 0 的

**sbb 与 sub 的区别：**

sub ax, bx 的结果是 ax - bx

sbb ax, bx 的结果是 ax - bx - cf（进/借位标志）



这里 oldbit 初始化的时候不用管值是多少，最后经过 sbbl 汇编，只会有两个值

一个是 0，一个是 \-1

那么最后检查 oldbit 的值是否为 0，若 oldbit 的值为  0，则 \*addr 第 nr 位的值为 0，否则为 1

也就是说，这个函数返回 true 的话，该测试位上的值就为 0，反之则为 1



这个函数的主要作用就是先将对应位置上的数清零，然后检查该位是否清零了

### schedule 函数

写于：**kern/schedule/sched.c**

```c
void
schedule(void) {
    bool intr_flag;
    struct proc_struct *next;
    local_intr_save(intr_flag);
    {
        current->need_resched = 0;  // 设置当前进程不需要再次被调度，应该是防止并发
        if (current->state == PROC_RUNNABLE) {  // 如果当前进程的状态是执行状态
            sched_class_enqueue(current);       // 将该进程加入到等待队列的队头
        }
        if ((next = sched_class_pick_next()) != NULL) {  // 从队尾拿出一个进程
            sched_class_dequeue(next);
        }
        if (next == NULL) {
            next = idleproc;    // next 进程设置为 idleproc
        }
        next->runs ++;          // 下一个进程的时间片加一
        if (next != current) {  // 如果下一个进程不等于当前进程，那么就运行下一个进程
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

将进程加入到等待队列中

### sched_class_dequeue 函数

写于：**kern/schedule/sched.c**

```c
static inline void
sched_class_dequeue(struct proc_struct *proc) {
    sched_class->dequeue(rq, proc);
}
```

将进程从等待队列中剔除

### sched_class_pick_next 函数

写于：**kern/schedule/sched.c**

```c
static inline struct proc_struct *
sched_class_pick_next(void) {
    return sched_class->pick_next(rq);
}
```

从队尾拿出一个进程

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

将进程从等待队列中剔除，将等待中的进程数减一

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

从队尾选出一个进程，该函数就是找是否有就绪进程存在，有就去执行它

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

## 练习2: 实现 Stride Scheduling 调度算法

Stride 算法的原理是对于每一个进程，有一个 stride 值和一个 pass 值

每次进行调度时，选择 stride 最小的进程运行，并将这个进程的 stride 加上 pass

pass 越小那么被调度的次数就会越多。在实验中，pass 依托优先级实现

优先级越大，pass 即用一个大常数除以优先级得到的值就越小，也就意味着被调度的次数越多



该实验使用的数据结构是优先队列中的最小堆，实验具体实现如下

- 首先，需要设置一个大整数，将来的所有pass值都由这个大整数除以进程的优先级得到：

```c
#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include <default_sched.h>

#define USE_SKEW_HEAP 1

/* You should define the BigStride constant here*/
/* LAB6: YOUR CODE */
#define BIG_STRIDE    0x7FFFFFFF /* ??? */
```

- 初始化时，需要将列表、斜堆清空，并置运行列表中的进程数为 0

```c
/* The compare function for two skew_heap_node_t's and the
 * corresponding procs*/
static int
proc_stride_comp_f(void *a, void *b)
{
     struct proc_struct *p = le2proc(a, lab6_run_pool);
     struct proc_struct *q = le2proc(b, lab6_run_pool);
     int32_t c = p->lab6_stride - q->lab6_stride;
     if (c > 0) return 1;
     else if (c == 0) return 0;
     else return -1;
}
```

- 运行队列初始化

```c
/*
 * stride_init initializes the run-queue rq with correct assignment for
 * member variables, including:
 *
 *   - run_list: should be a empty list after initialization.
 *   - lab6_run_pool: NULL
 *   - proc_num: 0
 *   - max_time_slice: no need here, the variable would be assigned by the caller.
 *
 * hint: see proj13.1/libs/list.h for routines of the list structures.
 */
static void
stride_init(struct run_queue *rq) {
     /* LAB6: YOUR CODE */
     list_init(&(rq->run_list));  // 初始化等待链表
     rq->lab6_run_pool = NULL;    // 初始化优先队列节点
     rq->proc_num = 0;            // 初始化进程数
}
```

- 将进程加入运行队列时，插入到斜堆中，并将运行队列的进程计数加一，同时需要在进程的数据结构中关联运行队列

```c
/*
 * stride_enqueue inserts the process ``proc'' into the run-queue
 * ``rq''. The procedure should verify/initialize the relevant members
 * of ``proc'', and then put the ``lab6_run_pool'' node into the
 * queue(since we use priority queue here). The procedure should also
 * update the meta date in ``rq'' structure.
 *
 * proc->time_slice denotes the time slices allocation for the
 * process, which should set to rq->max_time_slice.
 * 
 * hint: see proj13.1/libs/skew_heap.h for routines of the priority
 * queue structures.
 */
static void
stride_enqueue(struct run_queue *rq, struct proc_struct *proc) {
     /* LAB6: YOUR CODE */
#if USE_SKEW_HEAP
     // 将 proc 插入优先队列中
     // skew_heap_insert 函数返回斜堆根节点，即 stride 最小的进程
     rq->lab6_run_pool =
          skew_heap_insert(rq->lab6_run_pool, &(proc->lab6_run_pool), proc_stride_comp_f);
#else
     assert(list_empty(&(proc->run_link)));
     list_add_before(&(rq->run_list), &(proc->run_link));
#endif
     // 如果时间片为 0（即刚初始化）或者大于最大时间片，则将其设为最大时间片
     if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
          proc->time_slice = rq->max_time_slice;
     }
     proc->rq = rq;    // 给进程控制块的当前进程在运行队列中的指针添加信息
     rq->proc_num ++;  // 进程数加一
}
```

- 将进程从运行队列移走时，需要将进程从斜堆中删除，并将运行队列的进程计数减一

```c
/*
 * stride_dequeue removes the process ``proc'' from the run-queue
 * ``rq'', the operation would be finished by the skew_heap_remove
 * operations. Remember to update the ``rq'' structure.
 *
 * hint: see proj13.1/libs/skew_heap.h for routines of the priority
 * queue structures.
 */
static void
stride_dequeue(struct run_queue *rq, struct proc_struct *proc) {
     /* LAB6: YOUR CODE */
#if USE_SKEW_HEAP
     // 将 proc 进程从优先队列中删除
     rq->lab6_run_pool =
          skew_heap_remove(rq->lab6_run_pool, &(proc->lab6_run_pool), proc_stride_comp_f);
#else
     assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
     list_del_init(&(proc->run_link));
#endif
     rq->proc_num --;  // 进程数减一
}
```

- 选择进程调度

```c
/*
 * stride_pick_next pick the element from the ``run-queue'', with the
 * minimum value of stride, and returns the corresponding process
 * pointer. The process pointer would be calculated by macro le2proc,
 * see proj13.1/kern/process/proc.h for definition. Return NULL if
 * there is no process in the queue.
 *
 * When one proc structure is selected, remember to update the stride
 * property of the proc. (stride += BIG_STRIDE / priority)
 *
 * hint: see proj13.1/libs/skew_heap.h for routines of the priority
 * queue structures.
 */
static struct proc_struct *
stride_pick_next(struct run_queue *rq) {
     /* LAB6: YOUR CODE */
#if USE_SKEW_HEAP
     if (rq->lab6_run_pool == NULL) return NULL;  // 如果等待池中没有进程了，就返回 NULL
     // 依靠运行池得到进程控制块的地址
     struct proc_struct *p = le2proc(rq->lab6_run_pool, lab6_run_pool);
#else
     list_entry_t *le = list_next(&(rq->run_list));

     if (le == &rq->run_list)
          return NULL;
     
     struct proc_struct *p = le2proc(le, run_link);
     le = list_next(le);
     while (le != &rq->run_list)
     {
          struct proc_struct *q = le2proc(le, run_link);
          if ((int32_t)(p->lab6_stride - q->lab6_stride) > 0)
               p = q;
          le = list_next(le);
     }
#endif
     if (p->lab6_priority == 0)  // 如果优先级等于 0，即优先级最低
          // 那么将步长加上最大步长，将该步长设置为最大，溢出也不会影响
          p->lab6_stride += BIG_STRIDE;
     // 否则就将步长加上(最大步长/优先级)来得到新的步长
     else p->lab6_stride += BIG_STRIDE / p->lab6_priority;
     return p;
}
```

- 时间片部分同 RR 算法思想

```c
/*
 * stride_proc_tick works with the tick event of current process. You
 * should check whether the time slices for current process is
 * exhausted and update the proc struct ``proc''. proc->time_slice
 * denotes the time slices left for current
 * process. proc->need_resched is the flag variable for process
 * switching.
 */
static void
stride_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
     /* LAB6: YOUR CODE */
     if (proc->time_slice > 0) {  // 如果时间片大于 0
          proc->time_slice --;  // 那么减小时间片
     }
     if (proc->time_slice == 0) {  // 如果时间片已经用完
          proc->need_resched = 1;  // 那么该进程就可以进行调度
     }
}
```

- 定义一个 c 语言类的实现，提供调度算法的切换接口

```c
struct sched_class default_sched_class = {
     .name = "stride_scheduler",
     .init = stride_init,
     .enqueue = stride_enqueue,
     .dequeue = stride_dequeue,
     .pick_next = stride_pick_next,
     .proc_tick = stride_proc_tick,
};
```

## 扩展练习 Challenge 1 ：实现 Linux 的 CFS 调度算法

咕咕咕

## 扩展练习 Challenge 2 ：在ucore上实现尽可能多的各种基本调度算法(FIFO, SJF,...)，并设计各种测试用例，能够定量地分析出各种调度算法在各种指标上的差异，说明调度算法的适用范围

咕咕咕