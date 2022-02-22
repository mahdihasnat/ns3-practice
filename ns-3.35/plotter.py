import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

prots = ['speed']

def plot(dfs):

	col_names = dfs[0].keys().values.tolist()

	for i in range(1, len(col_names)):

		for df in dfs:
			time_col = df[col_names[0]].tolist()
			val_col = df[col_names[i]].tolist()
			ndf = pd.DataFrame(data=val_col, index=time_col, columns=[df.name])
			print(val_col)
			print(time_col)
			# plot bar graph , with annotation
			# plt.bar(time_col, val_col, label=df.name, width=1.5)
			splot=sns.barplot(x=time_col, y=val_col, data=ndf)
			for p in splot.patches:
				splot.annotate(format(p.get_height(), '.1f'), 
							(p.get_x() + p.get_width() / 2., p.get_height()), 
							ha = 'center', va = 'center', 
							xytext = (0, 9), 
							textcoords = 'offset points')
			
			
			# plt.legend()
			plt.title(col_names[i] + ' vs ' + col_names[0])
			plt.xlabel(col_names[0])
			plt.ylabel(col_names[i])
			plt.grid()
			# plt.show()
			plt.savefig(col_names[i] + ' vs ' + col_names[0] + '.png')
			plt.clf()

if __name__ == '__main__':
	dfs = []
	for prot in prots:
		df = pd.read_csv('' + prot + '_variation.dat')
		df.name = prot
		dfs.append(df)
	plot(dfs)
