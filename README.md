ESP32  SD card connection 

SPI:

MISO  GPIO19。
MOSI  GPIO23。
SCK   GPIO18。
CS    GPIO5。

SDMMC
D02   GPIO12
D00   MISO  GPIO2。
CMD   MOSI  GPIO15
SCK   SCK   GPIO14
D03   CS    GPIO13。
D01   GPIO4

I2C 
SDA  GPIO16
SCL  GPIO17

Need to use SD card in good quality, 
otherwise the SD card writting may draw the power for a while and interfere I2C reading