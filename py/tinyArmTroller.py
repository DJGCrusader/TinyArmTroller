import serial
import time

port = '/dev/ttyUSB1'

ser = serial.Serial(port,38400)
state = 0

baseMin = -850
baseMax = 850

def degToSteps(a):
    out = [0,0,0,0,0,0]
    out[0] = int(a[0]/90.0 * 850)
    out[1] = int((a[1]+35+90)/180.0 * 1450)
    out[1] = 0
    out[2] = int((a[2]-35-90+145)/180.0* 1450)
    out[2] = 0
    out[3] = int(((a[3]+a[4])/90.0)*200)
    out[4] = int(((a[3]-a[4])/90.0)*200)
    out[5] = int(a[5]*800)
    print out
    return out

def writeOut(v):
    out = ''
    for i in range(0,len(v)):
        out+=str(v[i])+','
    out = out[0:len(out)-1]
    out = out+'\n\r'
    print out
    ser.write(out)


def test1():
    flag = 0
    print 'Waiting for Serial communication with '+port+'...'
    while 1:
        x = ser.readline()
        print x[0:len(x)-2]
        if x[0:len(x)-2] == 'gimme':
            time.sleep(.1)
            if flag == 0:
                flag = 1
                writeOut([850,0,0,0,0,0])
            elif flag == 1:
                flag = 0
                writeOut([425,0,0,0,0,0])
test1()

"""

"""
