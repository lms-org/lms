include_directories("include")
set ( TEST_MODULE_SOURCES 
	"src/test.cpp"
)

add_library ( testModule SHARED ${TEST_MODULE_SOURCES})
