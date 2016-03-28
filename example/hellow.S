# x86_64, GAS syntax

_hellow:
    # according to the ABI, the first argument to the function is in %rdi.
    # This contains the file descriptor for the socket we downloaded through.
    # We don't need it but firwroks expects to close it after we return.
    # So keep a backup on the stack.
    push %rdi
    
    # we're pretty much free to do what we like with the other general-purpose
    # registers.

    # From Linux manpages 4.04, syscall(2)
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
    .ascii "Hello world!\n\0"
text_end:
