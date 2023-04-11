# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS
    "main.c"
    "modem.c"
    "common.c"
    "task_heartBeat.c"
    "task_adc.c"
    "task_modem.c"
    "task_temp.c"
)
    
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -Wno-unused-but-set-variable)
