import os

import librosa
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.preprocessing import LabelEncoder, scale
data_path = r'C:\Users\Administrator\Desktop\team\sh19-main\Dataset'
data = pd.read_csv(os.path.join(data_path, 'dataset.csv'),on_bad_lines='skip')
print(data.shape)
sound_classes=["Smoke detector, smoke alarm","Walk, footsteps","Speech","Toilet flush","Bark","Hair dryer"
,"Water tap, faucet","Shatter","Crying, sobbing","Screaming","Telephone","Laughter","Music","Knock","Explosion"]
le = LabelEncoder()
le.fit(sound_classes)
data['label'] = le.transform(data['mid'])
import random
def parse_wav(data):
    n_mfcc = 20
    seconds = 6 
    t = 188
    all_mfcc = np.empty((0, n_mfcc, t))
    all_mfcc_m = np.empty((0, n_mfcc))
    all_mfcc_scale = np.empty((0, n_mfcc, t))
    labels = []

    for i, row in data.iterrows():
        id = row[0]
        print(id)
        wav_file = os.path.join(data_path, 'audio', str(id))
        time_series, sampling_rate = librosa.load(wav_file,sr =16000, res_type='kaiser_fast')
        for x in range(len(time_series) // (sampling_rate*seconds)):
            random_time_start = random.randint(0,len(time_series) -sampling_rate*seconds )
            mfcc = librosa.feature.mfcc(y=time_series[random_time_start : random_time_start +( sampling_rate*seconds)], sr=sampling_rate)
            mfcc_m = np.mean(mfcc, axis=1).T

            if mfcc.shape[1] < t:
                padding = np.zeros((n_mfcc, t - mfcc.shape[1]))
                mfcc = np.concatenate([mfcc, padding], axis=1)
            labels.append(data['label'][i])
            all_mfcc = np.vstack((all_mfcc, [mfcc]))
            all_mfcc_m = np.vstack((all_mfcc_m, [mfcc_m]))

            mfcc_scale = scale(mfcc)
            all_mfcc_scale = np.vstack((all_mfcc_scale, [mfcc_scale]))


    return all_mfcc, all_mfcc_m, all_mfcc_scale,labels
all_mfcc, all_mfcc_m, all_mfcc_scale,y = parse_wav(data)
print(all_mfcc.shape, all_mfcc_m.shape, all_mfcc_scale.shape,len(y))
print(all_mfcc_scale.shape)
my_str_2 = r'C:\Users\Administrator\Desktop\team\sh19-main\Dataset\npz\mfcc_scale1.npz'
np.savez(my_str_2, all_mfcc=all_mfcc, all_mfcc_m=all_mfcc_m, y=y, all_mfcc_scale=all_mfcc_scale)