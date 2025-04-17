#include "libtap/libtap.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "export.h"

typedef struct memory_buffer {
  // Position of head in buffer
  size_t position;
  // Size of written data
  size_t size;
  // Maximum capacity of buffer
  size_t capacity;
  // Whether this interface owns the buffer,
  // if false, the buffer cannot be resized or deallocated.
  bool owned;
  // Buffer storing the data
  char *buffer;
} memory_buffer;

memory_buffer *dyn_mem_buffer_new(size_t initial_capacity) {
  memory_buffer *buff = calloc(1, sizeof(memory_buffer));
  if (!buff) {
    return NULL;
  }
  buff->buffer = malloc(initial_capacity);
  if (!buff->buffer) {
    free(buff);
    return NULL;
  }
  buff->capacity = initial_capacity;
  buff->owned = true;
  return buff;
}

memory_buffer *static_mem_buffer_new(char *buffer, size_t capacity) {
  memory_buffer *buff = calloc(1, sizeof(memory_buffer));
  if (!buff) {
    return NULL;
  }
  buff->buffer = buffer;
  buff->capacity = capacity;
  buff->owned = false;
  return buff;
}

static size_t mem_read_cb(tap_buffer *buffer, void *userdata, char *dst,
                          size_t capacity) {
  (void)buffer; /* UNUSED */
  memory_buffer *buff = userdata;

  size_t remaining_bytes = buff->size - buff->position;
  size_t read_bytes = capacity < remaining_bytes ? capacity : remaining_bytes;
  if (read_bytes == 0) {
    return 0;
  }

  memcpy(dst, buff->buffer + buff->position, read_bytes);
  buff->position += read_bytes;

  return read_bytes;
}

static size_t mem_write_cb(tap_buffer *buffer, void *userdata, char *src,
                           size_t length) {
  memory_buffer *buff = userdata;

  size_t remaining_alloced = buff->capacity - buff->position;
  if (remaining_alloced < length) {
    if (buff->owned) {
      size_t target_size = buff->capacity != 0 ? buff->capacity : 2;
      while (target_size < buff->position + length) {
        target_size *= 2;
      }

      char *replacement_buffer = realloc(buff->buffer, target_size);
      if (!replacement_buffer) {
        tap_buffer_set_fatal(buffer);
        return 0;
      }
      buff->buffer = replacement_buffer;
      buff->capacity = target_size;
    } else {
      // Cannot resize, so write as much as possible before EOF
      length = remaining_alloced;
    }
  }

  memcpy(buff->buffer + buff->position, src, length);
  buff->position += length;
  buff->size = buff->size > buff->position ? buff->size : buff->position;

  return length;
}

static bool mem_seek_cb(tap_buffer *buffer, void *userdata, long offset,
                        int whence) {
  memory_buffer *buff = userdata;

  long target_pos;
  switch (whence) {
  case SEEK_SET:
    target_pos = offset;
    break;
  case SEEK_CUR:
    target_pos = (long)buff->position + offset;
    break;
  case SEEK_END:
    target_pos = (long)buff->size + offset;
    break;
  default:
    tap_buffer_set_fatal(buffer);
    return false;
  }

  if (target_pos < 0 || target_pos > (long)buff->position) {
    tap_buffer_set_fatal(buffer);
    return false;
  }
  buff->position = target_pos;

  return true;
}

static bool mem_close_cb(tap_buffer *buffer, void *userdata) {
  (void)buffer; /* UNUSED */
  memory_buffer *buff = userdata;

  if (buff->owned) {
    free(buff->buffer);
  }
  free(buff);
  return true;
}

static tap_buffer *tap_buffer_memory_init(memory_buffer *mem_buffer) {
  tap_buffer *buffer = tap_buffer_new(mem_buffer);
  if (!buffer) {
    return NULL;
  }
  tap_buffer_set_read(buffer, mem_read_cb);
  tap_buffer_set_write(buffer, mem_write_cb);
  tap_buffer_set_seek(buffer, mem_seek_cb);
  tap_buffer_set_close(buffer, mem_close_cb);
  return buffer;
}

LIBTAP_EXPORT tap_buffer *tap_buffer_dynamic_memory(size_t initial_capacity) {
  memory_buffer *buff = dyn_mem_buffer_new(initial_capacity);
  if (!buff) {
    return NULL;
  }
  return tap_buffer_memory_init(buff);
}

LIBTAP_EXPORT tap_buffer *tap_buffer_static_memory(char *buffer,
                                                   size_t capacity) {
  memory_buffer *buf = static_mem_buffer_new(buffer, capacity);
  if (!buf) {
    return NULL;
  }
  return tap_buffer_memory_init(buf);
}
