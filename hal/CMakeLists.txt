project(iot_module)

set(srcs T_sys.c)

add_library(hal STATIC ${incs} ${srcs})