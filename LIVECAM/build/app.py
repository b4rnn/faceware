import os
import ast
import signal
import string
import random
import subprocess
import itertools
import operator
import itertools
import threading, queue
from flask import Flask, flash, request, redirect, render_template ,jsonify
from werkzeug.utils import secure_filename
from flask_pymongo import PyMongo
from flask_cors import CORS, cross_origin
from bson.objectid import ObjectId
from bson.json_util import dumps
from collections import defaultdict, Iterable

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

tf_queue = queue.Queue()
import sys
import os.path
sys.path.append(os.path.abspath(os.path.join(
    os.path.dirname(__file__), os.path.pardir)))

app.config['MONGO_DBNAME'] = 'photo_bomb'
app.config['MONGO_URI'] = 'mongodb://192.168.0.51:27017/photo_bomb'

mongo = PyMongo(app)

@app.route('/' , methods=['POST'])
def channels():
    if request.method == 'POST':

        dict=request.json
        id =dict.get('query')
        video_list = ast.literal_eval(id)
        for video in video_list:
            proc = subprocess.Popen(["./ENGINE",str(video)])
            print(list(video))
    return '200'

@app.route('/controls/api/pid/<id>' , methods=['POST'])
def processes(id):
    try:
        os.kill(int(id), signal.SIGTERM) #or signal.SIGKILL 
    except:
        print('process not found')
    return '200'
   
