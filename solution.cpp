/*
CodeChef Tool Chest
Copyright (C) 2013 Henrik Mühe

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <queue>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <valarray>
#include <vector>


static const uint64_t MaxNodes = 1000;

/// Union Find datastructure
/// Assumptions:
/// - Nodes are indexed by dense integer starting from 0
/// - Total number of nodes is known in advance
/// - Not thread safe
class UnionFind {
	/// Structure storing a node
	struct Node {
		Node* parent;
		uint32_t rank;
	};

	/// Store all nodes, must not resize!
	std::vector<Node> nodes;
	/// The number of partitions inside this structure
	uint64_t partitionCount;

public:
	/// Constructor
	UnionFind(uint64_t count) : partitionCount(count) {
		nodes.reserve(count);
		for (uint64_t index=0; index<count; ++index) {
			nodes.push_back(Node{nullptr,0});			
		}
	}

	/// Find the partition representative for a node
	Node* find(uint64_t index) {
		static std::array<Node*,64> path;
		uint64_t pathIndex=0;

		// Walk up to the root remembering the path
		Node* rep = &nodes[index];
		while (rep->parent) {
			path[pathIndex++]=rep;
			rep = rep->parent;
		}

		// Set parent of all nodes on the path to root (path compression)
		for (uint64_t index=0; index<pathIndex; ++index) {
			path[index]->parent=rep;
		}

		return rep;
	}

	/// Merge two partition trees
	void merge(Node* a, Node* b) {
		if (a==b) return;

		// We merge two partitions so the partition count will decrease
		--partitionCount;

		// Make the smaller tree a child of the larger one if possible (union by rank)
		if (a->rank > b->rank) {
			b->parent=a;
		} else if (b->rank > a->rank) {
			a->parent=b;
		} else {
			a->parent=b;
			++a->rank;
		}
	}

	/// Return the number of partitions
	uint64_t getPartitionCount() const { return partitionCount; }
};


// Adapted and simplified version of https://github.com/vitaut/format
static const char TwoDigitConversion[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

template<class T>
class FormatUnsigned {
	static const uint32_t MaxLength = std::numeric_limits<T>::digits10 + 1;
	char buffer[MaxLength];
	char* ptr;

public:
	FormatUnsigned(T value) {
		ptr = buffer+MaxLength;
		*--ptr = '\0';
		while (value >= 100) {
			unsigned index=(value%100) * 2;
			value/=100;
			*--ptr = TwoDigitConversion[index+1];
			*--ptr = TwoDigitConversion[index];
		}
		if (value < 10) {
			*--ptr = static_cast<char>('0'+value);
		} else {
		    unsigned index = static_cast<unsigned>(value * 2);
			*--ptr = TwoDigitConversion[index+1];
			*--ptr = TwoDigitConversion[index];
		}
	}

	const char* c_str() const { return ptr; }
};


class FixedWriter {
	/// The output buffer
	std::vector<char> buffer;
	/// The index inside the buffer
	uint64_t index;

public:
	/// Constructor
	FixedWriter(uint64_t maxOutputSize) : index(0) {
		buffer.resize(maxOutputSize);
	}
	/// Writes an int into the stream
	template<class T>
	void writeNumber(const T& value) {
		//index+=sprintf(&buffer[index],"%d",value);
		FormatUnsigned<T> i(value);
		for (const char* ptr=i.c_str(); *ptr!='\0'; ++ptr) {
			buffer[index++]=*ptr;
		}
	}
	/// Writes a character
	void writeChar(char c) {
		buffer[index++]=c;
	}
	/// Flush to fd
	void flush(int fd) {
		auto len=write(fd,buffer.data(),index);
		assert(len==index);
	}
};

/// Buffered reader used to efficiently read simple structured input from stdin
class BufferedReader {
	/// Size of the input buffer
	static const uint64_t BufferSize = 1024*200;
	/// The input fd
	int input;
	/// The buffer
	char buffer[BufferSize];
	/// The current and end pointer
	const char* currentPtr;
	const char* endPtr;

public:
	/// Constructor
	BufferedReader(int input) : input(input),currentPtr(buffer),endPtr(buffer) {}
	/// Read more data into buffer
	bool replenish() {
		auto len=read(input,buffer,BufferSize);
		endPtr=buffer+len;
		currentPtr=buffer;
		return len>0;
	}
	/// Reads a number from the buffer
	template<class T>
	T readNumber() {
		T number=0;
		while (true) {
			for (; currentPtr!=endPtr; ++currentPtr) {
				if (*currentPtr>='0'&&*currentPtr<='9') {
					number = (number*10) + (*currentPtr - '0');
				} else {
					goto done;
				}
			}
			if (!replenish()) break;
		}
		done:
		return number;
	}
	/// Reads space from the buffer
	void readSpace() {
		while (true) {
			for (; currentPtr!=endPtr; ++currentPtr) {
				if (*currentPtr!=' '&&*currentPtr!='\n') {
					goto done;
				}
			}
			if (!replenish()) 
				break;
		}
		done:
		return;
	}
};

/// Adjacency Graph
struct AdjacencyGraph {
	struct OutEdge {
		int32_t to;
		int32_t weight;
	};
	std::vector<std::list<OutEdge>> adjacency;
	uint64_t edgeCount;

	/// Constructor
	AdjacencyGraph(int32_t count) : edgeCount(0) { adjacency.resize(count); }
	/// Move constructor
	AdjacencyGraph(AdjacencyGraph&& other) : adjacency(std::move(other.adjacency)),edgeCount(other.edgeCount) { other.edgeCount=0; }
	/// Move assignment operator
	AdjacencyGraph& operator=(AdjacencyGraph&& other) { adjacency=std::move(other.adjacency); edgeCount=other.edgeCount; other.edgeCount=0; }
	/// Add an edge to the graph
	void addEdge(int32_t from,int32_t to,int32_t weight) {
		++edgeCount;
		adjacency[from].push_back(OutEdge{to,weight});
	}
	/// Display the graph
	void dump() const {
		for (uint64_t from=0;from<adjacency.size();++from) {
			for (auto& outEdge : adjacency[from]) {
				std::cout << from << " -- " << outEdge.to << "[color=red,labelfontcolor=red,label=\"" << outEdge.weight << "\"];" << std::endl;
			}
		}
	}
	/// Selector into the result matrix
	uint64_t at(uint64_t row,uint64_t col) { return adjacency.size()*row + col; }
	/// Solve DFS
	void solveDFS(int32_t before,int32_t row,int32_t from,int32_t minWeight,int32_t* solution) {
		solution[at(row,from)]=minWeight;
		for (auto& outEdge : adjacency[from]) {
			if (outEdge.to!=before) 
				solveDFS(from,row,outEdge.to,std::min(minWeight,outEdge.weight),solution);
		}
	}
	/// Solve
	std::vector<int32_t> solve() {
		// Allocate result vector, use at to access an element directly
		std::vector<int32_t> solution; solution.resize(adjacency.size()*adjacency.size());

		for (uint64_t from=0; from<adjacency.size(); ++from) {
			solveDFS(-1,from,from,999999999,solution.data());
			solution[at(from,from)]=0;
		}

		FixedWriter out(MaxNodes*MaxNodes*12);

		int32_t* solPtr = solution.data();
		for (uint64_t from=0,fromlimit=adjacency.size(); from!=fromlimit; ++from) {
			for (uint64_t to=0,limit=adjacency.size(); to!=limit; ++to) {
				out.writeNumber(*solPtr++);
				if (to!=limit-1) out.writeChar(' ');
			}
			out.writeChar('\n');
		}
		out.flush(1);

		return solution;
	}
};

/// EdgeWeightGraph
struct EdgeWeightGraph {
	struct Edge {
		int32_t from;
		int32_t to;
		int32_t weight;
		bool operator<(const Edge& other) const { return weight>other.weight; }
	};
	uint64_t nodeCount;
	std::vector<Edge> edges;

	EdgeWeightGraph(uint64_t nodeCount) : nodeCount(nodeCount) {}

	AdjacencyGraph mst() {
		AdjacencyGraph g(nodeCount);

		// Initialize cycle detection through union find and sort edges by weight
		UnionFind uf(nodeCount);
		std::sort(edges.begin(),edges.end());

		// Iterate over sorted edges adding them to the output graph if they don't add a cycle
		for (auto& edge : edges) {
			if (uf.find(edge.from)!=uf.find(edge.to)) {
				g.addEdge(edge.from,edge.to,edge.weight);
				g.addEdge(edge.to,edge.from,edge.weight);
				uf.merge(uf.find(edge.from),uf.find(edge.to));
				if (uf.getPartitionCount() == 1) return std::move(g);
			}
		}

		// If we return here, we have more than one connected component
		return std::move(g);
	}

	void dump() {
		for (uint64_t index=0; index<nodeCount; ++index)
			std::cout << index << ";" << std::endl;
		for (auto& edge : edges) {
			std::cout << edge.from << " -- " << edge.to << " [color=black,label=\"" << edge.weight << "\"];" << std::endl;
		}
	}
};

/// Simple test case for union find
static void testUnionFind() {
	UnionFind uf(10);
	assert(uf.find(0)!=uf.find(1));
	assert(uf.find(0)==uf.find(0));
	uf.merge(uf.find(0),uf.find(1));
	assert(uf.find(0)==uf.find(1));
	assert(uf.find(0)==uf.find(0));
}

static void readBuffered() {
	BufferedReader b(0);
	int32_t vcount = b.readNumber<int32_t>(); b.readSpace();
	int32_t ecount = b.readNumber<int32_t>(); b.readSpace();
	EdgeWeightGraph g(vcount);
	g.edges.reserve(ecount);
	for (uint64_t eindex=0; eindex<ecount; ++eindex) {
		int32_t from = b.readNumber<int32_t>(); b.readSpace();
		int32_t to = b.readNumber<int32_t>(); b.readSpace();
		int32_t weight = b.readNumber<int32_t>(); b.readSpace();
		g.edges.push_back(EdgeWeightGraph::Edge{from,to,weight});
	}

	auto mst=g.mst();
	auto res=mst.solve();

	// std::cout << "graph a {" << std::endl;
	// g.dump();
	// mst.dump();
	// std::cout << "}" << std::endl;
}

/*
	Strategy:
		- Read input
		- Build minimum spanning tree
		- Find all paths in spanning tree
		- Generate result
*/

int main(int argc,char* argv[]) {
	readBuffered();
}
