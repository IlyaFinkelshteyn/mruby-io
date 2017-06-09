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

#ifndef MRB_IO_H
#define MRB_IO_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "mruby.h"
// #include <signal.h>
#include <sys/types.h>


mrb_value internal_sysopen(mrb_state *mrb);
static int mrb_io_modestr_to_flags(mrb_state *mrb, const char *modestr);
static int mrb_io_flags_to_modenum(mrb_state *mrb, int flags);
int mrb_cloexec_open(mrb_state *mrb, const char *pathname, mrb_int flags, mrb_int mode);
void mrb_fd_cloexec(mrb_state *mrb, int fd);



#if defined(__cplusplus)
} /* extern "C" { */
#endif
#endif /* MRB_IO_H */
