global _start
_start:
  mov rax, 0
  push rax
  mov rax, 2
  push rax
  push QWORD [rsp + 0]
  mov rax, 1
  push rax
  pop rbx
  pop rax
  sub rax, rbx
  push rax
  pop rax
  test rax, rax
  jz label_0
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
  pop rax
mov [rsp + 8], rax
  push QWORD [rsp + 8]
  mov rax, 60
  pop rdi
  syscall
  add rsp, 0
label_0:
  push QWORD [rsp + 0]
  pop rax
  test rax, rax
  jz label_2
  mov rax, 42
  push rax
  mov rax, 60
  pop rdi
  syscall
  add rsp, 0
  jmp label_1
label_2:
  mov rax, 21
  push rax
  mov rax, 60
  pop rdi
  syscall
  add rsp, 0
label_1:

  mov rax, 60
  mov rdi, 0
  syscall
