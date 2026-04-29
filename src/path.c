#include "stz2/path.h"
#include "stz2/cmd.h"

#include <errno.h>    // IWYU pragma: keep
#include <sys/stat.h> // stat
#include <unistd.h>   // access, F_OK

#include <libgen.h> // basename, dirname

bool path_exists(Str0 path)
{
    if (!path.buf) return false;

    struct stat info = {0};

    int err = stat(path.buf, &info);
    if (err != 0) return false;

    return S_ISDIR(info.st_mode)    //
           || S_ISREG(info.st_mode) //
           || S_ISLNK(info.st_mode);
}

Str path_basename(Buf* b, Str0 path)
{
    if (!path.buf) return StrNull;

    Str0  pname = Str0_(str_copy(b, Str_(path), true));
    char* name  = basename(pname.buf);
    if (!name) return StrNull;

    return Str_Chars(name);
}

Str path_dirname(Buf* b, Str0 path)
{
    if (!path.buf) return StrNull;

    Str0  pname = Str0_(str_copy(b, Str_(path), true));
    char* name  = dirname(pname.buf);
    if (!name) return StrNull;

    return Str_Chars(name);
}

Str path_extension(Buf* b, Str0 path)
{
    if (!path.buf) return StrNull;

    Str0  pname = Str0_(str_copy(b, Str_(path), true));
    char* name  = basename(pname.buf);
    if (!name) return StrNull;

    Str0 p = Str0_Chars(name);
    RANGE(i, pname.len)
    {
        if (!(p.buf[i] == '.')) continue;
        return (Str){&p.buf[i], p.len - i};
    }
    return StrNull;
}

Strs path_split(Buf* b, Str0 path, isize maxlen)
{ return str_splitc(b, Str_(path), '/', maxlen, STRS_SPLIT_IGNORE_EMPTY); }

Str0 path_join(Buf* b, Str0 parent, Str0 path)
{
    CChars fmt = arr_last(parent) == '/' ? "%.*s%.*s" : "%.*s/%.*s";
    Str    p   = str_fmt(b, fmt, _s(parent), _s(path));

    // TODO: Check overflow
    b->buf[b->len] = '\0';
    b->len++;
    return Str0_(p);
}

int path_mkdir(Str0 path, int mode)
{
    int err = 0;
    // int mode = 0755;

    if (path_exists(path)) return 0;

    err = mkdir(path.buf, mode);
    if (err) return err;

    return 0;
}

int path_delete(Str0 path)
{
    int err = 0;

    err = access(path.buf, F_OK);
    if (err) return err;

    err = remove(path.buf);
    if (err) return err;

    return 0;
}

int path_read_text(Buf* b, Str0 path, Str* dst)
{
    int err;

    // Open and get size
    FILE* f = fopen(path.buf, "rb");
    if (f == NULL) return errno;

    err = fseek(f, 0, SEEK_END);
    if (err) goto __close;
    isize size = ftell(f);
    if (size < 0) goto __close;

    Str data = str_new(b, size);

    err = fseek(f, 0, SEEK_SET);
    if (err) goto __close;

    isize nread = fread(data.buf, sizeof(char), data.len, f);
    err         = -(nread != data.len);
    if (err) goto __close;

__close:

    fclose(f); // Ignore error from close
    if (err) return err;

    // On success
    *dst = data;

    return 0;
}

int path_write_text(Str0 path, Str src, const char* mode)
{
    int err;

    FILE* f = fopen(path.buf, mode);
    err     = -(f == NULL);
    if (err) return errno;

    isize nwritten = fwrite(src.buf, sizeof(char), src.len, f);
    err            = -(nwritten != src.len);
    if (err) goto __close;

__close:

    fclose(f); // Ignore error from close
    if (err) return err;

    return 0;
}

int path_list(Buf* b, Str0 fd_cmd, Str0s* paths)
{
    int err = 0;

    // Let's divide stdout, stderr by 80%, 20%
    CmdResult res     = {};
    isize     max_out = (buf_avail(b, sizeof(char)) * 8) / 10;
    isize     max_err = (buf_avail(b, sizeof(char)) * 2) / 10;

    err = cmd_exec(b, fd_cmd, CmdShellBash, max_out, max_err, &res);
    if (err) return err;
    if (res.status) return err;

    Strs lines = str_splitc(b, res.out, '\n', -1, STRS_SPLIT_IGNORE_EMPTY | STRS_SPLIT_SUBSTITUTE_NULL);

    // To ensure cast works, can be removed later
    RANGE(i, lines.len) { Assert(IsNullTerm(lines.buf[i]), ""); }

    *paths = *(Str0s*)&lines;

    return 0;
}
