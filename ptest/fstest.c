#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "uv.h"
//#include "task.h"

#include <errno.h>
#include <string.h> /* memset */
#include <fcntl.h>


/* compile n build : # gcc -Wall -g fstest.c -o fstest.test ../.libs/libuv.a -pthread -I ../include/

to build and run libuv tests:

# ./gyp_uv.py -f make
# make -C out
# ./out/Debug/run-tests
*/

static uv_fs_t open_req1;
static uv_fs_t read_req;
static uv_fs_t close_req;
static uv_fs_t write_req;
static uv_fs_t ftruncate_req;
static uv_loop_t* loop;

static int read_cb_count;

static char buf[32];
static char test_buf[] = "test-buffer\n";
static uv_buf_t iov;

static void read_cb(uv_fs_t* req) {
  int r;

  printf("Inside ....read_cb \n");
  assert(req == &read_req);
  assert(req->fs_type == UV_FS_READ);
  assert(req->result >= 0);  /* FIXME(bnoordhuis) Check if requested size? */
  read_cb_count++;
  uv_fs_req_cleanup(req);
  if (read_cb_count == 1) {
    assert(strcmp(buf, test_buf) == 0);
    r = uv_fs_ftruncate(loop, &ftruncate_req, open_req1.result, 7,
        NULL);
  } else {
    assert(strcmp(buf, "test-bu") == 0);
    r = uv_fs_close(loop, &close_req, open_req1.result, NULL);
  }
  assert(r == 0);
  printf("exiting ....read_cb \n");
}


int main()
{
  int r;

  loop = uv_default_loop();

  uv_run(loop, UV_RUN_DEFAULT);

  r = uv_fs_open(NULL, &open_req1, "test_file", O_WRONLY | O_CREAT,
      S_IWUSR | S_IRUSR, NULL);
  assert(r >= 0);
  uv_fs_req_cleanup(&open_req1);

  iov = uv_buf_init(test_buf, sizeof(test_buf));
  r = uv_fs_write(NULL, &write_req, open_req1.result, &iov, 1, -1, NULL);
  assert(r >= 0);
  assert(write_req.result >= 0);
  uv_fs_req_cleanup(&write_req);

  r = uv_fs_close(NULL, &close_req, open_req1.result, NULL);
  assert(r == 0);
  assert(close_req.result == 0);
  uv_fs_req_cleanup(&close_req);

  r = uv_fs_open(NULL, &open_req1, "test_file", O_RDONLY, 0, NULL);
  assert(r >= 0);
  assert(open_req1.result >= 0);
  uv_fs_req_cleanup(&open_req1);

  memset(buf, 0, sizeof(buf));
  iov = uv_buf_init(buf, sizeof(buf));
 // r = uv_fs_read(NULL, &read_req, open_req1.result, &iov, 1, -1, read_cb);
  r = uv_fs_read(NULL, &read_req, open_req1.result, &iov, 1, -1, NULL);
  assert(r >= 0);
  assert(read_req.result >= 0);
  assert(strcmp(buf, test_buf) == 0);
  uv_fs_req_cleanup(&read_req);

  iov = uv_buf_init(buf, sizeof(buf));
  r = uv_fs_read(NULL, &read_req, open_req1.result, &iov, 1,
                 read_req.result, NULL);
  assert(r == 0);
  assert(read_req.result == 0);
  uv_fs_req_cleanup(&read_req);

  r = uv_fs_close(NULL, &close_req, open_req1.result, NULL);
  assert(r == 0);
  assert(close_req.result == 0);
  uv_fs_req_cleanup(&close_req);

  /* Cleanup */
  unlink("test_file");

  return 0;
}
