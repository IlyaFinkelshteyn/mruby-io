/*
** io.h - IO class
*/

#ifndef MRUBY_IO_H
#define MRUBY_IO_H

#if defined(__cplusplus)
extern "C" {
#endif

struct mrb_io {
  int fd;   /* file descriptor, or -1 */
  int fd2;  /* file descriptor to write if it's different from fd, or -1 */
  int pid;  /* child's pid (for pipes)  */
  unsigned int readable:1,
               writable:1,
               sync:1;
};

#define FMODE_READABLE             0x00000001
#define FMODE_WRITABLE             0x00000002
#define FMODE_READWRITE            (FMODE_READABLE|FMODE_WRITABLE)
#define FMODE_BINMODE              0x00000004
#define FMODE_APPEND               0x00000040
#define FMODE_CREATE               0x00000080
#define FMODE_TRUNC                0x00000800

#define E_IO_ERROR                 (mrb_class_get(mrb, "IOError"))
#define E_EOF_ERROR                (mrb_class_get(mrb, "EOFError"))

mrb_value mrb_io_fileno(mrb_state *mrb, mrb_value io);

#if !defined(__APPLE__) && !defined(__linux__)
off_t lseek(int fd, mrb_int offset, mrb_int whence);
off_t close(int fd);
off_t isatty(int fd);
off_t read(int fd, char *buf, mrb_int whence);
off_t write(int fd, char *buf, mrb_int whence);
#else
mrb_value internal_sysopen(mrb_state *mrb);
static int mrb_io_modestr_to_flags(mrb_state *mrb, const char *modestr);
static int mrb_io_flags_to_modenum(mrb_state *mrb, int flags);
int cloexec_open(mrb_state *mrb, const char *pathname, mrb_int flags, mrb_int mode);
void fd_cloexec(mrb_state *mrb, int fd);
#endif



#if defined(__cplusplus)
} /* extern "C" { */
#endif
#endif /* MRUBY_IO_H */
