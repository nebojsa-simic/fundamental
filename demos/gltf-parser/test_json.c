#include "fundamental/console/console.h"
#include "fundamental/string/string.h"
#include "fundamental/json/json.h"
int main(void) {
    const char *data = "{\"name\":\"Sponza\",\"version\":\"2.0\"}";
    char out[64];
    fun_json_query_string(data, fun_string_length(data), "name", out, sizeof(out));
    fun_console_write("name: ");
    fun_console_write_line(out);
    return 0;
}
