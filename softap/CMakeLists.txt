project(iot_module)

set(srcs T_softap.c)

add_library(softap STATIC ${incs} ${srcs})