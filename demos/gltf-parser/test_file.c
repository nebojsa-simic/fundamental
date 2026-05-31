#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/memory/memory.h"
#include "fundamental/file/file.h"
#include "fundamental/async/async.h"
int main(void) {
    fun_console_write_line("alloc...");
    MemoryResult mem = fun_memory_allocate(4096);
    if (fun_error_is_error(mem.error)) { fun_console_write_line("alloc err"); return 1; }
    fun_console_write_line("read...");
    AsyncResult read = fun_read_file_in_memory((Read){
        .file_path = "Sponza.gltf",
        .output = mem.value,
        .bytes_to_read = 1024,
    });
    fun_console_write_line("await...");
    fun_async_await(&read, -1);
    fun_console_write_line("done");
    fun_memory_free(&mem.value);
    return 0;
}
