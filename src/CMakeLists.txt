file(GLOB main_SRC
"*.h"
"*.cc"
)

add_executable(main ${main_SRC})
target_link_libraries(main pthread boost_program_options)