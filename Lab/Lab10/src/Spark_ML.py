#!/usr/bin/env python
# coding: utf-8

# In[1]:


import pyspark
from pyspark import SparkContext
from pyspark import SparkFiles
from pyspark.ml.feature import StringIndexer, OneHotEncoder, VectorAssembler
from pyspark.ml import Pipeline
from pyspark.ml.feature import OneHotEncoderEstimator
from pyspark.ml.linalg import DenseVector
from pyspark.ml.classification import LogisticRegression
from pyspark.ml.evaluation import BinaryClassificationEvaluator
from pyspark.ml.tuning import ParamGridBuilder, CrossValidator
from pyspark.sql import Row
from pyspark.sql import SQLContext
from pyspark.sql.types import *
from pyspark.sql.functions import *
from time import *


# ## Basic operation with PySpark

# In[2]:


url = "https://raw.githubusercontent.com/guru99-edu/R-Programming/master/adult_data.csv"


# In[3]:


sc = SparkContext()
sc.addFile(url)
sqlContext = SQLContext(sc)


# In[4]:


# df with inderShema to True
df = sqlContext.read.csv(SparkFiles.get("adult_data.csv"), header=True, inferSchema= True)


# In[5]:


df.printSchema()


# In[6]:


# see the data with show
df.limit(5).toPandas()


# In[7]:


# df with inderShema to False
df_string = sqlContext.read.csv(SparkFiles.get("adult_data.csv"), header=True, inferSchema=False)


# In[8]:


df_string.printSchema()


# In[9]:


df_string.limit(5).toPandas()


# In[10]:


# Write a custom function to convert the data type of DataFrame columns
def convertColumn(df, names, newType):
    for name in names: 
        df = df.withColumn(name, df[name].cast(newType))
    return df 

# List of continuous features
CONTI_FEATURES  = ['age', 'fnlwgt','capital-gain', 'educational-num', 'capital-loss', 'hours-per-week']

# Convert the type
df_string = convertColumn(df_string, CONTI_FEATURES, FloatType())

# Check the dataset
df_string.printSchema()


# In[11]:


# Select columns
df_string.select('age','fnlwgt').show(5)


# In[12]:


"""
Count by group
If you want to count the number of occurence by group, you can chain:

1.groupBy()
2.count()
together. In the example below, you count the number of rows by the education level.
"""
df_string.groupBy("education").count().sort("count",ascending=True).show()


# In[13]:


"""
Describe the data
To get a summary statistics, of the data, you can use describe(). It will compute the :

1.count
2.mean
3.standarddeviation
4.min
5.max
"""
df_string.describe().show()


# In[14]:


df_string.describe('capital-gain').show()


# In[15]:


"""
Crosstab computation
In some occasion, it can be interesting to see the descriptive statistics between two pairwise columns. For instance, you can count the number of people with income below or above 50k by education level. This operation is called a crosstab.
"""
df_string.crosstab('age', 'income').sort("age_income").show()


# In[16]:


"""
Drop column
There are two intuitive API to drop columns:

drop(): Drop a column
dropna(): Drop NA's
"""
df_string.drop('educational-num').columns


# In[17]:


"""
Filter data
You can use filter() to apply descriptive statistics in a subset of data. For instance, you can count the number of people above 40 year old
"""
df_string.filter(df_string.age > 40).count()


# In[18]:


df.groupby('marital-status').agg({'capital-gain': 'mean'}).show()


# ## Data preprocessing

# In[19]:


# 1 Select the column
age_square = df_string.select(col("age")**2)

# 2 Apply the transformation and add it to the DataFrame
df_string = df_string.withColumn("age_square", col("age")**2)

df_string.printSchema()


# In[20]:


df_string = df_string.withColumnRenamed("native-country", "native_country")
COLUMNS = ['age', 'age_square', 'workclass', 'fnlwgt', 'education', 'educational-num', 'marital-status',
           'occupation', 'relationship', 'race', 'gender', 'capital-gain', 'capital-loss',
           'hours-per-week', 'native_country', 'income']
