# Room-lighting
Programm to control the addressable LED-strip in my room with ESP8266, [Node-Red](https://github.com/node-red/node-red) with [Mosquitto](https://github.com/eclipse/mosquitto) and [Neopixelbus by Makuna](https://github.com/Makuna/NeoPixelBus).
When I was looking for cool effect/animation functions I found numerous videos on Youtube, but all lacking the code to copy and explore, so I had to come up with something myself and this is it. To be fair I copied some functions from Adafruits examples, hence the ```Wheel()``` function. Now I am no expert in programming so some things in here just work I guess.

## On my Setup
The board I use is a Nodemcu v3 with a Strip of 300 SK6812 RGBW LEDs and local Node-Red server, so youd have to make changes to the code accordingly. As mentioned in the wiki of the NeoPixelBus the pin to connect the strip to is [automatically set to GPIO3](https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods#neoesp8266dma800kbpsmethod) on ESP8266. On ESP32 you have to add the designated Pin.
Because I use one half of the strip to light the working area of my room and the other half is facing the wall my bed is standing, ```void thecolor(c, i)``` came out little more complicated to turn each side on and off without effecting the other side. Reason for turning all LEDs white in setup was mounting this whole thing to the ceiling and turning it on with the light switch of my room. I am happy to say that I was able to get rid of every delay used to make the animations happen.

## What my Node-Red-Dash looks like
<img src="https://github.com/Rhababaer/Room-lighting/blob/main/nodered_dash.PNG">

## Managing Functions

### thecolor(c, i)
Takes color value and Pixel index and decides whether to turn white, ```add``` color component to white, ```invert``` to black/off or by default use the given  color. As mentioned in the previous paragraph, the function also handles the destinction on which half of the strip is to be turned white or black, hence the bed and work conditions.

### brightness(color), rgblum(color, lum)
As the names suggest are used to control the brightness. The brightness of the Adafruit Code is handled by a library function I pretty much copied. The functions split the combined uint32_t into their r g b w components. Brightness directly uses input from a Node-Red widget and turns the input into a ```RgbwColor``` variable.
Rgblum does the same but uses a separate variable to control brightness levels, ignores the white channel and outputs the same combined uint32_t color value.

## Animation Functions
### customcolor
Loops through all pixels and assigns a solid color

### rainbow
Changes the color of the whole strip once per time interval, from color to color.

### rainbowCycle, stereorainbowCycle, bwstereorainbowCycle
Id describe it as the classic addressable led effect. Each pixel gets assigned a color of the ```Wheel``` function, so that the strip takes the colors of a continous rainbow. This whole pallette gets moved one pixel each interval. The stereo functions just move the rainbow from or to the ends of the whole strip.

### rainbowbreathe, custombreathe, cyclebreathe
These are combination of the above with a sine wave function, that is used to change the brightness over time. Each interval is therefore picking a point in the sine function. 

### police, anipolice
I really like all those cyberpunk, red blue purple  colors and these functions sprung from just that. I again used sine functions. This time there are two, shifted 180° and ranging from 255 to 0. One is responsible for blue and the other for red, so that these two colors are alternating. The animated one moves both sine waves around the strip.

### rainbowwipe
Starts at the ends of the strip, picks a color from ```randomcolor``` and applies it to the next pixel each interval until all of the strip is covered, then picks another color.

### water
This is where it starts to get a little convoluted. The basic idea is having 4 shades of blue, assign these to 4 different sinewaves, making two pairs and have the pairs move in opposite directions. Now to the code side:
The waves are generated in ```wavegen()```. In this subfunction ```wave[][]``` represent the four waves, the first dimension specifies the index and the second stores the rgb color values. The wave array is then further summarized into the two pairs ```flood1[]``` and ```flood2[]```. ```sortbuf``` through ```sort[]``` is telling the function which wave to put in which flood array.  Finally ```water()``` handles the iteration through the flood arrays, splits the uint32_t into uint8_t rgb and assigns the highest color value respectively.

### fire
Compared to water, this is quiet simple, there are 2 cosine functions (to mix things up) with the same wavelength but different "heigth". The lower one is just red the other is for yellow. glowing red -> getting brighter -> yellow flaring -> getting darker etc.. Now each Pixel gets a random starting point and iterates through the generated sinewave array.

### thunder
Something I wanted for listening to scary horror podcasts/audiobooks/relaxing thunderstorms. Bursts of random repetitions (2-5), at a random point of the strip (```loc```) of random length/number of pixels (```leng```) with white+color color and random intensity per repetition (r-/g-/b-```colour```). ```case 0:``` most of the randomness gets assigned. ```case 1:```  random intensity if its not the first flash, then the random pixels get the color and flash. ```case 2:``` reset the pixels to black and revert to ```case 1```. When the number of flashes is reached, the function picks a new pixels for the next lightning.

### Problem I had with Adafruits Neopixel Library
Random resets every few hours. Namely hardware wdt resets of my ESP-8266 probably caused by number of LEDs in my setup (300) and timings in the show-function as discussed in this [thread](https://stackoverflow.com/questions/61265671/neopixel-sample-code-crashing-when-using-a-higher-number-of-pixels). I tested this with minimal code and let it run overnight with putty logging the serial port:
```C++
#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 300
#define BRIGHTNESS 75

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, 2, NEO_GRBW + NEO_KHZ800);

void setup() {

// put your setup code here, to run once: 
  Serial.begin(115200);
  Serial.println("start setup");

  //initialize all pixels on white
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  for(uint16_t i=0; i< strip.numPixels(); i++){
    strip.setPixelColor(i, 0, 0, 0,255);  
  }

  Serial.println("end setup");
}

void loop() {
Serial.print("Start loop::");

            for(uint16_t i=0; i< strip.numPixels(); i++){
              strip.setPixelColor(i, 0, 0, 0,255);  
            }
            Serial.print("strip show::");
            strip.show();

Serial.println("end loop");
```
```C++
#include <NeoPixelBus.h>
//const uint8_t PixelPin = 4;
const uint16_t PixelCount = 150;

NeoPixelBus<NeoRgbwFeature, Neo800KbpsMethod> strip(PixelCount);//PixelPin);

void setup() {

// put your setup code here, to run once: 
  Serial.begin(115200);
  Serial.println("start setup");

  //initialize all pixels on white
  strip.Begin();
  for(uint16_t i=0; i< PixelCount; i++){
    strip.SetPixelColor(i, RgbwColor(0, 0, 0,0));  
  }

  Serial.println("end setup");
}

void loop() {
Serial.print("Start loop::");

            for(uint16_t i=0; i< PixelCount; i++){
              strip.SetPixelColor(i,RgbwColor(0, 0, 0,255));  
            }
            Serial.print("strip show::");
            strip.Show();

Serial.println("end loop");
}
```
To make porting from Adafruit Neopixel library to Makunas Neopixelbus as easy as possible and retain most of the Codebase I used these functions:
```C++
//strip.Color became
uint32_t stripColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w){
  return ((uint32_t)w << 24 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

//brightness (used to hand colors to strip.SetPixelColor() and modifies the brightness
RgbwColor brightness(uint32_t color){
    uint8_t r,g,b,w;
    w = (uint8_t)(color >> 24),
    r = (uint8_t)(color >> 16),
    g = (uint8_t)(color >>  8),
    b = (uint8_t)color;
    
    r = (r * vp4) >>8;
    g = (g * vp4) >>8;
    b = (b * vp4) >>8;
    w = (w * vp4) >>8;

    return(RgbwColor(g, r, b, w));
}

//rgblum does basically the same without white but in uint32_t to control color brightness separetly, also useful for Adafruit
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
```
