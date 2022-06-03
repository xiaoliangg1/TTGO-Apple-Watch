import base64
from io import BytesIO
import matplotlib.pyplot as plt
import random

from flask import Flask
from flask import request

app = Flask(__name__)

FILE = "data.txt"

TIME = "Time"
TEMP = "Temperature"
HUMIDITY = "Humidity"
STEP = "Step Count"

FIGURE = plt.figure()
FIGURE.patch.set_facecolor('#FFFFFF')

@app.route("/")
def hello():
    time_str = request.args.get("time")
    temp_str = request.args.get("temp")
    humid_str = request.args.get("humid")
    step_str = request.args.get("step")
    if (time_str != None):
        with open(FILE, "a") as data:
            data_str = f"{time_str}\t{temp_str}\t{humid_str}\t{step_str}\n"
            data.write(data_str)
        return  TIME + ": " + time_str + "\n\t" + \
                TEMP + ": " + temp_str + "\n\t" + \
                HUMIDITY + ": " + humid_str + "\n\t" + \
                STEP + ": " + step_str + "\n"
    return "No Data Received"

@app.route("/plot")   
def plotData():
    time_data = []
    temp_data = []
    humidity_data = []
    step_data = []
    with open(FILE, "r") as file:
        for line in file:
            line_item = line.strip("\n").split("\t")
            if line_item[0] != None and line_item[0] != 'None':
                time_data.append(float(line_item[0]))
                temp_data.append(float(line_item[1]))
                humidity_data.append(float(line_item[2]))
                step_data.append(float(line_item[3]))
            else: # for testing on AWS
                j,k,l = random.randint(0,100), random.randint(0,100), random.randint(0,100)
                print(j, k, l)
                time_data = [i for i in range(0,10)]
                temp_data = [i for i in range(j-10, j)]
                humidity_data = [i for i in range(k-10, k)]
                random.shuffle(humidity_data)
                step_data = [i for i in range(l-10, l)]
                random.shuffle(step_data)
    plt.cla()
    plt.clf()
    a = FIGURE.subplots(2,2)

    a[0][0].plot(time_data, temp_data)
    a[0][0].set_title('Time vs. Temp')
    a[0][1].plot(time_data, humidity_data)
    a[0][1].set_title('Time vs. Humidity')
    a[1][0].plot(time_data, step_data)
    a[1][0].set_title('Time vs. Step Count')
    a[1][1].plot(temp_data, humidity_data)
    a[1][1].set_title('Temp vs. Humidity')

    FIGURE.tight_layout()

    img = BytesIO()
    FIGURE.savefig(img, format="png")

    plot_img = base64.b64encode(img.getbuffer()).decode("ascii")
    return f'<meta http-equiv="refresh" content="5" > \n <center><img src="data:image/png;base64,{plot_img}" style="overflow: hidden; height:98vh; max-width:100%; object-fit: contain;"/></center>'