df_string = df_string.select(COLUMNS)
df_string.first()


# In[21]:


"""
Exclude Holand-Netherlands

When a group within a feature has only one observation, it brings no information to the model. On the contrary, it can lead to an error during the cross-validation.

Let's check the origin of the household
"""
df_string.filter(df_string.native_country == 'Holand-Netherlands').count()
df_string.groupby('native_country').agg({'native_country': 'count'}).sort(asc("count(native_country)")).show()


# In[22]:


df_remove = df_string.filter(df_string.native_country != 'Holand-Netherlands')


# ## Build a data processing pipeline

# In[23]:


# First of all, you select the string column to index. The inputCol is the name of the column in the dataset. outputCol is the new name given to the transformed column.
stringIndexer = StringIndexer(inputCol="workclass", outputCol="workclass_encoded")

# Fit the data and transform it
model = stringIndexer.fit(df_string)
indexed = model.transform(df_string)

# Create the news columns based on the group. 
encoder = OneHotEncoder(dropLast=False, inputCol="workclass_encoded", outputCol="workclass_vec")
encoded = encoder.transform(indexed)
encoded.show(2)


# In[24]:


# Encode the categorical data
categorical_variables = ['workclass', 'education', 'marital-status', 'occupation', 'relationship', 'race', 'gender', 'native_country']

indexers = [StringIndexer(inputCol=column, outputCol=column+"-index") for column in categorical_variables]

encoder = OneHotEncoderEstimator(
    inputCols=[indexer.getOutputCol() for indexer in indexers],
    outputCols=["{0}-encoded".format(indexer.getOutputCol()) for indexer in indexers]
)
assembler = VectorAssembler(
    inputCols=encoder.getOutputCols(),
    outputCol="categorical-features"
)


# In[25]:


# # Create a Pipeline.
pipeline = Pipeline(stages=indexers + [encoder, assembler])
pipelineModel = pipeline.fit(df_remove)
model = pipelineModel.transform(df_remove)


# In[26]:


model.printSchema()


# In[27]:


df_data = model.limit(5).toPandas()


# In[28]:


continuous_variables = ['age', 'fnlwgt', 'educational-num', 'capital-gain', 'capital-loss', 'hours-per-week']
assembler = VectorAssembler(
    inputCols=['categorical-features', *continuous_variables],
    outputCol='features'
)
train_df = assembler.transform(model)


# In[29]:


train_df.printSchema()


# In[30]:


train_df.limit(5).toPandas()['features'][0]


# In[31]:


indexer = StringIndexer(inputCol='income', outputCol='label')
train_df = indexer.fit(train_df).transform(train_df)


# In[32]:


# Split the data into train and test sets
train_data, test_data = train_df.randomSplit([.8,.2],seed=1234)


# In[33]:


lr = LogisticRegression(featuresCol='features', labelCol='label')
model = lr.fit(train_df)


# In[34]:


# Print the coefficients and intercept for logistic regression
print("Coefficients: " + str(model.coefficients))
print("Intercept: " + str(model.intercept))


# ## Train and evaluate the model

# In[35]:


predictions = model.transform(test_data)


# In[36]:


predictions.printSchema()


# In[37]:


predictions.limit(10).toPandas()[['label', 'prediction']]


# In[38]:


def accuracy_m(model): 
    predictions = model.transform(test_data)
    cm = predictions.select("label", "prediction")
    acc = cm.filter(cm.label == cm.prediction).count() / cm.count()
    print("Model accuracy: %.3f%%" % (acc * 100)) 
accuracy_m(model = model)


# In[39]:


evaluator = BinaryClassificationEvaluator(rawPredictionCol="rawPrediction")
print(evaluator.evaluate(predictions))
print(evaluator.getMetricName())


# In[ ]:




