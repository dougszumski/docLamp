import math 

#Parameters for fade sequence
pwmLim = 3
red = pwmLim
green = 0
blue = 0
colourList = []

#Generate the fade sequence
while (green < pwmLim):
    colourList.append( [red, green, blue] )
    green +=1

while (red > 0):
    colourList.append( [red, green, blue] )
    red -=1

while (blue < pwmLim):
    colourList.append( [red, green, blue] )
    blue +=1

while (green > 0):
    colourList.append( [red, green, blue] )
    green -=1

while (red < pwmLim):
    colourList.append( [red, green, blue] )
    red +=1

while (blue > 0):
    colourList.append( [red, green, blue] )
    blue -=1

numColours = len(colourList)

print "Number of colours: ", numColours

#Angle calculation 
angleIncr = (2*math.pi)/ numColours
angle = angleIncr
quadAngle = angle
countingUp = True
quadrant = 0

print "Pi by 2:", math.pi/2

posRatio = True
for i in range(numColours):
    ratio = int(math.tan(quadAngle)*1024)


    if (ratio >= 65535): ratio = 65535
    colourList[i].insert(0,str(ratio))
    colourList[i].append(quadrant)
    colourList[i].append(angle)
    #quadAngle -= quadrant*(math.pi/2)
    angle += angleIncr
    quadAngle += angleIncr

#Split colour list into quadrants...
lut0 = []
lut1 = []
lut2 = []
lut3 = []

for item in colourList:
    if item[4] == 0: lut0.append(item[0:5])
    elif item[4] == 1: lut1.append(item[0:5])
    elif item[4] == 2: lut2.append(item[0:5])
    else: lut3.append(item[0:5])

#Quadrants:
# 0 : x > 0, y > 0
# 1 : x < 0, y > 0
# 2 : x < 0, y < 0
# 3 : x > 0, y < 0

#Reverse LUT 2 and LUT 4:

lut1Copy = lut1
lut3Copy = lut3

#lut1.reverse()  #TODO REREVERSE THE SUB LISTS
#lut3.reverse()


#for i in len(lut1):
#    lut1[i][1:3] = lut1Copy[i][1:3]
#    lut3[i][1:3] = lut3Copy[i][1:3]



#FIXME: Do we need a special case for when theta approaches pi/2 and tan goes to infinity? Will this overflow a variable?

print "\nLUT 1: dim[",len(lut0),"][4]\n" 
for item in lut0:
    print '{', item[0], ',', item[1],',', item[2],',', item[3], '},'
print "\nLUT 2: dim[",len(lut1),"][4]\n" 
for item in lut1:
    print '{', item[0], ',', item[1],',', item[2],',', item[3], '},'
print "\nLUT 3: dim[",len(lut2),"][4]\n" 
for item in lut2:
    print '{', item[0], ',', item[1],',', item[2],',', item[3], '},'
print "\nLUT 4: dim[",len(lut3),"][4]\n" 
for item in lut3:
    print '{', item[0], ',', item[1],',', item[2],',', item[3], '},'

#print
#for item in lut0:   
#    print item
#for item in lut1:   
#    print item
#for item in lut2:   
#    print item
#for item in lut3:   
#    print item

#
#static char *DirectionPosition[9][5] = {
#    {"00", "10", "",   "01", ""  },
#    {"01", "11", "",   "02", "00"},
#    {"02", "12", "",   "03", "01"},
#    {"03", "13", "",   "04", "02"},
#    {"04", "14", "",   "",   "03"},
#    {"10", "20", "00", "11", ""  },
#    {"11", "21", "01", "12", "10"},
#    {"12", "22", "02", "13", "11"},
#    {"44", "",   "34", "",   "43"}
#};





