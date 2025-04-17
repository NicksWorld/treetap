#include "libtap/libtap.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "export.h"

size_t file_read_cb(tap_buffer *buffer, void *userdata, char *dst,
                    size_t capacity) {
  FILE *file = userdata;

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
      tap_buffer_set_fatal(buffer);
      return 0;
    }

    if (feof(file)) {
      return read;
    }
  }

  return read;
}

size_t file_write_cb(tap_buffer *buffer, void *userdata, char *src,
                     size_t length) {
  FILE *file = userdata;

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
        tap_buffer_set_fatal(buffer);
        return 0;
      }

      if (feof(file)) {
        return written;
      }
    }
  }

  return written;
}

bool file_seek_cb(tap_buffer *buffer, void *userdata, long offset, int whence) {
  FILE *file = userdata;

  for (;;) {
    int res = fseek(file, offset, whence);
    if (res == 0)
      return true;

    if (errno == EINTR) {
      clearerr(file);
      continue;
    }

    tap_buffer_set_fatal(buffer);
    return false;
  }
}

bool file_close_cb(tap_buffer *buffer, void *userdata) {
  FILE *file = userdata;
  int res = fclose(file);
  if (res == 0)
    return true;

  tap_buffer_set_fatal(buffer);
  return false;
}

LIBTAP_EXPORT tap_buffer *tap_buffer_file(FILE *file) {
  tap_buffer *buffer = tap_buffer_new(file);
  tap_buffer_set_read(buffer, file_read_cb);
  tap_buffer_set_write(buffer, file_write_cb);
  tap_buffer_set_seek(buffer, file_seek_cb);
  tap_buffer_set_close(buffer, file_close_cb);

  return buffer;
}

LIBTAP_EXPORT tap_buffer *tap_buffer_tmpfile(void) {
  FILE *file = tmpfile();
  if (!file) {
    return NULL;
  }
  return tap_buffer_file(file);
}
