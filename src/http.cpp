#include "main.h"

WebServer server(80); // Web server object. Will be listening in port 80 (default for HTTP)

void handleRoot() // Handler
{
#ifdef _DEBUG_HTTP
  _debug_println("handleRoot()");
#endif
  // server.send(200, "text/html", FPSTR(html_index)); // Response to the HTTP request
  server.send(200, "text/html", FS_read("/index.html")); // Response to the HTTP request
  return;
}

void handleWifiParam()
{
#ifdef _DEBUG_HTTP
  _debug_println("handleWifiParam()");
#endif
  String message;

  if (server.hasArg("wifiSSID") and server.hasArg("wifiPass")) // wifi parameter detected
  {
    if (FS_wifiWrite(server.arg("wifiSSID"), server.arg("wifiPass")))
    {
      message = F("New Wifi parameters have been set, SSID : ");
      message += server.arg("wifiSSID");
      message += F("<br> Device will restart with new parameters...");
      server.send(200, "text/html", message); // Response to the HTTP request
      restart();
      return;
    }
    else
    {
      message = F("wifi parameters are incorrect");
    }
    server.send(200, "text/html", message); // Response to the HTTP request
    return;
  }

  //server.send(200, "text/html", FPSTR(html_wifi)); // Response to the HTTP request
    server.send(200, "text/html", FS_read("/wifi.html")); // Response to the HTTP request
  return;
}

void handleConfig()
{
#ifdef _DEBUG_HTTP
  _debug_println("handleConfig()");
#endif
  if (server.args() != 0) // arguments detected
  {
    for (uint8_t i = 0; i < server.args(); i++)
    {
      custom_arg(server.argName(i), server.arg(i));
    }
  }
  //server.send(200, "text/html", FPSTR(config));
    server.send(200, "text/html", FS_read("/config.html")); // Response to the HTTP request
  return;
}

void handleRealTime()
{
#ifdef _DEBUG_HTTP
  _debug_println("handleRealTime()");
#endif
  //server.send(200, "text/html", FPSTR(realTime));
    server.send(200, "text/html", FS_read("/realtime.html")); // Response to the HTTP request
  return;
}

void handleJSON()
{
#ifdef _DEBUG_HTTP
  _debug_println("handeJSON()");
#endif
  if (server.args() != 0) // arguments detected
  {
    if (server.argName(0).equals("reset"))
    {
      if (server.arg(0).equals("simple"))
      {
        restart();
        return;
      }
      if (server.arg(0).equals("factory"))
      {
        factory_reset();
        return;
      }
    }
    else
    {
      for (uint8_t i = 0; i < server.args(); i++)
      {
        custom_arg(server.argName(i), server.arg(i));
      }
    }
  }
  server.send(200, "application/json", custom_JSON(true));
  return;
}

void handleNotFound()
{
#ifdef _DEBUG_HTTP
  _debug_println("handleNotFound()");
#endif
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + " : " + server.arg(i) + "\n";
  }
  // server.setContentLength(message.length());
  server.send(404, "text/plain", message);
  return;
}

void http_setup()
{
#ifdef _DEBUG_FUNCTION
  _debug_println(__FUNCTION__);
#endif

  server.on("/", handleRoot); // Associate the handler function to the path url/?argument1=valeur1&argument2=valeur2
  server.on("/wifi", handleWifiParam);
  server.on("/config", handleConfig);
#ifdef ACTIVE_WEBSOCKET
  server.on("/realtime", handleRealTime);
#endif
  server.on("/json", handleJSON);
  server.onNotFound(handleNotFound);

  server.begin(); // Start the server
#ifdef _DEBUG_HTTP
  _debug_println(F("Server listening"));
#endif
  return;
}

void http_loop()
{
#ifdef _DEBUG_FUNCTION
  _debug_println(__FUNCTION__);
#endif

  server.handleClient(); // Handling of incoming requests

  return;
}