#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256
static char receive[BUFFER_SIZE] = { 0 };

long get_int_from_input();

int main()
{
	printf("Starting device test code example of driver process killer...\n");
	int fd = open("/dev/killerProcess", O_RDWR); // Open the device with read/write access
	if (fd < 0) {
		perror("Failed to open the device...");
		return errno;
	}

	printf("Type in a process ID to kill:\n");
	long pid = get_int_from_input();

	printf("Killing process with ID: %ld, press ENTER to continue\n", pid);
	getchar(); // Wait for user input before continuing
	int ret = write(fd, &pid, sizeof(pid));
	if (ret < 0) {
		perror("Failed to write the message to the device.");
		return errno;
	}

	printf("Press ENTER to read back from the device...\n");
	getchar(); // Wait for user input before continuing

	printf("Reading from the device...\n");
	ret = read(fd, receive, BUFFER_SIZE);
	if (ret < 0) {
		perror("Failed to read the message from the device.");
		return errno;
	}

	printf("The received message is: [%s]\n", receive);
	printf("End of the program\n");
	return EXIT_SUCCESS;
}

long get_int_from_input()
{
	long num = -1;
	do {
		int scan_status = scanf("%ld", &num);
		while (getc(stdin) != '\n')
			; // Clear buffer if user registers a string

		if (scan_status == EOF)
			continue; // If scanf fails, try again

		return num;
	} while (1);
}