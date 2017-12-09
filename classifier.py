import pandas as pd
from sklearn.neighbors import KNeighborsClassifier as KNN
import random

with open("readings.py") as f:
	lines = f.read().split("\n")
	rows = []
	curx, cury = (0,0)
	count = 0
	counts = {}
	for i in range(len(lines)):
		line = lines[i]
		if len(line) < 6:
			curx = line.split(",")[0]
			cury = line.split(",")[1]
		elif line.find("---") < 0:
			readings = line.split("/")
			for read in readings[1:]:
				rows.append([count, read.split("*")[0], int(read.split("*")[1]) / -90 * 100])
			counts[count] = (curx, cury)
			count += 1

df = pd.DataFrame(rows, columns=["num", "bssid", "rssi"])
df = df.pivot(index='num', columns='bssid', values='rssi')
df = df.fillna(63)
df = df.drop("00:22:6B:5F:28:7E", axis=1)

df["num"] = df.index
df["pos"] = df["num"].apply(lambda x: str(counts[x][0]) + str(counts[x][1]))
df = df.drop("num", axis=1)

nn = KNN(n_neighbors=1, weights="distance")
nn.fit(df.drop("pos", axis=1), df["pos"]) 
ll = df.drop("pos", axis=1).iloc[100]
ll[:] = 63
kk = ll
#tar = #"/B4:E9:B0:E4:F7:12*-61/B4:E9:B0:E4:F7:11*-60/B4:E9:B0:E4:F7:13*-60/B4:E9:B0:E4:F7:10*-60/00:22:6B:5F:28:7E*-48/F0:29:29:92:26:A0*-74/02:2F:3F:43:52:3D*-76"
#tar = "/00:22:6B:5F:28:7E*-62/B4:E9:B0:E4:F7:11*-64/02:2F:3F:43:52:3D*-73/F0:29:29:92:25:A1*-72/F0:29:29:92:25:A3*-72/F0:29:29:92:25:A0*-72/F0:29:29:92:25:A4*-71/F0:29:29:92:25:A2*-72"
#tar = "/B4:E9:B0:E4:F7:10*-63/B4:E9:B0:E4:F7:14*-64/B4:E9:B0:E4:F7:13*-66/00:22:6B:5F:28:7E*-72/02:2F:3F:43:52:3D*-75"
#"/B4:E9:B0:E4:F7:11*-61/B4:E9:B0:E4:F7:10*-64/00:22:6B:5F:28:7E*-9/B4:E9:B0:E4:F7:12*-64"
#"/00:22:6B:5F:28:7E*-9/B4:E9:B0:E4:F7:14*-63/B4:E9:B0:E4:F7:11*-62"
#"/00:22:6B:5F:28:7E*-78/80:2A:A8:1A:A9:0A*-74/F0:29:29:92:25:A1*-61/F0:29:29:92:25:A3*-61/F0:29:29:92:25:A0*-61/F0:29:29:92:25:A4*-61/F0:29:29:92:25:A2*-61"

stuff = '''['5,27']
/00:22:6B:5F:28:7E*-70/80:2A:A8:1A:A9:0A*-68/F0:29:29:92:26:A1*-66/F0:29:29:92:26:A3*-67/F0:29:29:92:26:A0*-65/F0:29:29:92:26:A4*-66/F0:29:29:92:26:A2*-66/02:2F:3F:43:52:3D*-77/F0:29:29:92:25:A0*-65/F0:29:29:92:25:A4*-65
['5,27']
/00:22:6B:5F:28:7E*-66/80:2A:A8:1A:A9:0A*-69/F0:29:29:92:26:A4*-66/02:2F:3F:43:52:3D*-75/F0:29:29:92:25:A2*-66/F0:29:29:92:25:A1*-64/F0:29:29:92:25:A3*-64
['5,27']
/00:22:6B:5F:28:7E*-69/F0:29:29:92:26:A4*-66/F0:29:29:92:26:A2*-66/F0:29:29:92:26:A1*-66/F0:29:29:92:26:A3*-67/80:2A:A8:1A:A9:0A*-69/F0:29:29:92:26:A0*-66/02:2F:3F:43:52:3D*-77/F0:29:29:92:25:A0*-66
['5,27']
/00:22:6B:5F:28:7E*-73/80:2A:A8:1A:A9:0A*-66/F0:29:29:92:26:A4*-66/F0:29:29:92:26:A2*-67/F0:29:29:92:26:A1*-65/F0:29:29:92:26:A3*-66/02:2F:3F:43:52:3D*-77/F0:29:29:92:25:A1*-66/F0:29:29:92:25:A3*-64/F0:29:29:92:25:A0*-66
['5,27']
/00:22:6B:5F:28:7E*-67/80:2A:A8:1A:A9:0A*-68/F0:29:29:92:26:A0*-67
['5,27']
/B4:E9:B0:E4:F7:10*-65/B4:E9:B0:E4:F7:14*-66/00:22:6B:5F:28:7E*-66/80:2A:A8:1A:A9:0A*-75/00:25:9C:28:3D:EB*-81/C0:56:27:4C:F1:BC*-84/48:F8:B3:24:A8:3F*-73/02:2F:3F:43:52:3D*-84/F0:29:29:92:25:A1*-63/F0:29:29:92:25:A3*-65
'''
# for tar in stuff.split("\n")[:-1]:
# 	kk = ll.copy()
# 	if tar[0] == "[":
# 		continue
for cc in tar.split("/")[1:]:
	if cc.split("*")[0] in kk.index:
		kk[cc.split("*")[0]] = int(cc.split("*")[1]) / -90 * 100

print(nn.predict([kk]))


# from flask import Flask, jsonify, request, current_app, send_from_directory

# app = Flask(__name__, static_url_path="")

# last = "0,0"
# @app.route("/loc")
# def get_loc():
# 	return last

# @app.route('/index.html')
# def static_file():
# 	return send_from_directory('.', "index.html")	

# @app.route("/data")
# def uploadLocation():
# 	#try:
# 	kk = ll.copy()
# 	#tar = "/F0:29:29:92:6B:54*-55/F0:29:29:92:6B:52*-56/F0:29:29:92:6B:51*-56/F0:29:29:92:6B:53*-56/F0:29:29:92:6B:50*-55/00:1E:E5:85:F7:72*-63/FA:8F:CA:36:EA:D6*-64/C0:56:27:3A:34:29*-69/00:25:9C:28:3D:EB*-60/00:21:29:8A:FC:F3*-61"
# 	tar = request.args.get("data", "/")
# 	for cc in tar.split("/")[1:]:
# 		if cc.split("*")[0] in kk.index:
# 			kk[cc.split("*")[0]] = int(cc.split("*")[1])
# 	tmp = nn.predict([kk])[0]
# 	last = tmp
# 	return str(tmp)
# 	# except:
# 	# 	return "Badddd"
# app.run(host='0.0.0.0', use_reloader=False, threaded=True, port=5001)

