
idf_component_register(SRCS "ble.c" "main.c"
                    INCLUDE_DIRS ".")
set_source_files_properties("main.c" "parking.c" "tag.c" "server.c"
    PROPERTIES COMPILE_FLAGS
  
	-Wno-unused-function
)

set_source_files_properties("main.c" "parking.c" "tag.c" "server.c"
    PROPERTIES COMPILE_FLAGS
  
	-Wno-unused-variable
	
)

set_source_files_properties("main.c" "parking.c" "tag.c" "server.c"
    PROPERTIES COMPILE_FLAGS
 
	-Wno-discarded-qualifiers
)
