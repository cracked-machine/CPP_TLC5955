target_sources(${BUILD_NAME} PRIVATE
    catch_cpp_tlc5955.cpp
    tlc5955_tester.cpp
)

target_include_directories(${BUILD_NAME} PRIVATE 
    .
    ${CMAKE_BINARY_DIR}/stm32_interrupt_managers/include
    ${CMAKE_BINARY_DIR}/embedded_utils/include
)