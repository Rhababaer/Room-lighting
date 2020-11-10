#include<ESP8266WiFi.h> 
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>

// For Esp8266, the Pin is omitted and it uses GPIO3 due to DMA hardware use.  
// There are other Esp8266 alternative methods that provide more pin options, but also have
// other side effects.
#define PIN D4

#define NUM_LEDS 300
const uint16_t numpixels=300;

#define BRIGHTNESS 75

//wifi credentials
const char* ssid = "None of your business"; //your WiFi Name
const char* password = "still none of yours";  //Your Wifi Password
char auth[] = "think you get the idea";
//WiFiServer server(80);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRBW + NEO_KHZ800);

byte neopix_gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

//virtual pins

int bed=1,work=1,choice=1,vp4=75,v=20,m,vp7,vp8,vp9,invert=0,rgbh,add=0, sortint;
int sort[4]={2,0,1,3};

//bed
BLYNK_WRITE(V1){ bed = param.asInt(); }
//work
BLYNK_WRITE(V2){ work = param.asInt(); }
//choice
BLYNK_WRITE(V3){ choice = param.asInt(); }
//slider brightness
BLYNK_WRITE(V4){ vp4 = param.asInt(); }
//slider velocity
BLYNK_WRITE(V5){ v = param.asInt(); }
//slider multiplier
BLYNK_WRITE(V6){ m = param.asInt(); }
// R
BLYNK_WRITE(V7){ vp7 = param.asInt(); }
// G
BLYNK_WRITE(V8){ vp8 = param.asInt(); }
// B
BLYNK_WRITE(V9){ vp9 = param.asInt(); }
//invert
BLYNK_WRITE(V10){ invert = param.asInt(); }
//control water
BLYNK_WRITE(V11){ sortint = param.asInt();
                for(int i=3;i<=0;i--){
                sort[i]= sortint%10;
                sortint/=10;
                Serial.print(sort[i]);
                }
                }
//RGB helligkeit
BLYNK_WRITE(V12){ rgbh = param.asInt(); }
//add color
BLYNK_WRITE(V13){ add = param.asInt(); }


BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}


unsigned long timex; //time buffer
uint8_t nc[numpixels];
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
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  for(uint16_t i=0; i< strip.numPixels(); i++){
    thecolor( strip.Color(0,0,0,255), i);  
  }
  strip.show();

  Serial.println("strip initialized - start blynk");
  
  Blynk.begin(auth, ssid, password,IPAddress(xxx,xxx,xxx,xxx),8080);
  if(Blynk.connect()){Serial.println("blynk connected");}
  //ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);
  
  randomSeed(analogRead(0));
  //Serial.println("button state");
  //change button state to on
  Blynk.virtualWrite(V1, 1);
  Blynk.virtualWrite(V2, 1);
  Blynk.virtualWrite(V3, 1);
  Blynk.virtualWrite(V4, 75);
  Blynk.virtualWrite(V5, 20);
  Blynk.virtualWrite(V10, 0);
  Blynk.virtualWrite(V13, 0);
  bed=1;
  work=1;
  add=0;
  invert=0;
  choice=1;
  vp4=75;
  v=20;
  
  //sortbuf to input
  Blynk.virtualWrite(V11, 2013);
  //Blynk.syncAll();
  //for fire sim
  for(uint16_t i=0; i< numpixels; i++){
    nc[i]=random(schtep);
  }
  Serial.println("end setup");
}



/******---------------*****
*******------LOOP-----*****
*******------------------*/




