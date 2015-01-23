/* nanode ethernet vars and functions */

//uses ethercard library from 
//https://github.com/jcw/ethercard

//----- vars

// ethernet interface mac address, must be unique on the LAN
// add in find nanode MAC code!

unsigned long timer = 10000;
//Stash stash; //holds data to send

//for counting how long it's been since last successful connection:
unsigned long lastResponseTime = 0;

void nanodeUpdate()
{
  digitalWrite(resetterPin, HIGH);

  if (currTime - lastResponseTime > (10*60000)){ // we have not connected in 10 min
    Serial.println("RESET ME");
    nanodeReset();
  }
}

//----- check time, sendData if we've hit timer
boolean transmitTime(){
  if (currTime > timer) { //we've hit our transmit timer limit
    //tranmsitting = true;
    nanodeSendData();      //send out all curr data!
    timer = currTime + (transmitFrequency*1000); //reset timer
    return true;
  } else 
    return false;
  
}

//----- send data !
void nanodeSendData(){

  //char buffer[190] = "";
  //char charTmp[10] = "";

 /*sprintf(buffer, "{\"NO2\": {\"value\": %d, \"units\": \"ppm\"}", currNo2);
  sprintf(buffer + strlen(buffer), ",\"CO\":  {\"value\": %d, \"units\": \"ppm\"}", currCo);
  sprintf(buffer + strlen(buffer), ",\"Qual\": %d", currQuality);
  
  dtostrf(currHumidity, 1, 2, charTmp);
  sprintf(buffer + strlen(buffer), ",\"Hum\":  {\"value\": %d, \"units\": \"ppm\"}", charTmp);
  
  dtostrf(currTemp, 1, 2, charTmp);
  sprintf(buffer + strlen(buffer), ",\"Temp\":  {\"value\": %d, \"units\": \"C\"}}", charTmp);*/
  
  //_____________________________________DO NOT COMMENT OUT !!!!_______________________________________________________
  char No2[50] = "";
  sprintf(No2, "%d", currNo2);
  client.publish("AQE/Data/No2", No2);
  //updateThingSpeak("field1="+currNo2);
  
  //char Co[50] = "";
  //sprintf(Co, "%d", currCo);
  //client.publish("AQE/Data/Co", Co);
  
  //char Quality[50] ="";
  //sprintf(Quality, "%d", currQuality);
  //client.publish("AQE/Data/Quality", Quality);
 
  //char stuff[50]= "";
  //char Temperature[50] ="";
  //dtostrf(currTemp, 1, 2, stuff);
  //sprintf(Temperature, "%s", stuff);
  //client.publish("AQE/Data/Temperature", Temperature);
 
  //char stuffed[50] = "";
  //char Humidity[50] ="";
  //dtostrf(currHumidity, 1, 2, stuffed);
  //sprintf(Humidity, "%s", stuffed);
  //client.publish("AQE/Data/Humidity", Humidity);
 //___________________________________________________________________________________________________________________
 
  //sprintf(buffer, "{\"NO2\":%d", currNo2);
  //sprintf(buffer + strlen(buffer), ",\"CO\":%d", currCo);
  //sprintf(buffer + strlen(buffer), ",\"Qual\":%d", currQuality);
  
  //dtostrf(currHumidity, 1, 2, charTmp);
  //sprintf(buffer + strlen(buffer), ",\"Hum\":%s", charTmp);
  
  //dtostrf(currTemp, 1, 2, charTmp);
  //sprintf(buffer + strlen(buffer), ",\"Temp\":%s}", charTmp);
  //Serial.println(strlen(buffer));
  if (client.publish("AQE/Data/No2", No2))  
    lastResponseTime = currTime;
  
  //flashStatusLed();
}

void nanodeReset(){
  digitalWrite(resetterPin, LOW);
  Serial.println("nanode was reset");
}

