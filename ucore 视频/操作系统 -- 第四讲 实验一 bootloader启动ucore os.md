## x86启动顺序 - 从 bootloader 到 OS

### bootloader 做的事情

使能保护模式（protection mode）& 段机制（segment-level protection）

从硬盘上读取 kernel in ELF 歌视的 ucore kernel（跟在 MER 后面的扇区）并放到内存中固定位置

跳转到 ucore OS 的入口点（entry point）执行，这时控制权到了 ucore OS 手中

## x86启动顺序 - 使能保护模式

使能保护模式（protection mode），bootloader/OS 要设置 CR0 的 bit 0 为 1

段机制（segment-level protection）在保护模式下是自动使能的

