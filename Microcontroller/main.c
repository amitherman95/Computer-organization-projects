//Description:Main loop
//Coders: Amit Herman, Yakir Sharon

#include "config.h"
#include "config_bits.h"
#include "lcd.h"
#include "led.h"
#include "ssd.h"
#include "utils.h"
#include "btn.h" 
#include <sys/attribs.h>
#include <string.h>
#include <stdlib.h>
#include "swt.h"
#include "audio.h"
#include "mic.h"
#include "adc.h"
     
//==================================================
//              Global Configuration
//==================================================

#define _XTAL_FREQ 8000000
#define PB_FRQ  40000000
#define TMR_FREQ_SINE   48000 // 48 kHz
#define SIN_FREQ_UNIT (int)((float)((float)PB_FRQ/TMR_FREQ_SINE) + 0.5) // Sine Wave Frequency Calculation Unit
#define ONE_SECOND_INTERRUPT 1007790 // 1007800 produces 1 sec delay over 15 minutes, 1007790 is the optimal so far
#define ONE_SECOND_CYCLES 100
#define ONE_uS 10
#define TEN_mS 100
#define TIME_DISPLAY_INITIAL_INDEX 4
#define TIME_DISPLAY_LINE 1
#define SET_WORD_INITIAL_INDEX 4
#define ALARM_WORD_INITIAL_INDEX 4
#define SET_WORD_LINE 0
#define ALARM_WORD_LINE 0
#define DISPLAY_TIME_MODE 0
#define SET_TIME_MODE 1
#define SET_RING_MODE 2
#define STOPWATCH_MODE 3
#define DIGIT_DISPLAY_STRING_LENGTH 8
#define TIMER_OFF 0
#define TIMER_ON 1
#define BTN_PRESSED 1
#define BTN_UNPRESSED 0
#define STOPWATCH_RUN 1
#define STOPWATCH_HOLD 0
#define READY_TO_SET 0
#define SET_HOURS 1
#define SET_MINUTES 2
#define SET_SECONDS 3
#define NO_BLINKING 0
#define BLINKING 1
#define BLINK_POSITIVE 1
#define BLINK_NEGATIVE 0
#define ONE_HOUR 3600
#define ONE_MINUTE 60
#define ONE_SECOND 1
#define DECREASE_TIME 0
#define INCREASE_TIME 1
#define DISPLAY_TIME 0
#define ALARM_TIME 1
#define UNIT_MIN_SIZE 0
#define MAX_HOURS 23
#define MAX_MINUTES 59
#define MAX_SECONDS 59
#define MIDDLE_DECIMAL_POINT 0x04
#define STOPWATCH_INITIAL_VALUE 0x0000
#define ALARM_ENABLE_SWITCH 7
#define LED_ENABLE_SWITCH 6
#define SOUND_FREQ_1 500
#define SOUND_FREQ_2 1500
#define ALARM_MAX_TIME 60
#define ALARM_ON 1
#define ALARM_OFF 0
#define SINE_WAVE_AUDIO_MODE 0
#define COUNTER_DELAY_TIME 15
#define LEDS_RUN 1
#define LEDS_HOLD 0
#define LEDS_RUN_MAX_TIME 60
#define MAX_FREQ_INDEX 5
#define MAX_LED_INDEX 13
#define CLOCK_FLAG_UNINITIALIZED -1
#define ALL_LEDS_OFF 0x00
#define STRINGS_IDENTICAL 0
#define MAX_SOUND_FREQS 210
#define SOUND_FREQ_DIFFERENCE 2
#define FREQ_INDEX_INITIAL_VALUE 0
#define LED_INDEX_INITIAL_VALUE 0


void Timer4Setup()
{
    static int fTimerInitialised = 0;
    if(!fTimerInitialised)
    {        
                                          //  setup peripheral
        PR4 = ONE_SECOND_INTERRUPT;                        //             set period register, generates one interrupt every (unknown) seconds
        TMR4 = 0;                           //             initialize count to 0
        T4CONbits.TCKPS = 2;                //            1:64 prescale value
        T4CONbits.TGATE = 0;                //             not gated input (the default)
        T4CONbits.TCS = 0;                  //             PCBLK input (the default)
        T4CONbits.ON = 1;                   //             turn on Timer1
        IPC4bits.T4IP = 5;                  //             priority
        IPC4bits.T4IS = 0;                  //             subpriority
        IFS0bits.T4IF = 0;                  //             clear interrupt flag
        IEC0bits.T4IE = 1;                  //             enable interrupt
        fTimerInitialised = 1;
    }
}


