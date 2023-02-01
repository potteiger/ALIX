;
; `as.s` -- x86-64 EFI boot assembly
; Copyright (c) 2023 Alan Potteiger
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this
; file, You can obtain one at https://mozilla.org/MPL/2.0/.
;
bits 64

global setcr3
global getrsp
global er

section .text

er:
	mov rsp, r8
	mov rbp, 0

	cli
	mov cr3, rdx

	mov rdi, rcx
	call r9
