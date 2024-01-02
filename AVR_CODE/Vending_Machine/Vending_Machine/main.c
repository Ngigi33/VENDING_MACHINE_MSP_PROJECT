/*
 * Vending Machine v1.c
 *
 * Created: 26/03/2023 13:12:08
 */ 
#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#define Keypad_Dir DDRC
#define Keypad_Port PORTC
#define Keypad_Pin PINC

#define LED_Dir DDRA
#define LED_Port PORTA

#define LCD_Dir DDRD
#define LCD_Port PORTD
#define RS PD0
#define EN PD1

//IMPORTANT DECLARATIONS.
char src[5],dest[5];
char read_val;// what is read from A,B,C,D
char previous_key;
char entry = 1 ;
char allow_payment = 0;
char money_deposited = 0;
char i;


//ADC CONFIG
int Potval;
float MapedValue;
int ADC_Read(char channel);

/*LCD command write function*/
void LCD_Cmd(unsigned char cmd){
	/*Sending the first nibble of data (Higher 4 bits)*/
	LCD_Port = (LCD_Port & 0x03) | (cmd & 0xF0);/* Sending upper nibble */
	LCD_Port &= ~ (1<<RS); /* RS=0, command reg. */
	LCD_Port |= (1<<EN); /* Enable pulse ON */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN); /* Enable pulse OFF */
	_delay_us(200);
	/*Sending the second nibble of data (Lower 4 bits)*/
	LCD_Port = (LCD_Port & 0x03) | (cmd << 4);/* Sending lower nibble */
	LCD_Port |= (1<<EN); /* Enable pulse ON */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN); /* Enable pulse OFF */
	_delay_ms(2);
}

/*LCD Initialize function */
void LCD_Init (void){
	LCD_Dir = 0xF3; /* Make LCD command port direction as output pins*/
	_delay_ms(20); /* LCD Power ON delay always > 15ms */
	LCD_Cmd(0x02); /* Return display to its home position */
	LCD_Cmd(0x28); /* 2 line 4bit mode */
	LCD_Cmd(0x0C); /* Display ON Cursor OFF */
	LCD_Cmd(0x06); /* Auto Increment cursor */
	LCD_Cmd(0x01); /* Clear display */
	_delay_ms(2);
}

/*Clear LCD Function*/
void LCD_Clear(void){
	LCD_Cmd(0x01); /* clear display */
	LCD_Cmd(0x02); /* Return display to its home position */
}

/*LCD data write function */
void LCD_Char (unsigned char char_data){
	/*Sending the first nibble of data (Higher 4 bits)*/
	LCD_Port = (LCD_Port & 0x0F) | (char_data & 0xF0);/* Sending upper nibble */
	LCD_Port |= (1<<RS); /* RS=1, data reg. */
	LCD_Port |= (1<<EN); /* Enable pulse ON */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN); /* Enable pulse OFF */
	_delay_us(200);
	/*Sending the second nibble of data (Lower 4 bits)*/
	LCD_Port = (LCD_Port & 0x0F) | (char_data << 4); /* Sending lower nibble */
	LCD_Port |= (1<<EN); /* Enable pulse ON */
	_delay_us(1);
	LCD_Port &= ~ (1<<EN); /* Enable pulse OFF */
	_delay_ms(2);
}

/*Send string to LCD function */
void LCD_String (char *str){
	int i;
	/* Sends each char of string till the NULL */
	for(i=0;str[i]!=0;i++){
		LCD_Char(str[i]);
	}
}

void Print_String (char *str){
	int i;
	/* Sends each char of string till the NULL */
	for(i=0;str[i]!=0;i++){
		LCD_Char(str[i]);
		_delay_ms(50);
	}
}

