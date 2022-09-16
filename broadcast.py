import os
import re
import time
import signal
import string
import random
import requests
import simplejson as json
from bson.json_util import dumps
from flask_pymongo import PyMongo
from bson.objectid import ObjectId
from flask_cors import CORS, cross_origin
from collections import defaultdict, Iterable
from flask import Flask, flash, request, redirect, render_template ,jsonify

app=Flask(__name__)


cors = CORS(app, resources={

    r"/*": {
        "origins": "*"

    }
})

app.secret_key = "secret key"
#app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024

# Get current path
path = os.getcwd()
# file Upload
UPLOAD_FOLDER = os.path.join(path, '/var/www/html/uploads')

# Make directory if uploads is not exists
if not os.path.isdir(UPLOAD_FOLDER):
    os.mkdir(UPLOAD_FOLDER)

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Allowed extension you can set your own
ALLOWED_EXTENSIONS = set(['webp','m4v','vob', 'wmv', 'mov', 'mkv', 'webm', 'gif','mp4','avi','flv'])

import sys
import os.path
sys.path.append(os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.path.pardir)))

app.config['MONGO_DBNAME'] = 'photo_bomb'
app.config['MONGO_URI'] = 'mongodb://192.168.0.51:27017/photo_bomb'

mongo = PyMongo(app)

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


#BROADCAST UPON STARTING VIDEO SEARCH
@app.route('/videos/' , methods=['POST'])
def upload_file():
    if request.method == 'POST':
        query_list = []
        payload_response = {}
        payload_response_list = []

        #files = request.files.getlist('files[]')
        files = request.files.to_dict() 

        for file in files:
            if file and allowed_file(files[file].filename):
                _filename = files[file].filename
                file_extension = os.path.splitext(_filename)[1]
                nanotime=re.sub(r"\.", "", str(time.time()))
                filename = str(nanotime)+file_extension
                files[file].save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
                channel_name = str(nanotime)
                payload_response = {'id':str(channel_name),'videourl':"https://str.appb.casa:227/uploads/"+filename,'imageExtracted':[]}
                payload_response_list.append(payload_response)
                query = {str(channel_name+':/var/www/html/uploads/'+filename)}
                query_list.append(query)
        payload={'query':str(query_list)}
        print(payload)
        url = "http://127.0.0.1:5029"
        headers = {'Content-type': 'application/json'}
        requests.post(url, data=json.dumps(payload), headers=headers,timeout=10)
        return jsonify(payload_response_list)

#get video api
@app.route('/videos/api/file/<id>' , methods=['GET'])
def video(id):
    try:
        videos = mongo.db.videos
        output = []
        list_records = []
        for  video in videos.find({'id' : { "$eq" : id}}):
            output.append({'id' : video['id'], 'image' : video['image'], 'videourl' : video['videourl'], 'time' : video['time'], 'progress' : video['progress'], 'streamurl' : video['streamurl']})
        #print(output)
        dd = defaultdict(list)
        for d in output:
            for key, val in sorted(d.items()):
                dd[key].append(val)
            

        result = {}
        result_list = []
        image_list = []
        time_list = []
        progress_list = []

        for k,v in sorted(dd.items()):
            result[k]=list(set(v))
            if k == 'id':
                result[k] = ' '.join(set(map(str, v)))
            if k == 'videourl':
                result[k] = ' '.join(set(map(str, v)))
            if k=='streamurl':
                result[k] = ' '.join(set(map(str, v)))

            if k == 'image':
                for value in range(len(v)):
                    image_dict = {}
                    image_dict['image'] = v[value]
                    image_list.append(image_dict)
                result[k] = image_list
            
            if k == 'time':
                for value in range(len(v)):
                    time_dict = {}
                    time_dict['time'] = v[value]
                    time_list.append(time_dict)
                result[k] = time_list
            
            if k == 'progress':
                for value in range(len(v)):
                    progress_list.append(v[value])
                result[k] = progress_list[-1]
        kdict = {}
        m3u8_list = []
        for x,y in zip(result['image'],result['time']):
            kdict =  {**x, **y}
            m3u8_list.append(kdict)
        
        result['imageExtracted'] = result.pop('image')
        result['imageExtracted'] = m3u8_list
        result['videoTimestamp'] = result.pop('time')


        return jsonify(result)
    except KeyError:
        result ={"keyError":"No faces were found"}
        return jsonify(result)
        
if __name__ == "__main__":
    app.run(host='0.0.0.0',port=5005,use_reloader=False)
