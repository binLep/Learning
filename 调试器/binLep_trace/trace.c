#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <elf.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/mman.h>
#include <sys/reg.h>

typedef struct handle{
    Elf64_Ehdr* ehdr;
    Elf64_Phdr* phdr;
    Elf64_Shdr* shdr;
    uint8_t* mem;
    char* symname;
    Elf64_Addr symaddr;
    struct user_regs_struct pt_reg;
    char* exec; // 获取堆空间中 pid 对应程序的绝对路径
} handle_t;

int global_pid;
Elf64_Addr lookup_symbol(handle_t*, const char*);
char* get_exe_name(int);
void sighandler(int);
#define EXE_MODE 0
#define PID_MODE 1

int main(int argc, char** argv, char* envp){
    int fd, c, mode = 0;
    handle_t h;
    struct stat st;
    long trap, orig;
    int status, pid;
    char* args[2];

    /* 调试器主界面 */
    printf("Usage: %s [-ep <exe>/<pid>][f <frame>]\n", argv[0]);
    memset(&h, 0, sizeof(handle_t));
    while((c = getopt(argc, argv, "p:e:f:")) != EOF){
        switch(c){
        case 'p':
            pid = atoi(optarg);
            h.exec = get_exe_name(pid);
            if(h.exec == NULL){
                printf("Unable to retrieve executable path for pid: %d\n", pid);
                exit(-1);
            }
            mode = PID_MODE;
            break;
        case 'e':
            if((h.exec = strdup(optarg)) == NULL){
                perror("strdup");
                exit(-1);
            }
            mode = EXE_MODE;
            break;
        case 'f':
            if((h.symname = strdup(optarg)) == NULL){
                perror("strdup");
                exit(-1);
            }
            break;
        default:
            printf("Unknown option\n");
            break;
        }
    }

    /* 获取ELF文件符号信息 */
    if(h.symname == NULL){  // 程序必须有f选项，且必须有值
        puts("Specifying a function name with -f option id required");
        exit(-1);
    }
    if(mode == EXE_MODE){
        args[0] = h.exec;
        args[1] = NULL;
    }
    signal(SIGINT, sighandler);
    if((fd = open(h.exec, O_RDONLY)) < 0){
        perror("open");
        exit(-1);
    }
    if(fstat(fd, &st) < 0){
        perror("fstat");
        exit(-1);
    }
    h.mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(h.mem == MAP_FAILED){
        perror("mmap");
        exit(-1);
    }
    h.ehdr = (Elf64_Ehdr*)h.mem;
    h.phdr = (Elf64_Phdr*)(h.mem + h.ehdr->e_phoff);
    h.shdr = (Elf64_Shdr*)(h.mem + h.ehdr->e_shoff);

    if(h.mem[0] != 0x7f && !strcmp((char*)&h.mem[1], "ELF")){
        printf("%s is not an ELF file\n", h.exec);
        exit(-1);
    }
    if(h.ehdr->e_type != ET_EXEC){
        printf("%s is not an ELF executable\n", h.exec);
        exit(-1);
    }
    // 这个应该是这个调试器的不足而加的，只能用符号信息来断点
    if(h.ehdr->e_shstrndx == 0 || h.ehdr->e_shoff == 0 || h.ehdr->e_shnum == 0){
        puts("Section header table not found");
        exit(-1);
    }
    if((h.symaddr = lookup_symbol(&h, h.symname)) == 0){
        printf("Unable to find symbol: %s not found in executable\n", h.symname);
        exit(-1);
    }
    close(fd);
    /* 运行程序，开始调试 */
    if(mode == EXE_MODE){
        if((pid = fork()) < 0){
            perror("fork");
            exit(-1);
        }
        if(pid == 0){
            if(ptrace(PTRACE_TRACEME, pid, NULL, NULL) < 0){
                perror("PTRACE_TRACEME");
                exit(-1);
            }
            execve(h.exec, args, envp);
            exit(0);
        }
    }
    else if(mode == PID_MODE){
        if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0){
            perror("PTRACE_ATTACH");
            exit(-1);
        }
    }
    wait(&status);
    global_pid = pid;
    // 这里是-f选项对应的符号的符号表地址
    printf("Begining analysis of pid: %d at 0x%lx\n", pid, h.symaddr);
    if((orig = ptrace(PTRACE_PEEKTEXT, pid, h.symaddr, NULL)) < 0){
        perror("PTRACE_PEEKTEXT");
        exit(-1);
    }
    /* 设置断点*/
    trap = (orig & ~0xff) | 0xcc;
    // 将陷阱指令'int 3'写入地址
    if((ptrace(PTRACE_POKETEXT, pid, h.symaddr, trap)) < 0){
        perror("PTRACE_POKETEXT");
        exit(-1);
    }
    /* 开始追踪可执行程序 */
trace:
    if(ptrace(PTRACE_CONT, pid, NULL, NULL) < 0){
        perror("PTRACE_CONT");
        exit(-1);
    }
    wait(&status);
    if(WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP){
        if(ptrace(PTRACE_GETREGS, pid, NULL, &h.pt_reg) < 0){
            perror("PTRACE_GETREGS");
            exit(-1);
        }
        printf("\nExecutable %s (pid: %d) has hit a breakpoint 0x%lx\n", h.exec, pid, h.symaddr);
        printf(
            "%%rax:    0x%llx\n"
            "%%rbx:    0x%llx\n"
            "%%rcx:    0x%llx\n"
            "%%rdx:    0x%llx\n"
            "%%rsi:    0x%llx\n"
            "%%rdi:    0x%llx\n"
            "%%rbp:    0x%llx\n"
            "%%rsp:    0x%llx\n"
            "%%r8:     0x%llx\n"
            "%%r9:     0x%llx\n"
            "%%r10:    0x%llx\n"
            "%%r11:    0x%llx\n"
            "%%r12:    0x%llx\n"
            "%%r13:    0x%llx\n"
            "%%r14:    0x%llx\n"
            "%%r15:    0x%llx\n"
            "%%rip:    0x%llx\n"
            "%%eflags: 0x%llx\n"
            "%%cs:     0x%llx\n"
            "%%ss:     0x%llx\n"
            "%%ds:     0x%llx\n"
            "%%es:     0x%llx\n"
            "%%fs:     0x%llx\n"
            "%%gs:     0x%llx\n",
            h.pt_reg.rax,
            h.pt_reg.rbx,
            h.pt_reg.rcx,
            h.pt_reg.rdx,
            h.pt_reg.rsi,
            h.pt_reg.rdi,
            h.pt_reg.rbp,
            h.pt_reg.rsp,
            h.pt_reg.r8,
            h.pt_reg.r9,
            h.pt_reg.r10,
            h.pt_reg.r11,
            h.pt_reg.r12,
            h.pt_reg.r13,
            h.pt_reg.r14,
            h.pt_reg.r15,
            h.pt_reg.rip,
            h.pt_reg.eflags,
            h.pt_reg.cs,
            h.pt_reg.ss,
            h.pt_reg.ds,
            h.pt_reg.es,
            h.pt_reg.fs,
            h.pt_reg.gs);
        printf("\nPlease hit any key to continue: ");
        getchar();

        // 继续追踪程序
        if(ptrace(PTRACE_POKETEXT, pid, h.symaddr, orig) < 0){
            perror("PTRACE_POKETEXT");
            exit(-1);
        }
        h.pt_reg.rip = h.pt_reg.rip - 1;
        if(ptrace(PTRACE_SETREGS, pid, h.symaddr, &h.pt_reg) < 0){
            perror("PTRACE_SETREGS");
            exit(-1);
        }
        if(ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL) < 0){
            perror("PTRACE_SINGLESTEP");
            exit(-1);
        }
        wait(NULL);
        if(ptrace(PTRACE_POKETEXT, pid, h.symaddr, trap) < 0){
            perror("PTRACE_POKETEXT");
            exit(-1);
        }
        goto trace;
        if(WIFEXITED(status)){
            printf("Complete tracing pid: %d\n", pid);
        }
    }
    return 0;
}

Elf64_Addr lookup_symbol(handle_t* h, const char* symname){
    int i, j;
    char* strtab;
    Elf64_Sym* symtab;
    for(i = 0; i < h->ehdr->e_shnum; ++i){
        if(h->shdr[i].sh_type == SHT_SYMTAB){
            strtab = (char*)&h->mem[h->shdr[h->shdr[i].sh_link].sh_offset];
            symtab = (Elf64_Sym*)&h->mem[h->shdr[i].sh_offset];
            for(j = 0; j < h->shdr[i].sh_size / sizeof(Elf64_Sym); ++j){
                if(strcmp(&strtab[symtab->st_name], symname) == 0){
                    return symtab->st_value;
                }
                ++symtab;
            }
        }
    }
    return 0;
}

char* get_exe_name(int pid){
    char cmdline[0xff], path[0x200], * p;
    int fd;
    snprintf(cmdline, 0xff, "/proc/%d/cmdline", pid);
    if((fd = open(cmdline, O_RDONLY)) < 0){
        perror("open");
        exit(-1);
    }
    if(read(fd, path, 0x200) < 0){
        perror("read");
        exit(-1);
    }
    if((p = strdup(path)) == NULL){
        perror("strdup");
        exit(-1);
    }
    return p;
}

void sighandler(int sig){
    printf("Caught SIGINT: Detaching from %d\n", global_pid);
    if(ptrace(PTRACE_DETACH, global_pid, NULL, NULL) < 0 && errno){
        perror("PTRACE_DETACH");
        exit(-1);
    }
    exit(0);
}