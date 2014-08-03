//set clock division factor to 1 => 8Mhz with default fuses
#define initCPUClk() \
{ \
    CLKPR = 1 << CLKPCE; \
    CLKPR = 0; \
}

//this value needs to be calculated on a per chip basis
//default factory value is 141
//initially I went for 144. Could have gone for 13 values either side
#define calibrateOsc() \
{ \
    OSCCAL = 142; \
}


#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>


#define jtd_set(x) \
{ \
    __asm__ __volatile__ ( \
    "in __tmp_reg__,__SREG__" "\n\t" \
    "cli" "\n\t" \
    "out %0, %1" "\n\t" \
    "out __SREG__, __tmp_reg__" "\n\t" \
    "out %0, %1" "\n\t" \
    : \
    : "M" (_SFR_IO_ADDR(MCUCR)), "r" ((uint8_t)((x) ? (1<<JTD) : 0)) \
    : "r0"); \
}

#define disableAnalogCompar() \
{ \
    ACSR &= ~(1 << ACIE); \
    ACSR |= 1 << ACD; \
}

#define ledOn(pin) PORTA |= (1 << ((pin)+3))
#define ledOff(pin) PORTA &= ~(1 << ((pin)+3))

#define MAX_STEPS 50
static uint8_t gStepData[MAX_STEPS];
static uint8_t gNumSteps;


static void set7Seg(uint8_t num);


#define HIGH_ONE_SECOND 16
#define HIGH_HALF_SECOND 8
static void highDelay(uint8_t num64ms)
{
    for(uint8_t i = 0; i < num64ms; i++)
    {
        _delay_ms(64);
    }
}

static void initPortsAndRegs()
{
    //NOTE: jtd_set overwrites MCUCR
    jtd_set(1);
    disableAnalogCompar();

    //LEDS
    DDRA |= (1 << PINA4) | (1 << PINA5) | (1 << PINA6) | (1 << PINA7);

    //input with pull ups (START_INPUT & INPUT1 - INPUT4)
    PORTD |= (1 << PIND6) | (1 << PIND5) | (1 << PIND4) | (1 << PIND3) | (1 << PIND2);

    //PC0 - PC6 = output for 7 seg (7SEGA - 7SEGG)
    DDRC = 0x7F;
}

static void initSteps()
{
    uint8_t* stepDataPtr = gStepData;

    *stepDataPtr++ = 4;	// count
    *stepDataPtr++ = 1;
    *stepDataPtr++ = 2;
    *stepDataPtr++ = 3;
    *stepDataPtr++ = 4;
    gNumSteps++;

    *stepDataPtr++ = 6;	// count
    *stepDataPtr++ = 2;
    *stepDataPtr++ = 1;
    *stepDataPtr++ = 4;
    *stepDataPtr++ = 2;
    *stepDataPtr++ = 3;
    *stepDataPtr++ = 1;
    gNumSteps++;


    //Use ADC to gen get random numbers
    //NOTE: in practice, the numbers generated weren't that random
    //ADCSRA |= (1 << ADPS1) | (1 << ADPS0); //Set Prescaler to 8 for 1Mhz
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1); //Set Prescaler to 64 for 8Mhz
    ADMUX |= 1 << REFS0; //set REF to AVCC
    ADMUX |= 1 << ADLAR; //Left adjust result for 8-bit result
    //ADMUX 4:0 = 0 = ADC0 connected to the ADC
    ADCSRA |= 1 << ADEN; //Enable ADC

    #define NUM_RANDOM_VALS 6
    *stepDataPtr++ = NUM_RANDOM_VALS;
    for(uint8_t i = 0; i < NUM_RANDOM_VALS; i++)
    {
        uint8_t ADCRes = 0;
        for(uint8_t k = 0; k < 8; k++)
        {
            ADCSRA |= 1 << ADSC; //start conversion
            while(ADCSRA & (1 << ADSC)) {} //wait for conv to complete
            uint8_t tempADCRes = ADCH;
            ADCRes |= (tempADCRes & 1) << k;
            ADCSRA |= 1 << ADIF; //clear interrupt flag
        }

        do
        {
            if(ADCRes < 64) { ADCRes = 1; break; }
            if(ADCRes < 128) { ADCRes = 2; break; }
            if(ADCRes < 192) { ADCRes = 3; break; }
            if(ADCRes <= 255) { ADCRes = 4; break; }
        } while (1);
        *stepDataPtr++ = ADCRes;

        //DEBUG random
        //set7Seg(ADCRes);
        //highDelay(HIGH_ONE_SECOND);
    }
    gNumSteps++;
    ADCSRA &= ~(1 << ADEN); //disable ADC in case we want to use I2C/TWI

    gNumSteps = gNumSteps > 9 ? 9 : gNumSteps;
}

