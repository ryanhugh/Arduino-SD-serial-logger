/*
A small arduino program to log data from the UART port 
to an SD card very quickly. Works at 115200 baud.


Files on the SD card are named LOG00000.TXT, LOG00001.TXT, etc
Each file is always 512,000 bytes, and the parts that have not been written to yet are all 0's

You can change the arduino's internal serial buffer at C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\HardwareSerial.cpp
by changing the line "#define SERIAL_BUFFER_SIZE 64"
I had it set to 500


Adopted from the RawWrite.ino example found here:
https://github.com/greiman/FreeRTOS-Arduino/blob/master/libraries/SdFat/examples/RawWrite/RawWrite.ino

Also used code from OpenLog
https://github.com/sparkfun/OpenLog
*/

#include <SdFat.h>
#include <SdFatUtil.h>

// SD chip select pin
#define chipSelect SS

// number of blocks in the contiguous file
const uint32_t BLOCK_COUNT = 1000UL;

//make another file if the one fills up
uint32_t currBlock = 0;

// file system
SdFat sd;

// test file
SdFile file;

// file extent
uint32_t bgnBlock, endBlock;

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// store error strings in flash to save RAM
#define error(s) sd.errorHalt_P(PSTR(s))
//------------------------------------------------------------------------------


#define printn Serial.print
#define printf(...) Serial.print(F(__VA_ARGS__))


unsigned int bufferIndex;

uint8_t* pCache;


void makeFile(void)
{

    //Search for next available log spot
    char new_file_name[] = "LOG00000.TXT";
    int new_file_number=0;
    while(1)
    {
        new_file_number++;

        //There is a max of 65534 logs
        if(new_file_number > 65533) 
        {
            printf(("!Too many logs:2!"));
            return; //Bail!
        }

        //Splice the new file number into this file name
        sprintf(new_file_name, "LOG%05d.TXT", new_file_number); 

        //Try to open file, if fail (file doesn't exist), then break
        if (file.open(new_file_name, O_CREAT | O_EXCL | O_WRITE)) break;
    }

    //Close the new file that was just opened
    file.close(); 

    // delete existing file that was just created, so it can be recreated as continuous
    sd.remove(new_file_name);


    // create a contiguous file
    if (!file.createContiguous(sd.vwd(), new_file_name, 512UL*BLOCK_COUNT)) {
        error("createContiguous failed");
    }
    // get the location of the file's blocks
    if (!file.contiguousRange(&bgnBlock, &endBlock)) {
        error("contiguousRange failed");
    }
    //*********************NOTE**************************************
    // NO SdFile calls are allowed while cache is used for raw writes
    //***************************************************************

    // clear the cache and use it as a 512 byte buffer
    pCache = (uint8_t*)sd.vol()->cacheClear();


    // tell card to setup for multiple block write with pre-erase
    if (!sd.card()->erase(bgnBlock, endBlock)) error("card.erase failed");
    if (!sd.card()->writeStart(bgnBlock, BLOCK_COUNT)) {
        error("writeStart failed");
    }

    printf("begin:");
    printn(bgnBlock);
    printf("\n");
    printf("end:");
    printn(endBlock);
    printf("\n");
}


void setup(void) {
    Serial.begin(115200);
    while (!Serial) {}  // wait for Leonardo


    cout << pstr("Free RAM: ") << FreeRam() << endl;

    // initialize the SD card at SPI_FULL_SPEED for best performance.
    // try SPI_HALF_SPEED if bus errors occur.
    if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();

    makeFile();

}

char chr;

//------------------------------------------------------------------------------
void loop(void) {


    // write data
    while (Serial.available())
    {

        //read the data and record it in the buffer
        chr=Serial.read();

        pCache[bufferIndex]=chr;
        bufferIndex++;
        

        //flush whenever the the buffer fills up, or when an 18 is recieved
        if (bufferIndex==511 || chr==18)
        {

            signed int t=millis();

            // write a 512 byte block
            if (!sd.card()->writeData(pCache)) 
            {
                error("writeData failed");
            }


            //log some info
            printf("flushing took ");
            printn((signed)millis()-t);
            printf(" ms\n");
            printn((char)18);
            if(chr==18){
                printf("chr\n");
            }
            if (bufferIndex==511)
            {
                printf("buf\n");
            }
            
            //reset the count
            bufferIndex=0;
            currBlock++;
            if(currBlock==BLOCK_COUNT){
                
              if (!sd.card()->writeStop()) error("writeStop failed");

              file.close();
              makeFile();

              currBlock=0;

            }
        }
    }
}