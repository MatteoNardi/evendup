#include <errno.h>
#include <fcntl.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void copy_event(struct input_event *ev, struct libevdev_uinput *clone1,
		struct libevdev_uinput *clone2) {
	int err;
	err =
	    libevdev_uinput_write_event(clone1, ev->type, ev->code, ev->value);
	if (err != 0) perror("write 1");
	err =
	    libevdev_uinput_write_event(clone2, ev->type, ev->code, ev->value);
	if (err != 0) perror("write 2");
}

int main(int argc, char **argv) {
	int err = 0;
	int fd = -1;
	struct libevdev *source = NULL;
	struct libevdev_uinput *clone1 = NULL, *clone2 = NULL;
	struct input_event ev;

	if (argc < 2) {
		printf("Usage:\n");
		printf("evendup <input_device>\n");
		goto finish;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("Error opening source device");
		goto finish;
	}

	err = libevdev_new_from_fd(fd, &source);
	if (err) {
		perror("Error initializing source device");
		goto finish;
	}

	err = libevdev_grab(source, LIBEVDEV_GRAB);
	if (err) perror("Error grabbing device");

	err = libevdev_uinput_create_from_device(
	    source, LIBEVDEV_UINPUT_OPEN_MANAGED, &clone1);
	if (err != 0) {
		perror("Error copying device");
		goto finish;
	}
	printf("Copied %s to %s\n", argv[1],
	       libevdev_uinput_get_devnode(clone1));
	fflush(stdout);

	err = libevdev_uinput_create_from_device(
	    source, LIBEVDEV_UINPUT_OPEN_MANAGED, &clone2);
	if (err != 0) {
		perror("Error copying device 2");
		goto finish;
	}
	printf("Copied %s to %s\n", argv[1],
	       libevdev_uinput_get_devnode(clone2));
	fflush(stdout);

	do {
		err = libevdev_next_event(
		    source,
		    LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING,
		    &ev);
		if (err == LIBEVDEV_READ_STATUS_SYNC) {
			while (err == LIBEVDEV_READ_STATUS_SYNC) {
				copy_event(&ev, clone1, clone2);
				err = libevdev_next_event(
				    source, LIBEVDEV_READ_FLAG_SYNC, &ev);
			}
		} else if (err == LIBEVDEV_READ_STATUS_SUCCESS)
			copy_event(&ev, clone1, clone2);
	} while (err == LIBEVDEV_READ_STATUS_SYNC ||
		 err == LIBEVDEV_READ_STATUS_SUCCESS || err == -EAGAIN);

	if (err != LIBEVDEV_READ_STATUS_SUCCESS && err != -EAGAIN)
		fprintf(stderr, "Failed to handle events: %s\n",
			strerror(-err));

finish:
	if (clone2) libevdev_uinput_destroy(clone2);
	if (clone1) libevdev_uinput_destroy(clone1);
	if (source) libevdev_free(source);
	if (fd >= 0) close(fd);
	return err;
}
