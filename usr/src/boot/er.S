;
; ALIX: `boot/er.s` -- ALIX bootloader (x86-64 EFI), er phase
; Copyright (c) 2023 Alan Potteiger
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this
; file, You can obtain one at https://mozilla.org/MPL/2.0/.
;

bits 64

global pgaccess
global er

section .text

;
; Ensure bit #16 in `cr0` is disabled.
; Return value of `cr3`
;
pgaccess:
	mov rax, cr0	; clear bit #16 of `cr0`
	and rax, 0xFFFFFFFFFFF0FFFF
	mov cr0, rax

	mov rax, cr3	; return `cr3` to find PML4
	ret

;
; Final jump to the kernel
; rcx = kernel entry point
; rdx = pointer to kargtab struct
;
er:
	mov rbp, rsp
	mov rax, cr3
	mov cr3, rax

	mov rdi, rdx
	call rcx

