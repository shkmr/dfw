define reload
  monitor erase
  monitor flash
  load
  monitor memory
  set $pc = *0xfffe
end

define msg
  x/30s 'uif_ez.c'::msgbuf
end

define port
  printf "SEL P1=0x%02x P2=0x%02x P3=0x%02x P4=0x%02x P5=0x%02x P6=0x%02x\n", P1SEL, P2SEL, P3SEL, P4SEL, P5SEL, P6SEL
  printf "DIR P1=0x%02x P2=0x%02x P3=0x%02x P4=0x%02x P5=0x%02x P6=0x%02x\n", P1DIR, P2DIR, P3DIR, P4DIR, P5DIR, P6DIR
  printf "OUT P1=0x%02x P2=0x%02x P3=0x%02x P4=0x%02x P5=0x%02x P6=0x%02x\n", P1OUT, P2OUT, P3OUT, P4OUT, P5OUT, P6OUT
end
