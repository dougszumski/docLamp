import math 

intervals = 90
power = 11

#Angle calculation; split into 1 deg. ints over 90 degrees
angleIncr = (math.pi/2)/ intervals
angle = 0 

for i in range(intervals):
    print str(int(math.cos(angle)*(2**power))) +','
    angle += angleIncr
 
print "\nLUT dim:", intervals
print "Note: These are the upper limits of the interval"
print "Bit shift left ratio:", power
