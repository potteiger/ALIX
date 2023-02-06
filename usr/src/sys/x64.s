;
; `x64.s` -- Assorted assembly procedures for x86-64
; Copyright (c) 2023 Alan Potteiger
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this
; file, You can obtain one at https://mozilla.org/MPL/2.0/.
;

global outb

; Output a byte to said port
; 
; void	outb(uint16_t port, uint8_t byte);
;		rdi		rsi
outb:
	mov rax, rsi
	mov rdx, rdi
	out dx, al
	ret

