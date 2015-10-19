set remoteaddresssize 64
set remotetimeout 999999
target remote localhost:2000

define reload
  monitor erase
  monitor flash
  load
  monitor memory
  set $pc = *0xfffe
end

define exit_lpm
  set $r2 = $r2 & ~0x00f0
end
