set ( SOURCES 
	"src/console_wait.cpp"
	"src/interface.cpp"
)

set (HEADERS 
	"include/console_wait.h"
	${HEADERS_SHARED}
)

include_directories("include")
#TODO
#add_library ( console_wait SHARED ${SOURCES} ${HEADERS})
