# sk9822 IC driver for Raspberry Pi 3
Utils to create and send byte packet with pixels to RPI SPI output to drive sk9822 IC LEDs and UDP listener for LedMapper app

# UDP packet format
----------------------------------------------------------------------------------------------------
NNNNNNNN|NNNNNNNN|NNNNNNNN|NNNNNNNN|FFFFFFFF|FFFFFFFF|RRRRRRRR|GGGGGGGG|BBBBBBBB|RRRRRRRR|GGGGGGGG..
----------------------------------------------------------------------------------------------------
  led count in   |  led count in   |   end of header |  pixels
channel 1 (16bit)|channel 2 (16bit)|