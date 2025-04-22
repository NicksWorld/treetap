#include "libtap/libtap.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "export.h"

typedef struct memory_buffer {
  size_t head;     // Read/write head
  size_t size;     // Size of contained data
  size_t capacity; // Maximum capacity of buffer
  bool dynamic;    // Whether this buffer can by realloced
  char *buffer;    // Stored data
} memory_buffer;

static size_t mem_read_cb(tap_buffer *buffer, char *dst, size_t capacity) {
  memory_buffer *buff = buffer->userdata;

  size_t remaining_bytes = buff->size - buff->head;
  size_t read_bytes = capacity < remaining_bytes ? capacity : remaining_bytes;
  if (read_bytes == 0) {
    return 0;
  }

  memcpy(dst, buff->buffer + buff->head, read_bytes);
  buff->head += read_bytes;

  return read_bytes;
}

static size_t mem_write_cb(tap_buffer *buffer, char *src, size_t length) {
  memory_buffer *buff = buffer->userdata;

  size_t remaining_alloced = buff->capacity - buff->head;
  if (remaining_alloced < length) {
    if (buff->dynamic) {
      size_t target_size = buff->capacity != 0 ? buff->capacity : 2;
      while (target_size < buff->head + length) {
        target_size *= 2;
      }

      char *replacement_buffer = realloc(buff->buffer, target_size);
      if (!replacement_buffer) {
        buffer->fatal = true;
        return 0;
      }
      buff->buffer = replacement_buffer;
      buff->capacity = target_size;
    } else {
      // Cannot resize, so write as much as possible before EOF
      length = remaining_alloced;
    }
  }

  memcpy(buff->buffer + buff->head, src, length);
  buff->head += length;
  buff->size = buff->size > buff->head ? buff->size : buff->head;

  return length;
}

static bool mem_seek_cb(tap_buffer *buffer, long offset, int whence) {
  memory_buffer *buff = buffer->userdata;

  long target_pos;
  switch (whence) {
  case SEEK_SET:
    target_pos = offset;
    break;
  case SEEK_CUR:
    target_pos = (long)buff->head + offset;
    break;
  case SEEK_END:
    target_pos = (long)buff->size + offset;
    break;
  default:
    buffer->fatal = true;
    return false;
  }

  if (target_pos < 0 || target_pos > (long)buff->head) {
    buffer->fatal = true;
    return false;
  }
  buff->head = target_pos;

  return true;
}

static bool mem_close_cb(tap_buffer *buffer) {
  memory_buffer *buff = buffer->userdata;

  if (buff->dynamic) {
    free(buff->buffer);
  }
  free(buff);
  return true;
}

static void set_memory_callbacks(tap_buffer *buffer) {
  buffer->read = mem_read_cb;
  buffer->write = mem_write_cb;
  buffer->seek = mem_seek_cb;
  buffer->close = mem_close_cb;
}

LIBTAP_EXPORT void tap_buffer_memory(tap_buffer *buffer,
                                     size_t initial_capacity) {
  memory_buffer *buff = calloc(1, sizeof(memory_buffer));
  if (!buff) {
    buffer->fatal = true;
    return;
  }
  buff->buffer = malloc(initial_capacity);
  if (!buff->buffer) {
    buffer->fatal = true;
    return;
  }
  buff->dynamic = true;
  buff->capacity = initial_capacity;
  buffer->userdata = buff;
  set_memory_callbacks(buffer);
}

LIBTAP_EXPORT void tap_buffer_memory_static(tap_buffer *buffer,
                                            char *user_storage, size_t size,
                                            size_t capacity) {
  memory_buffer *buff = calloc(1, sizeof(memory_buffer));
  if (!buff) {
    buffer->fatal = true;
    return;
  }
  buff->buffer = user_storage;
  buff->size = size;
  buff->capacity = capacity;
  buffer->userdata = buff;
  set_memory_callbacks(buffer);
}

LIBTAP_EXPORT char* tap_buffer_memory_get(tap_buffer *buffer, size_t *size) {
  memory_buffer *buff = buffer->userdata;
  if (size) {
    *size = buff->size;
  }
  return buff->buffer;
}
