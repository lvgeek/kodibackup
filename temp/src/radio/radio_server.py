import si4703
import time
import socket

fm = si4703.si4703()
fm.init()

UDP_IP = "127.0.0.1"
UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
volume = 0

while True:
	data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
	if len(data) == 0:
		print "datalen=0"
		time.sleep(1)
		continue;

	comm, subcomm = data.split('_')
	print comm
	print subcomm
	
	if comm == 'seek':
		if subcomm == 'right':
			print 'seek channel right'
			fm.seek_right()
		elif subcomm == 'left':
			print 'seek channel left'
			fm.seek_left()
		sock.sendto(str(fm.get_channel()), addr)
	elif comm == 'tune':
		if subcomm == 'right':
			fm.tune_right()
			print 'tuning right'
		elif subcomm == 'left':
			fm.tune_left()
			print 'tuning left'
		else:
			print 'tuning to '+subcomm
			fm.tune(subcomm)
		sock.sendto(str(fm.get_channel()), addr)
	elif comm == 'volume':
		if subcomm == 'plus':
			if volume < 15:
				volume = volume + 1
			subcomm = str(volume)
		elif subcomm == 'minus':
			if volume > 0:
				volume = volume - 1
				subcomm = str(volume)
		else:
			volume = int(subcomm)
		fm.volume(volume)
		sock.sendto(str(volume), addr)
		print 'set volume to '+subcomm
	elif comm == 'toggle':
		fm.toggle_mute()
		sock.sendto(subcomm, addr)
	elif comm == 'get':
		if subcomm == 'frequency':
			sock.sendto(str(fm.get_channel()), addr)
			print 'current frequency is '+str(fm.get_channel())
		elif subcomm == 'rds':
			fm.print_registry()
			sock.sendto(str(fm.get_channel()), addr)
		elif subcomm == 'volume':
			sock.sendto(str(volume), addr)
	
	time.sleep(0.1)

