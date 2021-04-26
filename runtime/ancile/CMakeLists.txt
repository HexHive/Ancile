include_directories(..)

add_custom_target(ancile)


add_compiler_rt_runtime(clang_rt.ancile
        STATIC
        ARCHS x86_64 
        SOURCES ancile.cc
        CFLAGS -fvisibility=default -DNO_BACKTRACE
        PARENT_TARGET ancile)

