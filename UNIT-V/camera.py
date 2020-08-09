# Untitled - By: y-kuboyama - 水 7月 29 2020


from machine import UART
from board import board_info
from fpioa_manager import fm
from Maix import GPIO
import sensor, lcd, ubinascii

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)#QVGA=320x240
#sensor.set_windowing((64,160))
sensor.set_vflip(1)
sensor.set_hmirror(1)
sensor.run(1)
sensor.skip_frames()


fm.register(35, fm.fpioa.UART1_TX, force=True)
fm.register(34, fm.fpioa.UART1_RX, force=True)

uart = UART(UART.UART1, 1152000,8,0,0, timeout=1000, read_buf_len=4096)


while(True):
    img = sensor.snapshot()
    #print(str(ubinascii.b2a_base64(img.compress(60)))) // compress and send it over serial in base64
    # compress and send it over serial in base64
    uart.write(str(ubinascii.b2a_base64(img.compress(60))))
