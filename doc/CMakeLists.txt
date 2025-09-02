FIND_PACKAGE(Doxygen)

if(Doxygen_FOUND)
  set(obs_service_cpp_DOC_INPUT_DIR ${obs_service_cpp_SOURCE_DIR}/doc)
  set(obs_service_cpp_DOC_OUTPUT_DIR ${obs_service_cpp_BINARY_DIR}/doc)

  set(obs_service_cpp_DOC_MAINPAGE
      ${obs_service_cpp_DOC_INPUT_DIR}/obs_service_cpp.txt)
  set(obs_service_cpp_DOXYGEN_OUTPUT
      ${obs_service_cpp_DOC_OUTPUT_DIR}/html/index.html)

  set(obs_service_cpp_DOXYGEN_CONFIG_IN
      ${obs_service_cpp_DOC_INPUT_DIR}/Doxyfile.in)
  set(obs_service_cpp_DOXYGEN_CONFIG
      ${obs_service_cpp_DOC_OUTPUT_DIR}/Doxyfile)

  configure_file(${obs_service_cpp_DOXYGEN_CONFIG_IN}
      ${obs_service_cpp_DOXYGEN_CONFIG} @ONLY)

  configure_file(${obs_service_cpp_SOURCE_DIR}/README.md
      ${obs_service_cpp_DOC_OUTPUT_DIR}/README.dox COPYONLY)

  file (GLOB_RECURSE obs_service_cpp_DOC_HEADERS
        ${obs_service_cpp_SOURCE_DIR}/obs/*.hpp)

  doxygen_add_docs(apidocs
                   ${obs_service_cpp_DOC_MAINPAGE}
                   ${obs_service_cpp_DOC_HEADERS}
                   ${obs_service_cpp_DOC_OUTPUT_DIR}/README.dox
                   ALL USE_STAMP_FILE
                   WORKING_DIRECTORY ${obs_service_cpp_DOC_OUTPUT_DIR}
                   CONFIG_FILE ${obs_service_cpp_DOXYGEN_CONFIG})
else(Doxygen_FOUND)
  message (WARNING "Doxygen not found! API docs won't be built.")
endif(Doxygen_FOUND)
