import os
import ast
import signal
import string
import random
import subprocess
import itertools
import operator
import itertools
from werkzeug.utils import secure_filename
from flask_pymongo import PyMongo
from flask_cors import CORS, cross_origin
from bson.objectid import ObjectId
from bson.json_util import dumps
from flask import Flask, flash, request, redirect, render_template ,jsonify

app=Flask(__name__)

cors = CORS(app, resources={

    r"/*": {
        "origins": "*"

    }
})

app.secret_key = "secret key"
app.config['MONGO_DBNAME'] = 'photo_bomb'
app.config['MONGO_URI'] = 'mongodb://DATABASE_IP:27017/photo_bomb'
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