//CUSTOM FUNCTIONS
void Display_Message (char *line_1 ,char *line_2 ){
	LCD_Cmd(0x01);
	LCD_Cmd(0X02);
	LCD_String(line_1);
	LCD_Cmd(0xC0);
	LCD_String(line_2);
	_delay_ms(500);
	return;
}
/* Displays items on the menu */
void menu(){
	Display_Message("  Hello there!  ","--- Welcome --- ");
	_delay_ms(400);
	Display_Message("Enter A1 ","To Pick Fanta");
	_delay_ms(400);
	Display_Message("Enter A2 ","To Pick Sprite");
	_delay_ms(400);
	Display_Message("Enter A3 ","To Pick Coke");
	_delay_ms(400);
	Display_Message("Enter B1 ","To Pick Oreos");
	_delay_ms(400);
	Display_Message("Enter B2 ","To Pick Manji");
	_delay_ms(400);
	Display_Message("Enter B3 ","To Pick Digestive");
	_delay_ms(400);
	Display_Message("Enter C1 ","To Pick Haribo");
	_delay_ms(400);
	Display_Message("Enter C2 ","To Pick Orbit");
	_delay_ms(400);
	Display_Message("Enter C3 ","To Pick Smarties");
	_delay_ms(400);
}

// Interrupt service routine for push button 1 (Ksh. 5)
ISR(INT0_vect) {
	if (allow_payment == 1){
		money_deposited += 5;
		//_delay_ms(50);  	// Software debouncing control delay
		char money_dep_str[10];
		sprintf(money_dep_str, " Ksh %d", money_deposited);
		LCD_Cmd(0x01);
		Display_Message(" Received", money_dep_str);
		_delay_ms(50);  	// Software debouncing control delay
	}
}



// Interrupt service routine for push button 2 (Ksh. 10)
ISR(INT1_vect) {
	if (allow_payment == 1){
		money_deposited += 10;
		char money_dep_str[10];
		sprintf(money_dep_str, " Ksh %d", money_deposited);
		LCD_Cmd(0x01);
		Display_Message(" Received", money_dep_str);
		_delay_ms(50);  	// Software debouncing control delay
	}
}

// Interrupt service routine for push button 3 (Ksh. 20)
ISR(INT2_vect) {
	if (allow_payment == 1){
		money_deposited += 20;
		char money_dep_str[10];
		sprintf(money_dep_str, " Ksh %d", money_deposited);
		LCD_Cmd(0x01);
		Display_Message(" Received", money_dep_str);
		_delay_ms(50);  	// Software debouncing control delay
	}
}

void Display_LCD (char *selected_item ,char *item_price ){
	LCD_Cmd(0x01);
	LCD_Cmd(0X02);
	LCD_String("Item : ");
	LCD_String(selected_item);
	LCD_Cmd(0xC0);
	LCD_String("Price : ");
	LCD_String(item_price);
	return;
}

void Balance_Message (char *line_1 ,char *line_2 ){
	LCD_Cmd(0x01);
	LCD_Cmd(0X02);
	Print_String("Pay :");
	Print_String(line_1);
	LCD_Cmd(0xC0);
	Print_String("For : ");
	Print_String(line_2);
	return;
}
void LCD_Reset(){
	menu();
	LCD_Cmd(0x01); //Clearing screen
	LCD_Cmd(0X02); //Returns cursor to home position.
	LCD_String(" Pick Item : ");
	LCD_Cmd(0xC0);
	money_deposited=0;
	entry=1;
	allow_payment = 0 ;

}

