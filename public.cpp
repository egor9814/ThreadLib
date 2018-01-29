//
// Created by egor9814 on 01.01.18.
//

#include <cstring>
#include <lib_threads.hpp>
#include "public.hpp"

static unsigned long key = 0;

unsigned long _public_::get_key() {
	if (key == 0){
		auto nano = threads::util::currentTimeNanos();
		auto name = "egor9814_public_key";
		auto len = strlen(name);
		for(unsigned long i = 0; i < len; i++){
			nano ^= (name[i] << i);
		}
		key = nano;
	}
	return key;
}
