import numpy as np
import pprint
import random
import sys

def rebuild_graph():
	with open("graph.txt", "rb") as f:
		graph = {
			x.split(":",1)[0]:[y for y in x.split(":",1)[1].split(",") if len(y)>0] 
			for x in f.read().split("\n") if len(x)>0
		}
	
	edges = []
	for n in graph:
		edges += [[n,x] for x in graph[n]]

	return (graph, edges)

def parse_data(graph):
	with open("data.txt", "rb") as f:
		temp = {
			",".join(x.split(":",2)[0:2]):[float(p) for p in (",".join([y for y in x.split(":",2)[2].split(":") if len(y)>0])).split(",")]
			for x in f.read().split("\n") if len(x)>0
		}
		nodes = graph.keys()
		iterations = len(temp)/len(nodes)
		data = {
			i: {
				n:temp[n+","+str(i)] for n in nodes
			} 
			for i in range(iterations/4, 3*iterations/4)
		}
	# pprint.pprint(data)
	return data

def make_pairwise():
	graph,edges = rebuild_graph()

	random.shuffle(edges)

	data = parse_data(graph)

	iterations = data.keys()

	final_data = {i:[] for i in iterations}

	for index in iterations:
		random.shuffle(edges)
		
		for e in edges[:500]:
			temp = data[index][e[0]]+data[index][e[1]]
			temp.append(1)
			final_data[index].append(temp)

		nodes = data[index].keys()

		for i in range(len(nodes[:25])):
			for j in range(i+1,len(nodes[:25])):			
				temp = data[index][nodes[i]]+data[index][nodes[j]]
				if(nodes[j] not in graph[nodes[i]]):
					temp.append(0)
					final_data[index].append(temp)
		
		random.shuffle(final_data[index])

	data_arr = []
	for index in final_data:
		data_arr += final_data[index]

	data_arr = np.array(data_arr)
	labels = np.array([int(x) for x in data_arr[:,-1]]).astype(int)
		
	filename = sys.argv[1]

	np.savetxt("outputs/"+filename+"_data.txt", data_arr[:,:-1])
	np.savetxt("outputs/"+filename+"_labels.txt", labels, fmt="%d")

	
make_pairwise()