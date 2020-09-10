该代码来自于 NUAACTF 2018，一共是 3 个文件，分别是 `kvm.c`、`bits.h`、`vm.h`

## 小记录

1. KVM 的退出并不一定是虚拟机关机，虚拟机如果遇到 IO 操作，访问硬件设备，缺页中断等都会退出执行

    退出执行可以理解为将 CPU 执行上下文返回到 QEMU

2. 至少有 4 个 out 指令，KVM 才会退出，进而去访问 QEMU

## kmv.c

### 头文件

```c
/* kvm api
* see https://www.kernel.org/doc/Documentation/virtual/kvm/api.txt
*/
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "vm.h"
#include "bits.h"
```

这里的 api 链接很值得去看看

### main 函数

```c
int main(int argc, char *argv[])
{
	uint8_t *code;
	size_t len;
	puts("Welcome to your big-bro's new world!");
	puts("Input the code you want execute: ");
	code = (uint8_t *)malloc(0x100);  // kvm 运行时将会执行该内存中的代码
	if ( code == NULL )
		pexit("malloc");

	read(0, code, 0x100);             // 由用户输入 kvm 将要执行的代码

	vm *vm = kvm_init(code, 0x100);   // 进行 kvm 初始化工作
	kvm_execute(vm);                  // 运行 kvm

	return 0;
}
```

### kvm_init 函数

```c
vm *kvm_init(uint8_t code[], size_t len)
{
	int kvmfd, vmfd, api_ver;
    
    // 第一步：获取 KVM 的文件描述符
	kvmfd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
	if ( kvmfd < 0 )
		pexit("open /dev/kvm");
    
    // 第二步：确保是正确的 API 版本
	api_ver = ioctl(kvmfd, KVM_GET_API_VERSION, 0);
	if ( api_ver < 0 )
		pexit("KVM_GET_API_VERSION");
	if ( api_ver != KVM_API_VERSION )
	{
		error("Got kvm api version %d, expected %d\n",
			api_ver, KVM_API_VERSION);
	}
    
    // 第三步：创建虚拟机，获取该虚拟机的文件描述符
	vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
	if( vmfd < 0 )
		pexit("KVM_CREATE_VM");
    
    // 第四步：为这个虚拟机申请内存，并将代码（镜像）加载到虚拟机内存中，相当于物理机的 boot 过程
	void *mem = mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if ( mem == NULL )
		pexit("mmap(MEM_SIZE)");

	size_t entry = 0;
	memcpy((void *)mem + entry, code, len);

	// allocte memory for the guest
	struct kvm_userspace_memory_region region =
	{
		.slot = 0,
		.flags = 0,
		.guest_phys_addr = 0,
		.memory_size = MEM_SIZE,
		.userspace_addr = (size_t)mem
	};
    
    // 第五步：设置 KVM 的内存区域
	if ( ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region) < 0 )
		pexit("ioctl(KVM_SET_USER_MEMORY_REGIONI)");
    
    // 第六步：创建虚拟 CPU
	int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
	if ( vcpufd < 0 )
		pexit("ioctl(KVM_CREATE_VCPU)");
    
    // 第七步：为 VCPU 分配内存空间
	size_t vcpu_mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	struct kvm_run *run = (struct kvm_run *)mmap(0, vcpu_mmap_size,
						PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);

	vm *pvm = (vm *)malloc(sizeof(struct vm));
	*pvm = (struct vm){
		.mem = mem,
		.mem_size = MEM_SIZE,
		.vcpufd = vcpufd,
		.run = run
	};

	setup_regs(pvm, entry);
	setup_long_mode(pvm);

	return pvm;
}
```

### kvm_execute 函数

