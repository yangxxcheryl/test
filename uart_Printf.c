/**************************************************

***************************************************/
// File name: "uart_Printf.c"
/*************************************
 Editor: lihanyang
 Date:2010-2-10
*************************************/

#include	"B1404_LIB.h"
#include    <string.h>

#define vaStart(list, param) list = (char*)((int)&param + sizeof(param))
#define vaArg(list, type) ((type *)(list += sizeof(type)))[-1]

static void  PutRepChar(char c, unsigned char count)
{
	unsigned char  a;
	unsigned char * p;
	a  = (unsigned char)c;
	p = &a;
    while (count--)
		uart0SendData(p, 1);
}

static void  PutStringReverse(char *s, unsigned char index)
{
	unsigned char * p;
	p = (unsigned char *)s;
    while ((index--) > 0)
		uart0SendData(p+index, 1);
}

static void  PutNumber(int value, signed char radix, unsigned char width, char fill)
{
    char    buffer[40];
    unsigned char     bi = 0;
    unsigned int  uvalue;
    unsigned short  digit;
    unsigned char  left = 0;
    unsigned char  negative = 0;

    if (fill == 0)
        fill = ' ';

    if (width < 0)
    {
        width = -width;
        left = 1;
    }

    if (width < 0 || width > 80)
        width = 0;

    if (radix < 0)
    {
        radix = -radix;
        if (value < 0)
        {
            negative = 1;
            value = -value;
        }
    }

    uvalue = value;

    do
    {
        if (radix != 16)
        {
            digit = uvalue % radix;
            uvalue = uvalue / radix;
        }
        else
        {
            digit = uvalue & 0xf;
            uvalue = uvalue >> 4;
        }
        buffer[bi] = digit + ((digit <= 9) ? '0' : ('A' - 10));
        bi++;
		/*
        if (uvalue != 0)
        {
            if ((radix == 10)	// Ç§Î»·û
                && ((bi == 3) || (bi == 7) || (bi == 11) | (bi == 15)))
            {
                buffer[bi++] = ',';
            }
        }*/
    }
    while (uvalue != 0);

    if (negative)
    {
        buffer[bi] = '-';
        bi += 1;
    }

    if (width <= bi)
        PutStringReverse(buffer, bi);
    else
    {
        width -= bi;
        if (!left)
            PutRepChar(fill, width);
        PutStringReverse(buffer, bi);
        if (left)
            PutRepChar(fill, width);
    }
}

static char  *FormatItem(char *f,  int a)
{
    char   c;
	unsigned char uc;
    signed char    fieldwidth = 0;
    signed char    leftjust = 0;
    signed char    radix = 0;
    char   fill = ' ';

    if (*f == '0')
        fill = '0';

    while ((c = *f++) != 0)
    {
        if (c >= '0' && c <= '9')
        {
            fieldwidth = (fieldwidth * 10) + (c - '0');
        }
        else
            switch (c)
            {
                case '\000':
                    return (--f);
                case '%':
					uart0SendData("%", 1);
                    return (f);
                case '-':
                    leftjust = 1;
                    break;
                case 'c':
                    {
                        if (leftjust){
							uc = (unsigned char)(a & 0x7f);
							uart0SendData(&uc, 1);
                        	}

                        if (fieldwidth > 0)
                            PutRepChar(fill, fieldwidth - 1);

                        if (!leftjust){
							uc = (unsigned char)(a & 0x7f);
							uart0SendData(&uc, 1);
                        	}
                        return (f);
                    }
                case 's':
				/*	if (leftjust)
						uart0SendData((unsigned char *)a, strlen((char *)a));
					if (fieldwidth > strlen((char *)a))
						PutRepChar(fill,  fieldwidth - strlen((char *)a));
					if (!leftjust)
						uart0SendData((unsigned char *)a, strlen((char *)a));
				*/
					uart0SendString((_CONST unsigned char *)a);
					return (f);
                case 'd':
                case 'i':
                    radix = -10;
                    break;
                case 'u':
                    radix = 10;
                    break;
                case 'x':
                    radix = 16;
                    break;
                case 'X':
                    radix = 16;
                    break;
                case 'o':
                    radix = 8;
                    break;
                default:
                    radix = 3;
                    break;      /* unknown switch! */
            }
        if (radix)
            break;
    }

    if (leftjust)
        fieldwidth = -fieldwidth;

    PutNumber(a, radix, fieldwidth, fill);

    return (f);
}

void  uart_Printf(char *f, ...) /* variable arguments */
{
    char  *argP;
	unsigned char *p;
	char c;
	//c = 0x02;
	//uart0SendData(&c, 1);
    vaStart(argP, f);       /* point at the end of the format string */
    while (*f)
    {                       /* this works because args are all ints */
        if (*f == '%')
            f = FormatItem(f + 1, vaArg(argP, int));

        else{
		//	if(*f == '\n'){
		//		uart0SendData("\r", 1);
		//		}
			p = (unsigned char *)f;
			uart0SendData(p, 1);
			f ++;
        	}
    }
	//c = 0x03;		
	//uart0SendData(&c, 1);
}

void uart0SendInt(unsigned long Num)
{
  if(Num < 10)
  {
    uart0SendChar(Num + '0');
  }
  else
  {
    uart0SendInt(Num / 10);
    uart0SendInt(Num % 10);
  }
}

/*********************** File end *************************/

