
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>

static int print_event(struct input_event *ev) {
	if (ev->type == EV_SYN)
		printf(
		    "Event: time %ld.%06ld, ++++++++++++++++++++ %s "
		    "+++++++++++++++\n",
		    ev->input_event_sec, ev->input_event_usec,
		    libevdev_event_type_get_name(ev->type));
	else
		printf(
		    "Event: time %ld.%06ld, type %d (%s), code %d (%s), value "
		    "%d\n",
		    ev->input_event_sec, ev->input_event_usec, ev->type,
		    libevdev_event_type_get_name(ev->type), ev->code,
		    libevdev_event_code_get_name(ev->type, ev->code),
		    ev->value);
	return 0;
}

int main(int argc, char **argv) {
	int err = 0;
	int fd = -1, uifd1 = -1, uifd2 = -1;
	struct libevdev *source = NULL;
	struct libevdev_uinput *dest1 = NULL, *dest2 = NULL;
	struct input_event ev;

	if (argc < 2) {
		printf("Usage:\n");
		printf("keygrabber <input_device>\n");
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

	uifd1 = open("/dev/uinput", O_RDWR);
	if (uifd1 < 0) {
		perror("Error opening uinput (Needed to create new device)");
		goto finish;
	}
	err = libevdev_uinput_create_from_device(source, uifd1, &dest1);
	if (err != 0) {
		perror("Error copying device");
		goto finish;
	}
	printf("Copied %s to %s\n", argv[1],
	       libevdev_uinput_get_devnode(dest1));
	fflush(stdout);

	uifd2 = open("/dev/uinput", O_RDWR);
	if (uifd1 < 0) {
		perror("Error opening uinput (Needed to create new device)");
		goto finish;
	}
	err = libevdev_uinput_create_from_device(source, uifd2, &dest2);
	if (err != 0) {
		perror("Error copying device 2");
		goto finish;
	}
	printf("Copied %s to %s\n", argv[1],
	       libevdev_uinput_get_devnode(dest2));

	// TODO:
	// https://www.freedesktop.org/software/libevdev/doc/1.7.0/syn_dropped.html
	while (libevdev_next_event(
		   source,
		   LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING,
		   &ev) == LIBEVDEV_READ_STATUS_SUCCESS) {
		// print_event(&ev);
		err = libevdev_uinput_write_event(dest1, ev.type, ev.code,
						  ev.value);
		if (err != 0) perror("write 1");
		err = libevdev_uinput_write_event(dest2, ev.type, ev.code,
						  ev.value);
		if (err != 0) perror("write 2");
	}

finish:
	if (dest2) libevdev_uinput_destroy(dest2);
	if (uifd2 >= 0) close(uifd2);
	if (dest1) libevdev_uinput_destroy(dest1);
	if (uifd1 >= 0) close(uifd1);
	if (source) libevdev_free(source);
	if (fd >= 0) close(fd);
	return err;
}
