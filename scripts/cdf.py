import numpy as np
import scipy as stats
import matplotlib.pyplot as plt
with open('20.txt', 'r') as myfile:
    data=myfile.read()
#float(data)

#data = np.loadtxt('x.txt'.split(""))
#sorted_data = np.sort(data)
sorted_data = stats.sort(data)
yvals=np.arange(len(sorted_data))/float(len(sorted_data)-1)

plt.plot(sorted_data,yvals)

plt.show()
