# dendy-dumper
Dumper for famicom/dendy cartridges

PROTOCOL:
G - switch to PRG mode
P - switch to PPU mode
RXXX - read data from ( XXX * 0x10 ) address. Minimum 0x10 bytes
WXXXXYY - write byte YY to address XXXX
CXXXX - set counter for reading. 0x10 bytes per 1 reading
Y - soft reset cartridge. Just off M2 for 100 ms
M - detect mirroring. Returns: 'Z' - single-screen mirroring, 'V' - vertical, 'H' - horizontal, 'F' - four-screen

Dumper returns:
'.' - command completed successfuly
'?' - command failed
'!' - controller booted up

