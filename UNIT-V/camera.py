from machine import UART
from board import board_info
from fpioa_manager import fm
from Maix import GPIO
import sensor, lcd

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)#QVGA=320x240
sensor.set_vflip(1)
sensor.set_hmirror(1)
sensor.run(1)
sensor.skip_frames()


fm.register(35, fm.fpioa.UART1_TX, force=True)
fm.register(34, fm.fpioa.UART1_RX, force=True)

uart = UART(UART.UART1, 1152000,8,0,0, timeout=1000, read_buf_len=4096)


while(True):
    img = sensor.snapshot()
    img_buf = img.compress(quality=50)
    img_size1 = (img.size()& 0xFF0000)>>16
    img_size2 = (img.size()& 0x00FF00)>>8
    img_size3 = (img.size()& 0x0000FF)>>0
    data_packet = bytearray([0xFF,0xD8,0xEA,0x01,img_size1,img_size2,img_size3,0x00,0x00,0x00])

    uart.write(data_packet)
    uart.write(img_buf)
