import sys
import os
import time
import select
import tty
import termios

fd = sys.stdin.fileno()
old_settings = termios.tcgetattr(fd)

try:
    # Режим чтения по символам без ожидания Enter
    tty.setcbreak(fd)

    while True:
        # Спамим \n
        sys.stdout.write("\n")
        sys.stdout.flush()

        # Проверяем, есть ли ввод
        rlist, _, _ = select.select([sys.stdin], [], [], 0)

        if rlist:
            ch = os.read(fd, 1)
            sys.stdout.buffer.write(ch)
            sys.stdout.flush()

        time.sleep(0.2)
except (BrokenPipeError, KeyboardInterrupt):
    pass
finally:
    termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)