static uint8_t startPressed(uint8_t havedelay, uint8_t iterations)
{
    uint8_t ret = 0;

    for(uint8_t i = 0; i < iterations; i++)
    {
        if(bit_is_clear(PIND, 6))
        {
            ret = 1;
            break;
        }
        else
        {
            if(havedelay) { _delay_ms(64); }
        }
    }

    return ret;
}

static void rundemo()
{
    while(1)
    {
        ledOn(1);
        if(startPressed(1, 6)) break;
        ledOff(1);

        ledOn(2);
        if(startPressed(1, 6)) break;
        ledOff(2);

        ledOn(3);
        if(startPressed(1, 6)) break;
        ledOff(3);

        ledOn(4);
        if(startPressed(1, 6)) break;
        ledOff(4);
    }
}

static void set7Seg(uint8_t num)
{
    switch(num)
    {
        case 0:
        PORTC = 0x3F; //all on other than g
        break;

        case 1:
        PORTC = 0x6; //b & c
        break;

        case 2:
        PORTC = 0x5B; //a,b,d,e,g
        break;

        case 3:
        PORTC = 0x4F; //a, b, c,d,g
        break;

        case 4:
        PORTC = 0x66; //b,c,f,g
        break;

        case 5:
        PORTC = 0x6D; //a,c,d,f,g
        break;

        case 6:
        PORTC = 0x7D; //a,c,d,e,f,g
        break;

        case 7:
        PORTC = 0x7; //a,b,c
        break;

        case 8:
        PORTC = 0xFF;
        break;

        case 9:
        PORTC = 0x6F; //a,b,c,d,f,g
        break;

        default:
        //TODO: debug statement here
        break;
    }

}

static uint8_t readInputsLoop()
{
    uint8_t btn = 0;

    while(1)
    {
        if (bit_is_clear(PIND, 5))
        {
            while(1) if(bit_is_set(PIND,5)) break;
            btn = 1;
            break;
        }
        if (bit_is_clear(PIND, 4))
        {
            while(1) if(bit_is_set(PIND,4)) break;
            btn = 2;
            break;
        }
        if (bit_is_clear(PIND, 3))
        {
            while(1) if(bit_is_set(PIND,3)) break;
            btn = 3;
            break;
        }
        if (bit_is_clear(PIND, 2))
        {
            while(1) if(bit_is_set(PIND,2)) break;
            btn = 4;
            break;
        }
    }

    return btn;
}

static void missionComplete()
{
    uint8_t counter = 0;

    while(1)
    {
        set7Seg(counter);
        counter++;
        counter = counter > 9 ? 0 : counter;

        if(startPressed(0,1)) break;

        _delay_ms(16);
    }
}

static void newGame()
{
    uint8_t win = 1;

    ledOff(1);
    ledOff(2);
    ledOff(3);
    ledOff(4);

    highDelay(HIGH_HALF_SECOND);

    uint8_t* thisStepStart = gStepData;
    uint8_t* stepPtr = thisStepStart + 1;
    for(uint8_t i = 0; i < gNumSteps; i++)
    {
        uint8_t stepCount = *thisStepStart;
        set7Seg(i+1);

        for(uint8_t k = 0; k < stepCount; k++)
        {
            ledOn(*stepPtr);
            highDelay(HIGH_HALF_SECOND);
            ledOff(*stepPtr);
            _delay_ms(256);
            stepPtr++;
        }

        stepPtr = thisStepStart + 1;
        for(uint8_t k = 0; k < stepCount; k++)
        {
            uint8_t btnHit = readInputsLoop();
            ledOn(btnHit);
            _delay_ms(128);
            ledOff(btnHit);
            if(btnHit != *stepPtr)
            {
                win = 0;
                goto end;
            }
            stepPtr++;
        }

        thisStepStart = stepPtr;
        stepPtr++;
    }

end:
    if(win) { missionComplete(); }
}

int main(void)
{
    initCPUClk();
    calibrateOsc();
    initPortsAndRegs();
    initSteps();

    while(1)
    {
        set7Seg(0);
        rundemo();
        newGame();
    }
}

