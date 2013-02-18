SET(SRCS SAPPOROBDD/bddc SAPPOROBDD/BDD SAPPOROBDD/ZBDD
  graphillion/setset graphillion/util graphillion/zdd)

ADD_LIBRARY(${PROJECT_NAME} ${SRCS})
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)

ADD_EXECUTABLE(test_setset ${SRCS} test/setset)

ADD_TEST(setset ./test_setset)
