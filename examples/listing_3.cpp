#include <std23/function_ref.h>

using std23::function_ref;
using std23::nontype;

void parse_ini(function_ref<size_t(char *, size_t)> read_cb);

#include <stdio.h>

int main()
{
    auto fp = ::fopen("my.ini", "r");
    parse_ini({nontype<[](FILE *fh, auto ptr, auto n)
                       { return ::fread(ptr, 1, n, fh); }>,
               fp});
    ::fclose(fp);
}
