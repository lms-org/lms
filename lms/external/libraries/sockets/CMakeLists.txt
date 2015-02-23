set ( SOCKETS_SOURCES
	"src/SocketClient.cpp"
	"src/SocketServer.cpp"
)

add_library ( sockets SHARED ${SOCKETS_SOURCES})
