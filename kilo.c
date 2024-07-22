/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s)
{
    perror(s);
    exit(1);
}

void disableRawMode(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
}

void enableRawMode(void)
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disableRawMode);
    
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /*
     * BRKINT = break condition causes a SIGINT to be sent
     * ICRNL = fix ctrl-m by turning off translating carriage returns to
     * newlines
     * INPCK = enables parity checking, not applicable to modern terminals
     * ISTRIP = causes the 8th bit of each input byte to be stripped. off by
     * default
     * IXON = turn off ctrl-s and ctrl-q software flow control
     */
    raw.c_oflag &= ~(OPOST);
    /*
     * OPOST = turn off translating \n to \r\n
     */
    raw.c_cflag |= (CS8);
    /*
     * CS8 = not a flag. a bitmask with multiple bits, which we set with the
     * bitwise-OR operator '|' unlike all the flags we are turning off. sets
     * the character size (CS) to 8 bits per byte. On some systems it's already
     * set that way.
     */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /*
     * ECHO = turn off character echoing
     * ICANON = turn on canonical mode, meaning reading bytes (keys) at a time
     * rather than entire lines (which require an enter press)
     * IEXTEN = turn off ctrl-v
     * ISIG = turn off signal interrupts like Ctrl-C and Ctrl-Z
     */

    // Timeouts so that read() doesn't wait forever, allowing for animations
    // and stuff to show on screen
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}

/*** init ***/

int main(void)
{
    enableRawMode();

    // while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    while (1) {
        char c = '\0';
        // In Cygwin, when read() times out it returns -1 with an errno of
        // EAGAIN, so we won't treat EAGAIN as an error
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}
