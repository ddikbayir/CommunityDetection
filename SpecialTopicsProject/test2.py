import graphlab as gl
from graphlab import Vertex, Edge, SFrame, SGraph

g2 = SGraph()

verts = []

#initialize the Karate graph with unique label fields for each node
for i in range(34):
	verts.append(Vertex(i,attr={'label': str(i)}))

g2 = g2.add_vertices(verts)


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
		g2 = g2.add_edges(Edge(src,dst))
			

#visualize the graph
print(g2.summary())
#sf=g2.vertices
#test22 = sf[sf['__id'] == 5] #= 'test'
#test22['label'] = 'test'
#sf[sf['__id'] == 5] = test22
#test22.show()
#mutation test
#sf=g.vertices
#sf['label'] = sf.apply(lambda x: 'test' if x['__id'] == 1 else x['label'])
#sf[sf['__id'] == 1]['label']=sf['label'].apply(lambda x: 'test')
#neigh = g2.get_neighborhood([23])
#neigh.show(vlabel='label')
g2.show(vlabel='label') 