```c
void kvm_execute(vm *vm)
{
	char magic[4] = "flag";
	char tmp[4];
	void *shellcode;
	memset(tmp, 4, 0);
	memset(shellcode, 0x100, 0);

	uint8_t count = 0;
	while ( count < 4 )
	{
        // 运行虚拟机
		ioctl(vm->vcpufd, KVM_RUN, NULL);
        
        // 找到虚拟机的退出值，确定退出原因
		switch( vm->run->exit_reason )
		{
            // 由于 hlt 指令退出，KVM_EXIT_HLT 的值是 5
			case KVM_EXIT_HLT:
				fprintf(stderr, "KVM_EXIT_HLT\n");
				return;
            
            // 由于 IO 指令退出，KVM_EXIT_IO 的值是 2
			case KVM_EXIT_IO:
				tmp[count] = *( ((char *)(vm->run)) + vm->run->io.data_offset);
				break;
                
            // 由于执行 VM_ENTRY 失败，KVM_EXIT_FAIL_ENTRY 的值是 9
			case KVM_EXIT_FAIL_ENTRY:
				error("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx\n",
					vm->run->fail_entry.hardware_entry_failure_reason);
                
            // 初始化阶段无法找到 KVM 内核模块，该条件的值是 0x11
			case KVM_EXIT_INTERNAL_ERROR:
				error("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x\n",
					vm->run->internal.suberror);
            
            // KVM 关机，KVM_EXIT_SHUTDOWN 的值是 8
			case KVM_EXIT_SHUTDOWN:
				error("KVM_EXIT_SHUTDOWN\n");

			default:
				error("Unhandled reason: %d\n", vm->run->exit_reason);
		}
		count += 1;
	}

	if ( strncmp(magic, tmp, 4) == 0 )
	{
		puts("Welcome home! Do what you want to do!");
		shellcode = mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		puts("Input the code you want execute again: ");
		read(0, shellcode, 0x100);
		(*(void(*)())shellcode)();
	}
	else
	{
		puts("You can not go home!");
		exit(0);
	}
}
```

经过调试，可以知晓这里 tmp 字符数组的由来

tmp 是由 `vm->run->io.data_offset` 代码而来，而这段代码指的是经 out 操作被写入内存中的字符

kvm 在运行时会执行 code 内的代码，我们在 code 里写入我们想要构造的汇编

就好比将 fxxlag 放入 rax 后，进行如下汇编也可在最后使 tmp 得到 flag 字符串从而绕过检测

```assembly
movabs rax, 0x67616c787866
out dx, al
shr rax, 24
out dx, al
shr rax, 8
out dx, al
shr rax, 8
out dx, al
```

上面的汇编满足的 4 个 out 指令，并且每次写入的字符组合起来就是 flag 字符串

### setup_regs 函数

```c
/* set rip = entry point
 * set rsp = MAX_KERNEL_SIZE + KERNEL_STACK_SIZE (the max address can be used)
 *
 * set rdi = PS_LIMIT (start of free (unpaging) physical pages)
 * set rsi = MEM_SIZE - rdi (total length of free pages)
 * Kernel could use rdi and rsi to initalize its memory allocator.
 */
void setup_regs(vm *vm, size_t entry)
{
	struct kvm_regs regs;
	if (ioctl(vm->vcpufd, KVM_GET_REGS, &regs) < 0 )
		pexit("ioctl(KVM_GET_REGS)");

	regs.rip = entry;
	regs.rsp = MAX_KERNEL_SIZE + KERNEL_STACK_SIZE;
	regs.rdi = PS_LIMIT; //(start of free (unpaging) physical pages)
	regs.rsi = MEM_SIZE - regs.rdi;
	regs.rflags = 0x2;  // 初始化 flags 寄存器，x86 架构下需要设置，否则会出错
    
    // KVM_SET_REGS 设置寄存器
	if (ioctl(vm->vcpufd, KVM_SET_REGS, &regs) < 0)
		pexit("ioctl(KVM_SET_REGS)");
}
```

这里 kvm_regs 结构体的定义在 `/usr/include/x86_64-linux-gnu/asm/kvm.h`，具体源码可以自己看

## 偷懒啦偷懒啦

剩下的以后再说，先去继续学内核啦