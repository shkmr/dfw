/* 
   have fun with GDB's stack trace 
*/
#include <io.h>

#define toggle_led()  {P1OUT ^=  BIT0;}

tak(x, y, z)
{
  if (x <= y) 
    return z;
  else 
    return tak(tak(x-1, y, z),
               tak(y-1, z, x),
               tak(z-1, x, y));
}

main()
{ 
  WDTCTL = (WDTPW|WDTHOLD);
  P1DIR |= 0x01;

  while (1) {                          
    toggle_led();
    tak(10, 5, 1);
  }
}

/* EOF */
