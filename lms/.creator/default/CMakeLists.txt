set ( SOURCES 
	"src/$[MODULE_L].cpp"
	"src/interface.cpp"
)

set (HEADERS 
	"include/$[MODULE_L].h"
	${HEADERS_SHARED}
)

include_directories("include")

add_library ( $[LAYER_L]_$[MODULE_L] SHARED ${SOURCES} ${HEADERS})
