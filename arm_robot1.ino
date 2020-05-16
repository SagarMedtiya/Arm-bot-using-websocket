#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Servo.h>
#include<SPIFFS.h>
#include<ESPAsyncWebServer.h>

Servo servo01;
Servo servo02;
Servo servo03;
Servo servo04;
Servo servo05;
Servo servo06;

const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337; 
// Constants
const char* ssid = "";
const char* password = "";
AsyncWebServer server(80); 
// Globals
WebSocketsServer webSocket = WebSocketsServer(1337);

String s1, s2, s3, s4, s5, s6;

void RunServo(String servoId, int position) {
	if(servoId == "s1") {
		servo01.write(position);
    Serial.println(position);
		delay(100);
	}
	else if(servoId == "s2") {
		servo02.write(position);
		delay(100);
	}
	else if(servoId == "s3") {
		servo03.write(position);
		delay(100);
	}
	else if(servoId == "s4") {
		servo04.write(position);
		delay(100);
	}
	else if(servoId == "s5") {
		servo05.write(position);
		delay(100);
	}
}

void parse(String text){
  int n = text.length();
  char char_text[n + 1]; 
  // copying the contents of the 
  // string to char array 
  strcpy(char_text, text.c_str());
  char* separator = strtok(char_text, ":");
    String servoId = separator;
  separator = strtok(0, ":");
    int position = atoi(separator);
  Serial.println("ServoId: "+servoId + " Position: "+position);
  RunServo(servoId, position);    
}

 
// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {
 
  // Figure out the type of WebSocket event
  switch(type) {
 
    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
 
    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;
 
    // Echo text message back to client
    case WStype_TEXT:
      Serial.printf("[%u] Text: %s\n", num, payload);
  
    parse((char*)payload);
   //   webSocket.sendTXT(num, payload);
      break;
 
    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}
void onIndexRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/index.html", "text/html");
}
void onCSSRequest(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  Serial.println("[" + remote_ip.toString() +
                  "] HTTP GET request of " + request->url());
  request->send(SPIFFS, "/css.css", "text/css");
}


  String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
  }


void setup() {

  servo01.attach(2);
  servo02.attach(21);
  servo03.attach(4);
  servo04.attach(23);
  servo05.attach(18);
  servo06.attach(19);
  
  // Start Serial port
  Serial.begin(115200);
 if( !SPIFFS.begin()){
        Serial.println("Error mounting SPIFFS");
        while(1);
  }

  // Connect to access point
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
 
  // Print our IP address
  Serial.println("Connected!");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());
 // On HTTP request for root, provide index.html file
  //server.on("/", HTTP_GET, onIndexRequest);
 
  // On HTTP request for style sheet, provide style.css
//server.on("/css.css", HTTP_GET, onCSSRequest);
server.onNotFound([](AsyncWebServerRequest *request){
    String url = request->url().c_str();
    String path = url.substring(url.indexOf('/'), url.length()); 
    String contentType = getContentType(path);
    path += ".gz";
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    //response->addHeader("Content-Encoding", "gzip"); 
    request->send(response);
  });
server.begin();
  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  server.on("/feedbackView", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(SPIFFS, "/Feedback.txt");
  });
  
  server.on("/feedbackClear", HTTP_GET, [](AsyncWebServerRequest *request){
    File file = SPIFFS.open("/Feedback.txt", "w");
    file.close();
    request->send(200, "text/plain", "Feedback file cleared!");
  });
  
  server.on("/feedback", HTTP_GET, [](AsyncWebServerRequest *request){
 
    int paramsNr = request->params();
 
    for(int i=0;i<paramsNr;i++){
         
        AsyncWebParameter* p = request->getParam(i);
        
        String argument = p->name().c_str();
        if(argument.equals("Name")){ Name = p->value().c_str(); }
        else if(argument.equals("Email")){Email = p->value().c_str(); }
        else if(argument.equals("Message")){Feedback = p->value().c_str(); }
    }
    
    File file = SPIFFS.open("/Feedback.txt", "a");
    file.println("{");
    file.println("\"Name\":\""+Name+"\",");
    file.println("\"Email\":\""+Email+"\",");
    file.println("\"Feedback\":\""+Feedback+"\"");
    file.println("},");
    file.println();
    file.close();
  });
}


 
void loop() {
 
  // Look for and handle WebSocket data
  webSocket.loop();
}
