#include "libtap/libtap.h"

#include <sodium.h>

#include "signing.h"

static char *get_compression_string(tap_compression_format format) {
  switch (format) {
  case TAP_COMPRESS_GZIP:
    return "gzip";
  case TAP_COMPRESS_ZSTD:
    return "zstd";
  case TAP_COMPRESS_LZMA:
    return "lzma";
  case TAP_COMPRESS_BZIP2:
    return "bz2\0";
  case TAP_COMPRESS_NONE:
    return "none";
  default:
    return NULL;
  }
}

tap_error tap_package_write(tap_buffer *out, tap_keypair *keypair,
                            tap_compression_format compression,
                            tap_buffer *control, tap_buffer *data) {
  if (tap_buffer_write(out, "tap\0", 4) != 4) {
    return TAP_ERR_IO;
  }

  uint8_t version = tap_version();
  if (tap_buffer_write(out, &version, 1) != 1) {
    return TAP_ERR_IO;
  }

  char *compression_format = get_compression_string(compression);
  if (!compression_format) {
    return TAP_ERR_UNSUPPORTED;
  }

  if (tap_buffer_write(out, compression_format, 4) != 4) {
    return TAP_ERR_IO;
  }

  tap_buffer* content_block = tap_buffer_tmpfile();
  if (!content_block) {
    return TAP_ERR_IO;
  }

  size_t control_size, data_size;
  if (tap_buffer_copy(content_block, control, &control_size) != TAP_ERR_OK) {
    return TAP_ERR_IO;
  }
  if (tap_buffer_copy(content_block, data, &data_size) != TAP_ERR_OK) {
    return TAP_ERR_IO;
  }
  if (!tap_buffer_seek(content_block, 0, SEEK_SET)) {
    return TAP_ERR_IO;
  }

  // Sign the data block
  unsigned char signature[crypto_sign_BYTES];
  tap_keypair_sign(keypair, content_block, signature);

  if (tap_buffer_write(out, 

  return TAP_ERR_OK;
}
