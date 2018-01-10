#include <stdlib.h>

void _exit(int code);

void __attribute__((weak, noreturn)) _exit(int code __attribute__((unused))) {
  for(;;) {
	  __asm("nop");
  }
}

void __attribute__((weak,noreturn)) abort(void) {
  _exit(1);
}
