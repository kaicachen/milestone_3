#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "eecs388_lib.h"
#include "metal/i2c.h"

struct metal_i2c *i2c;
uint8_t bufWrite[5];
uint8_t bufRead[1];

//The entire setup sequence
void set_up_I2C()
{
    uint8_t oldMode;
    uint8_t newMode;
    _Bool success;

    bufWrite[0] = PCA9685_MODE1;
    bufWrite[1] = MODE1_RESTART;
    printf("%d\n",bufWrite[0]);
    
    i2c = metal_i2c_get_device(0);

    if(i2c == NULL){
        printf("Connection Unsuccessful\n");
    }
    else{
        printf("Connection Successful\n");
    }
    
    //Setup Sequence
    metal_i2c_init(i2c,I2C_BAUDRATE,METAL_I2C_MASTER);
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//reset
    delay(100);
    printf("resetting PCA9685 control 1\n");

    //Initial Read of control 1
    bufWrite[0] = PCA9685_MODE1;//Address
    success = metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,1,bufRead,1);//initial read
    printf("Read success: %d and control value is: %d\n", success, bufWrite[0]);
    
    //Configuring Control 1
    oldMode = bufRead[0];
    newMode = (oldMode & ~MODE1_RESTART) | MODE1_SLEEP;
    printf("sleep setting is %d\n", newMode);
    bufWrite[0] = PCA9685_MODE1;//address
    bufWrite[1] = newMode;//writing to register
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//sleep
    bufWrite[0] = PCA9685_PRESCALE;//Setting PWM prescale
    bufWrite[1] = 0x79;
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//sets prescale
    bufWrite[0] = PCA9685_MODE1;
    bufWrite[1] = 0x01 | MODE1_AI | MODE1_RESTART;
    printf("on setting is %d\n", bufWrite[1]);
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//awake
    delay(100);
    printf("Setting the control register\n");
    bufWrite[0] = PCA9685_MODE1;
    success = metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,1,bufRead,1);//initial read
    printf("Set register is %d\n",bufRead[0]);
} 

void breakup(int bigNum, uint8_t* low, uint8_t* high){
    *low = bigNum & 0xFF; // Masks the lowest 8 digits of bigNum and stores to the memory location of low
    *high = (bigNum >> 8) & 0xFF; // Bitshift right by 8 digits, then masks the highest 8 digits and stores in memory location of high
}


void steering(int angle){
    int cycle_value = getServoCycle(angle); // Converts angle to corresponding cycle value
    bufWrite[0] = PCA9685_LED1_ON_L; // Address for motor to operate
    bufWrite[1] = 0; 
    bufWrite[2] = 0; 
    breakup(cycle_value, &bufWrite[3], &bufWrite[4]); // Calls breakup to create two 8-bit numbers for the motor to read as how far to turn
    metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1); // Transfers the data to the PCA board
}


void stopMotor()
{
  bufWrite[0] = PCA9685_LED0_ON_L; //Address for motor to operate
  bufWrite[1] = 0; 
  bufWrite[2] = 0; 
  breakup(280, &bufWrite[3], &bufWrite[4]); // Calls breakup to create two 8-bit numbers for the motor to read as stop
  metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1); // Transfers the data to the PCA board
}


void driveForward(uint8_t speedFlag)
{
  bufWrite[0] = PCA9685_LED0_ON_L; //Address for motor to operate
  bufWrite[1] = 0; 
  bufWrite[2] = 0; 
  
  // Depending on speedFlag, calls breakup to create two 8-bit numbers for the motor to read as specific speed
  if(speedFlag == 1) {
    breakup(313, &bufWrite[3], &bufWrite[4]);
    } 
  else if(speedFlag == 2){
    breakup(315, &bufWrite[3], &bufWrite[4]);
    } 
  else if(speedFlag == 3) {
    breakup(317, &bufWrite[3], &bufWrite[4]);
    }

  metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1); // Transfers the data to the PCA board
}


void driveReverse(uint8_t speedFlag)
{
  bufWrite[0] = PCA9685_LED0_ON_L; //Address for motor to operate
  bufWrite[1] = 0; 
  bufWrite[2] = 0;

  // Depending on speedFlag, calls breakup to create two 8-bit numbers for the motor to read as specific speed
  if(speedFlag == 1) {
    breakup(267, &bufWrite[3], &bufWrite[4]);
    }
  else if(speedFlag == 2) {
    breakup(265, &bufWrite[3], &bufWrite[4]);
    }
  else if(speedFlag == 3) {
    breakup(263, &bufWrite[3], &bufWrite[4]);
    }

  metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1); // Transfers the data to the PCA board
}

