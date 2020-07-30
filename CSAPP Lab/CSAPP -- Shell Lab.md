## 前置

完成一个简单的shell程序，总体的框架和辅助代码都已经提供好了，我们需要完成的函数主要以下几个：

- eval: 主要功能是解析cmdline，并且运行. [70 lines]
- builtin cmd: 辨识和解析出bulidin命令: quit, fg, bg, and jobs. [25lines]
- do bgfg: 实现bg和fg命令. [50 lines]
- waitfg: 实现等待前台程序运行结束. [20 lines]
- sigchld handler: 响应SIGCHLD. 80 lines]
- sigint handler: 响应 SIGINT (ctrl-c) 信号. [15 lines]
- sigtstp handler: 响应 SIGTSTP (ctrl-z) 信号. [15 lines]

## eval

### 源码

```c
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    return;
}
```

### 题解

IDA 学习法，这个 eval 函数书上第 525 页有简单示例，可以先试试书上的示例加深理解

1. 注意每个子进程必须拥有自己独一无二的进程组 id，要不然就没有前台后台区分
2. 在 `fork()` 新进程前后要阻塞 SIGCHLD 信号，防止出现竞争（race）这种经典的同步错误

```c
void eval(char* cmdline){
    char* argv[MAXARGS];
    int bg;
    pid_t pid;
    sigset_t set;

    bg = parseline(cmdline, argv);  // 解析用户输入的字符串命令
    if(argv[0] == NULL) return;     // 如果嘛也没有，比如回车，就直接退出函数不做处理

    if(!builtin_cmd(argv)){         // 查找用户输入的命令是否为内置命令

        if(sigemptyset(&set) < 0)   // 初始化信号集合为空(执行成功则返回 0，如果有错误则返回 -1)
            unix_error("sigemptyset error");
        /* *
         * #define  SIGCHLD	17
         * #define  SIGINT	2
         * #define  SIGTSTP	20
         * */
        if(sigaddset(&set, SIGTSTP) || sigaddset(&set, SIGINT) || sigaddset(&set, SIGCHLD))
            unix_error("sigaddset error");

        /* #define  SIG_BLOCK 0 */
        if(sigprocmask(SIG_BLOCK, &set, NULL) < 0)
            unix_error("sigprocmask error");

        /* *
         * 把新建立的进程添加到新的进程组:
         * 当从 bash 运行 tsh 时，tsh 在 bash 前台进程组中运行。
         * 如果 tsh 随后创建了一个子进程，默认情况下，该子进程也将是 bash 前台进程组的成员。
         * 由于输入 ctrl-c 将向 bash 前台组中的每个进程发送一个 SIGINT，
         * 因此输入 ctrl-c 将向 tsh 以及 tsh 创建的每个进程发送一个 SIGINT，这显然是不正确的。
         * 这里有一个解决方案: 在 fork 之后，但在 execve 之前，子进程应该调用 setpgid(0,0)，
         * 这将把子进程放入一个新的进程组中，该进程组的 ID 与子进程的 PID 相同。
         * 这确保 bash 前台进程组中只有一个进程，即 tsh 进程。
         * 当您键入 ctrl-c 时，tsh 应该捕获结果 SIGINT，然后将其转发到适当的前台作业
         * */
        if((pid = fork()) < 0)
            unix_error("fork error");
        else if(!pid){
            /* #define  SIG_UNBLOCK 1 */
            sigprocmask(SIG_UNBLOCK, &set, NULL);  // 解除子进程的阻塞
            if(setpgid(0, 0) < 0)
                unix_error("setpgid error");
            if(execve(argv[0], argv, environ) < 0){
                printf("%s: command not found\n", argv[0]);
                exit(0);
            }
        }

        if(bg == FG){
            addjob(jobs, pid, BG, cmdline);
            sigprocmask(SIG_UNBLOCK, &set, NULL);  // 解除前台子进程的阻塞
        }
        else{
            addjob(jobs, pid, FG, cmdline);
            sigprocmask(SIG_UNBLOCK, &set, NULL);  // 解除(未定义、后台、已停止)子进程的阻塞
            if(!bg){
                if(waitpid(pid, &bg, 0) < 0){
                    unix_error("waiting: waitpid error");
                }
            }
        }

        if(pid){
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);
        }
    }
}
```





## 参考链接

https://www.jianshu.com/p/f7054d98c6b8