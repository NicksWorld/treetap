#ifndef LIBTAP_LIBTAP_H
#define LIBTAP_LIBTAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

int tap_version(void);

typedef enum tap_error {
  TAP_ERR_OK,
  TAP_ERR_PARAM,
  TAP_ERR_ALLOC,
  TAP_ERR_IO,
  TAP_ERR_UNSUPPORTED,
} tap_error;

typedef enum tap_compression_format {
  TAP_COMPRESS_GZIP,
  TAP_COMPRESS_ZSTD,
  TAP_COMPRESS_LZMA,
  TAP_COMPRESS_BZIP2,
  TAP_COMPRESS_NONE,
} tap_compression_format;

typedef struct tap_buffer tap_buffer;

/**
 * Read bytes from a buffer.
 *
 * Reads up to `capacity` bytes into `dst`, returning 0 on EOF or error.
 * On error, tap_buffer_fatal is set.
 */
typedef size_t tap_buffer_read_cb(tap_buffer *buffer, void *userdata, char *dst,
                                  size_t capacity);
/**
 * Writes bytes to a buffer.
 *
 * Attempts to write `length` bytes to the buffer, returning 0 and setting
 * tap_buffer_fatal on error, or returning the number of bytes written on eof.
 */
typedef size_t tap_buffer_write_cb(tap_buffer *buffer, void *userdata,
                                   char *src, size_t length);
/**
 * Seeks to a position in the buffer.
 *
 * Attempts to seek to the specified file location. Offset is the position
 * relative to whence, which is the same values as used in fseek.
 */
typedef bool tap_buffer_seek_cb(tap_buffer *buffer, void *userdata, long offset,
                                int whence);
/**
 * Closes or frees the backing buffer storage.
 *
 * This does not free the tap_buffer structure, which must be done using
 * tap_buffer_free.
 */
typedef bool tap_buffer_close_cb(tap_buffer *buffer, void *userdata);

/**
 * Creates a new tap_buffer with userdata as its backing storage
 *
 * New buffers should have applicable read/write/seek/close methods set
 * for operation on the data.
 */
tap_buffer *tap_buffer_new(void *userdata);
void tap_buffer_free(tap_buffer *buffer);

/**
 * Setters for the tap_buffer operation callbacks
 */
void tap_buffer_set_read(tap_buffer *buffer, tap_buffer_read_cb *read);
void tap_buffer_set_write(tap_buffer *buffer, tap_buffer_write_cb *write);
void tap_buffer_set_seek(tap_buffer *buffer, tap_buffer_seek_cb *seek);
void tap_buffer_set_close(tap_buffer *buffer, tap_buffer_close_cb *close);

/**
 * Dispatchers for tap_buffer operations
 */
size_t tap_buffer_read(tap_buffer *buffer, void *dst, size_t capacity);
size_t tap_buffer_write(tap_buffer *buffer, void *src, size_t length);
bool tap_buffer_seek(tap_buffer *buffer, long offset, int whence);
bool tap_buffer_close(tap_buffer *buffer);

/**
 * Set and check the fatal error status
 */
void tap_buffer_set_fatal(tap_buffer *buffer);
bool tap_buffer_fatal(tap_buffer *buffer);

/**
 * Pre-implemented buffer storages
 */
// Creates a buffer backed by a temporary file
tap_buffer *tap_buffer_tmpfile(void);
// Creates a buffer backed by a normal file
tap_buffer *tap_buffer_file(FILE *);
// Creates a buffer backed by a resizing block of memory.
tap_buffer *tap_buffer_dynamic_memory(size_t initial_capacity);
// Creates a buffer backed by a user-provided allocated block with a max size
// of `capacity`. The backing storage is not deallocated by the buffer.
tap_buffer *tap_buffer_static_memory(char *buffer, size_t capacity);

#endif
