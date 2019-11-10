#!/usr/bin/env python
# coding: utf-8

# In[17]:


import pyspark
from pyspark import SparkContext
from pyspark import SparkFiles
from pyspark.ml.feature import StringIndexer, OneHotEncoder, VectorAssembler
from pyspark.ml import Pipeline
from pyspark.ml.feature import OneHotEncoderEstimator
from pyspark.ml.linalg import DenseVector
from pyspark.ml.classification import LogisticRegression
from pyspark.mllib.classification import SVMWithSGD, SVMModel
from pyspark.mllib.regression import LabeledPoint
from pyspark.mllib.tree import RandomForest, RandomForestModel
from pyspark.mllib.util import MLUtils
from pyspark.mllib.evaluation import MulticlassMetrics
from pyspark.ml.evaluation import BinaryClassificationEvaluator
from pyspark.ml.tuning import ParamGridBuilder, CrossValidator
from pyspark.sql import Row
from pyspark.sql import SQLContext
from pyspark.sql.types import *
from pyspark.sql.functions import *
from time import *


# In[2]:


sc = SparkContext()
sqlContext = SQLContext(sc)


# In[65]:


# load an parse the data
def parsePoint(line):
    values = [float(x) for x in line.split(' ')]
    return values

def parsePoint_2(line):
    values = int(line)-1
    return values

# Get Training Data
x_train_str = sc.textFile('X_train.txt')
train_x = x_train_str.map(parsePoint)
train_y_str = sc.textFile('y_train.txt')
train_y = train_y_str.map(parsePoint_2)
# Get Testing Data
test_x_str = sc.textFile('X_test.txt')
test_x = test_x_str.map(parsePoint)
test_y_str = sc.textFile('y_test.txt')
test_y = test_y_str.map(parsePoint_2)


# In[68]:


# Format the training and testing data by labeledPoint
def formatData(x,y):
    data = []
    assert(len(x) == len(y))
    for i in range (len(x)):
        data.append(LabeledPoint(y[i],x[i]))
    
    return data

parsedData_train = formatData(train_x.collect(),train_y.collect())
train_data = sc.parallelize(parsedData_train)
parsedData_test = formatData(test_x.collect(),test_y.collect())
test_data = sc.parallelize(parsedData_test)


# In[74]:


model = RandomForest.trainClassifier(train_data, numClasses=12, categoricalFeaturesInfo={},
                                     numTrees=12, featureSubsetStrategy="auto",
                                     impurity='gini', maxDepth=4, maxBins=32, seed=42)


# In[75]:


print(model)
print(model.numTrees())
print(model.totalNumNodes())


# In[84]:


# Evaluate model on test instances and compute test error
predictions_test = model.predict(test_data.map(lambda x: x.features))
predictions_train = model.predict(train_data.map(lambda x: x.features))


# In[87]:


# accuracy
def accuracy(predcitions,y,length):
    count = 0
    for a, b in zip(predcitions,y):
        if a == b:
            count += 1
    return count/length*100

train_accuracy = accuracy(predictions_train.collect(),train_y.collect(),predictions_train.count())
test_accuracy = accuracy(predictions_test.collect(),test_y.collect(),predictions_test.count())


# In[88]:


print('train_accuracy: {}'.format(train_accuracy))
print('test_accuracy: {}'.format(test_accuracy))


# In[89]:


labelsAndPredictions = test_data.map(lambda lp: lp.label).zip(predictions_test)


# In[91]:


metrics = MulticlassMetrics(labelsAndPredictions)


# In[92]:


precision = metrics.precision()
recall = metrics.recall()
f1Score = metrics.fMeasure()
print("Summary Stats")
print("Precision = %s" % precision)
print("Recall = %s" % recall)
print("F1 Score = %s" % f1Score)


# In[ ]:




