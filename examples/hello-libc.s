.globl main
main:
  push $msg
	call printf
  addl $4, %esp
	mov $0, %eax
  ret

msg:
.asciz "Hello\n"