void loop() {
Serial.print("Start loop::");
//Serial.print("Blynk Run::"); 
  Blynk.run();
  //serial output
  if(!Blynk.connected())
    {
       Serial.println("not connected");
    }
//Serial.print("Color Selector::");
  //color selector
  uint32_t color = strip.Color(vp7, vp8, vp9);
  
  //set brightness
  strip.setBrightness(vp4); 


/******---------------*****
*******---Opttions----*****
*******------------------*/
//Serial.print("Switch Case::");
 
  switch(choice){
  
    //off-work-bed
    case 1: //Serial.println("case1");
            for(uint16_t i=0; i< strip.numPixels(); i++){
              thecolor( strip.Color(0,0,0), i);  
            }
            Serial.print("strip show::");
            strip.show();
            Serial.print("break::");
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

Serial.println("End of loop");
}






/******---------------*****
*******---functions---*****
*******------------------*/


//turn off when white
void thecolor(uint32_t c, uint16_t i){
      //Serial.print("thecolor ");
      //Serial.print(i);
      //Serial.print("::");
      strip.setPixelColor(i, rgblum(c, rgbh));     
      if(bed==1 && invert==0 && i < (strip.numPixels()/2) ){
        //Serial.print("bed=1::");
        if(add==1){strip.setPixelColor(i, vp7, vp8, vp9,255);/*Serial.print("add=1::");*/}
        else{strip.setPixelColor(i, 0, 0, 0,255);/*Serial.print("add=0::");*/}}
      else if(bed==1 && invert==1 && i < (strip.numPixels()/2) ){
        strip.setPixelColor(i, 0, 0, 0, 0); }
            
      if(work==1 && invert==0 && i >= (strip.numPixels()/2) ){
        //Serial.print("work=1::");
        if(add==1){strip.setPixelColor(i, vp7, vp8, vp9,255);/*Serial.print("add=1::");*/}
        else{strip.setPixelColor(i, 0, 0, 0,255);/*Serial.print("add=0::");*/}}
      else if(work==1 && invert==1 && i >= (strip.numPixels()/2) ){
        strip.setPixelColor(i, 0, 0, 0, 0); }
      
}

uint32_t rgblum(uint32_t color, uint8_t lum){
    uint8_t r,g,b;
    r = (uint8_t)(color >> 16),
    g = (uint8_t)(color >>  8),
    b = (uint8_t)color;
    
    r = (r * lum) >>8;
    g = (g * lum) >>8;
    b = (b * lum) >>8;
    
    return(strip.Color(r, g, b));
}


//random color generator
 uint32_t randomcolor(){
 uint32_t rc[6]={ strip.Color(230, 130, 0), //yellow
                  strip.Color(130, 30, 0),  //orange
                  strip.Color(255, 0, 0),   //red
                  strip.Color(200, 0, 200), //purple
                  strip.Color(0, 0, 255),   //blue
                  strip.Color(0, 255, 0)};  //green
 return(rc[random(6)]);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

 
//custom color
void customcolor(){
  for(uint16_t i=0; i< strip.numPixels(); i++){
  thecolor(strip.Color(vp7,vp8,vp9),i);  
  }
strip.show();
}


//rainbow
void rainbow(){
  uint16_t i;
    
  if(loopvar>=256){loopvar=0;}
  
  if(millis()-timex>v){
     timex=millis();
     for(i=0; i<strip.numPixels(); i++) {
      thecolor(Wheel((loopvar) & 255),i);
     }
     loopvar++;
     strip.show();
  }
}



//Rainbow function
void rainbowCycle(){
  uint16_t i;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
   // 5 cycles of all colors on wheel
    timex=millis();
    for(i=0; i< strip.numPixels(); i++) { 
      thecolor( Wheel(((i * 256 / strip.numPixels()) + loopvar) & 255), i);
    }
    loopvar++;
    strip.show();
  }

}


//stereo Rainbow
void stereorainbowCycle(){
  uint16_t h, i;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
   // 5 cycles of all colors on wheel
    timex=millis();
    for( i=0; i< (strip.numPixels()/2); i++) {
        
      thecolor( Wheel(((i * 256 / strip.numPixels()) + loopvar) & 255) , i);
      
      h=strip.numPixels()-1-i;
      thecolor( Wheel(((i * 256 / strip.numPixels()) + loopvar) & 255) , h);
      
    }
    loopvar++;
    strip.show();
  }
}


