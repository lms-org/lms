set ( WIFI_CLIENT_SOURCES 
	"src/wifi_client.cpp"
	"src/PhoenixClientListener.cpp"
	"src/interface.cpp"
)

add_library ( wifi_client SHARED ${WIFI_CLIENT_SOURCES})
