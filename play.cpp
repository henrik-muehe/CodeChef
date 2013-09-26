#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


struct BufferedReader {
	static const uint64_t BufferSize = 4096;
	/// The input fd
	int input;
	/// The buffer
	char buffer[BufferSize];
	/// The index inside the buffer
	int64_t currentIndex;
	/// The last+1 index inside the buffer
	int64_t endIndex;

	/// Constructor
	BufferedReader(int input) : input(input),currentIndex(0),endIndex(0) {}
	/// Read more data into buffer
	bool replenish() {
		endIndex=read(input,buffer,BufferSize);
		currentIndex=0;
		return endIndex>0;
	}
	/// Reads a number from the buffer
	template<class T>
	T readNumber() {
		T number=0;
		while (true) {
			for (; currentIndex!=endIndex; ++currentIndex) {
				if (buffer[currentIndex]>='0'&&buffer[currentIndex]<='9') {
					number *= 10;
					number += buffer[currentIndex]-'0';
				} else {
					goto done;
				}
			}
			if (!replenish()) 
				break;
		}
		done:
		return number;
	}
	/// Reads space from the buffer
	void readSpace() {
		while (true) {
			for (; currentIndex!=endIndex; ++currentIndex) {
				if (buffer[currentIndex]!=' '&&buffer[currentIndex]!='\n') {
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

static void readBuffered() {
	BufferedReader b(0);
	int32_t vcount = b.readNumber<int32_t>(); b.readSpace();
	int32_t ecount = b.readNumber<int32_t>(); b.readSpace();
	for (uint64_t eindex=0; eindex<ecount; ++eindex) {
		int32_t from = b.readNumber<int32_t>(); b.readSpace();
		int32_t to = b.readNumber<int32_t>(); b.readSpace();
		int32_t weight = b.readNumber<int32_t>(); b.readSpace();
	}
}

int main(int argc,char* argv[]) {
	readBuffered();
}
