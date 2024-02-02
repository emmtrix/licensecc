
ADD_EXECUTABLE(lccinspector inspector.cpp )

target_link_libraries(lccinspector 
     licensecc_static
     Boost::unit_test_framework 
     Boost::filesystem
     Boost::system)

install(TARGETS lccinspector RUNTIME DESTINATION bin/${LCC_PROJECT_NAME})

target_compile_options(lccinspector PRIVATE
          $<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
