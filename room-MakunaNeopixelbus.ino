//#include <WiFi.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <NeoPixelBus.h>

// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.

const uint16_t PixelCount = 300;

//wifi credentials
const char* ssid = "-.-.-."; //your WiFi Name
const char* password = "-.-.-.";  //Your Wifi Password
const char* mqtt_server = "-.-.-.";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

NeoPixelBus<NeoRgbwFeature, Neo800KbpsMethod> strip(PixelCount);//,pin has to be added into strip bracket for esp32


//virtual pins

int bed=1,work=1,choice=1,lum=75,v=30,m,vpR,vpG,vpB,invert=0,rgbh,add=0, sortint;
int sort[4]={2,0,1,3};

unsigned long timex; //time buffer
uint8_t nc[PixelCount];
const uint16_t schtep=300;
uint16_t sortbuf=0000, loopvar=0;
uint32_t flood1[schtep],flood2[schtep];

/******---------------*****
*******-----SETUP-----*****
*******------------------*/




void setup() {

// put your setup code here, to run once: 
  Serial.begin(115200);
  Serial.println("start setup");

  //initialize all pixels on white
  strip.Begin();
  
  for(uint16_t i=0; i< PixelCount; i++){
    thecolor( stripColor(0,0,0,255), i);  
  }
  strip.Show();

  Serial.println("strip initialized - start nodered");
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    reconnect();
  }

  client.publish("/Leds/mode","1");
  client.publish("/Leds/buttons/bedon","true");
  client.publish("/Leds/buttons/workon","true");
  client.publish("/Leds/buttons/noColor","false");
  client.publish("/Leds/buttons/noInvert","false");
  client.publish("/Leds/sliders/v","30");
  client.publish("/Leds/sliders/rgbLum","255");
  client.publish("/Leds/sliders/Lum","75");
  client.publish("/Leds/waterdir","2013");
  
  randomSeed(analogRead(0));
  //Serial.println("button state");
  //change button state to on
  
  //for fire sim
  for(uint16_t i=0; i< PixelCount; i++){
    nc[i]=random(schtep);
  }
  Serial.println("end setup");
}


//
//  -WIFI setup-
//
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//
//  -callback fn-
//
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

 if(topic=="Leds/mode"){
   choice = messageTemp.toInt(); 
}
else if(topic=="Leds/bed"){
  if(messageTemp == "true"){
        bed = 1;
      }
      else if(messageTemp == "false"){
        bed = 0;      
      }
}
else if(topic=="Leds/work"){
  if(messageTemp == "true"){
    work = 1;
  }
  else if(messageTemp == "false"){
    work = 0;      
  }
}
else if(topic=="Leds/addcolor"){
  if(messageTemp == "true"){
    add = 1;
  }
  else if(messageTemp == "false"){
    add = 0;      
  }
}
else if(topic=="Leds/invert"){
  if(messageTemp == "true"){
    invert = 1;
  }
  else if(messageTemp == "false"){
    invert = 0;      
  }
}
else if(topic=="Leds/modu"){
  m = messageTemp.toInt();
}
else if(topic=="Leds/v"){
  v = messageTemp.toInt();
}
else if(topic=="Leds/rgbLum"){
  rgbh = messageTemp.toInt();
}
else if(topic=="Leds/lum"){
  lum = messageTemp.toInt();
}
else if(topic=="Leds/colorPick"){
  char hexString[8];
      messageTemp.toCharArray(hexString,8);
      byte r,g,b;
      long l=strtol(hexString+1,NULL,16);
      r = l>>16;
      g = l<<8;
      b = l;
      vpR = r; 
      vpG = g;
      vpB = b;
}
else if(topic=="Leds/waterDir"){
  sortint = messageTemp.toInt();
  for(int i=3;i<=0;i--){
    sort[i]= sortint%10;
    sortint/=10;
    Serial.print(sort[i]);
  }
} 
}

//
//  -reconn fn-
//
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("Leds/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/******---------------*****
*******------LOOP-----*****
*******------------------*/




void loop() {

client.loop();


/******---------------*****
*******---Opttions----*****
*******------------------*/
//Serial.print("Switch Case::");
 
  switch(choice){
  
    //off-work-bed
    case 1: //Serial.println("case1");
            for(uint16_t i=0; i< PixelCount; i++){
              thecolor( stripColor(0,0,0,0), i);  
            }
            //Serial.print("strip show::");
            strip.Show();
            //Serial.print("break::");
    break;
    
    //Custom
    case 2: //Serial.println("case2");
            customcolor();
            //Serial.println("ccolor");
    break;
    
    //Rainbow
    case 3: //Serial.println("case3");
            rainbow();
            //Serial.println("rbow");
    break;
    
    //RainbowBreathe
    case 4: //Serial.println("case4");
            rainbowbreathe();
            //Serial.println("rbbreathe");
    break;
    
    //CustomBreathe
    case 5: //Serial.println("case5");
            custombreathe();
            //Serial.println("cbreathe");
    break;
    
    //RainbowCycle
    case 6: //Serial.println("case6");
            rainbowCycle();
            //Serial.println("rbcycle");
    break;
    
    //RainbowStereo
    case 7: //Serial.println("case7");
            stereorainbowCycle();
            //Serial.println("srbcycle"); 
    break;
    
    //RainbowStereoBW
    case 8: //Serial.println("case8");
            bwstereorainbowCycle();
            //Serial.println("bwrbcycle");
    break;
    
    //RainbowInstant
    case 9: //Serial.println("case9");
            rainbowinstant ();
            //Serial.println("rbinstant");
    break;
    
    //RainbowFlicker
    case 10: //Serial.println("case10");
            rainbowflicker();
            //Serial.println("rbflicker");
    break;
    
    //CustomFlicker
    case 11: //Serial.println("case11");
            customflicker();
            //Serial.println("cflicker");
    break;
    
    //Police
    case 12: //Serial.println("case12");
            police();
            //Serial.println("police");
    break;
    
    //PoliceAnimated
    case 13: //Serial.println("case13");
            anipolice(); 
            //Serial.println("apolice");
    break;
    
    //thunder
    case 14: //Serial.println("case14");
            thunder();
            //Serial.println("thunder");
    break;
    
    //RainbowWipe
    case 15: //Serial.println("case15");
            rainbowwipe();
            //Serial.println("rbwipe");
    break;
    
    //CycleBreathe
    case 16: //Serial.println("case16");
            cycleBreathe();
            //Serial.println("cycbreathe");
    break;
    
    //Fire
    case 17: //Serial.println("case17");
            fire();
            //Serial.println("fire");
    break;
    
    //Water
    case 18: //Serial.println("case18");
            water();
            //Serial.println("water");
    break;
    
    default: break;
  }

//Serial.println("End of loop");
}






/******---------------*****
*******---functions---*****
*******------------------*/


//turn off when white
void thecolor(uint32_t c, uint16_t i){
      //Serial.print("thecolor ");
      //Serial.print(i);
      //Serial.print("::");
      strip.SetPixelColor(i, brightness(rgblum(c, rgbh)));     
      if(bed==1 && invert==0 && i < (PixelCount/2) ){
        //Serial.print("bed=1::");
        if(add==1){strip.SetPixelColor(i, brightness(stripColor(vpR, vpG, vpB,255)));/*Serial.print("add=1::");*/}
        else{strip.SetPixelColor(i, brightness(stripColor(0, 0, 0,255)));/*Serial.print("add=0::");*/}}
      else if(bed==1 && invert==1 && i < (PixelCount/2) ){
        strip.SetPixelColor(i, RgbwColor(0, 0, 0, 0)); }
            
      if(work==1 && invert==0 && i >= (PixelCount/2) ){
        //Serial.print("work=1::");
        if(add==1){strip.SetPixelColor(i, brightness(stripColor(vpR, vpG, vpB,255)));/*Serial.print("add=1::");*/}
        else{strip.SetPixelColor(i, brightness(stripColor(0, 0, 0,255)));/*Serial.print("add=0::");*/}}
      else if(work==1 && invert==1 && i >= (PixelCount/2) ){
        strip.SetPixelColor(i, RgbwColor(0, 0, 0, 0)); }
      
}


RgbwColor brightness(uint32_t color){
    uint8_t r,g,b,w;
    w = (uint8_t)(color >> 24),
    r = (uint8_t)(color >> 16),
    g = (uint8_t)(color >>  8),
    b = (uint8_t)color;
    
    r = (r * lum) >>8;
    g = (g * lum) >>8;
    b = (b * lum) >>8;
    w = (w * lum) >>8;

    return(RgbwColor(g, r, b, w));
}


uint32_t rgblum(uint32_t color, uint8_t lum){
    uint8_t r,g,b,w;
    w = (uint8_t)(color >> 24),
    r = (uint8_t)(color >> 16),
    g = (uint8_t)(color >>  8),
    b = (uint8_t)color;
    
    r = (r * lum) >>8;
    g = (g * lum) >>8;
    b = (b * lum) >>8;
    
    return(stripColor(r, g, b, w));
}


uint32_t stripColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w){
  return ((uint32_t)w << 24 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}




//random color generator
 uint32_t randomcolor(){
 uint32_t rc[6]={ stripColor(230, 130, 0,0), //yellow
                  stripColor(130, 30, 0,0),  //orange
                  stripColor(255, 0, 0,0),   //red
                  stripColor(200, 0, 200,0), //purple
                  stripColor(0, 0, 255,0),   //blue
                  stripColor(0, 255, 0,0)};  //green
 return(rc[random(6)]);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return stripColor(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return stripColor(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return stripColor(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

 
//custom color
void customcolor(){
  for(uint16_t i=0; i< PixelCount; i++){
  thecolor(stripColor(vpR,vpG,vpB,0),i);  
  }
strip.Show();
}


//rainbow
void rainbow(){
  uint16_t i;
    
  if(loopvar>=256){loopvar=0;}
  
  if(millis()-timex>v){
     timex=millis();
     for(i=0; i<PixelCount; i++) {
      thecolor(Wheel((loopvar) & 255),i);
     }
     loopvar++;
     strip.Show();
  }
}



//Rainbow function
void rainbowCycle(){
  uint16_t i;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
   // 5 cycles of all colors on wheel
    timex=millis();
    for(i=0; i< PixelCount; i++) { 
      thecolor( Wheel(((i * 256 / PixelCount) + loopvar) & 255), i);
    }
    loopvar++;
    strip.Show();
  }

}


//stereo Rainbow
void stereorainbowCycle(){
  uint16_t h, i;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
   // 5 cycles of all colors on wheel
    timex=millis();
    for( i=0; i< (PixelCount/2); i++) {
        
      thecolor( Wheel(((i * 256 / PixelCount) + loopvar) & 255) , i);
      
      h=PixelCount-1-i;
      thecolor( Wheel(((i * 256 / PixelCount) + loopvar) & 255) , h);
      
    }
    loopvar++;
    strip.Show();
  }
}


//stereo Rainbow backwards
void bwstereorainbowCycle(){
  uint16_t h, i;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
   // 5 cycles of all colors on wheel
    timex=millis();
    for( i=0; i< (PixelCount/2); i++) {
        
      thecolor( Wheel(((i * 256 / PixelCount) - loopvar) & 255) , i);
      
      h=PixelCount-1-i;
      thecolor( Wheel(((i * 256 / PixelCount) - loopvar) & 255) , h);
      
    }
    loopvar++;
    strip.Show();
  }
}


//rainbow flicker
void rainbowflicker(){
  static bool bump=true;
  uint32_t rc=randomcolor();

  if(millis()-timex>v && bump==true){
  for(uint16_t i=0; i<PixelCount ; i++){  
    thecolor(rc, i);
  }
  timex=millis();
  bump=false;
  }
  else if (millis()-timex>v && bump==false){
  for(uint16_t i=0; i< PixelCount; i++){
    thecolor( stripColor(0,0,0,0), i); 
  }
  timex=millis();
  bump=true;
  }
  strip.Show();
}


//customflicker
void customflicker(){
  static bool bump=true;

  if(millis()-timex>v && bump==true){
  for(uint16_t i=0; i< PixelCount; i++){
    thecolor(stripColor(vpR,vpG,vpB,0), i);  
  }
  timex=millis();
  bump=false;
  }
  else if(millis()-timex>v && bump==false){
  for(uint16_t i=0; i< PixelCount; i++){
    thecolor( stripColor(0,0,0,0), i);  
  }
  timex=millis();
  bump=true;
  }
  strip.Show();
}


//rainbow breathe
void rainbowbreathe(){
  uint16_t i, k;
  if(loopvar>=256){loopvar=0;}

  if (millis()-timex>v){
    k=sin(loopvar*m*M_PI*2/(256))*127+128;
    for(i=0; i<PixelCount; i++) {
      thecolor( rgblum(Wheel((loopvar) & 255), k) , i);
    }
    loopvar++;
    strip.Show();
  }
}


//cycle breathe
void cycleBreathe(){
  uint16_t j, k;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
// 5 cycles of all colors on wheel
    k=sin(loopvar*m*M_PI*2/256)*127+128;
    for(j=0; j< PixelCount; j++) { 
      thecolor( rgblum(Wheel(((j * 256 / PixelCount) + loopvar) & 255), k), j);
    }
    timex=millis();
    loopvar++;
    strip.Show();
  }
}

//custom breathe
void custombreathe(){

  uint16_t i, k;
  uint16_t l=v*m;
 
    k=sin(millis()*(M_PI*2/l))* 127 + 128;
    for(i=0; i<PixelCount; i++) {
      thecolor( rgblum(stripColor(vpR,vpG,vpB,0), k) , i);
    }
    //Serial.print("show ");
    strip.Show();
}


//rainbow instant
void rainbowinstant (){

  if (millis()-timex>v){
  timex=millis();
  uint32_t rc=randomcolor();
  for(uint16_t i=0; i< PixelCount; i++){
  thecolor(rc, i);  
  }
  //Serial.print("show ");
  strip.Show();
  }
}


//police
void police(){
uint8_t red;
uint8_t blue;
  for(uint16_t i=0; i<PixelCount; i++) {
    red = 256 * 0.5 * (1 + sin( (m*M_PI/75) * i ))-1;
    blue= 256 * 0.5 * (1 - sin( (m*M_PI/75) * i ))-1; 
    thecolor( stripColor(red,0,blue,0), i );
   }
strip.Show();
  
}


//police animated
void anipolice(){
uint8_t red;
uint8_t blue;
  if(loopvar>=PixelCount){loopvar=0;}

  if (millis()-timex>v){

    timex=millis();
    for(uint16_t i=0; i < PixelCount ; i++){
      uint16_t k=i+loopvar;
      if(k>PixelCount){
        k=(i+loopvar)-PixelCount-1;
      }
    red = 220 * 0.5 * (1 + sin( (m*M_PI/75) * k ));
    blue= 220 * 0.5 * (1 - sin( (m*M_PI/75) * k )); 
    thecolor( stripColor(red,0,blue,0), i );
    }

  loopvar++;
  strip.Show();
  }
}


//Lightning
void thunder(){
  static uint8_t ir, it=0, thunderstep=0,leng;
  //uint8_t leng; //length
  //uint16_t loc; //location
  static uint16_t delai=0,loc; //delay
  uint8_t colour=255;
  uint8_t rcolour=vpR, gcolour=vpG, bcolour=vpB;
  
  switch(thunderstep){
      case 0:
      Serial.print("case0::loc::");
        leng=random(4,30); //length
        loc=random(0, PixelCount);//location
        ir=random(2,5);    //n repetitions
        it=0;              //repetition
        Serial.print(loc);
        Serial.print("::leng::");
        Serial.println(leng);
        thunderstep++;
      break;
  
      case 1:
        if(millis()-timex>delai){
          Serial.print("case1:");
          Serial.print(delai);
          Serial.print("::");
          if(it>0){
            uint8_t thisrand=random(30, 50);
            colour = colour * thisrand/100 ;
            rcolour = rcolour * thisrand/100 ;
            gcolour = gcolour * thisrand/100 ;
            bcolour = bcolour * thisrand/100 ;
          }
          for( uint16_t i=loc; i<(loc+leng) ;i++){
            uint16_t j=i;
            if(i>PixelCount-1){
              j = i - PixelCount;
            }
            Serial.print(j);
            Serial.print(":");
            thecolor( stripColor(rcolour,gcolour,bcolour,colour) , j);
          }
          Serial.println();
          it++;
          thunderstep++;
          delai=random(50,120);
          timex=millis();
          Serial.println("show light");
          strip.Show();
        }
      break;
  
      case 2:
        if(millis()-timex>delai){
            Serial.print("case2::");
            Serial.println(delai);
          for(uint16_t i=0; i<PixelCount; i++) {
            thecolor( stripColor(0,0,0,0), i );
          }
          Serial.println();
          Serial.println("show dark");
          strip.Show();
          
          if(it>=ir){
            thunderstep++;  
            delai=random(500,1000);
          }
          else{
            thunderstep=1;
            delai=random(50,170);
          }
          timex=millis();
        }
      break;
      
      default:
        if(millis()-timex>delai){
          Serial.print("def::");
          Serial.println(delai);
          thunderstep=0;
        }
      break;
  }
}


//RainbowWipe
void rainbowwipe(){
    static uint32_t rcb, rc=randomcolor();
      
      if(loopvar>=PixelCount/2){
        loopvar=0;
        while( rcb == rc ){
          rc=randomcolor();
        }
        }

      if (millis()-timex>v){
        uint16_t j=PixelCount -loopvar-1;
        thecolor(rc, loopvar);
        thecolor(rc, j);
        timex=millis();
        loopvar++;
        strip.Show();

    rcb=rc;
    }
  }


//fire sim
void fire(){
static uint8_t randa = random(1,5), randb = random(2,5);
int h=0;
uint16_t o=1,randc, r1 = (schtep/(randa*2)), r2=(schtep/(randb*randa*2));
uint8_t rg[schtep][2], g[schtep];

for( uint16_t i=0; i < schtep ; i++){
  rg[i][0]=255 * 0.5 * (1-cos( (M_PI*2/schtep) * i*randa ));
  if(i < schtep/(randa*randb)){
    g[i]=80 *  0.5 * (1-cos( (M_PI*2/(schtep)) * i*(randb*randa) ));
  }
  if( (o==1 && i >= (r1-r2)) || (o>1 && (o*schtep/randa+r1-r2 <= i)) ){
    rg[i][1]=g[h];
    h++;
    if(h >= schtep/(randa*randb) ){
      h=0;
      o++;}
  }else{
    rg[i][1]=0;
   }
}
if (millis()-timex>v){
for(uint16_t i=0; i< PixelCount; i++){
  thecolor( stripColor(rg[nc[i]][0],rg[nc[i]][1],0,0), i);  
  nc[i]++;
  if(nc[i]>=schtep){
    nc[i]=0;
  }
}

timex=millis();
strip.Show();
}
}



void water(){

uint8_t r1,g1,b1,r2,g2,b2;

if(sortbuf != sortint){
Serial.println("Wavegen!!");
wavegen();
}

if(loopvar>=schtep){loopvar=0;}

if (millis()-timex>v){
timex=millis();
  for(int j=0;j<PixelCount;j++){
    uint16_t nc1=loopvar+j;
    int16_t nc2=j-loopvar;
    if(nc1>=schtep){nc1=(j+loopvar)-schtep;}
    if(nc2<0){nc2=schtep+j-loopvar;}
      
    r1 = (uint8_t)(flood1[nc1] >> 16),
    g1 = (uint8_t)(flood1[nc1] >>  8),
    b1 = (uint8_t)flood1[nc1];
    r2 = (uint8_t)(flood2[nc2] >> 16),
    g2 = (uint8_t)(flood2[nc2] >>  8),
    b2 = (uint8_t)flood2[nc2];
      
      thecolor(stripColor(max(r1,r2),max(g1,g2),max(b1,b2),0),j);
  }
  loopvar++;
  strip.Show(); 
}
}

float wavsine(uint16_t offset,int32_t i){
  return(1+sin( (2*M_PI/schtep)* i*offset));
}

void wavegen(){
  
uint16_t wave[4][3],offset0=random(5,10),offset1=random(3,10),offset2=random(3,10),offset3=random(3,10), tide1=5*random(6,10), tide2=5*random(6,10), tide3=5*random(6,10);
wave[0][0]=wave[0][1]=wave[1][0]=wave[2][1]=0;

for(uint16_t i=0; i<schtep; i++){
           
  wave[0][2]= 255*0.01 *(4+48*wavsine(offset0,i));
      
  wave[1][1]= 60*0.01 *tide1 *wavsine(offset1,i);
  wave[1][2]= 160*0.01*tide1*wavsine(offset1,i);
     
  wave[2][0]= 60*0.01 *tide2 *wavsine(offset2,i);
  wave[2][2]= 160*0.01 *tide2 *wavsine(offset2,i);
      
  wave[3][0]= 30*0.01 *tide3 *wavsine(offset3,i);
  wave[3][1]= 30*0.01 *tide3 *wavsine(offset3,i);
  wave[3][2]= 150*0.01 *tide3 *wavsine(offset3,i);

  flood1[i]=stripColor(max(wave[sort[0]][0],wave[sort[1]][0]),max(wave[sort[0]][1],wave[sort[1]][1]),max(wave[sort[0]][2],wave[sort[1]][2]),0);
  flood2[i]=stripColor(max(wave[sort[2]][0],wave[sort[3]][0]),max(wave[sort[2]][1],wave[sort[3]][1]),max(wave[sort[2]][2],wave[sort[3]][2]),0);
}
sortbuf=sortint;
}

void bubbleUnsort(int *list, int elem)
{
 for (int a=elem-1; a>0; a--)
 {
   int r = random(a+1);
   if (r != a)
   {
     int temp = list[a];
     list[a] = list[r];
     list[r] = temp;
   }
 }
}
