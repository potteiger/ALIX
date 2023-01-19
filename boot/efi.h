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

/*
 * Full UEFI API support will not be provided. Only defining what is needed or
 * is likely to be needed.
 *
 * UEFI Specification Version 2.10 is being referenced:
 * 	https://uefi.org/specs/UEFI/2.10/index.html
 */

/*
 * Protocol GUIDs
 */

#define EFI_LOADED_IMAGE_DEVICE_PATH_PROTOCOL_GUID (efi_guid) \
			{ 0xbc62157e, 0x3e33, 0x4fec, \
			{ 0x99, 0x20, 0x2d, 0x3b, 0x36, 0xd7, 0x50, 0xdf }}

#define EFI_LOADED_IMAGE_PROTOCOL_GUID (efi_guid) \
			{ 0x5B1B31A1, 0x9562, 0x11d2,\
    			{ 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }}

/*
 * UEFI boolean... 1 byte value
 */

#define efi_true 1
#define efi_false 0
typedef int8_t efi_bool;

/*
 * UEFI status codes
 */

#define EFI_SUCCESS 		0	/* what you think? */
#define EFI_LOAD_ERROR 		1 	/* Image failed to load */
#define EFI_INVALID_PARAMETER 	2 	/* A parameter was incorrect */
#define EFI_UNSUPPORTED 	3 	/* Operation is not supported */
#define EFI_BAD_BUFFER_SIZE	4	/* Buffer was not the proper size */
#define EFI_BUFFER_TOO_SMALL 	5 	/* Buffer is not large enough */
#define EFI_NOT_READY 		6 	/* No data pending upon return */
#define EFI_DEVICE_ERROR 	7 	/* Physical device reported an error */
#define EFI_WRITE_PROTECTED 	8 	/* Device cannot be written to */
#define EFI_OUT_OF_RESOURCES 	9 	/* Resource has run out */
#define EFI_VOLUME_CORRUPTED 	10 	/* Inconstancy was detected */
#define EFI_VOLUME_FULL 	11 	/* No more space on the file system */
#define EFI_NO_MEDIA 		12 	/* No medium to perform the operation */
#define EFI_MEDIA_CHANGED 	13 	/* Medium in the device has changed */
#define EFI_NOT_FOUND 		14 	/* Item was not found */
#define EFI_ACCESS_DENIED 	15 	/* Access was denied */
#define EFI_NO_RESPONSE 	16 	/* Server was not found */
#define EFI_NO_MAPPING 		17 	/* Mapping to a device does not exist */
#define EFI_TIMEOUT 		18 	/* Timeout time expired */
#define EFI_NOT_STARTED 	19 	/* Protocol has not been started */
#define EFI_ALREADY_STARTED 	20 	/* Protocol has already been started */
#define EFI_ABORTED 		21 	/* Operation was aborted. */
#define EFI_ICMP_ERROR 		22 	/* ICMP error occurred */
#define EFI_TFTP_ERROR 		23 	/* TFTP error occurred */
#define EFI_PROTOCOL_ERROR 	24 	/* Protocol error occurred */
#define EFI_INCOMPATIBLE_VERSION 25 	/* Function version incompatibile */
#define EFI_SECURITY_VIOLATION 	26	/* Security violation */
#define EFI_CRC_ERROR 		27	/* CRC error was detected */
#define EFI_END_OF_MEDIA 	28 	/* Beginning or end of media reached */
#define EFI_END_OF_FILE 	31 	/* End of the file was reached */
#define EFI_INVALID_LANGUAGE 	32 	/* Language specified was invalid */
#define EFI_COMPROMISED_DATA 	33 	/* Compromised data */
#define EFI_IP_ADDRESS_CONFLICT 34 	/* Address conflict address */
#define EFI_HTTP_ERROR 		35 	/* HTTP error occurred */

typedef uint64_t efi_status;

/*
 * Protocol opening attributes
 */

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL	0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL		0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL		0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER	0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER		0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE		0x00000020

struct efi_system_table;

/*
 * This API is shit. I'm going to take an opportunity to go on a small rant
 * right here for anyone reading through this header that is curious. EFI is so
 * obviously influenced by Microsoft, it's god awful. The PE/COFF executables,
 * the calling convention, FAT (though that probably was a good choice but
 * beside the point). And I haven't even gotten to the API itself yet. I can't
 * imagine how painful it must be to develop anything using Windows API's, and
 * this has given me a sneak peak. Never will I do that willingly.
 *
 * - EFI_STATUS is fucking retarded, why are status codes encoded?
 * 
 * - The naming convention is just ugly and offensive, feels like you're being
 *   screamed at the entire time you're working. Luckily, I can fix this.
 * 
 * - typedef's of function pointers??? This is is just offensive and makes all
 *   the code and examples unreadable as hell, it's stupid. I defined all of
 *   the structs containing the function pointers to avoid function pointer
 *   typedefs. Works just fine...
 * 
 * - Why is this API just spider webs of struct/function pointers? Hard as hell
 *   to follow. Some of the diagrams in the specification I thought were jokes
 *   at first, they look absolutely ridiculous.
 *
 * - Why the hell are these functions doing sooo many things? What happened to
 *   do one thing and do it well? These functions take practically 42 arguments!
 *   So many things passed by reference that really don't need to be. Why can't
 *   this operate with any sort of reverence to classic C convention or
 *   STANDARD?
 *
 * - This API just shits on the C language. I'll keep repeating this.
 *
 * - I'll add to the list as I keep working...
 *
 */

