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

#include <stdlib.h>
#include <string.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mruby/hash.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/ext/io.h"

#include <sys/stat.h>
#include <fcntl.h>

#if MRUBY_RELEASE_NO < 10000
#include "error.h"
#else
#include "mruby/error.h"
#endif

#include <errno.h>



typedef struct mrb_execarg {
    struct {
        mrb_value in;
        mrb_value out;
        mrb_value err;
    } fd;
    char **envp;
    char *filename;
    char **argv;
    int argc;
} mrb_execarg;

static int mrb_execarg_argv_to_strv(mrb_state *mrb, mrb_value *argv, mrb_int len, char **result);
static void mrb_execarg_fill(mrb_state *mrb, mrb_value env, mrb_value *argv, mrb_int argc, mrb_value opts, struct mrb_execarg *eargp);
static int mrb_io_modestr_to_flags(mrb_state *mrb, const char *modestr);
static int mrb_io_flags_to_modenum(mrb_state *mrb, int flags);



struct mrb_execarg*
mrb_execarg_new(mrb_state *mrb)
{
    mrb_int argc;
    mrb_value *argv, env, opts;
    struct mrb_execarg *eargp;

    mrb_get_args(mrb, "o|*", &env, &argv, &argc);

    switch (mrb_type(env)) {
        case MRB_TT_HASH:
            break;

        case MRB_TT_STRING:
            mrb_get_args(mrb, "*", &argv, &argc);
            env = mrb_nil_value();
            break;

        default:
            mrb_raisef(mrb, E_TYPE_ERROR, "no implicit conversion of %S into String",
                       mrb_obj_value(mrb_class(mrb, env)));
    }

    if (argc > 1 && mrb_hash_p(argv[argc - 1])) {
        opts = argv[argc - 1];
        argc--;
    } else opts = mrb_nil_value();

    eargp = malloc(sizeof(struct mrb_execarg));
    mrb_execarg_fill(mrb, env, argv, argc, opts, eargp);

    return eargp;
}

static void
mrb_execarg_fill(mrb_state *mrb, mrb_value env, mrb_value *argv, mrb_int argc, mrb_value opts, struct mrb_execarg *eargp)
{
    int ai, use_cmd;
    char **result;
    char *shell;
    const char *tCmd;
	char fbuf[/*MAXPATHLEN*/80]; // TODO
    mrb_value argv0 = mrb_nil_value();

    ai   = mrb_gc_arena_save(mrb);
    tCmd = mrb_string_value_ptr(mrb, argv[0]);

    if (mrb_hash_p(opts)) {
        eargp->fd.in  = mrb_hash_get(mrb, opts, mrb_check_intern(mrb, "in", 2));
        eargp->fd.out = mrb_hash_get(mrb, opts, mrb_check_intern(mrb, "out", 3));
        eargp->fd.err = mrb_hash_get(mrb, opts, mrb_check_intern(mrb, "err", 3));
    } else {
        eargp->fd.in  = eargp->fd.out = eargp->fd.err = mrb_nil_value();
    }

#if defined(__APPLE__) || defined(__linux__)
    use_cmd = (argc > 1 || !strrchr(tCmd, ' ')) ? 1 : 0;
#else
    use_cmd = (argc > 1 || strstr(tCmd, ".exe")) ? 1 : 0;
#endif

    if (use_cmd) {
        result = (char **)mrb_malloc(mrb, sizeof(char *) * (argc + 1));
        mrb_execarg_argv_to_strv(mrb, argv, argc, result);
    } else {
        argc   = 3;
        result = (char **)mrb_malloc(mrb, sizeof(char *) * (argc + 1));





    #if defined(__APPLE__) || defined(__linux__)
        shell = getenv("SHELL");
        // if (!shell) shell = strdup(dln_find_exe_r("sh", 0, fbuf, sizeof(fbuf)));
        result[1] = strdup("-c");
    #else
        shell = getenv("ComSpec");
        if (!shell) shell = strdup("C:\\WINDOWS\\system32\\cmd.exe");
        result[1] = strdup("/c");
    #endif
        result[0] = shell;
        result[2] = mrb_str_to_cstr(mrb, argv[0]);
    }

    result[argc] = NULL;

#if defined(__APPLE__) || defined(__linux__)
    if (result[0][0] != '/') {
        argv0 = mrb_str_new(mrb, "/bin/", 5); //TODO redundand?
    }
#else
    if (result[0][1] != ':') {
        argv0 = mrb_str_new(mrb, "C:\\GnuWin\\bin\\", 14); // TODO: !!!
    }
#endif

    if (mrb_bool(argv0)) {
        mrb_str_cat_cstr(mrb, argv0, result[0]);
        result[0] = mrb_str_to_cstr(mrb, argv0);
    }

    eargp->envp     = NULL;
    eargp->filename = result[0];
    eargp->argv     = result;
    eargp->argc     = argc;

    if (mrb_test(env)) {
        mrb_int len;
        mrb_value keys;
        char **envp;
        int i;

        keys = mrb_hash_keys(mrb, env);
        len  = RARRAY_LEN(keys);
        envp = (char **)mrb_malloc(mrb, sizeof(char *) * (len + 1));

        for (i = 0; i < len; ++i) {
            mrb_value key  = mrb_ary_ref(mrb, keys, i);
            mrb_value val  = mrb_hash_get(mrb, env, key);
            mrb_value skey = mrb_symbol_p(key) ? mrb_sym2str(mrb, mrb_symbol(key)) : key;
            mrb_value sval = mrb_convert_type(mrb, val, MRB_TT_STRING, "String", "to_s");
            mrb_int slen   = RSTRING_LEN(skey) + RSTRING_LEN(sval) + 1;
            char str[slen];

            sprintf(str, "%s=%s",
                mrb_string_value_cstr(mrb, &skey),
                mrb_string_value_cstr(mrb, &sval));

            envp[i] = strdup(str);
        }

        envp[i]     = NULL;
        eargp->envp = envp;
    }

    mrb_gc_arena_restore(mrb, ai);
}

