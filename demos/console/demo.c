// Minimal Console Demo - Fundamental Library
// Compile: gcc -std=c17 -Os -I ../../include demo.c ../../src/console/console.c ../../arch/console/windows-amd64/console.c ../../arch/memory/windows-amd64/memory.c -o demo.exe

#include "fundamental/console/console.h"

int main(void)
{
	fun_console_write_line("=== Console Demo ===");
	fun_console_write_line("Hello from Fundamental Library!");
	fun_console_write_line("This demonstrates basic console output.");
	fun_console_write("No newline: ");
	fun_console_write("part1, ");
	fun_console_write("part2\n");
	fun_console_flush();
	return 0;
}
