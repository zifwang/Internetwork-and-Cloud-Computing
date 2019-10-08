#!/usr/bin/env python
# coding: utf-8

# In[15]:


import numpy as np
import pandas as pd
from sklearn.metrics import f1_score
import tensorflow as tf
import keras
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation


# In[6]:


# Read data by using pandas lib
df = pd.read_csv('data_banknote_authentication.txt', sep=",", header=None)


# In[29]:


def data_generator(df):
    """
        Function to generate training data and testing data
        Argument: df dataframe (type pd)
        Returns: 
                x_train_shuffled
                y_train_shuffled
                x_test_shuffled
                y_test_shuffled
    """
    pos_data_type_1 = 0
    for i in range (0,len(df[4])):
        if df[4][i] == 1:
            pos_data_type_1 = i
            break
    
    # Generate Data
    
    length = len(df)
    num_elements = len(df.keys())
    
    x_train = np.zeros((length-400,num_elements-1))
    y_train = np.zeros((length-400,1))
    x_test = np.zeros((400,num_elements-1))
    y_test = np.zeros((400,1))
    
    for i in range (0,num_elements-1):
        x_test[0:200,i] = df[i][0:200]
        x_test[200:,i] = df[i][pos_data_type_1:pos_data_type_1+200]
        x_train[0:pos_data_type_1-200,i] = df[i][200:pos_data_type_1]
        x_train[pos_data_type_1-200:,i] = df[i][pos_data_type_1+200:]
    
    y_test[0:200,0] = df[4][0:200]
    y_test[200:,0] = df[4][pos_data_type_1:pos_data_type_1+200]
    y_train[0:pos_data_type_1-200,0] = df[4][200:pos_data_type_1]
    y_train[pos_data_type_1-200:,0] = df[4][pos_data_type_1+200:]
    
    #  Shuffle
    pi_trained = np.random.permutation(x_train.shape[0])
    x_train_shuffled = x_train[pi_trained]
    y_train_shuffled = y_train[pi_trained]
    y_train_shuffled = y_train_shuffled.reshape(y_train_shuffled.shape[0],)
    
    pi_test = np.random.permutation(x_test.shape[0])
    x_test_shuffled = x_test[pi_test]
    y_test_shuffled = y_test[pi_test]
    y_test_shuffled = y_test_shuffled.reshape(y_test_shuffled.shape[0],)
    
    return x_train_shuffled, y_train_shuffled, x_test_shuffled, y_test_shuffled


# In[31]:


x_train, y_train, x_test, y_test = data_generator(df)
# Create DL model 
model = Sequential()
model.add(Dense(units=35, activation='relu', input_dim=x_train.shape[1]))
model.add(Dropout(0.5))
model.add(Dense(units=1, activation='sigmoid'))
model.compile(optimizer='Adam',
              loss='binary_crossentropy',
              metrics=['accuracy'])

hist = model.fit(x_train, y_train, epochs=25, batch_size=16, verbose = 1)


# In[35]:


test_score = model.evaluate(x_test, y_test)

print("Deep Learning Model:")
print("Training accuracy is: {:.3f}".format(hist.history['acc'][24]))
print("Test accuracy is: {}".format(test_score[1]))
print("Test f-score is: {}".format(test_score[1]))

