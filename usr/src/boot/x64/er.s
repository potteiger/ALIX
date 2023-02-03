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
global getcr3
global prep_map

section .text

prep_map:
	mov rax, 0x80000033 ; disable bit 16 in `cr0`
	mov cr0, rax

	mov rax, cr3 ; return CR3 to find PML4
	ret

er:
	mov rbp, rsp
	mov rax, cr3
	mov cr3, rax

	mov rdi, rdx
	call rcx

