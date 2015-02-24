set ( SOURCES 
	"src/scheduler.cpp"
	"src/interface.cpp"
)

set (HEADERS 
	"include/scheduler.h"
	${HEADERS_SHARED}
)

include_directories("include")

add_library ( scheduler SHARED ${SOURCES} ${HEADERS})
