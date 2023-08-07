import serial
import time

cntrlr = serial.Serial('COM5', 19200, timeout=None)
setPoint = 10.0
print(setPoint)
temp = 0.0
while True:
	cmd = cntrlr.readline()
	if cmd == b'MDA\r\n':
		cntrlr.write(("DA= {:.1f}\n".format(setPoint)).encode())
	elif cmd == b'M2\r\n':
		cntrlr.write(("T2= {:.1f}\n".format(temp)).encode())
	elif cmd[:3] == b'DA ':
		setPoint = float(cmd[3:-2])
		print("Setpoint", setPoint)
	elif cmd[:2] == b'MS':
		if temp < (setPoint - 0.05) or temp > (setPoint + 0.05):
			cntrlr.write(("SR= 16\n").encode())
	
		else:
			cntrlr.write(("SR= 0\n").encode())

	else:	# default
		print(cmd)
	if temp < (setPoint - 0.05): # subtract buffer for float errors
		temp += 0.1
	elif temp > (setPoint + 0.05):
		temp -= 0.1
