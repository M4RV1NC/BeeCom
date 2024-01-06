#include <WebSocketsServer.h>

// #define DEBUG_WEBSOCKET_PRINT

void webSocket_setup();
void webSocket_loop();
void webSocket_print(String data, uint8_t client = 0);