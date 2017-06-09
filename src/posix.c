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
