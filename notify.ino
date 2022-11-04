

String incommingText;


void connectGoogleSpeakers() {

  Serial.println("connecting to Google Home 'livingroom'...");
  if (ghn1.device(displayName1, "en") != true)   { //checkin language is set to english. 
    Serial.println(ghn1.getLastError());
    return;
  }

  Serial.print("found Google Home 'livingroom'(");
  Serial.print(ghn1.getIPAddress());
  Serial.print(":");
  Serial.print(ghn1.getPort());
  Serial.println(")");


  Serial.println("connecting to Google Home 'garage'...");
  if (ghn2.device(displayName2, "en") != true)   {
    Serial.println(ghn2.getLastError());
    return;
  }

  Serial.print("found Google Home 'garage'(");
  Serial.print(ghn2.getIPAddress());
  Serial.print(":");
  Serial.print(ghn2.getPort());
  Serial.println(")");

  server.begin();


}





void Scan_for_incomming_messages() {

  WiFiClient webclient = server.available();   // Listen for incoming clients

  if (webclient) {                             // If a new client connects,
    Serial.println("New Webclient.");          // print a message out in the serial port
    while (webclient.connected()) {            // loop while the client's connected
      if (webclient.available()) {             // if there's bytes to read from the client,

        Serial.println("Read client data : ");
        incommingText = webclient.readString();

        Serial.print("Incomming");
        Serial.println(incommingText);
        webclient.println();
     // webclient.println();


//the webpage starts with: GET /...
// starting with "0" send to all google home speaker.
// if you type in a browser the ip of the node in my case:  '192.168.122.215/0__hallo'  both  google nest will speak
// if you type in a browser the ip of the node in my case:  '192.168.122.215/1__hallo' the first google nest will speak

        if (incommingText.startsWith("GET /0")) {
          fix_incommingText(0);
        }
        else if (incommingText.startsWith("GET /1")) {
          fix_incommingText(1);
        }
        else  if (incommingText.startsWith("GET /2")) {
          fix_incommingText(2);
        }
        else Serial.print("false input");
      }

      // Close the connection
      webclient.stop();
      Serial.println("Webclient disconnected.");
      Serial.println("");
    }
  }
}



void fix_incommingText(byte channel) {

  Serial.print("pre fix");
  Serial.println(incommingText);

  int stringstart = incommingText.indexOf("GET"); // find the start of the strin
  int stringstop = incommingText.indexOf("HTTP"); // find end of string

  if ((stringstart < 0) || (stringstop < 0)) {
    ghn1.notify("warning, string read false");
    Serial.print("warning, string read false");
  }

  else {
    incommingText = incommingText.substring((stringstart + 9), (stringstop - 1)); //get rid of unused characters

    incommingText.replace("%20", " "); //replace %20 for a space
    incommingText.replace("_", " ");

    Serial.print("Fixed result:");
    Serial.print(incommingText);
    
    Broadcast(channel);
  }
}


void Broadcast(byte channel) {

  //the notify function doesnt't seem to accept string variables so they must be converted to chars
  char* uitgoingText = &incommingText[0];

  Serial.print("uitgoingText: ");
  Serial.println(uitgoingText);


  if (channel == 0) {
    ghn1.notify(uitgoingText);
    ghn2.notify(uitgoingText);
    Serial.println("broadcasting all channels");

  }

  else if (channel == 1) {
    ghn1.notify(uitgoingText);
    Serial.println("broadcasting channel 1");
  }

  else  if (channel == 2) {
    ghn2.notify(uitgoingText);
    Serial.println("broadcasting channel 2");
  }

  else  {
    ghn1.notify(uitgoingText);
    Serial.println("warning, cannel not defined");
  }

  uitgoingText = NULL;
  delete uitgoingText; //clean memory

}


void sayA() {
  Serial.println("sayA");
  ghn1.notify("Long press detected");
}


void sayB() {
  Serial.println("sayB");
  ghn2.notify("Testing, 1,2 Testing ");

}
