set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -D_STATIC -fuse-ld=lld -fstandalone-debug -Wall -Wextra -fcolor-diagnostics -fparse-all-comments -stdlib=msvcrt --target=x86_64-pc-windows-msvc"
)