void toggle_led(char *item){

	if(strcmp(item, "Fanta") == 0){
		for (char counter=1 ;counter<=6;counter++){
			LED_Port ^=(1<<PA1|1<<PA4);
			_delay_ms(1000);
			}} else if(strcmp(item, "Sprite") == 0){
			for (char counter=1 ;counter<=6;counter++){
				LED_Port ^=(1<<PA1|1<<PA5);
				_delay_ms(1000);
				}} else if(strcmp(item, "Coke") == 0){
				for (char counter=1 ;counter<=6;counter++){
					LED_Port ^=(1<<PA1|1<<PA6);
					_delay_ms(1000);
					}} else if(strcmp(item, "Oreos") == 0){
					for (char counter=1 ;counter<=6;counter++){
						LED_Port ^=(1<<PA2|1<<PA4);
						_delay_ms(1000);
						}} else if(strcmp(item, "Manji") == 0){
						for (char counter=1 ;counter<=6;counter++){
							LED_Port ^=(1<<PA2|1<<PA5);
							_delay_ms(1000);
							}} else if(strcmp(item, "Digestive") == 0){
							for (char counter=1 ;counter<=6;counter++){
								LED_Port ^=(1<<PA2|1<<PA6);
								_delay_ms(1000);
								}} else if(strcmp(item, "Haribo") == 0){
								for (char counter=1 ;counter<=6;counter++){
									LED_Port ^=(1<<PA3|1<<PA4);
									_delay_ms(1000);
									}} else if(strcmp(item, "Orbit") == 0){
									for (char counter=1 ;counter<=6;counter++){
										LED_Port ^=(1<<PA3|1<<PA5);
										_delay_ms(1000);
										}} else if(strcmp(item, "Smarties") == 0){
										for (char counter=1 ;counter<=6;counter++){
											LED_Port ^=(1<<PA3|1<<PA6);
											_delay_ms(1000);
											}} else{
											for (char counter=1 ;counter<=6;counter++){
												LED_Port ^=(1<<PA1|1<<PA2|1<<PA3|1<<PA4|1<<PA5|1<<PA6);
												_delay_ms(450);
											}}
										};
 
void payment(char *item ,int amount_due){
	_delay_ms(500);
	allow_payment = 1 ;
	char amount_due_str[10];
	sprintf(amount_due_str, "Ksh %d", amount_due);
	while(money_deposited < amount_due) {
		Balance_Message( amount_due_str,item);
		_delay_ms(200);
		
	}
	if (money_deposited >=amount_due){
		int balance = money_deposited - amount_due;
		char balance_str[10];
		sprintf(balance_str, "Balance : Ksh  %d", balance);
		Display_Message("Dispensing ...  ", balance_str);
		_delay_ms(500);
		toggle_led(item);
		if(balance > 0){
			Display_Message("Pick Change ", balance_str);
			_delay_ms(3000);
			Display_Message("Thank you"," Kwaheri!");
			_delay_ms(1500);
			LCD_Reset();
			return;
		}
		Display_Message("Thankyou","Kwaheri!");
		_delay_ms(1500);
		LCD_Reset();
		return;
	}

}
void reveal_item(void){
	// Create temporary variable to hold concatenated data
	char concatenated[20];
	// Concatenate src and dest into temporary variable
	strcpy(concatenated, dest);
	strcat(concatenated, src);

	if(strcmp(concatenated, "A1") == 0){
		Display_LCD("Fanta", "Ksh 15");
		payment("Fanta",15);
		return;
		
		}else if(strcmp(concatenated, "A2") == 0){
		Display_LCD("Sprite", "Ksh 25");
		payment("Sprite",25);
		return;
		
		}else if(strcmp(concatenated, "A3") == 0){
		Display_LCD("Coke", "Ksh 30");
		payment("Coke",30);
		return;

		}else if(strcmp(concatenated, "B1") == 0){
		Display_LCD("Oreos", "Ksh 10");
		payment("Oreos",10);
		return;

		}else if(strcmp(concatenated, "B2") == 0){
		Display_LCD("Manji", "Ksh 12");
		payment("Manji",12);
		return;

		}else if(strcmp(concatenated, "B3") == 0){
		Display_LCD("Digestive", "Ksh 15");
		payment("Digestive",15);
		return;

		}else if(strcmp(concatenated, "C1") == 0){
		Display_LCD("Haribo", "Ksh 20");
		payment("Haribo",20);
		return;

		}else if(strcmp(concatenated, "C2") == 0){
		Display_LCD("Orbit", "Ksh 15");
		payment("Orbit",15);
		return;

		}else if(strcmp(concatenated, "C3") == 0){
		Display_LCD("Smarties", "Ksh 18");
		payment("Smarties",18);
		return;

		}else{
		Display_Message("Error!" ,"Error!");
		_delay_ms(500);
		LCD_Reset();
	}

}