typedef struct efi_guid {
	uint32_t	data1;
	uint16_t	data2;
	uint16_t	data3;
	uint8_t		data4[8];
} efi_guid;

/*
 * Precedes many EFI tables
 */

typedef struct efi_table_header {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
} efi_table_header;

/*
 * EFI memory descriptor, array of these are returned from get_memory_map()
 */

typedef struct efi_memory_descriptor {
	uint32_t		type;
	uint64_t		physical_start;
	uint64_t		virtual_start;
	uint64_t		number_of_pages;
	uint64_t		attribute;
} efi_memory_descriptor;

/*
 * UEFI device paths
 */

typedef struct efi_device_path_protocol {
	uint8_t	type;
	uint8_t sub_type;
	uint8_t	length[2];
 } efi_device_path_protocol;

/*
 * Information and procedures attached to loaded EFI image
 */

typedef struct efi_loaded_image_protocol {
	uint32_t			revision;
	void *				parent_handle;
	struct efi_system_table *	system_table;

   	/*
	 * Source location of image
	 */

	void *				device_handle;
	efi_device_path_protocol *	file_path;
	void *				reserved;

   	/*
	 * Image's load options
	 */
	
	uint32_t			load_options_size;
	void *				load_options;

	/*
	 * Where the image was loaded
	 */

   	void *				image_base;
	uint64_t			image_size;
	uint64_t			image_code_type;
	uint64_t			image_data_type;
	void *				unload;
} efi_loaded_image_protocol;

/*
 * File system access and manipulation
 */

typedef struct efi_file_protocol {
	uint64_t	revision;
	void *		open;
	void *		close;
	void *		delete;
	void *		read;
	void *		write;
	void *		get_position;
	void *		set_position;
	void *		get_info;
	void *		set_info;
	void *		flush;
	void *		open_ex;
	void *		read_ex;
	void *		write_ex;
	void *		flush_ex;
} efi_file_protocol;

/*
 * Get access to root of file system
 */

typedef struct efi_simple_file_system_protocol {
	uint64_t	revision;
	
	efi_status (*open_volume)
		(	struct efi_simple_file_system_protocol *,
			efi_file_protocol **	);

} efi_simple_file_system_protocol;

/*
 * Pointers to all boot services
 */

typedef struct efi_boot_services {

	efi_table_header	hdr;
	
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

	efi_status (*get_memory_map)
			(	uint64_t *,
				efi_memory_descriptor *,
				uint64_t *,
				uint64_t *,
				uint32_t *	);
	
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

	efi_status (*open_protocol)
			(	void *,
			 	efi_guid *,
				void **,
				void *,
				void *,
				uint32_t	);

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

} efi_boot_services;

/*
 * Basic UEFI terminal output
 */

typedef struct efi_simple_text_output_protocol {

	EFI_STATUS (*reset)
		(struct efi_simple_text_output_protocol *, efi_bool);
	
	EFI_STATUS (*output_string)
		(struct efi_simple_text_output_protocol *, int16_t *);
	
	void *				test_string;
	void *				query_mode;
	void *				set_mode;

	EFI_STATUS (*set_attribute)
		(struct efi_simple_text_output_protocol *, uint64_t attribute);

	EFI_STATUS (*clear_screen)
		(struct efi_simple_text_output_protocol *);
	
	void *				set_cursor_position;
	void *				enable_cursor;
	void *				mode;
} efi_simple_text_output_protocol;

/*
 * Pointers to boot and run time services and other tables
 */

typedef struct efi_system_table {
	efi_table_header			hdr;
	int16_t *				firmware_vendor;
	uint32_t				firmware_revision;
	void *					console_in_handle;
	uint64_t				console_in;
	void *					console_out_handle;
	efi_simple_text_output_protocol *	console_out;
	void *					stderr_handle;
	uint64_t				std_err;
	uint64_t				runtime_services;
	efi_boot_services *			boot_services;
	uint64_t				number_of_table_entries;
	uint64_t				configuration_table;
} efi_system_table;

#endif /* _EFI_H_ */

