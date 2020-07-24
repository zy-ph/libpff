/*
 * Input/Output (IO) handle functions
 *
 * Copyright (C) 2008-2020, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include "libpff_allocation_table.h"
#include "libpff_codepage.h"
#include "libpff_definitions.h"
#include "libpff_file_header.h"
#include "libpff_index.h"
#include "libpff_index_node.h"
#include "libpff_index_tree.h"
#include "libpff_index_value.h"
#include "libpff_item_descriptor.h"
#include "libpff_io_handle.h"
#include "libpff_libbfio.h"
#include "libpff_libcdata.h"
#include "libpff_libcerror.h"
#include "libpff_libcnotify.h"
#include "libpff_libfcache.h"
#include "libpff_libfdata.h"
#include "libpff_libfmapi.h"
#include "libpff_local_descriptor_node.h"
#include "libpff_unused.h"

#include "pff_file_header.h"

const uint8_t pff_file_signature[ 4 ] = { 0x21, 0x42, 0x44, 0x4e };

/* Creates an IO handle
 * Make sure the value io_handle is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libpff_io_handle_initialize(
     libpff_io_handle_t **io_handle,
     libcerror_error_t **error )
{
	static char *function = "libpff_io_handle_initialize";

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( *io_handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid IO handle value already set.",
		 function );

		return( -1 );
	}
	*io_handle = memory_allocate_structure(
	              libpff_io_handle_t );

	if( *io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create IO handle.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *io_handle,
	     0,
	     sizeof( libpff_io_handle_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear file.",
		 function );

		goto on_error;
	}
	( *io_handle )->ascii_codepage = LIBPFF_CODEPAGE_WINDOWS_1252;

	return( 1 );

on_error:
	if( *io_handle != NULL )
	{
		memory_free(
		 *io_handle );

		*io_handle = NULL;
	}
	return( -1 );
}

/* Frees an IO handle
 * Returns 1 if successful or -1 on error
 */
int libpff_io_handle_free(
     libpff_io_handle_t **io_handle,
     libcerror_error_t **error )
{
	static char *function = "libpff_io_handle_free";
	int result            = 1;

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( *io_handle != NULL )
	{
		if( libpff_io_handle_clear(
		     *io_handle,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to clear IO handle.",
			 function );

			result = -1;
		}
		memory_free(
		 *io_handle );

		*io_handle = NULL;
	}
	return( result );
}

/* Clears the IO handle
 * Returns 1 if successful or -1 on error
 */
