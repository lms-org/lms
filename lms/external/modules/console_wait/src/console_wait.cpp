#include <console_wait.h>

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

bool Console_wait::initialize() {
	printf("Init: console_wait\n");
	struct termios tattr;
	/* Make sure stdin is a terminal. */
	if (!isatty (STDIN_FILENO))
	{
		fprintf (stderr, "Not a terminal.\n");
		exit (EXIT_FAILURE);
	}

	/* Save the terminal attributes so we can restore them later. */
	tcgetattr (STDIN_FILENO, &saved_attributes);
	
	/* Set the funny terminal modes. */
	tcgetattr (STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
	
	//TODO

	return true;
}

bool Console_wait::deinitialize() {
	printf("Deinit: console_wait\t");

	tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);

	return true;
}

bool Console_wait::cycle () {
	char c;
	read(STDIN_FILENO, &c, 1);
	return true;
}
