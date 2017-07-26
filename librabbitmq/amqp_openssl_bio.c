/*
 * Portions created by Alan Antonuk are Copyright (c) 2017 Alan Antonuk.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


#include "amqp_openssl_bio.h"
#include "amqp_socket.h"
#include "threads.h"

#include <errno.h>
#if ((defined(_WIN32)) || (defined(__MINGW32__)) || (defined(__MINGW64__)))
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include <winsock2.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
#endif

#ifdef MSG_NOSIGNAL
# define AMQP_USE_AMQP_BIO
#endif

#ifdef AMQP_USE_AMQP_BIO

#ifdef ENABLE_THREAD_SAFETY
static pthread_once_t bio_init_once = PTHREAD_ONCE_INIT;
#endif

static int bio_initialized = 0;
static BIO_METHOD amqp_bio_method;

static int amqp_openssl_bio_should_retry(int res) {
  if (res == -1) {
    int err = amqp_os_socket_error();
    if (
#ifdef EWOULDBLOCK
        err == EWOULDBLOCK ||
#endif
#ifdef WSAEWOULDBLOCK
        err == WSAEWOULDBLOCK ||
#endif
#ifdef ENOTCONN
        err == ENOTCONN ||
#endif
#ifdef EINTR
        err == EINTR ||
#endif
#ifdef EAGAIN
        err == EAGAIN ||
#endif
#ifdef EPROTO
        err == EPROTO ||
#endif
#ifdef EINPROGRESS
        err == EINPROGRESS ||
#endif
#ifdef EALREADY
        err == EALREADY ||
#endif
        0) {
      return 1;
    }
  }
  return 0;
}

static int amqp_openssl_bio_write(BIO* b, const char *in, int inl) {
  int flags = 0;
  int fd;
  int res;

#ifdef MSG_NOSIGNAL
  flags |= MSG_NOSIGNAL;
#endif

  BIO_get_fd(b, &fd);
  res = send(fd, in, inl, flags);

  BIO_clear_retry_flags(b);
  if (res <= 0 && amqp_openssl_bio_should_retry(res)) {
    BIO_set_retry_write(b);
  }

  return res;
}

static int amqp_openssl_bio_read(BIO* b, char* out, int outl) {
  int flags = 0;
  int fd;
  int res;

#ifdef MSG_NOSIGNAL
  flags |= MSG_NOSIGNAL;
#endif

  BIO_get_fd(b, &fd);
  res = recv(fd, out, outl, flags);

  BIO_clear_retry_flags(b);
  if (res <= 0 && amqp_openssl_bio_should_retry(res)) {
    BIO_set_retry_read(b);
  }

  return res;
}

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
static int BIO_meth_set_write(BIO_METHOD *biom,
                              int (*wfn)(BIO *, const char *, int)) {
  biom->bwrite = wfn;
  return 0;
}

static int BIO_meth_set_read(BIO_METHOD *biom,
                              int (*rfn)(BIO *, char *, int)) {
  biom->bread = rfn;
  return 0;
}
#endif

static void amqp_openssl_bio_init(void) {
  memcpy(&amqp_bio_method, BIO_s_socket(), sizeof(amqp_bio_method));
  BIO_meth_set_write(&amqp_bio_method, amqp_openssl_bio_write);
  BIO_meth_set_read(&amqp_bio_method, amqp_openssl_bio_read);

  bio_initialized = 1;
}

#endif  /* AMQP_USE_AMQP_BIO */

BIO_METHOD* amqp_openssl_bio(void) {
#ifdef AMQP_USE_AMQP_BIO
  if (!bio_initialized) {
#ifdef ENABLE_THREAD_SAFETY
    pthread_once(&bio_init_once, amqp_openssl_bio_init);
#else
    amqp_openssl_bio_init();
#endif /* ifndef ENABLE_THREAD_SAFETY */
  }

  return &amqp_bio_method;
#else
  return BIO_s_socket();
#endif
}
