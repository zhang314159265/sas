.code32
.globl _start
_start:
  # print
  # pusha # why pusha cause illegal instruction error at runtime
  movl $4, %eax
  movl $1, %ebx
  movl $msg, %ecx
  movl $(msgend - msg), %edx
  int $0x80
  # popa

  # exit
  movl $1, %eax
  movl $0, %ebx
  int $0x80

msg:
.ascii "Hello, world....\n"
msgend:
