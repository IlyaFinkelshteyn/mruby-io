/* MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mruby.h"

#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/ext/io.h"

#include "internal.c"

#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#if MRUBY_RELEASE_NO < 10000
#include "error.h"
#else
#include "mruby/error.h"
#endif

//TODO redundand
int cloexec_open(mrb_state *mrb, const char *pathname, mrb_int flags, mrb_int mode);
void fd_cloexec(mrb_state *mrb, int fd);




mrb_value
mrb_argv0(mrb_state *mrb)
{
    const char *argv0 = getenv("_");

    if (!argv0)
        return mrb_nil_value();

    return mrb_str_new_cstr(mrb,argv0);
}

mrb_value
mrb_progname(mrb_state *mrb)
{
    const char *argv0 = getenv("_");
    const char *progname;

    if (!argv0)
        return mrb_nil_value();

    progname = strrchr(argv0, '/');

    if (progname)
        progname++;
    else
        progname = argv0;

    return mrb_str_new_cstr(mrb, progname);
}

mrb_value
internal_sysopen(mrb_state *mrb)
{
  mrb_value path = mrb_nil_value();
  mrb_value mode = mrb_nil_value();
  mrb_int fd, flags, perm = -1;
  const char *pat;
  int modenum;

  mrb_get_args(mrb, "S|Si", &path, &mode, &perm);
  if (mrb_nil_p(mode)) {
    mode = mrb_str_new_cstr(mrb, "r");
  }
  if (perm < 0) {
    perm = 0666;
  }

  pat = mrb_string_value_cstr(mrb, &path);
  flags = mrb_io_modestr_to_flags(mrb, mrb_string_value_cstr(mrb, &mode));
  modenum = mrb_io_flags_to_modenum(mrb, flags);
  fd = cloexec_open(mrb, pat, modenum, perm);
  return mrb_fixnum_value(fd);
}

int
cloexec_open(mrb_state *mrb, const char *pathname, mrb_int flags, mrb_int mode)
{
  mrb_value emsg;
  int fd, retry = FALSE;

#ifdef O_CLOEXEC
  /* O_CLOEXEC is available since Linux 2.6.23.  Linux 2.6.18 silently ignore it. */
  flags |= O_CLOEXEC;
#elif defined O_NOINHERIT
  flags |= O_NOINHERIT;
#endif
reopen:
  fd = open(pathname, flags, mode);
  if (fd == -1) {
    if (!retry) {
      switch (errno) {
        case ENFILE:
        case EMFILE:
        mrb_garbage_collect(mrb);
        retry = TRUE;
        goto reopen;
      }
    }

    emsg = mrb_format(mrb, "open %S", mrb_str_new_cstr(mrb, pathname));
    mrb_str_modify(mrb, mrb_str_ptr(emsg));
    mrb_sys_fail(mrb, RSTRING_PTR(emsg));
  }

  if (fd <= 2) {
    fd_cloexec(mrb, fd);
  }
  return fd;
}

void
fd_cloexec(mrb_state *mrb, int fd)
{
#if defined(F_GETFD) && defined(F_SETFD) && defined(FD_CLOEXEC)
  int flags, flags2;

  flags = fcntl(fd, F_GETFD);
  if (flags == -1) {
    mrb_sys_fail(mrb, "fcntl");
  }
  if (fd <= 2) {
    flags2 = flags & ~FD_CLOEXEC; /* Clear CLOEXEC for standard file descriptors: 0, 1, 2. */
  }
  else {
    flags2 = flags | FD_CLOEXEC; /* Set CLOEXEC for non-standard file descriptors: 3, 4, 5, ... */
  }
  if (flags != flags2) {
    if (fcntl(fd, F_SETFD, flags2) == -1) {
      mrb_sys_fail(mrb, "fcntl");
    }
  }
#endif
}