int stopwatch_time = STOPWATCH_INITIAL_VALUE; // Stopwatch time variable
long Sound_Freq_Index=FREQ_INDEX_INITIAL_VALUE; // Frequency Array Index
int LED_Vals_Index=LED_INDEX_INITIAL_VALUE; // LED Value Array Index
const unsigned char LED_vals[14]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02}; // Constant array of LED VALUES
long Sound_Freqs[MAX_SOUND_FREQS]; // Array of Sound Frequencies for Timer3's Period Register
long cycle_counter = 0; // Counter which helps generating 1 second before writing to the screen if necessary
long current_time = 0; //Time variable, counts how much seconds have passed, min value=0, max value=23h*60m*60s + 59m*60s + 59s  = 86399
long ring_time = 60; //Ring Time variable, holds the numerical value (in seconds) of the time set to ring the alarm clock.
char clock_digits_buffer[] = {'0','0',':','0','0',':','0','0','\0'}; // Copy of clock digits array, for value reading purposes.
char ring_time_buffer[] = {'0','0',':','0','1',':','0','0','\0'}; // Copy of ring digits array, for value reading purposes.
char clock_digits[] = {'0','0',':','0','0',':','0','0','\0'}; // Actual Display String (HH : MM : SS)
char ring_time_digits[] = {'0','0',':','0','0',':','0','0','\0'}; // Auxilliary String which represents the Ring Time value (HH : MM : SS)
unsigned char BTNC_SetTime_counter = READY_TO_SET; // Counts the number of times BTNC has been pressed during the SET TIME Mode
unsigned char BTNC_SetRing_counter = READY_TO_SET; // Counter the number of times BTNC has been pressed during the SET RING Mode
unsigned char blink_flag = BLINK_POSITIVE; // This flag indicates the previous blinking state (negative or positive)
unsigned char Alarm_Ring_Flag = 0; // This flag determines whether the alarm should be ringing or not.
unsigned char Alarm_Ring_Counter = 0; // If the alarm rings, counts up to 60 seconds and stops the alarm. 
unsigned char LED_Run_Flag = 0; // This flag determines whether the LEDS should run or not.
unsigned char LED_Run_Counter = 0; // If the LEDs run, counts up to 60 seconds and holds the LEDs.
unsigned char stopwatch_run_flag = STOPWATCH_HOLD; // This flag indicates the stopwatch state
unsigned char BTNC_Flag=0; // Flag which indicates a continuous button press (BTNC)
unsigned char BTNR_Flag=0; // Flag which indicates a continuous button press (BTNR)
unsigned char BTNL_Flag=0; // Flag which indicates a continuous button press (BTNL)
unsigned char clock_mode=DISPLAY_TIME_MODE; // Global auxilliary mode variable
long BTNC_Delay_Counter=0; // Delay counter of relevant action performance (BTNC)
long BTNR_Delay_Counter=0; // Delay counter of relevant action performance (BTNR)
long BTNL_Delay_Counter=0; // Delay counter of relevant action performance (BTNL)

void Set_Led_Values_Grouped(unsigned char val) // Sets LED0-LED7 according to val, with a BITWISE AND operation for every LED.
{
    LED_SetValue(0, val & 0x01);
    LED_SetValue(1, val & 0x02);
    LED_SetValue(2, val & 0x04);
    LED_SetValue(3, val & 0x08);
    LED_SetValue(4, val & 0x10);
    LED_SetValue(5, val & 0x20);
    LED_SetValue(6, val & 0x40);
    LED_SetValue(7, val & 0x80);
}

void BTNC_SetTime_Counter_Increase () // This function cyclically increases the value of BTNC SetTime counter
{                                     // According to this counter, the Set Time option is set (HH, MM, SS)
    if (BTNC_SetTime_counter==SET_SECONDS)  
        BTNC_SetTime_counter=READY_TO_SET;
    else
        BTNC_SetTime_counter++;       
}

void BTNC_SetRing_Counter_Increase () // This function cyclically increases the value of BTNC SetRing counter
{                                     // According to this counter, the Set Ring option is set (HH, MM, SS)
    if (BTNC_SetRing_counter==SET_SECONDS)  
        BTNC_SetRing_counter=READY_TO_SET;
    else
        BTNC_SetRing_counter++;       
}

 void format_time(long time, char clk_digits[], char clk_digits_buffer[]) // This function receives time value, and two strings pointers
{                                                                        // It converts the current time to digits appropriately and places them in the strings
        int h,m,s;
        
        //calculate hours
        h = time/3600;
        //calculate minutes
        m = time%3600 / 60;
         //calculate seconds
        s = time%60;
        
        //assign the calculated values into the digital clock string
        clk_digits[0] = h/10 + 48; //hours
        clk_digits[1] = h%10 + 48;
        
        clk_digits[3] = m/10 + 48; //mins
        clk_digits[4] = m%10 + 48;
        
        clk_digits[6] = s/10 + 48; //seconds
        clk_digits[7] = s%10 + 48;
        
        clk_digits_buffer[0] = h/10 + 48; //hours
        clk_digits_buffer[1] = h%10 + 48;
        
        clk_digits_buffer[3] = m/10 + 48; //mins
        clk_digits_buffer[4] = m%10 + 48;
        
        clk_digits_buffer[6] = s/10 + 48; //seconds
        clk_digits_buffer[7] = s%10 + 48;
}
    

void advance_current_time_counter() // This function cyclically advances the current time counter global variable.
{
    current_time++;
      if (current_time == 86400)
         current_time = 0;
}

void blink_digits(unsigned char i, unsigned char j, unsigned char mode) // This function blinks the currently set digits according to a flag variable
{
   
    if (blink_flag == BLINK_NEGATIVE) // Blink "Negative"
    {
        if (mode==SET_TIME_MODE) // Erase the desired digits accoring to the input mode
        {
         clock_digits[i]=' ';
         clock_digits[j]=' ';
        }
        
        else
        {
            ring_time_digits[i]=' ';
            ring_time_digits[j]=' ';
        }
        
        blink_flag=BLINK_POSITIVE; // After blinking, change the flag state.
    }
    
    else    // Blink "Positive"
    {
        if (mode==SET_TIME_MODE) // If in SET TIME mode, blink "positive" with the current time setting.
            format_time(current_time,clock_digits, clock_digits_buffer); 
        
        else if (mode==SET_RING_MODE) // If in SET RING mode, blink "positive" with the current time setting.
            format_time(ring_time,ring_time_digits, ring_time_buffer);           
            
        blink_flag=BLINK_NEGATIVE; // After blinking, change the flag state.
    }
}

unsigned char convert_unit_digits_to_num(unsigned char a, unsigned char b, unsigned char i, unsigned char j, char dest[], char source[])
{           // This function converts two desired unit digits to an integer.
    unsigned char unit;
    
    dest[a]=source[i];
    dest[b]=source[j];
    unit=atoi(dest);
    
    return unit;
}

