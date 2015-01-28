/* sensor vars and read functions */

//debounce time (milliseconds) between each analog sensor reading
const int sensorDebounceTime = 15;

int tmp102Address = 0x48; //temp sensor 2-wire address

CS_MQ7 MQ7(4);  //sensor "tog" pin on 4

void readSensors(){
  //sensors
  //Serial.println("reading sensors");
  currNo2 = getNo2();
  delay(sensorDebounceTime);
  currCo = getCO();
  delay(sensorDebounceTime);
  currQuality = getQuality();
  delay(sensorDebounceTime);
  currHumidity = getHumidity();
  delay(sensorDebounceTime);
  currTemp = getTemperature();
  delay(sensorDebounceTime);
  
  MQ7.CoPwrCycler(); //for CO sensor power cycling
}

//--------- e2v MiCS-2170 NO2 sensor ---------
int getNo2(){
  int thisReading = 10;

  //set reference voltage to 3.3 here?
  thisReading = analogRead(No2SensorPin);
  return thisReading;
}

//--------- MQ-7 carbon monixide sensor ---------
int getCO(){
  int thisReading = 11;

    if (MQ7.currentState() == LOW){ //not heating, ready to read.
      thisReading = analogRead(CoSensorPin);
    } 
    else { //heating, leave value where it was
      thisReading = currCo;
    }

  return thisReading;
  
}

//--------- MQ-135 air quality sensor ---------
int getQuality()
{
  int thisReading = 12;
  char buffer[10] = "";

  thisReading = analogRead(qualitySensorPin);
  
  return thisReading;
}

//--------- DHT22 humidity ---------
float getHumidity()
{
  float h = dht.readHumidity();
  //Serial.println("Read humidity");
  
  if (isnan(h)){
    //failed to get reading from DHT    
    delay(2500);
    h = dht.readHumidity();
    if(isnan(h)){
      return -1; 
    }
  } 
  else return h;
  
}

//--------- DHT22 temperature ---------
float getTemperature(){
  float t = dht.readTemperature();

  if (isnan(t)){
    //failed to get reading from DHT
    delay(2500);
    t = dht.readTemperature();
    if(isnan(t)){
      return -1; 
      
    }
  } 
  return t;
}
