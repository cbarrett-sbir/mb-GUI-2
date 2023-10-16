import serial
import time

#cntrlr = serial.Serial('COM5', 19200, timeout=None)
cntrlr = serial.Serial('/dev/tty.usbmodem93491801', 19200, timeout=None)
setPoint = 0.0
readyWindow = 0.1
address = "192.168.200.101"
i = 1
errdev = "78"
errorString = "ERROR {}".format(i)
status = 32;
print(setPoint)
temp = 0.0
while True:
	cmd = cntrlr.readline()
	print(cmd)
	if cmd == b'MDA\r\n':
		cntrlr.write(("DA= {:.1f}\n".format(setPoint)).encode())

	elif cmd == b'M2\r\n':	
		cntrlr.write(("T2= {:.1f}\n".format(temp)).encode())

	elif cmd[:3] == b'DA ':
		setPoint = float(cmd[3:-2])

	elif cmd[:2] == b'MS':
		if temp < (setPoint - readyWindow) or temp > (setPoint + readyWindow):
			cntrlr.write(("SR= 16\n").encode())
		else:
			cntrlr.write(("SR= {}\n".format(status)).encode())

	# request for ready window
	elif cmd[:2] == b'ML':
		cntrlr.write(("L= {:.1f}\n".format(readyWindow)).encode())

	# set ready window
	elif cmd[:1] == b'L':
		readyWindow = float(cmd[2: -2])

	# request for ip address
	elif cmd[:5] == b'MADDR':
		cntrlr.write((("ADDR= {}\n".format(address)).encode()))
	
	# set ip address
	elif cmd[:4] == b'ADDR':
		address = cmd.decode()[6:-1]

	elif cmd[:7] == b'MERRDEV':
		cntrlr.write((("ERRDEV= {}\n".format(errdev)).encode()))

	elif cmd[:7] == b'MERRSTR':
		if i == 1:
			errstr = "ERS= Unexpected end of message, no data found\n"
		else:
			errstr = "ERS= No command match\n"
		cntrlr.write(errstr.encode())

	elif cmd[:6] == b'ERRCLR':
		i += 1
		if (i > 2):
			status = 0
			errdev = ""

	else:	# default
		#print(cmd)
		continue
	if temp < (setPoint - 0.05): # subtract buffer for float errors
		temp += 0.1
	elif temp > (setPoint + 0.05):
		temp -= 0.1
