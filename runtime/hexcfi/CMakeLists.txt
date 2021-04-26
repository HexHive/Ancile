include_directories(..)

set(HexCFI_CFLAGS -ggdb -U_FORTIFY_SOURCE)

add_custom_target(hexcfi)


add_compiler_rt_runtime(clang_rt.hexcfi
        STATIC
        ARCHS x86_64 
        SOURCES hexcfi.cc
        CFLAGS ${HexCFI_CFLAGS} -fvisibility=default -DNO_BACKTRACE
        PARENT_TARGET hexcfi)
