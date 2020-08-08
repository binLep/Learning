## 练习0：填写已有实验

将 lab1 的 `kern/debug/kdebug.c`、`kern/init/init.c` 以及 `kern/trap/trap.c` 直接复制到 lab3 里

再将 lab2 的 `kern/mm/pmm.c` 和 `kern/mm/default_pmm.c` 复制到 lab3 里即可

## 练习1：给未被映射的地址映射上物理页

### do_pgfault 函数源码

写于：**kern/mm/vmm.c**

```c
/* do_pgfault - interrupt handler to process the page fault execption
 * @mm         : the control struct for a set of vma using the same PDT
 * @error_code : the error code recorded in trapframe->tf_err which is setted by x86 hardware
 * @addr       : the addr which causes a memory access exception, (the contents of the CR2 register)
 *
 * CALL GRAPH: trap--> trap_dispatch-->pgfault_handler-->do_pgfault
 * The processor provides ucore's do_pgfault function with two items of information to aid in diagnosing
 * the exception and recovering from it.
 *   (1) The contents of the CR2 register. The processor loads the CR2 register with the
 *       32-bit linear address that generated the exception. The do_pgfault fun can
 *       use this address to locate the corresponding page directory and page-table
 *       entries.
 *   (2) An error code on the kernel stack. The error code for a page fault has a format different from
 *       that for other exceptions. The error code tells the exception handler three things:
 *         -- The P flag   (bit 0) indicates whether the exception was due to a not-present page (0)
 *            or to either an access rights violation or the use of a reserved bit (1).
 *         -- The W/R flag (bit 1) indicates whether the memory access that caused the exception
 *            was a read (0) or write (1).
 *         -- The U/S flag (bit 2) indicates whether the processor was executing at user mode (1)
 *            or supervisor mode (0) at the time of the exception.
 */
int
do_pgfault(struct mm_struct *mm, uint32_t error_code, uintptr_t addr) {
    int ret = -E_INVAL;
    //try to find a vma which include addr
    struct vma_struct *vma = find_vma(mm, addr);

    pgfault_num++;
    //If the addr is in the range of a mm's vma?
    if (vma == NULL || vma->vm_start > addr) {
        cprintf("not valid addr %x, and  can not find it in vma\n", addr);
        goto failed;
    }
    //check the error_code
    switch (error_code & 3) {
    default:
            /* error code flag : default is 3 ( W/R=1, P=1): write, present */
    case 2: /* error code flag : (W/R=1, P=0): write, not present */
        if (!(vma->vm_flags & VM_WRITE)) {
            cprintf("do_pgfault failed: error code flag = write AND not present, but the addr's vma cannot write\n");
            goto failed;
        }
        break;
    case 1: /* error code flag : (W/R=0, P=1): read, present */
        cprintf("do_pgfault failed: error code flag = read AND present\n");
        goto failed;
    case 0: /* error code flag : (W/R=0, P=0): read, not present */
        if (!(vma->vm_flags & (VM_READ | VM_EXEC))) {
            cprintf("do_pgfault failed: error code flag = read AND not present, but the addr's vma cannot read or exec\n");
            goto failed;
        }
    }
    /* IF (write an existed addr ) OR
     *    (write an non_existed addr && addr is writable) OR
     *    (read  an non_existed addr && addr is readable)
     * THEN
     *    continue process
     */
    uint32_t perm = PTE_U;
    if (vma->vm_flags & VM_WRITE) {
        perm |= PTE_W;
    }
    addr = ROUNDDOWN(addr, PGSIZE);

    ret = -E_NO_MEM;

    pte_t *ptep=NULL;
    /*LAB3 EXERCISE 1: YOUR CODE
    * Maybe you want help comment, BELOW comments can help you finish the code
    *
    * Some Useful MACROs and DEFINEs, you can use them in below implementation.
    * MACROs or Functions:
    *   get_pte : get an pte and return the kernel virtual address of this pte for la
    *             if the PT contians this pte didn't exist, alloc a page for PT (notice the 3th parameter '1')
    *   pgdir_alloc_page : call alloc_page & page_insert functions to allocate a page size memory & setup
    *             an addr map pa<--->la with linear address la and the PDT pgdir
    * DEFINES:
    *   VM_WRITE  : If vma->vm_flags & VM_WRITE == 1/0, then the vma is writable/non writable
    *   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
    *   PTE_U           0x004                   // page table/directory entry flags bit : User can access
    * VARIABLES:
    *   mm->pgdir : the PDT of these vma
    *
    */
#if 0
    /*LAB3 EXERCISE 1: YOUR CODE*/
    ptep = ???              //(1) try to find a pte, if pte's PT(Page Table) isn't existed, then create a PT.
    if (*ptep == 0) {
                            //(2) if the phy addr isn't exist, then alloc a page & map the phy addr with logical addr

    }
    else {
    /*LAB3 EXERCISE 2: YOUR CODE
    * Now we think this pte is a  swap entry, we should load data from disk to a page with phy addr,
    * and map the phy addr with logical addr, trigger swap manager to record the access situation of this page.
    *
    *  Some Useful MACROs and DEFINEs, you can use them in below implementation.
    *  MACROs or Functions:
    *    swap_in(mm, addr, &page) : alloc a memory page, then according to the swap entry in PTE for addr,
    *                               find the addr of disk page, read the content of disk page into this memroy page
    *    page_insert ： build the map of phy addr of an Page with the linear addr la
    *    swap_map_swappable ： set the page swappable
    */
        if(swap_init_ok) {
            struct Page *page=NULL;
                                    //(1）According to the mm AND addr, try to load the content of right disk page
                                    //    into the memory which page managed.
                                    //(2) According to the mm, addr AND page, setup the map of phy addr <---> logical addr
                                    //(3) make the page swappable.
        }
        else {
            cprintf("no swap_init_ok but ptep is %x, failed\n",*ptep);
            goto failed;
        }
   }
#endif
   ret = 0;
failed:
    return ret;
}
```