void increase_hours(unsigned char time_type) // This procedure increases hours of the given time mode (DISPLAY/ALARM)
{
   unsigned char hours;
   char tmp_digits_str[2];
   
   switch (time_type)
   {
       case ALARM_TIME :
       {
         hours=convert_unit_digits_to_num(0, 1, 0, 1, tmp_digits_str, ring_time_buffer); // Convert ring time hour digits to a number
     
         if (hours==MAX_HOURS)                    // Decrease ALARM hours cyclically            
           ring_time=ring_time-(23*ONE_HOUR); 
         else
           ring_time=ring_time+ONE_HOUR;
          
         format_time(ring_time, ring_time_digits, ring_time_buffer); // Convert time to digits, and write it to the LCD.
         LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
         break;
       }
       
       case DISPLAY_TIME :
       {
         hours=convert_unit_digits_to_num(0, 1, 0, 1, tmp_digits_str, clock_digits_buffer); // Convert ring time hour digits to a number
     
         if (hours==MAX_HOURS)                         // Decrease DISPLAY hours cyclically       
           current_time=current_time-(23*ONE_HOUR);
         else
           current_time=current_time+ONE_HOUR;
          
         format_time(current_time, clock_digits, clock_digits_buffer); // Convert result to string and write it to LCD
         LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
          
         break;
       }
       
       default :
       {
          break; 
       }
   }
     
   
}

void decrease_hours(int time_type) // This procedure decreases hours of the given time mode (DISPLAY/ALARM)
{
   unsigned char hours;  
   char tmp_digits_str[2];
   
   switch (time_type)
   {
       case ALARM_TIME :
       {
         hours=convert_unit_digits_to_num(0, 1, 0, 1, tmp_digits_str, ring_time_buffer); // Convert ring time hour digits to a number
                             
              if (hours==UNIT_MIN_SIZE)                            // Cyclically decrease hours 
                 ring_time=ring_time+(23*ONE_HOUR);
              else
                 ring_time=ring_time-ONE_HOUR;
                  
              format_time(ring_time, ring_time_digits, ring_time_buffer);
              LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
         break;
       }
       
       case DISPLAY_TIME :
       {
          hours=convert_unit_digits_to_num(0, 1, 0, 1, tmp_digits_str, clock_digits_buffer); // Convert current time hour digits to a number
                             
                if (hours==UNIT_MIN_SIZE)                               
                    current_time=current_time+(23*ONE_HOUR);
                else
                    current_time=current_time-ONE_HOUR;
                    
                format_time(current_time, clock_digits, clock_digits_buffer);
                LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);           
          
         break;
       }
       
       default :
       {
          break; 
       }
   }
   
}

