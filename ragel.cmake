find_program(RAGEL "ragel")

function(ragel_gen in_rl)

	add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${in_rl}.cpp
		COMMAND ${RAGEL} -C -G2 -o ${CMAKE_CURRENT_BINARY_DIR}/${in_rl}.cpp ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.rl
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.rl
	)

	add_custom_target(ragel_${in_rl} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${in_rl}.cpp)

endfunction(ragel_gen)

if (RAGEL)
	message(STATUS "ragel found: ${RAGEL}")
else (RAGEL)
	message(FATAL_ERROR "ragel not found")
endif (RAGEL)
