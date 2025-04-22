#include "libtap/libtap.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "export.h"

LIBTAP_EXPORT size_t tap_buffer_file_read_cb(tap_buffer *buffer, char *dst,
                    size_t capacity) {
  FILE *file = buffer->userdata;

  size_t read = 0;
  while (read < capacity) {
    size_t try_read = capacity - read;
    size_t n = fread(dst + read, 1, try_read, file);
    read += n;

    if (n == try_read)
      continue;

    if (ferror(file)) {
      if (errno == EINTR) {
        clearerr(file);
        continue;
      }
      buffer->fatal = true;
      return 0;
    }

    if (feof(file)) {
      return read;
    }
  }

  return read;
}

LIBTAP_EXPORT size_t tap_buffer_file_write_cb(tap_buffer *buffer, char *src,
                     size_t length) {
  FILE *file = buffer->userdata;

  size_t written = 0;
  while (written < length) {
    size_t try_write = length - written;
    size_t n = fwrite(src, 1, try_write, file);
    written += n;

    if (n != try_write) {
      if (ferror(file)) {
        if (errno == EINTR) {
          clearerr(file);
          continue;
        }
        buffer->fatal = true;
        return 0;
      }

      if (feof(file)) {
        return written;
      }
    }
  }

  return written;
}

LIBTAP_EXPORT bool tap_buffer_file_seek_cb(tap_buffer *buffer, long offset, int whence) {
  FILE *file = buffer->userdata;

  for (;;) {
    int res = fseek(file, offset, whence);
    if (res == 0)
      return true;

    if (errno == EINTR) {
      clearerr(file);
      continue;
    }

    buffer->fatal = true;
    return false;
  }
}

LIBTAP_EXPORT bool tap_buffer_file_close_cb(tap_buffer *buffer) {
  FILE *file = buffer->userdata;
  int res = fclose(file);
  if (res == 0)
    return true;

  buffer->fatal = true;
  return false;
}

LIBTAP_EXPORT void tap_buffer_file(tap_buffer* buffer) {
  buffer->read = tap_buffer_file_read_cb;
  buffer->write = tap_buffer_file_write_cb;
  buffer->seek = tap_buffer_file_seek_cb;
  buffer->close =  tap_buffer_file_close_cb;
}
