#python file
import serial
import csv
import time

#ser = serial.Serial('/dev/ttyAMA1', 115200)
#with open('milestone3.csv') as csv_file:
    #csv_reader = csv.reader(csv_file, delimiter=',')
    #for line in csv_reader:
       # angle = line[0]
        #speed = line[1]
        #time = line[2]
        #command_str = str(angle) + ' ' +  str(speed) + ' ' + str(time)
        #print(command_str)
        #ser.write(bywith open("commands.csv", newline="") as csvfile:
ser1 = serial.Serial("/dev/ttyAMA1", 115200)
reader = csv.reader(csvfile)
for row in reader:
        print(row)
        # angle, speed, duration
        # command = f"a:{row[0]};s:{row[1]};d:{row[2]};"
        command = f"a:{row[0]};s:{row[1]};"
        ser1.write(command.encode())
        time.sleep(float(row[2])), duration
        # command = f"a:{row[0]};s:{row[1]};d:{row[2]};"
        command = f"a:{row[0]};s:{row[1]};"
        ser1.write(command.encode())
        time.sleep(float(row[2]))