//stereo Rainbow backwards
void bwstereorainbowCycle(){
  uint16_t h, i;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
   // 5 cycles of all colors on wheel
    timex=millis();
    for( i=0; i< (strip.numPixels()/2); i++) {
        
      thecolor( Wheel(((i * 256 / strip.numPixels()) - loopvar) & 255) , i);
      
      h=strip.numPixels()-1-i;
      thecolor( Wheel(((i * 256 / strip.numPixels()) - loopvar) & 255) , h);
      
    }
    loopvar++;
    strip.show();
  }
}


//rainbow flicker
void rainbowflicker(){
  static bool bump=true;
  uint32_t rc=randomcolor();

  if(millis()-timex>v && bump==true){
  for(uint16_t i=0; i<strip.numPixels() ; i++){  
    thecolor(rc, i);
  }
  timex=millis();
  bump=false;
  }
  else if (millis()-timex>v && bump==false){
  for(uint16_t i=0; i< strip.numPixels(); i++){
    thecolor( strip.Color(0,0,0), i); 
  }
  timex=millis();
  bump=true;
  }
  strip.show();
}


//customflicker
void customflicker(){
  static bool bump=true;

  if(millis()-timex>v && bump==true){
  for(uint16_t i=0; i< strip.numPixels(); i++){
    thecolor(strip.Color(vp7,vp8,vp9), i);  
  }
  timex=millis();
  bump=false;
  }
  else if(millis()-timex>v && bump==false){
  for(uint16_t i=0; i< strip.numPixels(); i++){
    thecolor( strip.Color(0,0,0), i);  
  }
  timex=millis();
  bump=true;
  }
  strip.show();
}


//rainbow breathe
void rainbowbreathe(){
  uint16_t i, k;
  if(loopvar>=256){loopvar=0;}

  if (millis()-timex>v){
    k=sin(loopvar*m*M_PI*2/(256))*127+128;
    for(i=0; i<strip.numPixels(); i++) {
      thecolor( rgblum(Wheel((loopvar) & 255), k) , i);
    }
    loopvar++;
    strip.show();
  }
}


//cycle breathe
void cycleBreathe(){
  uint16_t j, k;
  if(loopvar>=256*5){loopvar=0;}

  if (millis()-timex>v){
// 5 cycles of all colors on wheel
    k=sin(loopvar*m*M_PI*2/256)*127+128;
    for(j=0; j< strip.numPixels(); j++) { 
      thecolor( rgblum(Wheel(((j * 256 / strip.numPixels()) + loopvar) & 255), k), j);
    }
    timex=millis();
    loopvar++;
    strip.show();
  }
}

//custom breathe
void custombreathe(){

  uint16_t i, k;
  uint16_t l=v*m;
 
    k=sin(millis()*(M_PI*2/l))* 127 + 128;
    for(i=0; i<strip.numPixels(); i++) {
      thecolor( rgblum(strip.Color(vp7,vp8,vp9), k) , i);
    }
    //Serial.print("show ");
    strip.show();
}


//rainbow instant
void rainbowinstant (){

  if (millis()-timex>v){
  timex=millis();
  uint32_t rc=randomcolor();
  for(uint16_t i=0; i< strip.numPixels(); i++){
  thecolor(rc, i);  
  }
  //Serial.print("show ");
  strip.show();
  }
}


//police
void police(){
uint8_t red;
uint8_t blue;
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    red = 256 * 0.5 * (1 + sin( (m*M_PI/75) * i ))-1;
    blue= 256 * 0.5 * (1 - sin( (m*M_PI/75) * i ))-1; 
    thecolor( strip.Color(red,0,blue), i );
   }
strip.show();
  
}


