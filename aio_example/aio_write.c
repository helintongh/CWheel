#include <fcntl.h>
#include <linux/aio_abi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>

#define PFATAL(x...)                                                           \
	do {                                                                   \
		fprintf(stderr, "[-] SYSTEM ERROR : " x);                      \
		fprintf(stderr, "\n\tLocation : %s(), %s:%u\n", __FUNCTION__,  \
			__FILE__, __LINE__);                                   \
		perror("      OS message ");                                   \
		fprintf(stderr, "\n");                                         \
		exit(EXIT_FAILURE);                                            \
	} while (0)

inline static int io_setup(unsigned nr, aio_context_t *ctxp)
{
	return syscall(__NR_io_setup, nr, ctxp);
}

inline static int io_destroy(aio_context_t ctx)
{
	return syscall(__NR_io_destroy, ctx);
}

inline static int io_submit(aio_context_t ctx, long nr, struct iocb **iocbpp)
{
	return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

inline static int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
			       struct io_event *events,
			       struct timespec *timeout)
{
	// This might be improved.
	return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

#define BUF_SZ (4096)

int main()
{
    int fd = open("/home/he/test_aio.log", O_CREAT | O_RDWR);
    if (fd < 0) {
        PFATAL("open(/home/he/test_aio.log)");
    }

    aio_context_t ctx = 0;
    int r = io_setup(128, &ctx);
    if (r < 0) {
        PFATAL("io_setup()");
    }

    char buf[BUF_SZ] = {0};
    char *test_str = "test aio write function! write this string to file!";
    memcpy(buf, test_str, strlen(test_str));
    int bytes_size = strlen(buf);
    struct iocb cb = {.aio_fildes = fd,
                      .aio_lio_opcode = IOCB_CMD_PWRITE,
                      .aio_buf = (uint64_t)&buf[0],
                      .aio_nbytes = (uint64_t)bytes_size};
    struct iocb *list_of_iocb[1] = {&cb};
    printf("before io_submit file content is %s\n", buf);
    r = io_submit(ctx, 1, list_of_iocb);
    if (r != 1) {
        PFATAL("io_submit()");
    }
    printf("after io_submit the file content is %s\n", buf);

    struct io_event events[1] = {};
    r = io_getevents(ctx, 1, 1, events, NULL);
    if (r != 1) {
        PFATAL("io_getevents()");
    }

    printf("write %lld bytes from /home/he/test_aio.log\n", events[0].res);
    
    io_destroy(ctx);
    close(fd);
    return 0;
}