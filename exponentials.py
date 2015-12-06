import matplotlib.pyplot as plt
import numpy as np
 
t = np.arange(0, 1, .001)

minFr = 400	#this should probably be the f0 of the pressed key, yes, that's exatly what it should be
maxFr = 20000

exponent = np.log(maxFr)

exp = np.exp(exponent*t)


plt.plot(t,exp)
# plt.axis([-.1,.1.1,-.8,.8])
# plt.xlabel('time')
# plt.ylabel('amplitude')
plt.show()
