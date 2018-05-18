#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

#define safe_pipe(fd) assert(pipe(fd) == 0)
#define safe_write(fd, c, size) assert(write(fd, c, size) == size)
#define safe_exec(...) assert(execlp(__VA_ARGS__) != -1)
#define safe_close(fd) assert(close(fd) == 0)

void f_usage(void)
{
	puts("usage: ./exe --in in.txt --out out.txt --key key.txt");
}

pid_t safe_fork(void)
{
	pid_t pid = fork();

	assert(pid != -1);
	return pid;
}

int safe_creat(const char *path, mode_t mode)
{
	int fd = creat(path, mode);

	assert(fd != -1);
	return fd;
}

int main(int argc, char *argv[])
{
	char *path_i, *path_o, *path_k;
	const struct option long_opt[] = {
		{"in",   1, 0, 'i' },
		{"out",  1, 0, 'o' },
		{"key",  1, 0, 'k' },
		{"help", 0, 0, 'h' },
		{0,      0, 0, 0   }
	};

	path_i = path_o = path_k = NULL;

	while (1) {
		int c = getopt_long(argc, argv, "i:o:k:h", long_opt, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'i':
			path_i = optarg;
			break;
		case 'o':
			path_o = optarg;
			break;
		case 'k':
			path_k = optarg;
			break;
		default:
			f_usage();
			exit(EXIT_SUCCESS);
		}
	}

	if (!(path_i && path_o && path_k)) {
		f_usage();
		exit(EXIT_SUCCESS);
	}

	int i_pipe_fd[2], k_pipe_fd[2];

	safe_pipe(i_pipe_fd);
	safe_pipe(k_pipe_fd);

	if (safe_fork() == 0) {
		safe_close(1); //stdout shoutout pacikam
		assert(dup(i_pipe_fd[1]) == 1);
		safe_close(i_pipe_fd[0]);
		safe_close(i_pipe_fd[1]);
		safe_close(k_pipe_fd[0]);
		safe_close(k_pipe_fd[1]);
		safe_exec("cat", "cat", path_i, NULL);
		exit(EXIT_SUCCESS);
	}

	if (safe_fork() == 0) {
		safe_close(1); //stdout shoutout pacikam
		assert(dup(k_pipe_fd[1]) == 1);
		safe_close(i_pipe_fd[0]);
		safe_close(i_pipe_fd[1]);
		safe_close(k_pipe_fd[0]);
		safe_close(k_pipe_fd[1]);
		safe_exec("cat", "cat", path_k, NULL);
		exit(EXIT_SUCCESS);
	}

	safe_close(i_pipe_fd[1]);
	safe_close(k_pipe_fd[1]);

	int fd_o = safe_creat(path_o, 00700);

	while (1) {
		char f, s;

		if (1 != read(i_pipe_fd[0], &f, 1))
			break;
		if (1 != read(k_pipe_fd[0], &s, 1))
			break;

		f ^= s;

		safe_write(fd_o, &f, 1);
	}

	safe_close(fd_o);
	safe_close(i_pipe_fd[0]);
	safe_close(k_pipe_fd[0]);
	exit(EXIT_SUCCESS);
}