void decrease_minutes(int time_type) // This procedure decreases minutes of the given time mode (DISPLAY/ALARM)
{
   unsigned char minutes;
   char tmp_digits_str[2];
   
   switch (time_type)
   {
       case ALARM_TIME :
       {
                   minutes=convert_unit_digits_to_num(0, 1, 3, 4, tmp_digits_str, ring_time_buffer); // Convert ring time hour digits to a number
                             
                             if (minutes==UNIT_MIN_SIZE) // Cyclically decrease minutes
                                 ring_time=ring_time+(59*ONE_MINUTE);
                             else
                                 ring_time=ring_time-ONE_MINUTE;
                  
                  format_time(ring_time, ring_time_digits, ring_time_buffer);
                  LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       case DISPLAY_TIME :
       {
                  minutes=convert_unit_digits_to_num(0, 1, 3, 4, tmp_digits_str, clock_digits_buffer); // Convert current time hour digits to a number
                             
                             if (minutes==UNIT_MIN_SIZE)
                                 current_time=current_time+(59*ONE_MINUTE);                           
                             else
                                 current_time=current_time-ONE_MINUTE;
                    
                  format_time(current_time, clock_digits, clock_digits_buffer);
                  LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       default :
       {
           break;
       }
   
   }
   
}

void increase_minutes(int time_type) // This procedure increases minutes of the given time mode (DISPLAY/ALARM)
{
   unsigned char minutes;
   char tmp_digits_str[2];
   
   switch (time_type)
   {
       case ALARM_TIME :
       {
                  minutes=convert_unit_digits_to_num(0, 1, 3, 4, tmp_digits_str, ring_time_buffer); // Convert ring time hour digits to a number
                             
                             if (minutes==MAX_MINUTES)         // Cyclically increase minutes                        
                                 ring_time=ring_time-(59*ONE_MINUTE);
                             else
                                 ring_time=ring_time+ONE_MINUTE;
                  
                    format_time(ring_time, ring_time_digits, ring_time_buffer);
                    LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX); 
           break;
       }
       
       case DISPLAY_TIME :
       {
                     minutes=convert_unit_digits_to_num(0, 1, 3, 4, tmp_digits_str, clock_digits_buffer); // Convert current time hour digits to a number
                             
                             if (minutes==MAX_MINUTES)
                                 current_time=current_time-(59*ONE_MINUTE);
                             else
                                 current_time=current_time+ONE_MINUTE;
                    
                    format_time(current_time, clock_digits, clock_digits_buffer);
                    LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       default :
       {
           break;
       }
   
   }
}

void decrease_seconds(int time_type) // This procedure decreases seconds of the given time mode (DISPLAY/ALARM)
{
    unsigned char seconds; 
    char tmp_digits_str[2];
    
    switch (time_type)
   {
       case ALARM_TIME :
       {
                   seconds=convert_unit_digits_to_num(0, 1, 6, 7, tmp_digits_str, ring_time_buffer); // Convert ring time hour digits to a number
                             
                             if (seconds==UNIT_MIN_SIZE) // Cyclically decrease seconds
                                 ring_time=ring_time+(59*ONE_SECOND);                            
                             else
                                 ring_time=ring_time-ONE_SECOND;
                  
                  format_time(ring_time, ring_time_digits, ring_time_buffer);
                  LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       case DISPLAY_TIME :
       {
                  seconds=convert_unit_digits_to_num(0, 1, 6, 7, tmp_digits_str, clock_digits_buffer); // Convert current time hour digits to a number
                             
                             if (seconds==UNIT_MIN_SIZE) // Cyclically decrease seconds
                                 current_time=current_time+(59*ONE_SECOND);                           
                             else
                                 current_time=current_time-ONE_SECOND;
                    
                  format_time(current_time, clock_digits, clock_digits_buffer);
                  LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       default :
       {
           break;
       }
   
   }
}

void increase_seconds(int time_type) // This procedure increases seconds of the given time mode (DISPLAY/ALARM)
{
    unsigned char seconds;
    char tmp_digits_str[2];
    
    switch (time_type)
   {
       case ALARM_TIME :
       {
                   seconds=convert_unit_digits_to_num(0, 1, 6, 7, tmp_digits_str, ring_time_buffer); // Convert ring time hour digits to a number
                             
                             if (seconds==MAX_SECONDS)                // Cyclically increase seconds             
                                 ring_time=ring_time-(59*ONE_SECOND);                          
                             else
                                 ring_time=ring_time+ONE_SECOND;

                  format_time(ring_time, ring_time_digits, ring_time_buffer);
                  LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       case DISPLAY_TIME :
       {
                   seconds=convert_unit_digits_to_num(0, 1, 6, 7, tmp_digits_str, clock_digits_buffer); // Convert current time hour digits to a number
                             
                             if (seconds==MAX_SECONDS) // Cyclically inrease seconds
                                 current_time=current_time-(59*ONE_SECOND);                            
                             else
                                 current_time=current_time+ONE_SECOND;
                    
                  format_time(current_time, clock_digits, clock_digits_buffer);
                  LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
           break;
       }
       
       default :
       {
           break;
       }
   
   }
}


void digits_increase_decrease(int time_type, int operation, unsigned char mode) // This function is responsible for deciding whether to decrease or increase time units
{
    unsigned char BTNC_Counter;

    
    if (mode==SET_TIME_MODE) // Sample the correct BTNC Counter's value, according to mode.
        BTNC_Counter=BTNC_SetTime_counter;
    else if (mode==SET_RING_MODE)
        BTNC_Counter=BTNC_SetRing_counter;
    
    switch (BTNC_Counter) // DECREASE OR INCREASE TIME UNITS ACCORDING TO time type AND operation
    {
        
        case SET_HOURS :
        {
            if (operation==INCREASE_TIME)
                increase_hours(time_type);          
            else if (operation==DECREASE_TIME)
                decrease_hours(time_type);
            
          break;   
        }
        
        case SET_MINUTES :
        {
            if (operation==INCREASE_TIME)
                increase_minutes(time_type);           
            else if (operation==DECREASE_TIME)
                decrease_minutes(time_type);
            
          break;
        }
        
        case SET_SECONDS :
        {
            if (operation==INCREASE_TIME)
                increase_seconds(time_type);
            
            else if (operation==DECREASE_TIME)
                decrease_seconds(time_type);
                
            break;
        }
        
        default :
        {
            break;
        }
    }
}

void Stopwatch_Run_Hold () // This function switches between holding the Stopwatch and running it.
{
     if (stopwatch_run_flag==STOPWATCH_RUN)
        stopwatch_run_flag=STOPWATCH_HOLD;
     else
        stopwatch_run_flag=STOPWATCH_RUN;    
}

void stop_and_reset_stopwatch () // This procedure stops and resets the stopwatch.
{
    stopwatch_run_flag=STOPWATCH_HOLD;
    stopwatch_time=STOPWATCH_INITIAL_VALUE;
    SSD_WriteDigitsGrouped(STOPWATCH_INITIAL_VALUE, MIDDLE_DECIMAL_POINT);   
}
void Alarm_Reset_and_Close () // This procedure resets the Alarm enabling settings and closes the Audio unit.
{
    Alarm_Ring_Counter=0;
    Sound_Freq_Index=0;
    Alarm_Ring_Flag=ALARM_OFF;
    AUDIO_Close();
}

void LEDS_Reset_and_Close () // This procedure turns off all LEDs, resets counters and indexes.
{
    Set_Led_Values_Grouped(ALL_LEDS_OFF);
    LED_Vals_Index=0;
    LED_Run_Flag=LEDS_HOLD;
    LED_Run_Counter=0;
}
void advance_stopwatch_time () // This procedure advances the current Stopwatch time.
{
    int first_digit, second_digit, third_digit, fourth_digit;

    
    first_digit=((stopwatch_time & 0x000F));  // Acquires the state of all digits, from right to left
    second_digit=((stopwatch_time & 0x00F0) >> 4);
    third_digit=((stopwatch_time & 0x0F00) >> 8);
    fourth_digit=((stopwatch_time & 0xF000) >> 12);
    
    
    
    if (stopwatch_time==0x9999) // If the Stopwatch has reached the maximal time possible (99:99), reset it.
        stopwatch_time=0x0000;
    
     
    else if (first_digit==0x09 && second_digit==0x09 && third_digit==0x09) // Special time addition for X9:99 seconds
        stopwatch_time=stopwatch_time+0x0667;
    
    else if (first_digit==0x09 && second_digit==0x09) // Special time addition for 99/100 of a second
        stopwatch_time=stopwatch_time+0x0067;
    
    else if (first_digit==0x09)                     // Special time addition for 9/100 of a second
        stopwatch_time=stopwatch_time+0x0007;

    else
        stopwatch_time++;
        
    SSD_WriteDigitsGrouped(stopwatch_time, MIDDLE_DECIMAL_POINT); // Write Stopwatch time to the SSD, with a Decimal Point in the middle.
}

void DISPLAY_TIME_INIT() // Initialization procedure for DISPLAY TIME MODE.
{
   BTNC_SetTime_counter=READY_TO_SET; // Reset Set Time counter
   BTNC_SetRing_counter=READY_TO_SET; // Reset Set Ring counter
   LCD_WriteStringAtPos("     ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Delete "ALARM" or "SET" that might have been written previously 
   SSD_WriteDigitsGrouped(STOPWATCH_INITIAL_VALUE, MIDDLE_DECIMAL_POINT);
   format_time(current_time, clock_digits, clock_digits_buffer); // Translate the current time to digits
   LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX); // Write the current time on line 0, and begin from index=17.
   clock_mode=DISPLAY_TIME_MODE;
}

void SET_TIME_INIT () // Initialization procedure for SET TIME mode.
{
    BTNC_SetRing_counter=READY_TO_SET; // Reset the BTNC Set Ring counter
    stopwatch_time=STOPWATCH_INITIAL_VALUE;
    SSD_WriteDigitsGrouped(STOPWATCH_INITIAL_VALUE, MIDDLE_DECIMAL_POINT); // Initialize Stopwatch
    stopwatch_run_flag=STOPWATCH_HOLD;
    LCD_WriteStringAtPos("SET", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Add the word "SET" to the current time, while NOT blinking.
    format_time(current_time, clock_digits, clock_digits_buffer); // No blinking  
    LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
    clock_mode=SET_TIME_MODE;
}

void SET_RING_INIT () // Initialization procedure for SET RING mode.
{
   BTNC_SetTime_counter=READY_TO_SET; // Reset the Set Time BTNC Counter
   stopwatch_time=STOPWATCH_INITIAL_VALUE;
   SSD_WriteDigitsGrouped(STOPWATCH_INITIAL_VALUE, MIDDLE_DECIMAL_POINT); // Initialize Stopwatch
   stopwatch_run_flag=STOPWATCH_HOLD; 
   LCD_WriteStringAtPos("ALARM", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Add the word "ALARM" to the current time, while NOT blinking.
   format_time(ring_time, ring_time_digits, ring_time_buffer); // No blinking 
   LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
   clock_mode=SET_RING_MODE;
}

void INIT_ALARM_IN_EFFECT () // This procedure puts the Alarm in effect.
{
    Alarm_Ring_Flag=ALARM_ON;
    LED_Run_Flag=LEDS_RUN;
    LED_Run_Counter=0;
    Alarm_Ring_Counter=0;
    LED_Vals_Index=LED_INDEX_INITIAL_VALUE;
    Sound_Freq_Index=FREQ_INDEX_INITIAL_VALUE;
}


// *********** TIMER 4 INTERRUPT HANDLER **********
// THIS INTERRUPT HANDLER IS WHERE THE MAIN OPERATION OF THIS PROGRAM IS PERFORMED.
// IT IS DIVIDED INTO 2 SECTIONS : POLLING SECTION (OCCURS ON EVERY INTERRUPT), AND UPDATE SECTION (OCCRUS EVERY 1 SECOND)
// BY USING A GLOBAL CYCLE COUNTER, THE POLLING SECTION AND THE UPDATE SECTION ARE SEPARATED. 
void __ISR(_TIMER_4_VECTOR, ipl5auto) Timer4SR(void) // Timer 4 Interrupt Handler
{
   unsigned char mode;
   
   IFS0bits.T4IF = 0;                  // Clear interrupt flag
   
   mode = ((SWT_GetGroupValue() & 0x03)); // Get the SWT states, to determine work mode.
    
    if (strcmp(ring_time_buffer, clock_digits_buffer)==STRINGS_IDENTICAL) // Compares both strings of Alarm Time and Current Time
        INIT_ALARM_IN_EFFECT();
   
    if (SWT_GetValue(ALARM_ENABLE_SWITCH) && mode == DISPLAY_TIME_MODE) // Checks if the Alarm Switch is enabled and the current mode is DISPLAY TIME MODE
           {                                                               // Start ringing for 60 seconds if so, and shifts between frequencies every interrupt.
             if (Alarm_Ring_Counter!=ALARM_MAX_TIME && Alarm_Ring_Flag==ALARM_ON)
               {   
                      if (cycle_counter%SOUND_FREQ_DIFFERENCE==0)  
                         {
                             AUDIO_Init(SINE_WAVE_AUDIO_MODE); // Send ring info and initializations
                             PR3=20000000/Sound_Freqs[Sound_Freq_Index]; // Assign the desired frequency to Timer3's Period Register.
                             OC1R = PR3 / 2;
                             OC1RS = PR3 / 2;
                            if (Sound_Freq_Index==MAX_SOUND_FREQS-1) // Cyclically advance the Sound Freq Array Index
                                Sound_Freq_Index=FREQ_INDEX_INITIAL_VALUE;
                            else
                                Sound_Freq_Index++;
                         }
                }
             else
                 Alarm_Reset_and_Close(); // If 60 seconds have passed, or the switch is off, or the mode has changed - stop ringing and reset settings.
            }
      
       else
           Alarm_Reset_and_Close(); // If 60 seconds have passed, or the switch is off, or the mode has changed - stop ringing and reset settings.

   
 if (cycle_counter < ONE_SECOND_CYCLES)   // *************** POLLING SECTION (OCCURS ON EVERY INTERRUPT) *****************
 {
     cycle_counter++; // Continue counting (we have not reached 1 second yet)
     
     switch (mode)            
     { 
         case DISPLAY_TIME_MODE :         
          {
              if (clock_mode!=DISPLAY_TIME_MODE)
                  DISPLAY_TIME_INIT(); // Initialize settings for DISPLAY TIME MODE
            
             
                   if (BTN_GetValue('R')==BTN_PRESSED && BTNR_Flag==BTN_UNPRESSED) // RUN or HOLD the Stopwatch
                      {
                      Stopwatch_Run_Hold(); 
                      BTNR_Flag=BTN_PRESSED; // Activate flag
                      }
             
                   else if (BTN_GetValue('R')==BTN_UNPRESSED)
                      BTNR_Flag=BTN_UNPRESSED; // Release flag
           
                    
                   if (BTN_GetValue('L')==BTN_PRESSED && BTNL_Flag==BTN_UNPRESSED) // STOP and RESET the Stopwatch
                     {
                     stop_and_reset_stopwatch();
                     BTNL_Flag=BTN_PRESSED; // Activate flag
                     }
           
                   else if (BTN_GetValue('L')==BTN_UNPRESSED)
                       BTNL_Flag=BTN_UNPRESSED; // Release flag
           
           if (stopwatch_run_flag==STOPWATCH_RUN)
               advance_stopwatch_time(); // Advance the stopwatch time over the SSD.

             break;
         }
         
         case SET_TIME_MODE :
         {
             if (clock_mode!=SET_TIME_MODE)
                 SET_TIME_INIT();
             
                if (BTN_GetValue('C')==BTN_PRESSED && BTNC_Flag==BTN_UNPRESSED) // If BTNC was pressed with released flag...
                 {
                     BTNC_SetTime_Counter_Increase(); // Increase counter, in order to move to next setting.
                     BTNC_Flag=BTN_PRESSED; // Turn on BTNC Flag for continuous press
                     
                     switch (BTNC_SetTime_counter)
                     {
                         case READY_TO_SET :
                         {
                             LCD_WriteStringAtPos("SET", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Add the word "SET" to the current time, while NOT blinking.
                             format_time(current_time, clock_digits, clock_digits_buffer); // No blinking  
                             LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                             break;
                         }
                         case SET_HOURS :
                         {
                             blink_flag=BLINK_NEGATIVE;
                             format_time(current_time, clock_digits, clock_digits_buffer);
                             blink_digits(0,1, SET_TIME_MODE); // Blink HH (Hours)
                             LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                             LCD_WriteStringAtPos("    ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                             break;
                         }
                         
                         case SET_MINUTES :
                         {
                            blink_flag=BLINK_NEGATIVE;
                            format_time(current_time, clock_digits, clock_digits_buffer);
                            blink_digits(3,4, SET_TIME_MODE); // Blink MM (Minutes)
                            LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                            LCD_WriteStringAtPos("    ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                             break;
                         }
                         
                         case SET_SECONDS :
                         {
                             blink_flag=BLINK_NEGATIVE;
                             format_time(current_time, clock_digits, clock_digits_buffer);
                             blink_digits(6,7, SET_TIME_MODE); // Blink SS (Seconds)
                             LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                             LCD_WriteStringAtPos("    ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                             break;
                         }
                         
                         default :
                         {
                             break;
                         }
                     }
                     
                 }
                        
                else if (BTN_GetValue('C')==BTN_UNPRESSED) // if BTNC was unpressed.
                    BTNC_Flag=BTN_UNPRESSED; // Release flag
                
                
                
               if (BTN_GetValue('L')==BTN_PRESSED && BTNL_Flag==BTN_UNPRESSED) // If BTNL was pressed with flag released
                 {
                  digits_increase_decrease(DISPLAY_TIME, DECREASE_TIME, mode); // Decrease ALARM time according to the BTNC counter value
                  BTNL_Flag=BTN_PRESSED;
                  BTNL_Delay_Counter=0;
                 }
             
               else if (BTN_GetValue('L')==BTN_PRESSED && BTNL_Flag==BTN_PRESSED) // If BTNL was pressed with flag activated
                 {
                     if (BTNL_Delay_Counter==COUNTER_DELAY_TIME) // Decrease digits if counter has reached maximum count
                     {
                         digits_increase_decrease(DISPLAY_TIME, DECREASE_TIME, mode);
                         BTNL_Delay_Counter=0;
                     }
                     
                     else
                         BTNL_Delay_Counter++;
                    
                 }
             
               else if (BTN_GetValue('L')==BTN_UNPRESSED) // If BTNL was unpressed
                 {
                     BTNL_Flag=BTN_UNPRESSED;
                     BTNL_Delay_Counter=0;
                 }
                
                 if (BTN_GetValue('R')==BTN_PRESSED && BTNR_Flag==BTN_UNPRESSED) // If BTNR was pressed was flag released
                   {
                         digits_increase_decrease(DISPLAY_TIME, INCREASE_TIME, mode);// Increase ALARM time according to the BTNC counter value
                         BTNR_Flag=BTN_PRESSED;
                         BTNR_Delay_Counter=0;
                   }
             
                 else if  (BTN_GetValue('R')==BTN_PRESSED && BTNR_Flag==BTN_PRESSED) // If BTNR was pressed with flag activated
                 {
                     if (BTNR_Delay_Counter==COUNTER_DELAY_TIME) // Increase digits if counter has reached maximum count
                     {
                         digits_increase_decrease(DISPLAY_TIME, INCREASE_TIME, mode);
                         BTNR_Delay_Counter=0;
                     }
                     
                     else
                         BTNR_Delay_Counter++;
                 }
                 
                 else if (BTN_GetValue('R')==BTN_UNPRESSED) // If BTNR was unpressed, release the flag and reset delay counter
                 {
                     BTNR_Flag=BTN_UNPRESSED;
                     BTNR_Delay_Counter=0;
                 }
 
             
             break;
          }
         
         case SET_RING_MODE :
         {
             if (clock_mode!=SET_RING_MODE)
                 SET_RING_INIT();
             
                if (BTN_GetValue('C')==BTN_PRESSED && BTNC_Flag==BTN_UNPRESSED) // If BTNC was pressed
                 {
                     BTNC_SetRing_Counter_Increase(); // Increase counter, in order to move to next setting.
                     BTNC_Flag=BTN_PRESSED; // Turn on BTNC Flag for continuous press
                     
                     switch (BTNC_SetRing_counter)
                     {
                         case READY_TO_SET :
                         {
                            LCD_WriteStringAtPos("ALARM", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Add the word "ALARM" to the current time, while NOT blinking.
                            format_time(ring_time, ring_time_digits, ring_time_buffer); // No blinking 
                            LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                            break;
                         }
                         case SET_HOURS :
                         {
                             blink_flag=BLINK_NEGATIVE;
                             format_time(ring_time, ring_time_digits, ring_time_buffer);
                             blink_digits(0,1, SET_RING_MODE); // Blink HH (Hours)
                             LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                             LCD_WriteStringAtPos("     ", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                             break;
                         }
                         
                         case SET_MINUTES :
                         {
                            blink_flag=BLINK_NEGATIVE;
                            format_time(ring_time, ring_time_digits, ring_time_buffer);
                            blink_digits(3,4, SET_RING_MODE); // Blink HH (Hours)
                            LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                            LCD_WriteStringAtPos("     ", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                             break;
                         }
                         
                         case SET_SECONDS :
                         {
                             blink_flag=BLINK_NEGATIVE;
                             format_time(ring_time, ring_time_digits, ring_time_buffer);
                             blink_digits(6,7, SET_RING_MODE); // Blink HH (Hours)
                             LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
                             LCD_WriteStringAtPos("     ", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                             break;
                         }
                         
                         default :
                         {
                             break;
                         }
                     }
                 }
                        
                 else if (BTN_GetValue('C')==BTN_UNPRESSED)
                    BTNC_Flag=BTN_UNPRESSED;
             
             
                 if (BTN_GetValue('L')==BTN_PRESSED && BTNL_Flag==BTN_UNPRESSED) // If BTNL was pressed with flag released
                 {
                  digits_increase_decrease(ALARM_TIME, DECREASE_TIME, mode); // Decrease ALARM time according to the BTNC counter value
                  BTNL_Flag=BTN_PRESSED;
                  BTNL_Delay_Counter=0;
                 }
             
                 else if (BTN_GetValue('L')==BTN_PRESSED && BTNL_Flag==BTN_PRESSED) // if BTNL was pressed with flag activated
                 {
                     if (BTNL_Delay_Counter==COUNTER_DELAY_TIME)
                     {
                         digits_increase_decrease(ALARM_TIME, DECREASE_TIME, mode); // Decrease digits and reset delay counter
                         BTNL_Delay_Counter=0;
                     }
                     
                     else
                         BTNL_Delay_Counter++;
                    
                 }
             
                 else if (BTN_GetValue('L')==BTN_UNPRESSED)
                 {
                     BTNL_Flag=BTN_UNPRESSED;
                     BTNL_Delay_Counter=0;
                 }
                 
                
                 
                
                 if (BTN_GetValue('R')==BTN_PRESSED && BTNR_Flag==BTN_UNPRESSED) // If BTNR was pressed with flag released...
                   {
                         digits_increase_decrease(ALARM_TIME, INCREASE_TIME, mode);// Increase ALARM time according to the BTNC counter value
                         BTNR_Flag=BTN_PRESSED;
                         BTNR_Delay_Counter=0;
                   }
             
                 else if  (BTN_GetValue('R')==BTN_PRESSED && BTNR_Flag==BTN_PRESSED) // if BTNR was pressed with flag activated...
                 {
                     if (BTNR_Delay_Counter==COUNTER_DELAY_TIME)
                     {
                         digits_increase_decrease(ALARM_TIME, INCREASE_TIME, mode); // Increase digits and reset delay counter
                         BTNR_Delay_Counter=0;
                     }
                     
                     else
                         BTNR_Delay_Counter++;
                 }
                 
                 else if (BTN_GetValue('R')==BTN_UNPRESSED) // Release flag and reset delay counter
                 {
                     BTNR_Flag=BTN_UNPRESSED;
                     BTNR_Delay_Counter=0;
                 }
             
             break;
         }
         
         default :
         {
             break;
         }
   }
 }   
   
   else // ********************** UPDATE SECTION (AFTER 1 SECOND COUNT) *******************************
   {
       cycle_counter = 0; // If counted 100 cycles, reset cycle counter (this means we have reached 1 second)                                                    // Activates the Alarm flag and LEDS flag
  
       if (SWT_GetValue(LED_ENABLE_SWITCH) && mode == DISPLAY_TIME_MODE) // If SW6 is enabled, and the mode is DISPLAY TIME
       {
           if (LED_Run_Counter!=LEDS_RUN_MAX_TIME && LED_Run_Flag==LEDS_RUN) // Activate and RUN LEDs
           {
               Set_Led_Values_Grouped(LED_vals[LED_Vals_Index]); // Set LEDs value according to the constant LED_vals array.
               
               if (LED_Vals_Index==MAX_LED_INDEX)
                   LED_Vals_Index=LED_INDEX_INITIAL_VALUE;
               else
                   LED_Vals_Index++;
               
               LED_Run_Counter++;
           }
           
           else
            LEDS_Reset_and_Close();
           
       }
       
       else
        LEDS_Reset_and_Close();
       
       if (SWT_GetValue(ALARM_ENABLE_SWITCH) && mode == DISPLAY_TIME_MODE) // Checks if the Alarm Switch is enabled and the current mode is DISPLAY TIME MODE
           {                                                               // Start ringing for 60 seconds if so
             if (Alarm_Ring_Counter!=ALARM_MAX_TIME && Alarm_Ring_Flag==ALARM_ON)
                  Alarm_Ring_Counter++;
             else
                 Alarm_Reset_and_Close(); // If 60 seconds have passed, or the switch is off, or the mode has changed - stop ringing and reset settings.
           }
      
       else
           Alarm_Reset_and_Close(); // If 60 seconds have passed, or the switch is off, or the mode has changed - stop ringing and reset settings.

       switch (mode)
       {
           case DISPLAY_TIME_MODE :
           {
            advance_current_time_counter(); // Increment the time counter by 1
            format_time(current_time, clock_digits, clock_digits_buffer); // Translate the current time to digits
            LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX); // Write the current time on line 0, and begin from index=17.
            break;
           }
           
           case SET_TIME_MODE :
           {             
                if (BTNC_SetTime_counter == READY_TO_SET)
                 {                   
                    LCD_WriteStringAtPos("SET", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Add the word "SET" to the current time, while NOT blinking.
                    format_time(current_time, clock_digits, clock_digits_buffer); // No blinking                 
                 }

                if (BTNC_SetTime_counter == SET_HOURS) 
                {
                    blink_digits(0,1, SET_TIME_MODE); // Blink HH (Hours)
                    LCD_WriteStringAtPos("    ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces                 
                }

                if (BTNC_SetTime_counter == SET_MINUTES)
                {
                   blink_digits(3,4, SET_TIME_MODE); // Blink MM (Minutes)
                   LCD_WriteStringAtPos("    ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces                
                }

                if (BTNC_SetTime_counter == SET_SECONDS)
                {
                    LCD_WriteStringAtPos("    ", SET_WORD_LINE, SET_WORD_INITIAL_INDEX); // Erase the word "SET" and replace it with 3 whitespaces
                    blink_digits(6,7, SET_TIME_MODE);// Blink SS (Seconds)
                }
               LCD_WriteStringAtPos(clock_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
               break;
           }
           
           case SET_RING_MODE :
           {
               advance_current_time_counter();
            
            if (BTNC_SetRing_counter == READY_TO_SET)
            
                {
                    LCD_WriteStringAtPos("ALARM", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Add the word "ALARM" to the current ring time, while NOT blinking.
                    format_time(ring_time, ring_time_digits, ring_time_buffer); // No blinking
                      
                }

             if (BTNC_SetRing_counter == SET_HOURS) 
                {
                    
                    LCD_WriteStringAtPos("     ", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Erase the word "ALARM" and replace it with 5 whitespaces
                    blink_digits(0,1, SET_RING_MODE); // Blink HH (Hours)
                    
                }

               if (BTNC_SetRing_counter == SET_MINUTES)
                {
                    LCD_WriteStringAtPos("     ", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Erase the word "ALARM" and replace it with 5 whitespaces
                    blink_digits(3,4, SET_RING_MODE); // Blink MM (Minutes)
                    
                }

                if (BTNC_SetRing_counter == SET_SECONDS)
                {
                    LCD_WriteStringAtPos("     ", ALARM_WORD_LINE, ALARM_WORD_INITIAL_INDEX); // Erase the word "ALARM" and replace it with 5 whitespaces
                    blink_digits(6,7, SET_RING_MODE);// Blink SS (Seconds)
                    
                }
               
               LCD_WriteStringAtPos(ring_time_digits, TIME_DISPLAY_LINE, TIME_DISPLAY_INITIAL_INDEX);
               break;
           }
       }
   }
 
}

void Sound_Freq_Init () // This procedure initializes the FREQUENCY ARRAY
{                       // Creates a mirror of frequencies between the first half and second half of the array
                        // This results in an alternating increase and decrease in frequencies ("Siren")
    int i=0;
    int j=0;
    
    while (j<= (MAX_SOUND_FREQS/2)) // Initialize first half of the FREQUENCY ARRAY
    {
        Sound_Freqs[j]=(long)(i+600);
        i=i+40;
        j++;
    }
    
    j=0;
    
    for (i=MAX_SOUND_FREQS/2; i<=MAX_SOUND_FREQS-1; i++) // Initialize the second half of the FREQUENCY ARRAY with the first's mirror
    {
        Sound_Freqs[i]=Sound_Freqs[(MAX_SOUND_FREQS/2)-j];
        j++;
    }
}
//==============================================================================
//This procedure initializes general settings and every required component in the PCB
//==============================================================================
void init()
{
    LCD_Init(); // Initialize LCD settings
    LED_Init(); // Initialize LEDs settings
    BTN_Init(); // Initialize BTN settings
    SSD_Init(); // Initialize Seven Segment Display
    SSD_WriteDigitsGrouped(STOPWATCH_INITIAL_VALUE, MIDDLE_DECIMAL_POINT); // Initialize Stopwatch
    Sound_Freq_Init(); // Initialize the Sound Frequencies Array
    Timer4Setup(); // Initialize Timer 4 settings
    SWT_Init(); // Initialize SWITCHES settings
}



void main()
{
    init(); // Initialize all required components and perform general initialization procedures
    while(1); // Loop forever (or until shutdown) and wait for TIMER 4 Interrupt Handler to take over
}