void display (char *pressed_key){
	
	
	if (strcmp(pressed_key, "=") == 0){
		LCD_Reset();
		return;

	}
	
	if(strcmp(pressed_key, "ON") == 0){
		reveal_item();
		entry=1; //Resets the entry counter.
		return;
	}
	
	if (entry == 1){
		dest[0] = '\0';  // Clear the dest string before appending a new character
		strcat(dest, pressed_key);
		LCD_String(dest);
		//Display_Message("Input 1", dest);
		entry=2;
		return;
	}
	
	if (entry == 2){
		src[0] = '\0';   // Clear the src string before appending a new character
		strcat(src, pressed_key);
		LCD_String(src);
		//Display_Message("Input 2", src);
		return;
	}
};




void alert(){
		while(1){
			Display_Message("---- Alert! ----" ,"---- Alert! ----"  );
			LED_Port ^=(1<<PA7);
			_delay_ms(1000);
	}};


void Read_Keypad(void)
{
	
	read_val=0xFF & Keypad_Pin;
	switch(read_val)
	{
		case 0x05:
		_delay_ms(3);
		display("C");
		break;
		
		case 0x06:
		_delay_ms(3);
		display("B");
		break;
		
		case 0x07:
		_delay_ms(3);
		display("A");
		break;
		
		case 0x09:
		_delay_ms(3);
		display("3");
		break;
		
		case 0x0A:
		_delay_ms(3);
		display("2");
		break;
		
		case 0x0B:
		_delay_ms(3);
		display("1");
		break;
		
		case 0x0D:
		_delay_ms(3);
		display("=");
		break;
		
		case 0x0E:
		_delay_ms(3);
		display("ON");
		break;
	}
		
	
}

void ADC_Init()				/* ADC Initialization function */
{
	LED_Dir&=~(1<<PA0);
	ADCSRA = 0x87;			/* Enable ADC, with freq/128 */
	ADMUX = 0x40;			/* Vref: Avcc, ADC channel: 0 */
}
	
int ADC_Read(char channel){
	ADMUX = 0; // Reset ADMUX register
	ADCSRA |= (1<<ADEN); // Enable ADC
	ADMUX |= (1<<REFS0); // Set reference voltage to AVCC
	ADMUX |= (channel & 0x07); // Set input channel
	ADCSRA |= (1<<ADSC); // Start conversion
	while(ADCSRA & (1<<ADSC)); // Wait for conversion to complete
	return ADC;
}



int main(void)
{
	LED_Dir = 0x00;
	LED_Port = 0x00;
	// Enable external interrupts for push buttons 1, 2, and 3
	MCUCR |= (1 << ISC01)|(1 << ISC00) | (1 << ISC11)|(1 << ISC10); // Enable interrupt on rising edge
	MCUCSR |= (1 << ISC2); //Triggers interrup 2 on the rising edge.
	GICR |= (1 << INT0) | (1 << INT1) | (1 << INT2); // Enable external interrupts
	sei();
	
	//Configuring ADC
	ADC_Init();
	//Configuring keypad
	Keypad_Dir= 0x00;// set as input
	Keypad_Port= 0X00;// setting pull up on port B
	
	//Configuring led's
	LED_Dir |=(1<<PA1|1<<PA2|1<<PA3|1<<PA4|1<<PA5|1<<PA6|1<<PA7);
	LCD_Init();
	//alert();
	LCD_Reset();
	
    /* Replace with your application code */
    while (1) 
    {
	Read_Keypad();
	/*Potval = ADC_Read(0); //Reads 10but ADC channel in integers 0-1023 
		if(Potval >=(0.5*Potval)){
			char PotVal_str[10];
			sprintf(PotVal_str, "Ksh %d", Potval);
			Display_Message("Pot reading ", PotVal_str);
			alert();
		}	*/
	_delay_ms(400);	//Debouncing technique
		
    }



	return 0;
}


