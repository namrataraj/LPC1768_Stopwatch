#include <LPC17xx.h>
#include <stdint.h>

// Function prototypes
void scan(void);
void stopwatch(void);
void write(uint32_t temp2, uint8_t type);
void delay(unsigned int milliseconds);
void lcd_init(void);
void lcd_comdata(uint8_t data, uint8_t type);
void lcd_puts(unsigned char *str);
void clear_ports(void);

// Global variables
unsigned char key;
unsigned char Msg1[] = "STOPWATCH: ";

//unsigned char Msg2[] = "STOP: ";
unsigned long int tempn=0x00002000;

unsigned char start[] = "00";
unsigned char k;

unsigned char SCAN_CODE[16] = {0x11, 0x21, 0x41, 0x81,
                                0x12, 0x22, 0x42, 0x82,
                                0x14, 0x24, 0x44, 0x84,
                                0x18, 0x28, 0x48, 0x88};

int main(void) {
    // Initialize system and LCD
    SystemInit();
    SystemCoreClockUpdate();
    lcd_init();

    // Start stopwatch
    stopwatch();

    return 0;
}

void stopwatch(void) {
    unsigned int sec = 0, running = 0;

    while (1) {
				LPC_GPIO2->FIOCLR = 0x00003C00; //first clear the port and send appropriate value for
				LPC_GPIO2->FIOSET = tempn; //enabling the row
        scan(); // Scan for key press
			
			lcd_comdata(0xc0, 0);

        if (key == 0x18) { // Start
            running = 1;
        } else if (key == 0x28) { // Stop
            running = 0;
        } else if (key == 0x48) { // Reset
            sec = 0;
            running = 0;
            
        }
				lcd_comdata(0xc0, 0);
            lcd_puts(start);
				
				

        if (running) {
            sec++;
            if (sec > 59) sec = 0; // Reset seconds if over 59
            lcd_comdata(0xc0, 0);
            k = sec / 10 + 0x30;
            lcd_puts(&k);
            k = 0x30;
            lcd_puts(&k);
            k = sec % 10 + 0x30;
            lcd_puts(&k);
            delay(1000000); // Delay 1 second
        }
    }
}

void scan(void) {
    unsigned long int temp3;

    temp3 = LPC_GPIO1->FIOPIN;
    temp3 &= 0x07800000;
    if (temp3 != 0x00000000) {
        temp3 >>= 19;
			tempn >>= 10;
        key = temp3|tempn;
    }
}

void delay(unsigned int milliseconds) {
    LPC_TIM0->TCR = 0x02;
    LPC_TIM0->TCR = 0x01;
    while (LPC_TIM0->TC < milliseconds);
    LPC_TIM0->TCR = 0x00;
}

void lcd_init(void) {
    LPC_PINCON->PINSEL1 &= 0xFC003FFF; // P0.23 to P0.28
    LPC_GPIO0->FIODIR |= 0x0F << 23 | 1 << 27 | 1 << 28;
    clear_ports();
    delay(3200);
    lcd_comdata(0x33, 0);
    delay(30000);
    lcd_comdata(0x32, 0);
    delay(30000);
    lcd_comdata(0x28, 0); // Function set
    delay(30000);
    lcd_comdata(0x0c, 0); // Display on cursor off
    delay(800);
    lcd_comdata(0x06, 0); // Entry mode set increment cursor right
    delay(800);
    lcd_comdata(0x01, 0); // Display clear
    delay(10000);
	  lcd_puts(&Msg1[0]);
	//lcd_comdata(0xc0, 0);
	 // lcd_puts(&Msg2[0]);
}

void lcd_comdata(uint8_t temp1, uint8_t type) {
    uint32_t temp2;

    temp2 = temp1 & 0xF0; // Move data (26-8+1) times : 26 - HN place, 4 - Bits
    temp2 = temp2 << 19; // Data lines from 23 to 26
    write(temp2, type);
    temp2 = temp1 & 0x0F; // 26-4+1
    temp2 = temp2 << 23;
    write(temp2, type);
    delay(1000);
}

void write(uint32_t temp2, uint8_t type) { /*write to command/data reg */
    clear_ports();
    LPC_GPIO0->FIOPIN = temp2; // Assign the value to the data lines
    if (type == 0)
        LPC_GPIO0->FIOCLR = 1 << 27; // clear bit RS for Command
    else
        LPC_GPIO0->FIOSET = 1 << 27; // set bit RS for Data
    LPC_GPIO0->FIOSET = 1 << 28; // EN=1
    delay(25);
    LPC_GPIO0->FIOCLR = 1 << 28; // EN =0
}

void lcd_puts(unsigned char *buf1) {
	unsigned int i=0;
	unsigned int temp3;
	while(buf1[i]!='\0') {
		temp3 = buf1[i];
		lcd_comdata(temp3, 1);
		i++;
		if(i==16)
			lcd_comdata(0xc0, 0);
	}
	return;
}

void clear_ports(void) { /* Clearing the lines at power on */
    LPC_GPIO0->FIOCLR = 0x0F << 23; // Clearing data lines
    LPC_GPIO0->FIOCLR = 1 << 27; // Clearing RS line
    LPC_GPIO0->FIOCLR = 1 << 28; // Clearing Enable line
}


