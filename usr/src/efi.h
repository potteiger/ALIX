/*
 * ALIX: `efi.h` -- EFI data types/structures
 * Copyright (c) 2023 Alan Potteiger
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef _EFI_H_
#define _EFI_H_

/*
 * Not intended to cover much of the EFI API, only what is required by this
 * bootloader.
 *
 * UEFI Specification Version 2.10 is being referenced:
 * 	https://uefi.org/specs/UEFI/2.10/index.html
 */

/*
 * Protocol GUIDs
 */
#define EFI_LOADED_IMAGE_PROTOCOL_GUID (Efi_guid) \
			{ 0x5B1B31A1, 0x9562, 0x11d2,\
    			{ 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }}

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID (Efi_guid) \
			{ 0x0964e5b22, 0x6459, 0x11d2, \
  			{ 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }}

#define EFI_FILE_INFO_ID (Efi_guid) \
			{ 0x09576e92, 0x6d3f, 0x11d2, \
  			{ 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b }}

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID (Efi_guid) \
			{ 0x9042a9de, 0x23dc, 0x4a38, \
  			{ 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a }}

/*
 * UEFI boolean... 1 byte value
 */
#define EFI_TRUE 1
#define EFI_FALSE 0
typedef int8_t Efi_bool;

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

typedef uint64_t Efi_status;

/*
 * efi_handle is just a void pointer...
 */
typedef void* Efi_handle;

/*
 * Protocol opening attributes
 */

#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL	0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL		0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL		0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER	0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER		0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE		0x00000020

struct Efi_system_table;

/*
 * EFI memory designations for memory maps, pages, pools
 */
typedef enum Efi_memory_type {

	Efi_reserved_memory_type,
	Efi_loader_code,
	Efi_loader_data,
	Efi_boot_services_code,
	Efi_boot_services_data,
	Efi_runtime_services_code,
	Efi_runtime_services_data,
	Efi_conventional_memory,
	Efi_unusable_memory,
	Efi_ACPI_reclaim_memory,
	Efi_ACPI_memoryNVS,
	Efi_memory_mapped_IO,
	Efi_memory_mapped_IO_port_space,
	Efi_pal_code,
	Efi_persistent_memory,
	Efi_unaccepted_memory_type,
	Efi_max_memory_type

} Efi_memory_type;

typedef enum Efi_allocate_type {
	
	Efi_allocate_any_pages,
	Efi_allocate_max_address,
	Efi_allocate_address,
	Efi_max_allocate_type

} Efi_allocate_type;

typedef struct Efi_guid {

	uint32_t	data1;
	uint16_t	data2;
	uint16_t	data3;
	uint8_t		data4[8];

} Efi_guid;

typedef struct Efi_time {

	uint16_t	year;
	uint8_t		month;
	uint8_t		day;
	uint8_t		hour;
	uint8_t		minute;
	uint8_t		second;
	uint8_t		pad1;
	uint32_t	nanosecond;
	int16_t		timeZone;
	uint8_t		daylight;
	uint8_t		pad2;

} Efi_time;

/*
 * Precedes many EFI tables
 */
typedef struct Efi_table_header {

	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;

} Efi_table_header;

/*
 * EFI memory descriptor, array of these are returned from get_memory_map()
 */
typedef struct Efi_memory_descriptor {

	uint32_t		type;
	uint64_t		physical_start;
	uint64_t		virtual_start;
	uint64_t		number_of_pages;
	uint64_t		attribute;

} Efi_memory_descriptor;

/*
 * UEFI device paths
 */
typedef struct Efi_device_path_protocol {

	uint8_t	type;
	uint8_t sub_type;
	uint8_t	length[2];

} Efi_device_path_protocol;

/*
 * Information and procedures attached to loaded EFI image
 */
typedef struct Efi_loaded_image_protocol {

	uint32_t			revision;
	Efi_handle			parent_handle;
	struct Efi_system_table *	system_table;

   	/*
	 * Source location of image
	 */
	Efi_handle			device_handle;
	Efi_device_path_protocol *	file_path;
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

} Efi_loaded_image_protocol;

/*
 * File system access and manipulation
 */
typedef struct Efi_file_info {

	uint64_t	size;
	uint64_t	file_size;
	uint64_t	physical_size;
	Efi_time	create_time;
	Efi_time	last_access_time;
	Efi_time	modification_time;
	uint64_t	attribute;
	int16_t		file_name[];

} Efi_file_info;

#define EFI_FILE_MODE_READ       0x0000000000000001
#define EFI_FILE_MODE_WRITE      0x0000000000000002
#define EFI_FILE_MODE_CREATE     0x8000000000000000

typedef struct Efi_file_protocol {

	uint64_t	revision;

	Efi_status (*open)
	(
		struct Efi_file_protocol *	this,
  		struct Efi_file_protocol **	new_handle,
  		int16_t *			file_name,
  		uint64_t			open_mode,
  		uint64_t			attributes
	);

	void *		close;
	void *		delete;

	Efi_status (*read)
	(
		struct Efi_file_protocol *	this,
		uint64_t *			buffer_size,
		void *				buffer
	);

	void *		write;
	void *		get_position;

	Efi_status (*set_position)
	(
		struct Efi_file_protocol *	this,
		uint64_t 			position
	);

	Efi_status (*get_info)
	(
		struct Efi_file_protocol *	this,
		Efi_guid *			information_type,
		uint64_t *			buffer_size,
		void *				buffer
	);

	void *		set_info;
	void *		flush;
	void *		open_ex;
	void *		read_ex;
	void *		write_ex;
	void *		flush_ex;

} Efi_file_protocol;

/*
 * Get access to root of file system
 */
