global _start
_start:
  mov rax, 64
  push rax
  mov rax, 2
  push rax
  pop rbx
  pop rax
  div rbx
  push rax
  mov rax, 2
  push rax
  pop rbx
  pop rax
  add rax, rbx
  push rax
  mov rax, 2
  push rax
  pop rbx
  pop rax
  mul rbx
  push rax
  mov rax, 1
  push rax
  pop rbx
  pop rax
  add rax, rbx
  push rax
  mov rax, 1
  push rax
  push QWORD [rsp + 8]
  pop rax
  test rax, rax
  jz if_label_0
  mov rax, 69
  push rax
  mov rax, 60
  pop rdi
  syscall
  add rsp, 0
if_label_0:
  mov rax, 42
  push rax
  mov rax, 60
  pop rdi
  syscall

  mov rax, 60
  mov rdi, 0
  syscall
