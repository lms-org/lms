set ( WIFI_SERVER_SOURCES 
	"src/wifi_server.cpp"
	"src/interface.cpp"
)

add_library ( wifi_server SHARED ${WIFI_SERVER_SOURCES})
