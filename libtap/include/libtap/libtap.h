#ifndef LIBTAP_LIBTAP_H
#define LIBTAP_LIBTAP_H

#include <stdint.h>

#define TAP_VERSION 1

typedef enum {
  TAP_ARCH_NEUTRAL = 0,
  TAP_ARCH_AMD64
} tap_architecture_t;

typedef enum {
  TAP_COMPRESS_NONE = 0,
  TAP_COMPRESS_GZIP,
  TAP_COMPRESS_BZIP2,
  TAP_COMPRESS_XZ
} tap_compression_t;

typedef enum {
  // Everyone is okay! ...including me. ~ahill
  TAP_ERROR_OK = 0
} tap_error_t;

typedef struct __attribute__((__packed__)) {
  // Must always be "mapl\x9A"
  uint8_t magic[5];
  // Must always be equal to TAP_VERSION
  uint8_t format;
  // Must be zero
  uint16_t padding;
  // Must be the ed25519 signature of the remaining file
  // FIXME: What is the best way to include a static assertion to compare our 64
  //        bytes to crypto_sign_BYTES. ~ahill
  uint8_t signature[64];
  // Must be the XXH64 hash of the public ed25519 key the package was signed with
  uint64_t signature_hash;
  // Must be the current version, where the newer versions will always have a
  // larger number than the previous versions.
  uint64_t version;
  // Should be zero in most cases. Used to artifically increase version (package
  // revisions, etc.).
  uint8_t epoch;
  // Must be a value that can be mapped to tap_architecture_t
  uint8_t architecture;
  // Must be a value that can be mapped to tap_compression_t
  uint8_t compression;
  // Must be zero
  uint8_t reserved[6];
  // The offset of the compressed archive from the beginning of the file in bytes
  uint64_t data_offset;
  // The length of the compressed archive in bytes
  uint64_t data_length;
  /* To be considered/implemented:
    - Package Name
    - Version String
    - Summary
    - Upstream URL
    - License
    - Maintainer(s)
    - Runtime Dependencies
    - Build Dependencies
  */
} tap_header_t;

#endif
