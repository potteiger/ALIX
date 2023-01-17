/*
 * efi.h -- EFI data types/structures
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _EFI_H_
#define _EFI_H_

#include <stdint.h>

/*
 * Full UEFI API support will not be provided. Only defining what is needed or
 * is likely to be needed.
 *
 * UEFI Specification Version 2.10 is being referenced:
 * 	https://uefi.org/specs/UEFI/2.10/index.html
 */

/*
 * UEFI boolean... 1 byte value
 */

#define true 1
#define false 0
typedef int8_t boolean;

/*
 * UEFI status codes
 */

#define EFI_SUCCESS 0
typedef uint64_t EFI_STATUS;

/*
 * Precedes all EFI tables
 */

typedef struct EFI_TABLE_HEADER {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
} EFI_TABLE_HEADER;

/*
 * Pointers to all boot services
 */

typedef struct EFI_BOOT_SERVICES {

	EFI_TABLE_HEADER	hdr;
	
	/*
	 * Task priority services
	 */

	void *			raise_TPL;
  	void *			restore_TPL;

	/*
	 * Memory services
	 */

	void *			allocate_pages;
	void *			free_pages;
	void *			get_memory_map;
	void *			allocate_pool;
	void *			free_pool;

	/*
	 * Event and timer services
	 */

	void *			create_event;
	void *			set_timer;
	void *			wait_for_event;
	void *			signal_event;
	void *			close_event;
	void *			check_event;

	/*
	 * Protocol handler services
	 */

	void *			install_protocol_interface;
	void *			reinstall_protocol_interface;
	void *			uninstall_protocol_interface;
	void *			handle_protocol;
	void *			reserved;
	void *			register_protocol_notify;
	void *			locate_handle;
	void *			locate_device_path;
	void *			install_configuration_table;

	/*
	 * Image services
	 */

	void *			load_image;
	void *			start_image;
	void *			exit;
	void *			unload_image;
	void *			exit_boot_services;

	/*
	 * Miscellaneous services
	 */

	void *			get_next_monotonic_count;
	void *			stall;
	void *			set_watchdog_timer;

	/*
	 * Driver support services
	 */

	void *			connect_controller;
	void *			disconnect_controller;

	/*
	 * Open and close protocol services
	 */

	void *			open_protocol;
	void *			close_protocol;
	void *			open_protocol_information;

	/*
	 * Library services
	 */

	void *			protocols_per_handle;
	void *			locate_handle_buffer;
	void *			locate_protocol;
	void *			install_multiple_protocol_interfaces;
	void *			uninstall_multiple_protocol_interfaces;

	/*
	 * 32-bit CRC services
	 */

	void *			calculate_crc32;

	/*
	 * Miscellaneous services
	 */

	void *			copy_mem;
	void *			set_mem;
	void *			create_event_ex;

} EFI_BOOT_SERVICES;

/*
 * Basic UEFI terminal output
 */

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	EFI_STATUS (*reset)
		(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, boolean);
	
	EFI_STATUS (*output_string)
		(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *, int16_t *);
	
	void *				TestString;
	void *				QueryMode;
	void *				SetMode;
	void *				SetAttribute;

	EFI_STATUS (*clear_screen)
		(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *);
	
	void *				SetCursorPosition;
	void *				EnableCursor;
	void *				Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

/*
 * Pointers to boot and run time services and other tables
 */

typedef struct EFI_SYSTEM_TABLE {
	EFI_TABLE_HEADER			hdr;
	int16_t *				firmware_vendor;
	uint32_t				firmware_revision;
	void *					console_in_handle;
	uint64_t				console_in;
	void *					console_out_handle;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *	console_out;
	void *					stderr_handle;
	uint64_t				std_err;
	uint64_t				runtime_services;
	uint64_t				boot_services;
	uint64_t				number_of_table_entries;
	uint64_t				configuration_table;
} EFI_SYSTEM_TABLE;

#endif /* _EFI_H_ */