int libpff_io_handle_clear(
     libpff_io_handle_t *io_handle,
     libcerror_error_t **error )
{
	static char *function = "libpff_io_handle_clear";
	int result            = 1;

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( memory_set(
	     io_handle,
	     0,
	     sizeof( libpff_io_handle_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear IO handle.",
		 function );

		result = -1;
	}
	io_handle->ascii_codepage = LIBPFF_CODEPAGE_WINDOWS_1252;

	return( result );
}

/* Reads the unallocated data blocks
 * Returns 1 if successful or -1 on error
 */
int libpff_io_handle_read_unallocated_data_blocks(
     libpff_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     libcdata_range_list_t *unallocated_data_block_list,
     libcerror_error_t **error )
{
	static char *function           = "libpff_io_handle_read_unallocated_data_blocks";
	off64_t allocation_table_offset = 0;
	size64_t allocation_block_size  = 0;

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( ( io_handle->file_type != LIBPFF_FILE_TYPE_32BIT )
	 && ( io_handle->file_type != LIBPFF_FILE_TYPE_64BIT )
	 && ( io_handle->file_type != LIBPFF_FILE_TYPE_64BIT_4K_PAGE ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported file type.",
		 function );

		return( -1 );
	}
	if( ( io_handle->file_type == LIBPFF_FILE_TYPE_32BIT )
	 || ( io_handle->file_type == LIBPFF_FILE_TYPE_64BIT ) )
	{
		allocation_table_offset = 0x4400;
		allocation_block_size   = 496 * 8 * 64;
	}
	else
	{
		allocation_table_offset = 0x22000;
		allocation_block_size   = 4072 * 8 * 512;
	}
	while( allocation_table_offset < (off64_t) io_handle->file_size )
	{
		if( libpff_allocation_table_read_file_io_handle(
		     unallocated_data_block_list,
		     file_io_handle,
		     allocation_table_offset,
		     (int) io_handle->file_type,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read allocation table at offset: %" PRIi64 ".",
			 function,
			 allocation_table_offset );

			return( -1 );
		}
		allocation_table_offset += allocation_block_size;
	}
	return( 1 );
}

/* Reads the unallocated page blocks
 * Returns 1 if successful or -1 on error
 */
int libpff_io_handle_read_unallocated_page_blocks(
     libpff_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     libcdata_range_list_t *unallocated_page_block_list,
     libcerror_error_t **error )
{
	static char *function           = "libpff_io_handle_read_unallocated_page_blocks";
	off64_t allocation_table_offset = 0;
	size64_t allocation_block_size  = 0;

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( ( io_handle->file_type != LIBPFF_FILE_TYPE_32BIT )
	 && ( io_handle->file_type != LIBPFF_FILE_TYPE_64BIT ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported file type.",
		 function );

		return( -1 );
	}
	allocation_table_offset = 0x4600;
	allocation_block_size   = 496 * 8 * 512;

	while( allocation_table_offset < (off64_t) io_handle->file_size )
	{
		if( libpff_allocation_table_read_file_io_handle(
		     unallocated_page_block_list,
		     file_io_handle,
		     allocation_table_offset,
		     (int) io_handle->file_type,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read allocation table at offset: %" PRIi64 ".",
			 function,
			 allocation_table_offset );

			return( -1 );
		}
		allocation_table_offset += allocation_block_size;
	}
	return( 1 );
}

/* Reads an index node
 * Returns 1 if successful or -1 on error
 */
int libpff_io_handle_read_index_node(
     libpff_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     libfdata_vector_t *vector,
     libfdata_cache_t *cache,
     int element_index,
     int element_data_file_index LIBPFF_ATTRIBUTE_UNUSED,
     off64_t element_data_offset,
     size64_t element_data_size LIBPFF_ATTRIBUTE_UNUSED,
     uint32_t element_data_flags LIBPFF_ATTRIBUTE_UNUSED,
     uint8_t read_flags LIBPFF_ATTRIBUTE_UNUSED,
     libcerror_error_t **error )
{
	libpff_index_node_t *index_node = NULL;
	static char *function           = "libpff_io_handle_read_index_node";

	LIBPFF_UNREFERENCED_PARAMETER( element_data_file_index )
	LIBPFF_UNREFERENCED_PARAMETER( element_data_size )
	LIBPFF_UNREFERENCED_PARAMETER( element_data_flags )
	LIBPFF_UNREFERENCED_PARAMETER( read_flags )

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	/* TODO check if element data size matches index node size
	 * remove corresponding LIBPFF_ATTRIBUTE_UNUSED, LIBPFF_UNREFERENCED_PARAMETER
	 */
	if( libpff_index_node_initialize(
	     &index_node,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create index node.",
		 function );

		goto on_error;
	}
	if( libpff_index_node_read_file_io_handle(
	     index_node,
	     file_io_handle,
	     element_data_offset,
	     io_handle->file_type,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read index node at offset: %" PRIi64 ".",
		 function,
		 element_data_offset );

		goto on_error;
	}
	if( libfdata_vector_set_element_value_by_index(
	     vector,
	     (intptr_t *) file_io_handle,
	     cache,
	     element_index,
	     (intptr_t *) index_node,
	     (int (*)(intptr_t **, libcerror_error_t **)) &libpff_index_node_free,
	     LIBFDATA_LIST_ELEMENT_VALUE_FLAG_MANAGED,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to set index node as element value.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( index_node != NULL )
	{
		libpff_index_node_free(
		 &index_node,
		 NULL );
	}
	return( -1 );
}