typedef struct Efi_simple_file_system_protocol {

	uint64_t	revision;

	/*
	 * Opens the root directory of a volume.
	 * Returns efi_file_protocol for it (root)
	 */
	Efi_status (*open_volume)
	(
		struct Efi_simple_file_system_protocol *this,
		Efi_file_protocol **root
	);

} Efi_simple_file_system_protocol;

/*
 * Pointers to all boot services
 */
typedef struct Efi_boot_services {

	Efi_table_header	hdr;
	
	/*
	 * Task priority services
	 */
	void *			raise_TPL;
  	void *			restore_TPL;

	/*
	 * Memory services
	 */

	Efi_status (*allocate_pages)
	(
		Efi_allocate_type	type,
		Efi_memory_type 	memory_type,
		uint64_t		pages,
		uint64_t *		memory
	);

	void *			free_pages;

	/*
	 * Returns the current memory map
	 */
	Efi_status (*get_memory_map)
	(
		uint64_t *		memory_map_size,
		Efi_memory_descriptor *	memory_map,
		uint64_t *		map_key,
		uint64_t *		descriptor_size,
		uint32_t *		descriptor_version
	);
	
	/*
	 * Allocate memory pool
	 */
	Efi_status (*allocate_pool)
	(
		Efi_memory_type		pool_type,
		uint64_t		size,
		void **			buffer
	);

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

	Efi_status (*exit)
	(
		Efi_handle	image_handle,
		Efi_status	exit_status,
		uint64_t	exit_data_size,
		int16_t *	exit_data
	);
	
	void *			unload_image;

	Efi_status (*exit_boot_services)
	(
		Efi_handle 	img_handle,
		uint64_t 	map_key
	);

	/*
	 * Miscellaneous services
	 */
	void *			get_next_monotonic_count;

	Efi_status (*stall)
	(
		uint64_t	microseconds
	);

	void *			set_watchdog_timer;

	/*
	 * Driver support services
	 */
	void *			connect_controller;
	void *			disconnect_controller;

	/*
	 * Open and close protocol services
	 */

	/*
	 * Opens requested protocol against supplied handle
	 */
	Efi_status (*open_protocol)
	(
		Efi_handle 	handle,
		Efi_guid *	protocol,
		void **		interface,
		Efi_handle	agent_handle,
		Efi_handle	controller_handle,
		uint32_t	attributes
	);

	void *			close_protocol;
	void *			open_protocol_information;

	/*
	 * Library services
	 */
	void *			protocols_per_handle;
	void *			locate_handle_buffer;

	Efi_status (*locate_protocol)
	(
		Efi_guid *	protocol,
		void *		registration,
		void **		interface
	);

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

} Efi_boot_services;

/*
 * Basic UEFI terminal output
 */
typedef struct Efi_simple_text_output_protocol {

	/*
	 * Resets text output
	 */
	Efi_status (*reset)
	(
		struct Efi_simple_text_output_protocol *this,
		Efi_bool				extended_verification
	);

	/*
	 * Writes a string to the output
	 */
	Efi_status (*output_string)
	(
		struct Efi_simple_text_output_protocol *this,
		int16_t *				string
	);
	
	void *				test_string;
	void *				query_mode;
	void *				set_mode;

	/*
	 * Sets background and foreground colors
	 */
	Efi_status (*set_attribute)
	(
		struct Efi_simple_text_output_protocol *this,
		uint64_t 				attribute
	);

	/*
	 * Clears the screen
	 */
	Efi_status (*clear_screen)
	(
		struct Efi_simple_text_output_protocol *this
	);
	
	void *				set_cursor_position;
	void *				enable_cursor;
	void *				mode;

} Efi_simple_text_output_protocol;

/*
 * Pointers to boot and run time services and other tables
 */
typedef struct Efi_system_table {

	Efi_table_header			hdr;
	int16_t *				firmware_vendor;
	uint32_t				firmware_revision;
	void *					console_in_handle;
	uint64_t				console_in;
	void *					console_out_handle;
	Efi_simple_text_output_protocol *	console_out;
	void *					stderr_handle;
	uint64_t				std_err;
	uint64_t				runtime_services;
	Efi_boot_services *			boot_services;
	uint64_t				number_of_table_entries;
	uint64_t				configuration_table;

} Efi_system_table;

typedef enum Efi_graphics_pixel_format {

	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax

} Efi_graphics_pixel_format;

typedef struct Efi_pixel_bitmask {

	uint32_t	RedMask;
	uint32_t	GreenMask;
	uint32_t	BlueMask;
	uint32_t	ReservedMask;

} Efi_pixel_bitmask;

typedef struct Efi_graphics_output_mode_information {

	uint32_t 			version;
	uint32_t 			horizontal_resolution;
	uint32_t 			vertical_resolution;
	Efi_graphics_pixel_format 	pixel_format;
	Efi_pixel_bitmask         	pixel_information;
	uint32_t			pixels_per_scan_line;

} Efi_graphics_output_mode_information;

typedef struct Efi_graphics_output_protocol_mode {

	uint32_t				max_mode;
	uint32_t				mode;
	Efi_graphics_output_mode_information *	info;
	uint64_t				size_of_info;
	uintptr_t				framebuffer_base;
	uint64_t				framebuffer_size;

} Efi_graphics_output_protocol_mode;

typedef struct Efi_graphics_output_protocol {

	Efi_status (*query_mode)
	(
		struct Efi_graphics_output_protocol *	this,
		uint32_t				mode_number,
		uint64_t *				sizeof_info,
		Efi_graphics_output_mode_information ** info
	);

	Efi_status (*set_mode)
	(
		struct Efi_graphics_output_protocol *	this,
		uint32_t				mode_number
	);

	void *	blt;
	Efi_graphics_output_protocol_mode *	mode;

} Efi_graphics_output_protocol;

#endif /* _EFI_H_ */

