global _start
_start:
  mov rax, 1
  push rax
  mov rax, 2
  push rax
  mov rax, 34
  push rax
  pop rbx
  pop rax
  mul rbx
  push rax
  pop rbx
  pop rax
  add rax, rbx
  push rax
  mov rax, 42
  push rax
  push QWORD [rsp + 8]

  mov rax, 60
  pop rdi
  syscall

  mov rax, 60
  mov rdi, 0
  syscall
