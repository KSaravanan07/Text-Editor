/*** includes ***/

#include <ctype.h> // iscntrl
#include <stdio.h> // printf
#include <stdlib.h> // texit
#include <termios.h> // terminal raw mode
#include <unistd.h> //for tcgetattr
#include <errno.h> //errno and EAGAIN

/*** define ***/

#define CTRL_KEY(k) ((k) & 0x1f) // DEFINES A MACRO that takes character ands it with 00011111, mimicing what ctrl key does (it strips bits 5 and 6 from what ASCII value of whatever character you press with ctrl)

/*** data ***/

struct termios orig_termios; //structure for terminal attributes

/*** terminal ***/

void die(const char *s) {
	perror(s); // prints s as well was whatever error was caused whose information is stored in global error variable errno
	exit(1); // exiting with non-zero value indicates error
}

void disableRawMode() {
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) die("tcsetattr"); //setting terminal attributes stored in orig_termios, TCAFLUSH waits till all output in buffer is sent to output, ignores any input in buffer then exits
	// if error occurs, tcsetattr returns -1, hence we call die then with string "tcsetattr"
}

void enableRawMode() {
	if(tcgetattr(STDIN_FILENO, &orig_termios) == 1) die("tcgetattr"); // getting current terminal attributes and storing in orig_termios
	atexit(disableRawMode); //at exit(be it exit() or returning main function disableRawMode() is called

	struct termios raw = orig_termios; // copying orig to raw

	raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | ICRNL | IXON); // turning off - sending break interrupt to terminal, parity checking flag, strips 8th bit to 0 bitflag, converts carriage return to new line, ctr-s/ctr-q xoff to stop terminal showing whats happening, xon to resume
	raw.c_oflag &= ~(OPOST); // turn off post processing of output
	raw.c_cflag |= CS8; // sets character size to be 8bit = 1 byte
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); // turn off - Echo (shows what you type), canonical mode, sigstp/sigint ctrl-d/ctrl-c, ctrl-v (pauses normal terminal commands and takes whatever input is given next as input)
	
	raw.c_cc[VMIN] = 0; // for read timeout, tells minimum byte size it must read before timing out
	raw.c_cc[VTIME] = 1; // minimum time it must wait before timeout (in 1/10th of a second = 100 ms here)


	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr"); // set raw attribute
}

char editorReadKey() {
	int nread;
	char c;
	while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if(nread == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

/*** input ***/

void editorProcessKeypress() {
	char c = editorReadKey();

	switch(c) {
		case CTRL_KEY('q'):
			exit(0);
			break; 
	}
}

/*** init ***/

int main() {
	enableRawMode();

	while(1) {
		editorProcessKeypress();
	}
	return 0;
}