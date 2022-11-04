
// here i used a cloned ir signal from a remote control to switch a Siemens extractor hood with a voicecommand


void toggleAfzuigkap() {
  Serial.println("send toggleAfzuigkap ");

  uint16_t rawData1[29] = {348, 298,  332, 596,  662, 296,  332, 298,  332, 296,  330, 298,  330, 612,  648, 596,  348, 296,  332, 296,  332, 300,  328, 300,  330, 298,  330, 298,  648};  // UNKNOWN C3124BAB
  uint16_t rawData2[29] = {338, 296,  332, 610,  648, 296,  332, 298,  332, 296,  332, 298,  330, 612,  648, 598,  346, 296,  332, 296,  334, 296,  332, 298,  332, 296,  332, 298,  644};  // UNKNOWN C3124BAB
  
  irsend.sendRaw(rawData1, 29, 38);  // Send a raw data capture at 38kHz.
  delay(100);
  irsend.sendRaw(rawData2, 29, 38);  // Send a raw data capture at 38kHz.
}



void toggleAfzuigkaplampjes() {
  Serial.println("send toggleAfzuigkaplampjes");

uint16_t rawData1[27] = {314, 322,  306, 632,  626, 324,  280, 348,  306, 322,  306, 322,  286, 652,  626, 634,  312, 320,  282, 348,  280, 348,  284, 346,  608, 646,  624};  // UNKNOWN 4B45CB74
uint16_t rawData2[27] = {312, 318,  308, 634,  624, 326,  278, 352,  278, 350,  282, 346,  282, 658,  624, 632,  314, 320,  308, 322,  288, 342,  280, 350,  594, 658,  624};  // UNKNOWN 4B45CB74
 
  irsend.sendRaw(rawData1, 27, 38);  // Send a raw data capture at 38kHz.
  delay(100);
  irsend.sendRaw(rawData1, 27, 38);  // Send a raw data capture at 38kHz.
}
