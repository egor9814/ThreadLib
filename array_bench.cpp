//
// Created by egor9814 on 09.01.18.
//

#include "../stl/include/stl_array.hpp"
#include "../stl/include/stl_random.hpp"
#include "../stl/include/stl_comparators.hpp"
#include <lib_async.hpp>
#include <list>
#include <set>
#include <sstream>

typedef stl::Array<int> IntArray;
using stl::Random;
using std::cout;
using std::endl;

const unsigned long COUNT = 8;
const unsigned long LIMIT = 1000000;

struct Sorter {
	const char* name = "unknown";
	long elapsedTime[COUNT]{0};
	std::set<unsigned long> exclude;
	std::function<IntArray(IntArray&)> method;

	inline bool isExclude(unsigned long count) const {
		return exclude.count(count) > 0;
	}

};

IntArray sort_method(Sorter& sorter, IntArray& array){
	return sorter.method(array);
}


struct GeneratorData {
	unsigned long count;
	const char* name;
	unsigned long lim;
	Random* r;

	IntArray* a;
};

void generate(const char* name, unsigned long lim, Random* r, IntArray* a){
	/*auto count = data->count;
	auto name = data->name;
	auto lim = data->lim;
	auto r = data->r;
	if(r == nullptr){
		r = new Random;
	}*/

	cout << "generating " << a->length() << " elements..." << endl;
	FILE* out = fopen(name, "w");
	if(!out){
		std::cerr << "cannot open file: " << name << endl;
		return;
	}
	//auto a = data->a;
	for(auto &i : *a){
		i = int(r->nextInt(lim) - lim/2);
		fprintf(out, "%i ", i);
	}
	fprintf(out, "\n");
	fclose(out);

	/*EXIT:
	if(data->r == nullptr){
		delete r;
	}*/
}


void sort(Sorter* sorter, const char* name, unsigned long index, IntArray* array){
	auto count = array->length();
	if(sorter->isExclude(count)){
		cout << "skipped " << sorter->name << "!" << endl;
		sorter->elapsedTime[index] = -2;
	} else {
		cout << "sorting by " << sorter->name << "..." << endl;
		auto sortResult = stl::Timer::benchmark<IntArray, Sorter&, IntArray&>(
				"ms", sort_method, *sorter, *array);
		if(sortResult.result.checkForSorting(stl::comparators::INT)){
			cout << "done " << sorter->name << "!" << endl;
			sorter->elapsedTime[index] = sortResult.elapsedTime;
		} else {
			cout << "failed " << sorter->name << "!" << endl;
			sorter->elapsedTime[index] = -1;
		}
		FILE* out = fopen(name, "a");
		fprintf(out, "%s: ", sorter->name);
		for(auto &i : sortResult.result){
			fprintf(out, "%i ", i);
		}
		fclose(out);
	}
}


template <IntArray::SortType T>
IntArray sort_impl(IntArray& array){
	IntArray a(array);
	a.sort<T>(stl::comparators::INT);
	return a;
}


int main(){
	const unsigned long SORTERS_COUNT = 6;
	Sorter sorters[SORTERS_COUNT] = {
			{ "Shaker", {0}, {100000, 500000, 1000000}, sort_impl<IntArray::ShakerSort> },
			{ "Shell", {0}, {100000, 500000, 1000000}, sort_impl<IntArray::ShellSort> },
			{ "Heap", {0}, {}, sort_impl<IntArray::HeapSort> },
			{ "Hoar", {0}, {}, sort_impl<IntArray::HoarSort> },
			{ "Bit", {0}, {}, [](IntArray& array) -> IntArray {
				IntArray a(array);
				a.bit_sort();
				return a;
			} },
			{ "Bit2", {0}, {}, [](IntArray& array) -> IntArray {
				IntArray a(array);
				a.bit_sort_2();
				return a;
			} }
	};

	unsigned long counts[8] = {
			10, 100, 1000, 5000, 10000, 100000, 500000, 1000000
	};
	const char* array_names[8] = {
			"arr10.txt", "arr100.txt", "arr1000.txt", "arr5000.txt", "arr10000.txt",
			"arr100000.txt", "arr500000.txt", "arr1000000.txt"
	};

	Random r;

	for(unsigned long i = 0; i < COUNT; i++) {
		IntArray array(counts[i]);
		/*threads::Thread gen(generate, array_names[i], LIMIT, &r, &array);
		gen.setName("Generator #" + std::to_string(i+1));
		gen.start();
		gen.join();*/
		generate(array_names[i], LIMIT, &r, &array);

		threads::AThread sortThreads[SORTERS_COUNT];
		for(unsigned long j = 0; j < SORTERS_COUNT; j++){
			sortThreads[j] = new threads::Thread(
					static_cast<std::string>("SortThread #") + std::to_string(j),
					sort, &(sorters[j]), array_names[i], i, &array);
			sortThreads[j]->start();
		}
		for(auto &t : sortThreads){
			t->await();
		}
		cout << endl;
	}


	size_t max = 0;
	for(auto &sorter : sorters){
		for(unsigned long j = 0; j < COUNT; j++){
			if(sorter.elapsedTime[j] >= 0 && sorter.elapsedTime[j] > max){
				max = (size_t)sorter.elapsedTime[j];
			}
		}
	}
	std::stringstream ss;
	ss << max;
	std::string str = ss.str();
	max = str.length();
	if(7 > max)
		max = 7;
	max += 2;

	ss = std::stringstream();
	ss << " |%" << (max+2) << "s";
	str = ss.str();
	const char* format = str.c_str();

	printf("\n\n\nTable of sort time:\nN\\Method");
	for(auto &sorter : sorters){
		printf(format, sorter.name);
	}
	printf("\n");

	ss = std::stringstream();
	ss << " |\033[1;32m%" << max << "lims\033[0m";
	str = ss.str();
	format = str.c_str();

	for(unsigned long i = 0; i < COUNT; i++){
		printf("%8lu", counts[i]);
		for(auto &sorter : sorters){
			if(sorter.elapsedTime[i] == -2){
				printf(" |");
				for(unsigned long j = 0; j < max-5; j++){
					printf(" ");
				}
				printf("\033[1;33mskipped\033[0m");
			} else if(sorter.elapsedTime[i] == -1){
				printf(" |");
				for(unsigned long j = 0; j < max-4; j++){
					printf(" ");
				}
				printf("\033[1;31mfailed\033[0m");
			} else {
				printf(format, sorter.elapsedTime[i]);
			}
		}
		printf("\n");
	}


	return 0;
}
