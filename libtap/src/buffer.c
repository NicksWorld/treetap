#include "libtap/libtap.h"

#include <stdbool.h>
#include <stdlib.h>

#include "export.h"

struct tap_buffer {
  void *userdata;
  // Callbacks to operate on the userdata
  tap_buffer_read_cb *read;
  tap_buffer_write_cb *write;
  tap_buffer_seek_cb *seek;
  tap_buffer_close_cb *close;

  bool fatal;
};

LIBTAP_EXPORT tap_buffer *tap_buffer_new(void *userdata) {
  tap_buffer *buffer = calloc(1, sizeof(tap_buffer));
  if (!buffer) {
    return NULL;
  }
  buffer->userdata = userdata;
  return buffer;
}
LIBTAP_EXPORT void tap_buffer_free(tap_buffer *buffer) { free(buffer); }

LIBTAP_EXPORT bool tap_buffer_fatal(tap_buffer *buffer) {
  return buffer->fatal;
}

/* Interface Method Setters */
LIBTAP_EXPORT void tap_buffer_set_read(tap_buffer *buffer,
                                       tap_buffer_read_cb *read) {
  buffer->read = read;
}
LIBTAP_EXPORT void tap_buffer_set_write(tap_buffer *buffer,
                                        tap_buffer_write_cb *write) {
  buffer->write = write;
}
LIBTAP_EXPORT void tap_buffer_set_seek(tap_buffer *buffer,
                                       tap_buffer_seek_cb *seek) {
  buffer->seek = seek;
}
LIBTAP_EXPORT void tap_buffer_set_close(tap_buffer *buffer,
                                        tap_buffer_close_cb *close) {
  buffer->close = close;
}

LIBTAP_EXPORT void tap_buffer_set_fatal(tap_buffer *buffer) {
  buffer->fatal = true;
}

/* Interface Dispatching Methods */
LIBTAP_EXPORT size_t tap_buffer_read(tap_buffer *buffer, void *dst,
                                     size_t capacity) {
  if (!buffer->read) {
    return 0; // Find better way of outputting error
  }

  return buffer->read(buffer, buffer->userdata, dst, capacity);
}
LIBTAP_EXPORT size_t tap_buffer_write(tap_buffer *buffer, void *src,
                                      size_t length) {
  if (!buffer->write) {
    return TAP_ERR_UNSUPPORTED;
  }

  return buffer->write(buffer, buffer->userdata, src, length);
}
LIBTAP_EXPORT bool tap_buffer_seek(tap_buffer *buffer, long offset,
                                   int whence) {
  if (!buffer->seek) {
    return TAP_ERR_UNSUPPORTED;
  }

  return buffer->seek(buffer, buffer->userdata, offset, whence);
}
LIBTAP_EXPORT bool tap_buffer_close(tap_buffer *buffer) {
  if (!buffer->close) {
    return true;
  }
  return buffer->close(buffer, buffer->userdata);
}

LIBTAP_EXPORT tap_error tap_buffer_copy(tap_buffer *dst, tap_buffer *src,
                                        size_t *copied) {
  char readbuf[1024];
  for (;;) {
    size_t read = tap_buffer_read(src, readbuf, 1024);
    if (read != 0) {
      size_t written = tap_buffer_write(dst, readbuf, read);
      if (copied) {
        *copied += written;
      }
      if (written != read) {
        return TAP_ERR_IO;
      }
      continue;
    }

    if (tap_buffer_fatal(src) || tap_buffer_fatal(dst)) {
      return TAP_ERR_IO;
    }
  }
}
