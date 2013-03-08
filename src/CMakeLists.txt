SET(SRCS SAPPOROBDD/bddc SAPPOROBDD/BDD SAPPOROBDD/ZBDD
  graphillion/util graphillion/zdd graphillion/setset)

ADD_LIBRARY(${PROJECT_NAME} ${SRCS} graphillion/graphset)
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)

ADD_EXECUTABLE(test_setset ${SRCS} test/setset)
ADD_EXECUTABLE(test_graphset ${SRCS} graphillion/graphset test/graphset)

ADD_TEST(setset ./test_setset)
ADD_TEST(graphset ./test_graphset)
