#include <libtap/libtap.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void test_buffer(tap_buffer *buffer) {
  assert(buffer != NULL);

  char write_buf[100];
  for (int i = 0; i < 100; i++) {
    write_buf[i] = (char)i;
  }

  // Write buffer 5 times, increasing the set by 1 each time
  for (int i = 0; i < 5; i++) {
    size_t write_result = buffer->write(buffer, write_buf, 100);
    assert(write_result == 100);
    for (int j = 0; j < 100; j++) {
      write_buf[j]++;
    }
  }

  bool seek_result = buffer->seek(buffer, 0, SEEK_SET);
  assert(seek_result);

  // Read data back, ensuring the correct values
  char read_buf[100];
  for (int i = 0; i < 5; i++) {
    size_t read = buffer->read(buffer, read_buf, 100);
    assert(read == 100);
    for (int j = 0; j < 100; j++) {
      assert(read_buf[j] == j + i);
    }
  }

  // Ensure reading past the end results in EOF.
  bool failed_read_eof = buffer->read(buffer, read_buf, 1);
  assert(failed_read_eof == 0);

  // Close the buffer
  buffer->close(buffer);
}

int main(void) {
  tap_buffer tmpfile_buffer;
  tmpfile_buffer.userdata = tmpfile();
  tap_buffer_file(&tmpfile_buffer);

  tap_buffer dyn_memory_buffer;
  tap_buffer_memory(&dyn_memory_buffer, 2);
  tap_buffer dyn_mem_zero_size;
  tap_buffer_memory(&dyn_mem_zero_size, 0);

  char *static_buf = malloc(500);
  tap_buffer static_mem;
  tap_buffer_memory_static(&static_mem, static_buf, 0, 500);

  test_buffer(&tmpfile_buffer);
  test_buffer(&dyn_memory_buffer);
  test_buffer(&dyn_mem_zero_size);
  test_buffer(&static_mem);

  free(static_buf);
  return 0;
}
