/* ************************************************************************** */
/** Descriptive File Name

  Author:Amit Herman

  @File Name
 main_clock.h

  @Summary
 Clock functions

  @Description
 hold definitions of functions and other constants  related to the clock of the lcd monitor.
 */
/* ************************************************************************** */

#ifndef _MAIN_CLOCK_H    /* Guard against multiple inclusion */
#define _MAIN_CLOCK_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */
#include "lcd.h"



/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


    /* ************************************************************************** */
    /* ************************************************************************** */
    /* Section: Constants                                                         */
    /* ************************************************************************** */
    /* ************************************************************************** */

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    /* ************************************************************************** */
    /** Descriptive Constant Name

      @Summary
        Brief one-line summary of the constant.
    
      @Description
        Full description, explaining the purpose and usage of the constant.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.
    
      @Remarks
        Any additional remarks
     */
#define EXAMPLE_CONSTANT 0


    // *****************************************************************************
    // *****************************************************************************
    // Section: Data Types
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */


    // *****************************************************************************

    /** Descriptive Data Type Name

      @Summary
        Brief one-line summary of the data type.
    
      @Description
        Full description, explaining the purpose and usage of the data type.
        <p>
        Additional description in consecutive paragraphs separated by HTML 
        paragraph breaks, as necessary.
        <p>
        Type "JavaDoc" in the "How Do I?" IDE toolbar for more information on tags.

      @Remarks
        Any additional remarks
        <p>
        Describe enumeration elements and structure and union members above each 
        element or member.
     */
 


    // *****************************************************************************
    // *****************************************************************************
    // Section: Interface Functions
    // *****************************************************************************
    // *****************************************************************************

    /*  A brief description of a section can be given directly below the section
        banner.
     */

    // *****************************************************************************
    /**
      @Function
       void format_time(long time, int main_clk []) 

      @Summary
        Brief one-line description of the function.

      @Description
        transforms the time in second to HH:MM:SS format and write the proper string that will be displayed
      @Precondition
        List and describe any required preconditions. If there are no preconditions,
        enter "None."

      @Parameters
        @param time time in seconds.
    
        @param main_clk pointer to the lcd clock array

      @Returns
     

      @Remarks
  
      @Example
        
     */
    
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
    
     //Name:Display digital clock
    //Parameters: number array of the main digital clock array
    //Description:Displays the time
 
    
    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
