#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80); 

void initWebServer() {
  //socket connections
  server.addHandler(&overview);
  server.addHandler(&automate);
  server.addHandler(&settings);

  // html route for request
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", overview_html, overview_html_len);
  });

  server.on("/general.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/css", general_css, general_css_len);
  });

  server.on("/general.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/javascript", general_js, general_js_len);
  });

  server.on("/overview.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", overview_html, overview_html_len);
  });

  server.on("/history.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", history_html, history_html_len);
  });

  server.on("/automate.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", automate_html, automate_html_len);
  });

  server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", settings_html, settings_html_len);
  });

  // server.on("/overall.html", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send_P(200, "text/html", overall_html, overall_html_len);
  // });

  server.on(datahandler.sensorReading, HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, datahandler.sensorReading, "text/plain");
  });

  server.begin();
}