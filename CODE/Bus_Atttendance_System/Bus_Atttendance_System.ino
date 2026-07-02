#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define SS_PIN D8
#define RST_PIN D0
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define SERVO_PIN D4
#define GATE_OPEN 180
#define GATE_CLOSED 0
Servo gateServo;

ESP8266WebServer server(80);


int reportHour = 0;
int reportMinute = 0;
bool timeSet = false;
long timeOffsetMinutes = 0;
const int allowedDelayMin = 5;
const int fineAmount = 50;

struct Conductor {
  byte uid[4];
  String name;
  String busNo;
  int fine;
  String lastTime;
  String lastStatus;
};

Conductor conductors[] = {
  {{0x27, 0x33, 0xB0, 0x02}, "MANISH", "G48", 0, "", ""},
  {{0x61, 0x87, 0xFE, 0x17}, "SURESH", "G45", 0, "", ""},
  {{0x12, 0x34, 0x56, 0x78}, "RAMESH", "G46", 0, "", ""},
  {{0xAB, 0xCD, 0xEF, 0x01}, "RAJESH", "G47", 0, "", ""}
};
const int conductorCount = sizeof(conductors) / sizeof(conductors[0]);

String logData = "";


void setup() {
  Serial.begin(9600);

  
  Wire.begin(D2, D1);
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED FAILED");
    while (1);
  }

  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  showIdleScreen();

  SPI.begin();
  mfrc522.PCD_Init();

  gateServo.attach(SERVO_PIN);
  gateServo.write(GATE_CLOSED);

  WiFi.softAP("Bus_Attendance", "");

  server.on("/", handleRoot);
  server.on("/settime", handleSetTime);
  server.on("/openGate", handleOpenGate);
  server.on("/download", handleDownload);
  server.on("/manifest.json", handleManifest);
  server.begin();
}


void loop() {
  server.handleClient();

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  int idx = findConductor();
  if (idx == -1) {
    showMessage("INVALID CARD");
    haltRFID();
    return;
  }

  if (!timeSet) {
    showMessage("SET TIME FIRST");
    haltRFID();
    return;
  }

  long currentMinutes = (millis() / 60000UL) + timeOffsetMinutes;
  if (currentMinutes < 0) currentMinutes += 1440;

  int scanHour = (currentMinutes / 60) % 24;
  int scanMinute = currentMinutes % 60;

  bool late = (scanHour * 60 + scanMinute) >
              (reportHour * 60 + reportMinute + allowedDelayMin);

  char timeBuf[6];
  sprintf(timeBuf, "%02d:%02d", scanHour, scanMinute);

  String status = late ? "LATE" : "ON TIME";
  if (late) conductors[idx].fine += fineAmount;

  conductors[idx].lastTime = timeBuf;
  conductors[idx].lastStatus = status;

  gateServo.write(GATE_OPEN);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("ATTENDANCE OK");
  display.println("----------------");
  display.print("Name: "); display.println(conductors[idx].name);
  display.print("Bus: "); display.println(conductors[idx].busNo);
  display.print("Time: "); display.println(timeBuf);
  display.print("Status: "); display.println(status);
  display.display();

  rebuildLog();

  delay(4000);
  gateServo.write(GATE_CLOSED);

  haltRFID();
}

void haltRFID() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

int findConductor() {
  for (int i = 0; i < conductorCount; i++) {
    bool match = true;
    for (byte j = 0; j < 4; j++) {
      if (mfrc522.uid.uidByte[j] != conductors[i].uid[j]) {
        match = false;
        break;
      }
    }
    if (match) return i;
  }
  return -1;
}


void showIdleScreen() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Bus Attendance");
  display.println("----------------");
  if (timeSet) {
    display.printf("Report: %02d:%02d\n", reportHour, reportMinute);
  } else {
    display.println("Set Time First");
  }
  display.println("\nScan RFID");
  display.display();
}

void showMessage(String msg) {
  display.clearDisplay();
  display.setCursor(0,25);
  display.println(msg);
  display.display();
}

void handleRoot() {
  String page = R"rawliteral(
<!DOCTYPE html><html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<link rel="manifest" href="/manifest.json">
<style>
body{margin:0;height:100vh;display:flex;justify-content:center;align-items:center;
background:linear-gradient(135deg,#667eea,#764ba2);font-family:Arial;}
.card{background:white;padding:20px;border-radius:20px;width:95%;max-width:420px;
text-align:center;box-shadow:0 10px 30px rgba(0,0,0,.3);}
input,button{width:100%;padding:14px;margin-top:10px;border:none;border-radius:10px;font-size:16px;}
button{background:#28a745;color:white;font-weight:bold;}
.open{background:#ff5722;}
.down{background:#2196f3;}
table{width:100%;margin-top:15px;border-collapse:collapse;font-size:12px;}
th,td{border:1px solid #ddd;padding:6px;}
th{background:#f2f2f2;}
</style>
</head>
<body>
<div class="card">
<h2>Bus Attendance</h2>

<form action="/settime">
<input type="time" name="report" required>
<input type="hidden" name="now" id="now">
<button>SET REPORT TIME</button>
</form>

<button class="open" onclick="location.href='/openGate'">OPEN GATE</button>
<button class="down" onclick="location.href='/download'">DOWNLOAD REPORT</button>

<table>
<tr><th>Name</th><th>Bus</th><th>Status</th><th>Fine</th></tr>
)rawliteral";

  page += logData;

  page += R"rawliteral(
</table>
</div>
<script>
document.getElementById('now').value=
(new Date().getHours()*60)+(new Date().getMinutes());
</script>
</body></html>
)rawliteral";

  server.send(200, "text/html", page);
}

void handleSetTime() {
  String report = server.arg("report");
  int phoneNow = server.arg("now").toInt();
  reportHour = report.substring(0,2).toInt();
  reportMinute = report.substring(3,5).toInt();
  timeOffsetMinutes = phoneNow - (millis()/60000UL);
  timeSet = true;
  server.sendHeader("Location","/");
  server.send(303);
}

void handleOpenGate() {
  gateServo.write(GATE_OPEN);
  showMessage("MANUAL OPEN");
  delay(4000);
  gateServo.write(GATE_CLOSED);
  server.sendHeader("Location","/");
  server.send(303);
}

void handleDownload() {
  String csv="Name,Bus,Arrival,Status,Fine\n";
  for(int i=0;i<conductorCount;i++){
    csv+=conductors[i].name+","+conductors[i].busNo+","+
         conductors[i].lastTime+","+conductors[i].lastStatus+","+
         String(conductors[i].fine)+"\n";
  }
  server.sendHeader("Content-Disposition","attachment; filename=bus_report.csv");
  server.send(200,"text/csv",csv);
}

void handleManifest() {
  server.send(200,"application/json",
"{\"name\":\"Bus Attendance\",\"short_name\":\"BusGate\",\"start_url\":\"/\",\"display\":\"standalone\"}");
}

void rebuildLog() {
  logData="";
  for(int i=0;i<conductorCount;i++){
    logData+="<tr><td>"+conductors[i].name+
             "</td><td>"+conductors[i].busNo+
             "</td><td>"+conductors[i].lastStatus+
             "</td><td>Rs "+String(conductors[i].fine)+"</td></tr>";
  }
}
