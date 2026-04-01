#include "stz2/cmd.h"

#include <errno.h>    // IWYU pragma: keep
#include <sys/wait.h> // waitpid, pid
#include <unistd.h>   // pipe, dup2, fork, close

int cmd_exec( //
    Buf*       a,
    Str0       command,
    Str0       shell,
    isize      out_maxlen,
    isize      err_maxlen,
    CmdResult* res)
{
    int err;

    int   stdout_pipe[2], stderr_pipe[2];
    pid_t pid;

    // Create pipes for stdout and stderr
    err = (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1);
    OnError_Return(err, err, "Failed: pipe: %s", strerror(errno));

    // Fork the process
    pid = fork();
    err = pid < 0;
    OnError_Return(err, err, "Failed: fork: %s", strerror(errno));

    if (pid == 0)
    { // Child process
        // Redirect stdout to the write end of the stdout pipe
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);

        // Redirect stderr to the write end of the stderr pipe
        dup2(stderr_pipe[1], STDERR_FILENO);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);

        // Execute the command
        err = execl(shell.buf, shell.buf, "-c", command.buf, (char*)NULL);
        OnError_Fatal(err, "Failed: execl: %s", strerror(errno));
    }
    else
    { // Parent process
        // Close the write ends of the pipes
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // Keep track of capacity for out of memory error
        // Fortunately, capacity is signed (ptrdiff_t)
        isize cap, num_bytes;

        Buf bout  = buf_new2(a, out_maxlen, ALLOC_NOZERO);
        cap       = bout.cap;
        num_bytes = 0;
        do
        {
            err = cap <= 0;
            OnError_Return(err, -ENOMEM,                                       //
                           "Buffer out of capacity: cap, num_bytes: %ld, %ld", //
                           cap, num_bytes);
            num_bytes  = read(stdout_pipe[0], bout.buf + bout.len, cap);
            bout.len  += num_bytes;
            cap       -= num_bytes;
        } while (num_bytes != 0);
        res->out = buf_shrink(a, &bout);
        close(stdout_pipe[0]);

        Buf berr  = buf_new2(a, err_maxlen, ALLOC_NOZERO);
        cap       = berr.cap;
        num_bytes = 0;
        do
        {
            err = cap <= 0;
            OnError_Return(err, -ENOMEM,                                       //
                           "Buffer out of capacity: cap, num_bytes: %ld, %ld", //
                           cap, num_bytes);
            num_bytes  = read(stderr_pipe[0], berr.buf + berr.len, cap);
            berr.len  += num_bytes;
            cap       -= num_bytes;
        } while (num_bytes != 0);
        res->err = buf_shrink(a, &berr);
        close(stderr_pipe[0]);

        // Wait for the child process to finish and get the exit status
        waitpid(pid, (int*)&res->status, 0);
        if (WIFEXITED(res->status)) { res->status = WEXITSTATUS(res->status); }
    }

    return 0;
}
