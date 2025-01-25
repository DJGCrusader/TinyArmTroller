"""
TinyArmTroller master

Daniel J. Gonzalez
dgonz@mit.edu

December 2012

This is the host program for TinyArm. 
Angle command is: [Base, Shoulder, Elbow, thetaE, thetaR, Gripper, RPM]
ik Command is: [X, Y, Z, thetaE, thetaR, Gripper, RPM]

"""

import serial
import time
import math

# 'deg', 'stp', 'ik', or 'string'
control = 'string'
sim = 0

port = '/dev/ttyUSB0'

if not sim:
    ser = serial.Serial(port,38400)

state = 0

baseStep0 = 0
baseStepMin = -900
baseStepMax = 900

homeAng = [0,125,-20,0,0,0,30]

def limitAnglesDeg(a):
    #Limit Base
    if a[0]>900:
        a[0] = 900
    elif a[0]<-900:
        a[0]=-900

    #Limit Arm
    if a[1]>140:
        a[1]=140
    elif a[1]<-20:
        a[1]=-20
        
    #Limit Elbow
    if a[2]>(a[1]):
        a[2]=a[1]
    elif a[2]<(a[1]-150):
        a[2]= (a[1]-150)

    #Limit Wrist 1

    #limit Wrist 2
    
    return a

def degToSteps(a):
    s = 'Ang Command: '
    for x in a:
        s+=str(x)+', '
    print s[0:len(s)-1]
    out = [0,0,0,0,0,0,0]
    out[0] = int(a[0]*10)#Base - Done
    out[1] = int((a[1]-140)*10)#Shoulder - Done
    out[2] = int((a[2]+10)*-6)#Elbow - Done
    out[3] = int(((a[3]-a[4])/90.0)*200)
    out[4] = int(((-1*a[3]-a[4])/90.0)*200)
    out[5] = int(a[5]*800)
    out[5] = -1*a[5]
    out[6] = a[6]
    
    return out

def writeOut(v):
    out = ''
    for i in range(0,len(v)):
        out+=str(v[i])+','
    out = out[0:len(out)-1]
    out = out+'\n\r'
    print "Sending stp vals: "+out[0:len(out)-2]
    if not sim:
        ser.write(out)

def inverseKinematics(myInput):
    #print 'Performing Inverse Kinematics on '+str(myInput)
    [x,y,z,thetaE,thetaR, gripAmount, mySpeed] = myInput
    # x = 5.125 is minimum x @ Shoulder height
    # x = 15.875 is maximum x
    # x = 11.8975 is max X @ ground and thetaE=-90
    thetaE = math.radians(thetaE)
    lg = 3.5 #inches
    l1 = 7
    l2 = 7
    xA = 1.6249
    yA = 7.25
    
    yE = z
    thetaBase = math.atan2(y,x)
    xE = math.sqrt((x**2) + (y**2))
    
    xC = xE-(lg*math.cos(thetaE))
    yC = yE-(lg * math.sin(thetaE))
    AC = math.sqrt((xC+xA)**2 + (yC-yA)**2)
    
    thetaAC = math.atan2(yC-yA,xC+xA)
    thetaA = math.acos((l2**2-l1**2-AC**2)/(-2*l1*AC))
    thetaB = math.acos((AC**2-l1**2-l2**2)/(-2*l1*l2))
    
    thetaShoulder = thetaAC + thetaA
    thetaElbow = thetaShoulder - (math.pi - thetaB)
    
    thetaBase = math.degrees(thetaBase)
    thetaShoulder = math.degrees(thetaShoulder)
    thetaElbow = math.degrees(thetaElbow)
    thetaE = math.degrees(thetaE)
    myOutput = [thetaBase, thetaShoulder, thetaElbow, thetaE, thetaR, gripAmount,mySpeed]
    return myOutput

