import graphlab as gl
import random as rn
from graphlab import SGraph, Vertex, Edge, aggregate
import time

def count(g):
	gf = g.vertices
	l_count =  gf.groupby(key_columns='label',operations={'count': aggregate.COUNT()})
	return l_count

def max(g):
	gf = g.vertices
	l_max = gf.groupby(key_columns='')

#function for label propagation
def LPA(g,src):
	#group the neighbors according to the labels
	
	#count the labels
	#start=time.time()
	#src_max = 
	#nseSrc = g.get_edges(src_ids=[src])
	#nseDst = g.get_edges(dst_ids=[src])
	nseOr = g.get_edges()
	#print(nseOr)
	merge = nseOr[(nseOr['__src_id'] == src) | (nseOr['__dst_id'] == src)]
	#print(merge)#merge = nseSrc.append(nseDst)
	#print(merge['__src_id'])
	#print(merge[(merge['__src_id'] != src) & (merge['__dst_id'] != src)])# or merge['__dst_id'] != src]

	#print(merge)
	merge2 = merge[merge['__src_id'] != src]
	#print(merge2)
	arr1 = merge2['__src_id']
	#print(arr1)
	#print(merge)
	merge3 = merge[merge['__dst_id'] != src]
	arr2 = merge3['__dst_id']
	#print(arr2)
	arr1=arr1.append(arr2)
	#print(arr1)
	nsv = g.get_vertices(ids=arr1)



	#print(nsv)
	l_count = nsv.groupby(key_columns='label',operations={'count':aggregate.COUNT()})
	#print(l_count)
	maxim = l_count['count'].max()
	#print(maxim)
	max_labels = l_count[l_count['count'] == maxim]['label']

	#print(len(max_labels))
	
	#end=time.time()
	#print(end-start)
	#print(max_labels)
	#print(max_labels)
	#start=time.time()
	
	random_label = rn.sample(range(0,len(max_labels)),1)
	
	#print(max_labels[random_label[0]])
	#if s.get_vertices
	return max_labels[random_label[0]]
	#end=time.time()
	#print(end-start)

	#start=time.time()

	#max_label = max_labels[random_label]['label']
	#end=time.time()
	#print(end-start)
	#print(max_label)
	#return max_label

def main():
	g = SGraph()

	verts = []

	#initialize the Karate graph with unique label fields for each node
	for i in range(0,34):
		verts.append(Vertex(i,attr={'label': str(i)}))


	g = g.add_vertices(verts)


	#prepare the path for the Karate network data
	fname = "./karate.txt"
	#read the edges from Karate.txt and add them to the SGraph object
	with open(fname) as f:
		
		for l in f:
			#print(l)
			
			#parse the src and dst ids for the next edge
			ids=l.split()
			src= int(ids[0])
			dst= int(ids[1])

			#add the edge as a graphlabl.Edge object to the graph
			g = g.add_edges(Edge(src,dst))
			

	#visualize the graph
	#print(g.summary())
	#randId=rn.sample(range(0,34),1)[0]
	#print(randId)
	#test = g.get_vertices(fields={'label':'1'})[randId]
	#test.show()
	
	#print(test)
	ids = range(0,34)

	#label propagation loop
	flag=False

	iteration=0
	#rounds=5

	#initialize neigh dict for performance
	gns = {}
	cur_max=0
	start=time.time()
	#start=time.time()
	while flag==False :
		#pick vertice iteration order randomly
		rn.shuffle(ids)
		flag=True
		#print(ids)
		start=time.time()
		for index in ids:
			#print(index)
			
			
			cur_max = LPA(g,index)
			if str(cur_max) != g.get_vertices(ids=[index])['label'][0]:
				flag=False
				g.vertices['label'] = g.vertices.apply(lambda x: str(cur_max) if x['__id'] == index else x['label'])
			
		#print(end-start)
		iteration += 1	
		print(iteration)
	end=time.time()
	#end=time.time()
	print(end-start)
	print iteration
	g.show(vlabel='label')
	
main()