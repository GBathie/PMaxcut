import matplotlib.pyplot as plt
import tikzplotlib as tpl
import numpy as np
import sys

def read_data(filename):
	f = open(filename, 'r')
	li = []
	name = ""
	p = ""
	x,y,z = [],[],[]
	txt = f.readlines()[0:] + ["F /a 1 a a a "] #otherwise we forget a line
	for l in txt:
		# print(l)
		if l[0] == 'F':
			li.append((name, x, y, z, p))
			x,y,z = [],[],[]
			name = l.split(" ")[1].split("/")[-1]
			p = l.split()[2]
		else:
			_, a,b,c = l.split()
			x.append(float(a))
			y.append(float(b))
			z.append(float(c))
	return li[1:]

def array_from(data):
	print("\t\\hline")
	print("\tFolder & p & m ILP/max& m lin/maxcut & failures/total\\\\")
	print("\t\\hline")
	l = []
	for d in data:
		name, x, y, z, p = d
		ax, ay, az = np.array(x), np.array(y), np.array(z)
		q1, q2 = np.average(ax/az), np.average(ay/az)
		count = np.count_nonzero(np.abs(ay - az) > 1)
		n = len(x)
		l.append("\t{} & {} & {:.3f} & {:.3f} & {}/{}\\\\".format(name, p, q1, q2, count, n))
	l.sort()
	for w in l:
		print(w)

def boxplot(data):
	lxy = {}
	lzy = {}
	n 	= {}
	for d in data:
		name, x, y, z, p = d
		ax, ay, az = np.array(x), np.array(y), np.array(z)
		if name not in lxy:
			lxy[name] = []
			lzy[name] = []
			n[name] = []
		lxy[name].append(ax/az) 
		lzy[name].append(ay/az)
		n[name].append(p)
		# print(n[name])
	# print(lxy, lzy, n)
	for name in lxy:
		print(n[name])
		plt.figure()
		plt.title("Ratio maxcut/p-maxcut for " + name)
		plt.boxplot(lxy[name], labels=n[name],showfliers=True)
		# plt.show()
		tpl.save("boxplot_xy_" + name +"_outliers.tex")
		
		plt.figure()
		plt.title("Ratio p-maxcut*/p-maxcut for  " + name)
		plt.boxplot(lzy[name], labels=n[name],showfliers=True)
		# print(lzy[name])
		# plt.show()
		# plt.savefig("boxplot_zy"+p+".pdf")
		tpl.save("boxplot_zy_"+name+"_outliers.tex")

def array():
	data = read_data(sys.argv[1])
	print(data)
	array_from(data)
	boxplot(data)



if __name__ == "__main__":
	array()