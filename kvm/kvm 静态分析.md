### open

源码

```c
open("/dev/kvm", O_RDWR | O_CLOEXEC);
```

IDA

```c
open("/dev/kvm", 0x80002);
```

---

### ioctl

源码

```c
ioctl(kvmfd, KVM_GET_API_VERSION, 0);
```

IDA

```c
ioctl(fd, 0xAE00uLL, 0LL);
```

---

源码

```c
ioctl(kvmfd, KVM_CREATE_VM, 0);
```

IDA

```c
ioctl(fd, 0xAE01uLL, 0LL);
```

---

源码

```c
ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region)
```

IDA

```c
ioctl(v11, 0x4020AE46uLL, &region)
```

---

源码

```c
ioctl(vmfd, KVM_CREATE_VCPU, 0);
```

IDA

```c
ioctl(v11, 0xAE41uLL, 0LL);
```

---

源码

```c
ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
```

IDA

```c
ioctl(fd, 0xAE04uLL, 0LL);
```

---

源码

```c
ioctl(vm->vcpufd, KVM_RUN, NULL);
```

IDA

```c
ioctl(*(vm + 4), 0xAE80uLL, 0LL);
```

---

源码

```c
ioctl(vm->vcpufd, KVM_GET_REGS, &regs)
```

IDA

```c
ioctl(*(vm + 4), 0x8090AE81uLL, &v4, entry, vm)
```

---

源码

```c
ioctl(vm->vcpufd, KVM_SET_REGS, &regs)
```

IDA

```c
ioctl(*(v3 + 16), 0x4090AE82uLL, &v4);
```

---

源码

```c
ioctl(vm->vcpufd, KVM_GET_SREGS, &sregs)
```

IDA

```c
ioctl(vm[4], 0x8138AE83uLL, &sregs)
```

---

源码

```c
ioctl(vm->vcpufd, KVM_SET_SREGS, &sregs)
```

IDA

```c
ioctl(vm[4], 0x4138AE84uLL, &sregs)
```

### mmap

源码

```c
mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE,
     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
```

IDA

```c
mmap(0LL, &dword_400000, 3, 33, -1, 0LL);
```

## kvm_run

### exit_reason

源码

```c
kvm_run->exit_reason
```

IDA

```c
*(kvm_run + 8)
```

```c
kvm_run[2]
```

### fail_entry.hardware_entry_failure_reason

源码

```c
kvm_run->fail_entry.hardware_entry_failure_reason
```

IDA

```c
*(kvm_run + 32)
```

### internal.suberror

源码

```c
kvm_run->internal.suberror
```

IDA

```c
*(kvm_run + 32)
```

### request_interrupt_window

源码

```c
kvm_run->request_interrupt_window
```

IDA

```c
*(kvm_run)
```

### io.data_offset

源码

```c
kvm_run->io.data_offset
```

IDA

```c
*(kvm_run + 5)
```

### io.port

源码

```c
kvm_run->io.port
```

IDA

```c
*(kvm_run + 17)
```

### io.direction

源码

```c
kvm_run->io.direction
```

IDA

```c
*(kvm_run + 32)
```

### io.count

源码

```c
kvm_run->io.count
```

IDA

```c
*(kvm_run + 33)
```

### io.size

源码

```c
kvm_run->io.size
```

IDA

```c
*(kvm_run + 36)
```

```c
kvm_run[9]
```

### io.data_offset

源码

```c
kvm_run->io.data_offset
```

IDA

```c
*(kvm_run + 40)
```

### KVM_EXIT

```c
#define KVM_EXIT_UNKNOWN          0
#define KVM_EXIT_EXCEPTION        1
#define KVM_EXIT_IO               2
#define KVM_EXIT_HYPERCALL        3
#define KVM_EXIT_DEBUG            4
#define KVM_EXIT_HLT              5
#define KVM_EXIT_MMIO             6
#define KVM_EXIT_IRQ_WINDOW_OPEN  7
#define KVM_EXIT_SHUTDOWN         8
#define KVM_EXIT_FAIL_ENTRY       9
#define KVM_EXIT_INTR             10
#define KVM_EXIT_SET_TPR          11
#define KVM_EXIT_TPR_ACCESS       12
#define KVM_EXIT_S390_SIEIC       13
#define KVM_EXIT_S390_RESET       14
#define KVM_EXIT_DCR              15 /* deprecated */
#define KVM_EXIT_NMI              16
#define KVM_EXIT_INTERNAL_ERROR   17
#define KVM_EXIT_OSI              18
#define KVM_EXIT_PAPR_HCALL	  19
#define KVM_EXIT_S390_UCONTROL	  20
#define KVM_EXIT_WATCHDOG         21
#define KVM_EXIT_S390_TSCH        22
#define KVM_EXIT_EPR              23
#define KVM_EXIT_SYSTEM_EVENT     24
#define KVM_EXIT_S390_STSI        25
#define KVM_EXIT_IOAPIC_EOI       26
#define KVM_EXIT_HYPERV           27
```

---

源码

```c
open("/dev/kvm", O_RDWR | O_CLOEXEC);
```

IDA

```c

```

---

源码

```c
open("/dev/kvm", O_RDWR | O_CLOEXEC);
```

IDA

```c

```

---

源码

```c
open("/dev/kvm", O_RDWR | O_CLOEXEC);
```

IDA

```c

```

---

源码

```c
open("/dev/kvm", O_RDWR | O_CLOEXEC);
```

IDA

```c

```

---

