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
				rows.append([count, read.split("*")[0], read.split("*")[1]])
			counts[count] = (curx, cury)
			count += 1

df = pd.DataFrame(rows, columns=["num", "bssid", "rssi"])
df = df.pivot(index='num', columns='bssid', values='rssi')
df = df.fillna(0)
df = df.drop("00:22:6B:5F:28:7E", axis=1)
print(df)

df["num"] = df.index
df["pos"] = df["num"].apply(lambda x: str(counts[x][0]) + str(counts[x][1]))
df = df.drop("num", axis=1)
print(df)

nn = KNN(n_neighbors=5)
nn.fit(df.drop("pos", axis=1), df["pos"]) 
ll = df.drop("pos", axis=1).iloc[100]
ll[:] = 0

# tar = "/F0:29:29:92:6B:54*-55/F0:29:29:92:6B:52*-56/F0:29:29:92:6B:51*-56/F0:29:29:92:6B:53*-56/F0:29:29:92:6B:50*-55/00:1E:E5:85:F7:72*-63/FA:8F:CA:36:EA:D6*-64/C0:56:27:3A:34:29*-69/00:25:9C:28:3D:EB*-60/00:21:29:8A:FC:F3*-61"
# for cc in tar.split("/")[1:]:
# 	ll[0][cc.split("*")[0]] = int(cc.split("*")[1]) + random.randint(0,1 )-1
# print(nn.predict(ll))


from flask import Flask, jsonify, request, current_app, send_from_directory

app = Flask(__name__, static_url_path="")

@app.route("/data")
def uploadLocation():
	#try:
	kk = ll
	#tar = "/F0:29:29:92:6B:54*-55/F0:29:29:92:6B:52*-56/F0:29:29:92:6B:51*-56/F0:29:29:92:6B:53*-56/F0:29:29:92:6B:50*-55/00:1E:E5:85:F7:72*-63/FA:8F:CA:36:EA:D6*-64/C0:56:27:3A:34:29*-69/00:25:9C:28:3D:EB*-60/00:21:29:8A:FC:F3*-61"
	tar = request.args.get("data", "/")
	for cc in tar.split("/")[1:]:
		if cc.split("*")[0] in kk.index:
			kk[cc.split("*")[0]] = int(cc.split("*")[1])
	print(nn.predict([kk]))
	return str(nn.predict([kk]))
	# except:
	# 	return "Badddd"
app.run(host='localhost', use_reloader=False, threaded=True, port=5001)