static int
mrb_execarg_argv_to_strv(mrb_state *mrb, mrb_value *argv, mrb_int len, char **result)
{
    char *buf;
    int i, ai;

    if (len < 1)
        mrb_raise(mrb, E_ARGUMENT_ERROR, "must have at least 1 argument");

    ai = mrb_gc_arena_save(mrb);

    for (i = 0; i < len; i++) {
        buf = (char *)mrb_string_value_cstr(mrb, &argv[i]);

        *result = buf;
        result++;
    }

    *result = NULL;
    result -= i;

    mrb_gc_arena_restore(mrb, ai);

    return 0;
}

static int
mrb_io_modestr_to_flags(mrb_state *mrb, const char *mode)
{
  int flags = 0;
  const char *m = mode;

  switch (*m++) {
    case 'r':
      flags |= FMODE_READABLE;
      break;
    case 'w':
      flags |= FMODE_WRITABLE | FMODE_CREATE | FMODE_TRUNC;
      break;
    case 'a':
      flags |= FMODE_WRITABLE | FMODE_APPEND | FMODE_CREATE;
      break;
    default:
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "illegal access mode %S", mrb_str_new_cstr(mrb, mode));
  }

  while (*m) {
    switch (*m++) {
      case 'b':
        flags |= FMODE_BINMODE;
        break;
      case '+':
        flags |= FMODE_READWRITE;
        break;
      case ':':
        /* XXX: PASSTHROUGH*/
      default:
        mrb_raisef(mrb, E_ARGUMENT_ERROR, "illegal access mode %S", mrb_str_new_cstr(mrb, mode));
    }
  }

  return flags;
}

static int
mrb_io_flags_to_modenum(mrb_state *mrb, int flags)
{
  int modenum = 0;

  switch(flags & (FMODE_READABLE|FMODE_WRITABLE|FMODE_READWRITE)) {
    case FMODE_READABLE:
      modenum = O_RDONLY;
      break;
    case FMODE_WRITABLE:
      modenum = O_WRONLY;
      break;
    case FMODE_READWRITE:
      modenum = O_RDWR;
      break;
  }

  if (flags & FMODE_APPEND) {
    modenum |= O_APPEND;
  }
  if (flags & FMODE_TRUNC) {
    modenum |= O_TRUNC;
  }
  if (flags & FMODE_CREATE) {
    modenum |= O_CREAT;
  }
#ifdef O_BINARY
  if (flags & FMODE_BINMODE) {
    modenum |= O_BINARY;
  }
#endif

  return modenum;
}
