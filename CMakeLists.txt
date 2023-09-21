cmake_minimum_required(VERSION 3.17)
set(NAME Test.Resource)
project(${NAME})
set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE HEADER_FILES *.h**)
file(GLOB_RECURSE SRC_FILES *.c**)

add_executable(
        ${NAME}
        ${HEADER_FILES}
        ${SRC_FILES}
)

#target_link_libraries(${PROJECT_NAME} PRIVATE
#        Core)
#
##add define for compile all sub-services as one lib
#if (COMPILE_SUB_SERVICES_AS_ONE_LIB)
#    message(STATUS "All services will be compiled as one lib")
#    add_definitions(-DSINGLE_SERVICE)
#
#    # link ServiceLib
#    target_link_libraries(
#            ${PROJECT_NAME} PRIVATE
#            ServiceLib)
#else ()
#    # link each sub services
#    target_link_libraries(${PROJECT_NAME} PRIVATE Service.Resource)
#
#    # link each sub implments
#    target_link_libraries(${PROJECT_NAME} PRIVATE Impl.Resource)
#    target_link_libraries(${PROJECT_NAME} PRIVATE Impl.Logger)
#
#endif ()