def main():
    state = 0
    print "TinyArmTroller Host Software \nDaniel J. Gonzalez\ndgonz@mit.edu\n"
    print "With the power off, please localize and then \nreset TinyArmTroller before continuing.\n"
    while 1:
        if not sim:
            x = ser.readline()
        else:
            if(state == 0):
                x = 'readyxx'
            elif(state == 1):
                x = 'gimmexx'
        if(state == 0):
            if x[0:len(x)-2] == 'ready':
                state = 1
            else:
                print "TinyArm Says \""+x[0:len(x)-2]+"\""
                print "Localizing gripper... Make sure 12V power supply is on."
        elif (state == 1):
            if x[0:len(x)-2] == 'gimme':
                if(control == 'string'):
                    outRaw = raw_input('Enter the desired phrase: ')
                else:
                    outRaw = raw_input('Enter 7 '+control+
                                       ' waypoint values separated by commas. Last val is speed in rpm. \nTo execute multiple waypoints, enter \"period space\" between waypoints:\n')
                    
                outRaw = outRaw.split('. ')
                
                while(len(outRaw)!=0):
                    print x
                    print outRaw
                    if x[0:len(x)-2] == 'gimme':
                        x = ''
                        if (outRaw[0]=='pickUp'):
                            outRaw.pop(0)
                            outRaw.insert(0,'ready')
                            outRaw.insert(1,'down')
                            outRaw.insert(2,'grab')
                            outRaw.insert(3,'lift')
                            outRaw.insert(4,'idle')
                        if (outRaw[0]=='smileyFace'):
                            drawHeight = '2'
                            outRaw.pop(0)
                            outRaw.insert(0,'7,-1,'+drawHeight+',0,0,0,30')#right eye
                            outRaw.insert(1,'8,-1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(2,'8,-1,3,0,0,0,30')
                            
                            outRaw.insert(3,'7,1,'+drawHeight+',0,0,0,30')#left eye
                            outRaw.insert(4,'8,1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(5,'8,1,3,0,0,0,30')
                            
                            outRaw.insert(6,'8,-1.5,'+drawHeight+',0,0,0,30')#mouth
                            outRaw.insert(7,'9,-1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(8,'9,1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(9,'8,1.5,'+drawHeight+',0,0,0,30')
                            outRaw.insert(10,'8,1.5,3,0,0,0,30')
                            outRaw.insert(11,'idle')
                        if (outRaw[0]=='hex'):
                            drawHeight = '2'
                            outRaw.pop(0)
                            outRaw.insert(0,'7,-1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(1,'7,1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(2,'8,1.5,'+drawHeight+',0,0,0,30')
                            outRaw.insert(3,'9,1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(4,'9,-1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(5,'8,-1.5,'+drawHeight+',0,0,0,30')
                            outRaw.insert(6,'7,-1,'+drawHeight+',0,0,0,30')
                            outRaw.insert(7,'idle')
                        
                        if(outRaw[0] == 'home'):
                            writeOut([0,0,0,0,0,0,30])
                        elif(outRaw[0] == 'extend'):
                            writeOut(degToSteps([0,0,0,0,0,0,30]))
                        elif(outRaw[0] == 'up'):
                            writeOut(degToSteps([0,90,90,90,90,0,30]))
                        elif(outRaw[0] == 'ready'):
                            writeOut(degToSteps(limitAnglesDeg(inverseKinematics([4,0,1,-90,90,150,30]))))
                            #writeOut(degToSteps([0,35,-80,-90,0,150,30]))
                        elif(outRaw[0] == 'down'):
                            writeOut(degToSteps(limitAnglesDeg(inverseKinematics([4,0,0,-90,90,150,30]))))
                            #writeOut(degToSteps([0,28,-87,-90,0,150,30]))
                        elif(outRaw[0] == 'grab'):
                            writeOut(degToSteps(limitAnglesDeg(inverseKinematics([4,0,0,-90,90,0,30]))))
                            #writeOut(degToSteps([0,28,-87,-90,0,0,30])
                        elif(outRaw[0] == 'lift'):
                            writeOut(degToSteps(limitAnglesDeg(inverseKinematics([4,0,3,-90,90,0,30]))))
                        elif(outRaw[0] == 'idle'):
                            writeOut(degToSteps(limitAnglesDeg(inverseKinematics([7,0,3,0,0,0,30]))))
                            #writeOut(degToSteps([0,28,-87,-90,0,0,30]))    
                        else:
                            print outRaw
                            outRaw[0] = outRaw[0].split(',')
                            print outRaw
                            outRaw[0] = [float(x) for x in outRaw[0]]
                            print outRaw
                            if(control == 'ik'):
                                writeOut(degToSteps(limitAnglesDeg(inverseKinematics(outRaw[0]))))
                            if(control == 'deg'):
                                writeOut(degToSteps(limitAnglesDeg(outRaw[0])))
                            if(control == 'stp'):
                                writeOut(outRaw)
                        outRaw.pop(0)
                    x = ser.readline()
main()
