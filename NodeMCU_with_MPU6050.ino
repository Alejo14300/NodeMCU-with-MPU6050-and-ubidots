#include <ESP8266WiFi.h>
#include <Wire.h>

#define MPU 0x68
#define tam1 80
#define tam2 30

WiFiClient client;

const int sda= 4; // Pin I2C SDA
const int scl= 5; // Pin I2C SCL

String token =  "A1E-yLqATaPD6re9nnMqgT2YssRQLFSTVT";  // Put here your Ubidots TOKEN
const char* ssid =  "ETB-RAMIREZ"; // Put here your Wi-Fi SSID
const char* password = "16PPMA4UPTO1200"; // Put here your Wi-Fi password
String variable_id1 =  "5a08843f7625420fb68e63cd";
int variosenv[21];
float nodatos;
int conteo;
void ubiSave_value(String, String);
//Variables acelerometro
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
int respiracion,respiracionprom,contadorpromedio,total,contadorfiltro,respiracionfinal,tiempo,minimo,maximo,amplitudrespiracion,tiempo1,minimo1,maximo1,amplitudrespiracion1,alerta,contadoranomalia,activado;
long promedio,respiracionfiltrada;
int lecturasprom[tam1],lecturasfiltro[tam2];

void setup(){
    Serial.begin(9600);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  int n = WiFi.scanNetworks();

  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
    Serial.println("Going into sleep");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi connected");


//    client.wifiConnection(WIFISSID, PASSWORD);
//    //client.setDebug(true); // Uncomment this line to set DEBUG on
    nodatos=0;
    conteo=0;
    Wire.begin(sda, scl);
    Wire.beginTransmission(MPU);
      Wire.write(0x6B); //ESTE REGISTRO CONFIGURA EL MANEJO DE POTENCIA DEL SERVICIO
      Wire.write(0); //LE INDICA AL SISTEMA QUE LOS SENSORES NO ESTÃ‰N EN SLEEP MODE
    Wire.endTransmission(true);
   
    Wire.beginTransmission(MPU);
      Wire.write(0x1A); //en este registro se configura el filtro de los sensores
      Wire.write(6);  //se configura un filtro de 5 Hz
    Wire.endTransmission(true);

    Wire.beginTransmission(MPU);
      Wire.write(0x1C); //en este registro se configura el rango del acelerÃ³metro
      Wire.write(0);  //se configura un rango de 2G
    Wire.endTransmission(true);

  respiracion=0;
  contadorpromedio=0;
  promedio=0;
  respiracionfiltrada=0;
  total=0;
  contadorfiltro=0;
  tiempo=0;
  minimo=0;
  maximo=0;
  amplitudrespiracion=0;
  respiracionfinal=0;
  alerta=0;
  contadoranomalia=0;
  activado=0;

  for (int thisReading = 0; thisReading < tam1; thisReading++) {
    lecturasprom[thisReading] = 0;
  }

  for (int thisReading = 0; thisReading < tam2; thisReading++) {
    lecturasfiltro[thisReading] = 0;
  }
}
void loop(){
  Wire.beginTransmission(MPU);
   Wire.write(0x3B); //Pedir el registro 0x3B - corresponde al AcX
   Wire.endTransmission(false);
   Wire.requestFrom(MPU,6,true); //A partir del 0x3B, se piden 6 registros, 2 por cada eje del acelerÃ³metro
   AcX=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
   AcY=Wire.read()<<8|Wire.read();
   AcZ=Wire.read()<<8|Wire.read();
   
  respiracion=pow(pow(AcX,2)+pow(AcY,2)+pow(AcZ,2),0.5); 
  
  promedio = promedio - lecturasprom[contadorpromedio]; //se resta el Ãºltimo valor de la cola
  lecturasprom[contadorpromedio]=respiracion;
  promedio = promedio + (respiracion); // se aÃ±ade el Ãºltimo valor leÃ­do
  //contadorpromedio++; //se avance una posiciÃ³n en el vector
  total=promedio/tam1;

  if (contadorpromedio >= tam1-1) {
    contadorpromedio = 0; //Se devuelve al comienzo cuando se han promediado 100 muestras. Esto para obtener el valor dc de la seÃ±al
  }

  respiracionfiltrada=respiracionfiltrada-lecturasfiltro[contadorfiltro]/tam2;
  lecturasfiltro[contadorfiltro]=respiracion; //se guarda el valor actual de la respiraciÃ³n el cual reemplaza al Ãºltimo de la cola
  respiracionfiltrada=respiracionfiltrada+respiracion/tam2;
  
  if (contadorfiltro >= tam2-1) {
    contadorfiltro = 0; //Se devuelve al comienzo cuando se han promediado 100 muestras. Esto para obtener el valor dc de la seÃ±al
  }

  respiracionfinal=respiracionfiltrada-total;

  variosenv[conteo]= (int) respiracionfinal;
  if(conteo==20){
    while(conteo>0){
      Serial.println(variosenv[conteo]);
      ubiSave_value(String(variable_id1), String(variosenv[conteo]));
      //client.add("temperature", variosenv[conteo]);
      conteo--;
    }
    
    //client.sendAll(true);  
  }
  
//     //float value2 = analogRead(2)
//    //client.add("temperature", variosenv[nodatos]);
//    //client.add("switch", value2);
//    //client.sendAll(true);
    nodatos++;
    conteo++;
    Serial.print("Conteo= "); 
    Serial.println(conteo);
//    Serial.print("nodatos= "); 
//    Serial.println(nodatos);
    //Serial.print("Variosenv= "); 
    //Serial.println(variosenv[conteo]);
    delay(50);

}
void ubiSave_value(String variable_id, String value)
{
  String var = "{\"value\": " + value + "}"; // We'll pass the data in JSON format
  String length = String(var.length());

  if (client.connect("things.ubidots.com", 80))
  {
    Serial.println("Connected to Ubidots...");
    delay(100);

    client.println("POST /api/v1.6/variables/" + variable_id + "/values HTTP/1.1");
    Serial.println("POST /api/v1.6/variables/" + variable_id + "/values HTTP/1.1");
    client.println("Content-Type: application/json");
    Serial.println("Content-Type: application/json");
    client.println("Content-Length: " + length);
    Serial.println("Content-Length: " + length);
    client.println("X-Auth-Token: " + token);
    Serial.println("X-Auth-Token: " + token);
    client.println("Host: things.ubidots.com\n");
    Serial.println("Host: things.ubidots.com\n");
    client.print(var);
    Serial.print(var + "\n");
  }
  else
  {
    Serial.println("Ubidots connection failed...");
  }

  while (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  if (client.connected())
  {
    Serial.println("Disconnecting from Ubidots...");
    client.stop();
  }
}
