project(iot_module)

set(srcs example_apconfig.c)

add_executable(example_apconfig ${incs} ${srcs})
target_link_libraries(example_apconfig softap hal cjson)