void raspberrypi_int_handler(int devid, int * angle, int * speed, int * duration)
{
  //char * str = malloc(20 * sizeof(char)); // you can use this to store the received string
                                            // it is the same as char str[20]                

  // Extract the values of angle, speed and duration inside this function
  // And place them into the correct variables that are passed in

  char str[20];
  
  if (ser_isready(1)){
    ser_readline(1, 20, str);
    printf(str);
    sscanf(str, "%d %d %d", &angle, &speed, &duration);}

  free(str);
}

int parseCommand(char *str)
{
    // Run through the string array

    int parsingType = 0;
    char commandType = '\0';
    char commandValueStr[8] = {'\0'};
    int commandValueStrInt = 0;
    int commandValue = 0;
    int i;
    for (i = 0;; i++)
    {
        if (str[i] == '\0')
        {
            break;
        }
        // If we have terminated the command, go ahead and run it
        if (str[i] == ';')
        {
            // parse commandValueStr
            sscanf(commandValueStr, "%d", &commandValue);
            switch (commandType)
            {
            case 'a':
                // angle
                steering(commandValue);
                break;
            case 's':
                // speed
                if (commandValue > 0)
                {
                    driveForward(commandValue);
                }
                else if (commandValue < 0)
                {
                    driveReverse(commandValue);
                }
                else
                {
                    stopMotor();
                }
                break;
            case 'd':
                delay(commandValue * 1000);
                break;
            // default:
                // Don't pollute with errors
                // printf("Failed to parse command! %c\n", commandType);
            }

            parsingType = 0;
            commandType = '\0';
            commandValueStr[0] = '\0';
            commandValueStrInt = 0;
        }
        else if (str[i] == ':')
        {
            parsingType = 1;
        }
        // Getting commandValue
        else if (parsingType == 1)
        {
            commandValueStr[commandValueStrInt] = str[i];
            commandValueStrInt += 1;
        }
        // Getting commandType
        else if (parsingType == 0)
        {
            commandType = str[i];
        }
        else
        {
            printf('We ended up in an invalid state! What happened?');
        }
    }
}


int main()
{
    /*// Initialize I2C
    set_up_I2C();
    delay(2000);

    // Calibrate Motor
    printf("Calibrate Motor.\n");
    stopMotor();
    delay(2000);

    // initialize UART channels
    ser_setup(0); // uart0 (receive from raspberry pi)
    
    printf("Setup completed.\n");
    printf("Begin the main loop.\n");
    

    int angle, speed, duration;
    // Drive loop
    while (1) {
       if (ser_isready(0)) {
           raspberrypi_int_handler(1, &angle, &speed, &duration);
           //printf("Angle: %d, Speed: %d, Duration: %d", angle, speed, duration);
          
           steering(angle);

           if (speed > 0) {
            driveForward(speed);
           }
           else if (speed < 0){
            driveReverse(-speed);
           }
           else {
            stopMotor();
           }

           delay(duration * 1000);
       }
    }
    return 0;*/
    set_up_I2C();

    stopMotor();
    delay(2000);

    char buffer[64] = {'\0'};
    int bufferint = 0;
    // initialize UART channels
    ser_setup(0); // uart0 (debug)
    ser_setup(1); // uart1 (raspberry pi)
    printf("Serial connection completed.\n");
    printf("Begin the main loop.\n");
    while (1)
    {
        if (ser_isready(1))
        {
            buffer[bufferint] = ser_read(1);
            ser_write(0, buffer[bufferint]);
            if (buffer[bufferint] == '\r' || buffer[bufferint] == '\n')
            {
                bufferint = -1;
                buffer[0] = '\0';
            }
            else if (buffer[bufferint] == ';') {
                // If we encounter the end of the command, go ahead 
                // parse the command then
                // reset the bufferint and buffer,
                parseCommand(buffer);
                bufferint = -1;
                buffer[0] = '\0';
            }
            // somehow we ended up using all the buffer before a valid command
            else if (bufferint >= 63) {
                // overflow back to start
                bufferint = -1;
                buffer[0] = '\0';
            }
            bufferint += 1;
        }
    }
    return 0;
}