根据流程可以知道这个函数是在内核捕获缺页异常之后，通过 IDT 找到的函数，执行该函数来完成缺页异常的处理，先看两个结构体

### mm_struct 结构体

写于：**kern/mm/vmm.c**

```c
struct mm_struct {                 // 描述一个进程的虚拟地址空间 每个进程的 pcb 中会有一个指针指向本结构体
    list_entry_t mmap_list;        // 链接同一页目录表的虚拟内存空间中双向链表的头节点 
    struct vma_struct *mmap_cache; // 当前正在使用的虚拟内存空间
    pde_t *pgdir;                  // mm_struct 所维护的页表地址(用来找 PTE)
    int map_count;                 // 虚拟内存块的数目
    void *sm_priv;                 // 记录访问情况链表头地址(用于置换算法)
};
```

### vma_struct 结构体

写于：**kern/mm/vmm.c**

```c
struct vma_struct {          // 虚拟内存空间
    struct mm_struct *vm_mm; // 虚拟内存空间属于的进程
    uintptr_t vm_start;      // 连续地址的虚拟内存空间的起始位置和结束位置
    uintptr_t vm_end;
    uint32_t vm_flags;       // 虚拟内存空间的属性 (读/写/执行)
    list_entry_t list_link;  // 双向链表，从小到大将虚拟内存空间链接起来
};
```

### find_vma 函数

写于：**kern/mm/vmm.c**

```c
// find_vma - find a vma  (vma->vm_start <= addr <= vma_vm_end)
struct vma_struct *
find_vma(struct mm_struct *mm, uintptr_t addr) {
    struct vma_struct *vma = NULL;  // 初始化 vma 指针变量
    if (mm != NULL) {               // 如果 mm 指针变量不为 0，就进入 if 语句，不然直接返回 NULL
        vma = mm->mmap_cache;       // 将 vma 指针变量的值修改为当前正在使用的虚拟内存空间
        /* 如果当前正在使用的虚拟内存空间不为空，且地址处于正确的 vma 地址内，就不进入 if 语句 */
        if (!(vma != NULL && vma->vm_start <= addr && vma->vm_end > addr)) {
            /* 这下面是处理 vma 异常时的状况的 */
            bool found = 0;                                     // 设立标志位，用于之后确认是否找到符合的 vma
            list_entry_t *list = &(mm->mmap_list), *le = list;  // 这里 *list 和 *le 的值都是 mm->mmap_list
            while ((le = list_next(le)) != list) {              // 没遍历完双向链表就一直遍历
                vma = le2vma(le, list_link);                    // 根据链表找到对应的 vma 结构体基址
                if (vma->vm_start<=addr && addr < vma->vm_end) {// 如果该虚拟地址处在吻合的 vma 地址范围
                    found = 1;                                  // 更新标志位为 1，即找到了符合的 vma
                    break;
                }
            }
            if (!found) {                                       // 如果 found 为 0，则没有符合的 vma
                vma = NULL;                                     // 并且更新 vma 为 NULL，之后函数会返回 NULL
            }
        }
        if (vma != NULL) {                                      // vma 不为 NULL 就进入 if 语句
            mm->mmap_cache = vma;                               // 更新当前正在使用的虚拟内存空间为 vma
        }
    }
    return vma;
}
```

#### le2vma 宏

写于：**kern/mm/vmm.h**

```c
#define le2vma(le, member)                  \
    to_struct((le), struct vma_struct, member)
```

和 Lab2 中 提到的 le2page 函数基本是一个意思

作用是依靠作为 vma_struct 结构体中 member 成员变量的 le 变量，得到 le 成员变量所对应的 vma_struct 结构体的基地址

### ROUNDDOWN 宏

写于：**libs/defs.h**

```c++
/* *
 * Rounding operations (efficient when n is a power of 2)
 * Round down to the nearest multiple of n
 * */
#define ROUNDDOWN(a, n) ({                                          \
            size_t __a = (size_t)(a);                               \
            (typeof(a))(__a - __a % (n));                           \
        })
```

注释意思：四舍五入操作(当 n 是 2 的幂时有效)，四舍五入到 n 的倍数

其实应该只要 n 不是 0，都可以进行对于 a 的倍数的四舍五入

只是在这个 ucore 的代码里用的都是 2 的倍数（都用的 PGSIZE == 4096）

拿 4 举例的话就是，你能得到：4、8、12、16、20、...

如果 a 是 15 的话，`ROUNDDOWN(15, 4) == 12`

### VM_READ 宏

写于：**kern/mm/vmm.h**

```c
#define VM_READ                 0x00000001
```

### VM_WRITE 宏

写于：**kern/mm/vmm.h**

```c
#define VM_WRITE                0x00000002
```

### VM_EXEC 宏

写于：**kern/mm/vmm.h**

```c
#define VM_EXEC                 0x00000004
```

### PGSIZE 宏

写于：**kern/mm/mmu.h**

```c
#define PGSIZE          4096                    // bytes mapped by a page
```

### E_INVAL 宏

写于：**libs/error.h**

```c
#define E_INVAL             3   // Invalid parameter
```

### E_NO_MEM 宏

写于：**libs/error.h**

```c
#define E_NO_MEM            4   // Request failed due to memory shortage
```



### do_pgfault 函数答案

```

```

