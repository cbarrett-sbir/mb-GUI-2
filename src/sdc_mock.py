import serial
import time

cntrlr = serial.Serial('COM5', 19200, timeout=None)
setPoint = 10.0
readyWindow = 0.1
address = "192.168.200.101"
print(setPoint)
temp = 0.0
while True:
	cmd = cntrlr.readline()
	print("slave: ".encode() + cmd)
	if cmd == b'MDA\r\n':
		cntrlr.write(("DA= {:.1f}\n".format(setPoint)).encode())

	elif cmd == b'M2\r\n':
		cntrlr.write(("T2= {:.1f}\n".format(temp)).encode())

	elif cmd[:3] == b'DA ':
		setPoint = float(cmd[3:-2])
		#print("Setpoint", setPoint)

	elif cmd[:2] == b'MS':
		if temp < (setPoint - readyWindow) or temp > (setPoint + readyWindow):
			cntrlr.write(("SR= 16\n").encode())
		else:
			cntrlr.write(("SR= 0\n").encode())

	# request for ready window
	elif cmd[:2] == b'ML':
		cntrlr.write(("L= {:.1f}\n".format(readyWindow)).encode())
		print("master: ".encode() + ("L= {:.1f}\n".format(readyWindow)).encode())

	# set ready window
	elif cmd[:1] == b'L':
		readyWindow = float(cmd[2: -2])

	# request for ip address
	elif cmd[:5] == b'MADDR':
		cntrlr.write((("ADDR= {}\n".format(address)).encode()))
		print("master: ".encode() + ("ADDR= {}\r\n".format(address)).encode())
	
	# set ip address
	elif cmd[:4] == b'ADDR':
		address = cmd.decode()[5:-1]

	else:	# default
		#print(cmd)
		continue
	if temp < (setPoint - 0.05): # subtract buffer for float errors
		temp += 0.1
	elif temp > (setPoint + 0.05):
		temp -= 0.1