//police animated
void anipolice(){
uint8_t red;
uint8_t blue;
  if(loopvar>=strip.numPixels()){loopvar=0;}

  if (millis()-timex>v){

    timex=millis();
    for(uint16_t i=0; i < strip.numPixels() ; i++){
      uint16_t k=i+loopvar;
      if(k>strip.numPixels()){
        k=(i+loopvar)-strip.numPixels()-1;
      }
    red = 220 * 0.5 * (1 + sin( (m*M_PI/75) * k ));
    blue= 220 * 0.5 * (1 - sin( (m*M_PI/75) * k )); 
    thecolor( strip.Color(red,0,blue), i );
    }

  loopvar++;
  strip.show();
  }
}


//Lightning
void thunder(){
  static uint8_t ir, it=0, thunderstep=0,leng;
  //uint8_t leng; //length
  //uint16_t loc; //location
  static uint16_t delai=0,loc; //delay
  uint8_t colour=255;
  uint8_t rcolour=vp7, gcolour=vp8, bcolour=vp9;
  
  switch(thunderstep){
      case 0:
      Serial.print("case0::loc::");
        leng=random(4,30); //length
        loc=random(0, numpixels);//location
        ir=random(2,5);    //n repetitions
        it=0;              //repetition
        Serial.print(loc);
        Serial.print("::leng::");
        Serial.println(leng);
        thunderstep++;
      break;
  
      case 1:
        if(millis()-timex>delai){
          Serial.print("case1::");
          Serial.println(delai);
          if(it>0){
            uint8_t thisrand=random(30, 50);
            colour = colour * thisrand/100 ;
            rcolour = rcolour * thisrand/100 ;
            gcolour = gcolour * thisrand/100 ;
            bcolour = bcolour * thisrand/100 ;
          }
          for( uint16_t i=loc; i<(loc+leng) ;i++){
            uint16_t j=i;
            if(i>strip.numPixels()-1){
              j = i - strip.numPixels();
            }
            Serial.print(j);
            Serial.print(":");
            thecolor( strip.Color(rcolour,gcolour,bcolour,colour) , j);
          }
          Serial.println();
          it++;
          thunderstep++;
          delai=random(50,120);
          timex=millis();
          Serial.println("show light");
          strip.show();
        }
      break;
  
      case 2:
        if(millis()-timex>delai){
            Serial.print("case2::");
            Serial.println(delai);
          for(uint16_t i=0; i<strip.numPixels(); i++) {
            thecolor( strip.Color(0,0,0,0), i );
          }
          Serial.println();
          Serial.println("show dark");
          strip.show();
          
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
      
      if(loopvar>=strip.numPixels()/2){
        loopvar=0;
        while( rcb == rc ){
          rc=randomcolor();
        }
        }

      if (millis()-timex>v){
        uint16_t j=strip.numPixels() -loopvar-1;
        thecolor(rc, loopvar);
        thecolor(rc, j);
        timex=millis();
        loopvar++;
        strip.show();

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
for(uint16_t i=0; i< strip.numPixels(); i++){
  thecolor( strip.Color(rg[nc[i]][0],rg[nc[i]][1],0), i);  
  nc[i]++;
  if(nc[i]>=schtep){
    nc[i]=0;
  }
}

timex=millis();
strip.show();
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
  for(int j=0;j<strip.numPixels();j++){
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
      
      thecolor(strip.Color(max(r1,r2),max(g1,g2),max(b1,b2)),j);
  }
  loopvar++;
  strip.show(); 
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

  flood1[i]=strip.Color(max(wave[sort[0]][0],wave[sort[1]][0]),max(wave[sort[0]][1],wave[sort[1]][1]),max(wave[sort[0]][2],wave[sort[1]][2]));
  flood2[i]=strip.Color(max(wave[sort[2]][0],wave[sort[3]][0]),max(wave[sort[2]][1],wave[sort[3]][1]),max(wave[sort[2]][2],wave[sort[3]][2]));
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
