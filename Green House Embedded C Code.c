/*Step 1: Open Keil µVision

Launch the Keil µVision software on your computer.
Step 2: Create a New Project

Go to "Project" in the menu bar and select "New µVision Project..."
Choose a location to save your project and give it a name.
Click "Save".
Step 3: Select Target Device

In the "Select Device for Target" window, choose your target device (8051 microcontroller).
Click "OK".
Step 4: Add Source Files

Right-click on "Source Group 1" in the "Project" pane.
Select "Add Existing Files to Group 'Source Group 1'..."
Browse and select the source file containing the provided code.
Click "Add".

Create a C Source File with any name like, GHC.c
Paste The Code In that file*/

#include <reg51.h>

#define adc_data P0
#define lcd      P1

#define THRESHOLD 1000 // Methane threshold in ppm

unsigned char temp, gas, moist, methane_level;

sbit rs=P1^0;
sbit en=P1^1;

sbit OE      =P2^0;
sbit EOC     =P2^1;
sbit START   =P2^2;
sbit AA      =P2^4;
sbit BB      =P2^5;
sbit CC      =P2^6;
sbit ALE     =P2^7;

unsigned char adc_val;

void lcd_init();
void cmd(unsigned char a);
void dat(unsigned char b);
void show(unsigned char *s);
void lcd_delay();

unsigned char adc(unsigned int ch);
void ch_sel(unsigned int sel);

void motor_forward();
void motor_stop();

void fan_on();
void fan_off();

void main()
{
    lcd_init();
    cmd(0x80);
    show("Temp: ");
    cmd(0x89);
    show("Gas: ");
    cmd(0xC0);
    show("Moist: ");

    // Set P3.0 and P3.1 as output pins for the motor control
    P3 &= ~(1 << 0); // P3.0 as output
    P3 &= ~(1 << 1); // P3.1 as output

    // Set P3.2 and P3.3 as output pins for the exhaust fan control
    P3 &= ~(1 << 2); // P3.2 as output
    P3 &= ~(1 << 3); // P3.3 as output

    while(1) {
        cmd(0x86); // Move cursor to the seventh position of the first line
        temp = adc(0); // Reading Value from ADC Channel 0
        dat((temp / 100) % 10 + 0x30);
        dat((temp / 10) % 10 + 0x30);
        dat(temp % 10 + 0x30);

        cmd(0x8D); // Move cursor to the thirteenth position of the first line
        gas = adc(1); // Reading Value from ADC Channel 1
			 // Fan Control Logic
        if (gas == 0) {
            show("NO.");
					fan_off(); // Turn off exhaust fan if no gas is detected
        } else {
            show("Yes");
					fan_on(); // Turn on exhaust fan if gas is detected
					
        }

        cmd(0xC6); // Move cursor to the seventh position of the second line
        moist = adc(2); // Reading Value from ADC Channel 2
        dat((moist / 100) % 10 + 0x30);
        dat((moist / 10) % 10 + 0x30);
        dat(moist % 10 + 0x30);

        // Read methane level from ADC Channel 3
        cmd(0xC9); // Move cursor to the tenth position of the second line
        methane_level = adc(3);

       
     
        
        // Motor Control Logic
        if (temp < 30 && moist < 30) 
				{
            motor_forward(); // Turn on the motor if both temp and moisture are low  
        } 
				else if (moist > 30) 
				{
            motor_stop(); // No need for motor operation
            //fan_off(); // Turn off exhaust fan
        } 
				else if (temp < 20 && moist < 30) 
				{
            motor_forward(); // Turn on the motor at a constant speed
            //fan_off(); // Turn off exhaust fan
        } 
				else if (temp > 30 && moist > 70) 
				{
            motor_stop(); // No need for motor operation
            //fan_off(); // Turn off exhaust fan
        } 
				//else if (temp > 30 && moist < 30) 
				//{
           // motor_forward(); // Turn on the motor if temp is high and moisture is less than 30%
            //if (methane_level > THRESHOLD) {
                //fan_on(); // Turn on exhaust fan if methane level exceeds threshold
            //} else {
                //fan_off(); // Turn off exhaust fan if methane level is below threshold
           // }
        //} 
				else if (temp >30 && moist < 30)
				{
            motor_forward(); // Turn on the motor if  temp is high and moisture is low
          
        } 
				else 
				{
            motor_stop(); // Default: stop the motor
            
        }
    }
}

void lcd_init()
{
    cmd(0x02);
    cmd(0x28);
    cmd(0x0e);
    cmd(0x06);
    cmd(0x80);
}

void cmd(unsigned char a)
{
    rs = 0;
    lcd &= 0x0F;
    P1 |= (a & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
    lcd &= 0x0f;
    lcd |= (a << 4 & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
}

void dat(unsigned char b)
{
    rs = 1;
    lcd &= 0x0F;
    lcd |= (b & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
    lcd &= 0x0f;
    lcd |= (b << 4 & 0xf0);
    en = 1;
    lcd_delay();
    en = 0;
    lcd_delay();
}

void show(unsigned char *s)
{
    while(*s) {
        dat(*s++);
    }
}

void lcd_delay()
{
    unsigned int lcd_delay;
    for(lcd_delay = 0; lcd_delay <= 1000; lcd_delay++);
}

unsigned char adc(unsigned int ch)
{
    adc_data = 0xff;

    ALE = START = OE = AA = BB = CC = 0;
    EOC = 1;

    ch_sel(ch);

    ALE = 1;
    START = 1;
    ALE = 0;
    START = 0;
    while(EOC == 1);
    while(EOC == 0);
    OE = 1;
    adc_val = adc_data;
    OE = 0;
    return adc_val;
}

void ch_sel(unsigned int sel)
{
    switch(sel) {
        case 0: CC = 0; BB = 0; AA = 0; break;            // 000
        case 1: CC = 0; BB = 0; AA = 1; break;            // 001
        case 2: CC = 0; BB = 1; AA = 0; break;            // 010
        case 3: CC = 0; BB = 1; AA = 1; break;            // 011
        case 4: CC = 1; BB = 0; AA = 0; break;            // 100
        case 5: CC = 1; BB = 0; AA = 1; break;            // 101
        case 6: CC = 1; BB = 1; AA = 0; break;            // 110
        case 7: CC = 1; BB = 1; AA = 1; break;            // 111
    }
}

void motor_forward()
{
    // Drive the motor forward by setting P3.0 high and P3.1 low
    P3 |= (1 << 0); // Set P3.0 high
    P3 &= ~(1 << 1); // Set P3.1 low
}

void motor_stop()
{
    // Stop the motor by setting both P3.0 and P3.1 low
    P3 &= ~(1 << 0); // Set P3.0 low
    P3 &= ~(1 << 1); // Set P3.1 low
}

void fan_on()
{
    // Drive the exhaust fan forward by setting P3.2 high and P3.3 low
    P3 |= (1 << 2); // Set P3.2 high
    P3 &= ~(1 << 3); // Set P3.3 low
}

void fan_off()
{
    // Stop the exhaust fan by setting both P3.2 and P3.3 low
    P3 &= ~(1 << 2); // Set P3.2 low
    P3 &= ~(1 << 3); // Set P3.3 low
}
