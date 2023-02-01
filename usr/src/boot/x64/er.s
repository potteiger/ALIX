;
; `as.s` -- x86-64 EFI bootloader, `er` phase
; Copyright (c) 2023 Alan Potteiger
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this
; file, You can obtain one at https://mozilla.org/MPL/2.0/.
;

bits 64

global er

section .text

;
; Final phase of bootloader. We setup the kernel stack, disable interrupts,
; enable the new paging tables, call the kernel and pass a pointer to the
; `kargtab` structure.
;
; We're transitioning from the shitty EFI/Microsoft calling convention to the
; holy x86-64 SysV ABI. Function input is in rcx, rdx, r8, and r9. Kernel input
; is in rdi.
;

er:
	mov rsp, r8
	mov rbp, 0

	cli
	mov cr3, rdx

	mov rdi, rcx
	call r9

