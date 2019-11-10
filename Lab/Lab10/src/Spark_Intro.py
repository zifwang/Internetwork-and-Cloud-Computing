#!/usr/bin/env python
# coding: utf-8

# ## Spark Context

# In[1]:


import pyspark
from pyspark import SparkContext
sc = SparkContext()


# In[2]:


nums= sc.parallelize([1,2,3,4])


# In[3]:


nums.take(1)


# In[4]:


squared = nums.map(lambda x: x*x).collect()


# In[5]:


squared


# In[6]:


for num in squared:
    print('%i ' % (num))


# ## Spark SQL Contest

# In[7]:


from pyspark.sql import Row
from pyspark.sql import SQLContext

sqlContext = SQLContext(sc)


# In[8]:


# Create the list of tuple with the information
list_p = [('John',19),('Smith',29),('Adam',35),('Henry',50)]


# In[9]:


# Build a RDD
rdd = sc.parallelize(list_p)


# In[11]:


# Convert the tuples
ppl = rdd.map(lambda x: Row(name=x[0], age=int(x[1])))


# In[12]:


# Create a DataFrame context
DF_ppl = sqlContext.createDataFrame(ppl)


# In[13]:


DF_ppl.printSchema()


# In[ ]:




