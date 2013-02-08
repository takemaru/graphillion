SET(SRCS hudd/bddc hudd/BDD hudd/ZBDD graphillion/setset graphillion/util
  graphillion/zdd)

ADD_LIBRARY(${PROJECT_NAME} ${SRCS})
INSTALL(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)

ADD_EXECUTABLE(${PROJECT_NAME}_test ${SRCS} graphillion/test)
