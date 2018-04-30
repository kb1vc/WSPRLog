include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )

set(WSPRLogLib_SRCS 
  WSPRLog.cxx
  TimeCorr.cxx)

add_library(WSPRLogLib ${WSPRLogLib_SRCS})


set(WSPRLogTest_SRCS
    WSPRLogTest.cxx
)
add_executable(WSPRLogTest ${WSPRLogTest_SRCS})
target_link_libraries(WSPRLogTest WSPRLogLib ${Boost_LIBRARIES})


set(WSPRLogBandFilter_SRCS
    WSPRLogBandFilter.cxx
)
add_executable(WSPRLogBandFilter ${WSPRLogBandFilter_SRCS})
target_link_libraries(WSPRLogBandFilter WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS WSPRLogBandFilter DESTINATION bin)


set(WSPRLogLineFilter_SRCS
    WSPRLogLineFilter.cxx
    )
add_executable(WSPRLogLineFilter ${WSPRLogLineFilter_SRCS})
target_link_libraries(WSPRLogLineFilter  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS WSPRLogLineFilter DESTINATION bin)



set(WSPRLogSplitter_SRCS
    WSPRLogSplitter.cxx
    )
add_executable(WSPRLogSplitter ${WSPRLogSplitter_SRCS})
target_link_libraries(WSPRLogSplitter  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS WSPRLogSplitter DESTINATION bin)


set(WSPRLogHisto_SRCS
    WSPRLogHisto.cxx
    )

add_executable(WSPRLogHisto ${WSPRLogHisto_SRCS})
target_link_libraries(WSPRLogHisto  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogHisto DESTINATION bin)


set(WSPRLogXY_SRCS
    WSPRLogXY.cxx
    )

add_executable(WSPRLogXY ${WSPRLogXY_SRCS})

target_link_libraries(WSPRLogXY  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogXY DESTINATION bin)

set(tctest_SRCS
    tctest.cxx
    )

add_executable(tctest ${tctest_SRCS})

target_link_libraries(tctest WSPRLogLib
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})


set(WSPRLogDiffTime_SRCS
    WSPRLogDiffTime.cxx
    )

add_executable(WSPRLogDiffTime ${WSPRLogDiffTime_SRCS})

target_link_libraries(WSPRLogDiffTime  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogDiffTime DESTINATION bin)