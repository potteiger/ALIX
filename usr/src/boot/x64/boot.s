; `boot.s` -- x86-64 EFI boot
; Copyright (c) 2023 Alan Potteiger
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this
; file, You can obtain one at https://mozilla.org/MPL/2.0/.

extern boot

section .text
; Passes the pointer in `cr3` to our main function in C and calls it
global boots
boots:
	; 3rd argument is in `r8`
	mov r8, cr3

	call boot
	ret

