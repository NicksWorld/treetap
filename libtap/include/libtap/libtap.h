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
 * Reads `capacity` bytes into dst, returning the number read.
 * The return value will differ from `capacity` if EOF or an error occurs.
 */
typedef size_t tap_buffer_read_cb(tap_buffer *buffer, char *dst,
                                  size_t capacity);
/**
 * Writes bytes to a buffer.
 *
 * Writes `length` bytes into the buffer, returning the number written.
 * The return value will differ from `length` if EOF or an error occurs.
 */
typedef size_t tap_buffer_write_cb(tap_buffer *buffer, char *src,
                                   size_t length);
/**
 * Seeks to a position in the buffer.
 *
 * Attempts to seek to the specified file location. Offset is the position
 * relative to whence, which is the same values as used in fseek.
 */
typedef bool tap_buffer_seek_cb(tap_buffer *buffer, long offset, int whence);
/**
 * Closes and/or frees the backing buffer storage.
 */
typedef bool tap_buffer_close_cb(tap_buffer *buffer);

struct tap_buffer {
  void *userdata;

  // Methods for operating upon the userdata
  tap_buffer_read_cb *read;
  tap_buffer_write_cb *write;
  tap_buffer_seek_cb *seek;
  tap_buffer_close_cb *close;

  // Set upon a fatal error
  bool fatal;
};

/**
 * Pre-implemented buffer storages
 */
// Helper method to set the buffer_file callbacks
void tap_buffer_file(tap_buffer *buffer);

size_t tap_buffer_file_read_cb(tap_buffer *buffer, char *dst, size_t capacity);
size_t tap_buffer_file_write_cb(tap_buffer *buffer, char *src, size_t length);
bool tap_buffer_file_seek_cb(tap_buffer *buffer, long offset, int whence);
bool tap_buffer_file_close_cb(tap_buffer *buffer);

// Creates a buffer backed by a resizing block of memory. Closing the buffer
// deallocates the memory.
void tap_buffer_memory(tap_buffer* buffer, size_t initial_capacity);
// Get a reference to a memory buffer's contents.
// Performing io operations on the underlying buffer will invalidate the pointer.
char* tap_buffer_memory_get(tap_buffer* buffer, size_t *size);

// Creates a buffer backed by a user-provided allocated block, containing `size`
// written bytes, and a maximum capacity of `capacity`.
// The user_storage buffer is not freed upon buffer closure, and must be done
// by the user.
void tap_buffer_memory_static(tap_buffer* buffer, char *user_storage, size_t size, size_t capacity);

#endif
