/*
	A C++ version of Jean-Philippe Aumasson's BlaBla PRNG (modified from Orson Peters 'chacha.h' original code)
 
	Changes Copyright (c) 2024 Gahtan Syarif Nahdi, licence as below.
	
	Changes compared to Orson Peters original code:
 		- fix formatting
		- Changed word size from 32-bits to 64-bits
		- Changed round loop increment from 2 to 1
		- Changed matrix and rotational constants
		- Changed rotation direction from left to right
		- Rename class, functions, and variables
		- Removed SIMD intrinsics for simplicity
		- Replaced macros with functions
		- Added header guards
		- Added default round value of 10
		- Added default seed value and changed the default stream
		- Added namespace
		- Changed starting block index from 0 to 1
		- Changed iterator variable type to uint
		- Revised keysetup and seed sequence generation
		
	Original version of the code can be found at https://gist.github.com/orlp/32f5d1b631ab092608b1
 */

/*
    Copyright (c) 2024 Orson Peters <orsonpeters@gmail.com>
    
    This software is provided 'as-is', without any express or implied warranty. In no event will the
    authors be held liable for any damages arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose, including commercial
    applications, and to alter it and redistribute it freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not claim that you wrote the
       original software. If you use this software in a product, an acknowledgment in the product
       documentation would be appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be misrepresented as
       being the original software.
    
    3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BLABLA_H_A1CCEA90A34252471C6E8FB1FDD8919B32348DA7FF6CD3C907EFDCCA68935B
#define BLABLA_H_A1CCEA90A34252471C6E8FB1FDD8919B32348DA7FF6CD3C907EFDCCA68935B

#include <cstdlib>
#include <iosfwd>
#include <cstdint>
#include <limits>

namespace BlaBlaPRNG {
	
template<uint32_t R = 10>
class BlaBla {
public:
	typedef uint64_t result_type;
	
	explicit BlaBla(uint64_t seedval = 0x1ee106e9041096e4, uint64_t stream = 0x037926afc39dcbd9);
	template<class Sseq> explicit BlaBla(Sseq& seq);

	void seed(uint64_t seedval = 0x1ee106e9041096e4, uint64_t stream = 0x037926afc39dcbd9);
	template<class Sseq> void seed(Sseq& seq);
	
	uint64_t operator()();
	void discard(unsigned long long n);
    
	template<uint32_t R_> friend bool operator==(const BlaBla<R_>& lhs, const BlaBla<R_>& rhs);
	template<uint32_t R_> friend bool operator!=(const BlaBla<R_>& lhs, const BlaBla<R_>& rhs);

	template<typename CharT, typename Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const BlaBla<R>& rng);

	template<typename CharT, typename Traits>
	friend std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, BlaBla<R>& rng);

	static constexpr uint64_t min() { return std::numeric_limits<uint64_t>::min(); }
	static constexpr uint64_t max() { return std::numeric_limits<uint64_t>::max(); }

private:
	void generate_block();
	void blabla_core();
	void mix_func(int a, int b, int c, int d);
	
	static uint64_t rotate_right(uint64_t x, uint32_t n) { return (x >> n) | (x << (64 - n)); }

	uint64_t block[16];
	uint64_t block_idx;

	uint64_t keysetup[8];
	uint64_t ctr;
};


template<uint32_t R>
inline BlaBla<R>::BlaBla(uint64_t seedval, uint64_t stream) {
	seed(seedval, stream);
}

template<uint32_t R>
template<class Sseq> 
inline BlaBla<R>::BlaBla(Sseq& seq) {
	seed(seq);
}

template<uint32_t R>
inline void BlaBla<R>::seed(uint64_t seedval, uint64_t stream) {
	ctr = 0;
	block_idx = uint64_t(-1); // Block is assumed to be uninitialized.
	keysetup[0] = 0x1fe2c9482a400d2e; // Could be used for 128-bit seed.
	keysetup[1] = 0xe6c7993da713d61d; // Could be used for 128-bit stream. 
	keysetup[2] = seedval;
	keysetup[3] = stream;
}

template<uint32_t R>
template<class Sseq>
inline void BlaBla<R>::seed(Sseq& seq) {
	ctr = 0;
	block_idx = uint64_t(-1); // Block is assumed to be uninitialized.
	uint32_t seeds[8];
	seq.generate(seeds, seeds + 8);
	for (uint32_t i = 0; i < 4; ++i) {
		keysetup[i] = ((uint64_t)seeds[2 * i] << 32) | ((uint64_t)seeds[2* i + 1]);
	}
}


template<uint32_t R>
inline uint64_t BlaBla<R>::operator()() {
	uint64_t next_block_idx = ctr / 16;
	uint64_t idx_in_block = ctr % 16;
	if (next_block_idx != block_idx) {
		block_idx = next_block_idx;
		generate_block();
	}
	++ctr;
	
    return block[idx_in_block];
}

template<uint32_t R>
inline void BlaBla<R>::discard(unsigned long long n) {
	ctr += n;
}

template<uint32_t R>
inline void BlaBla<R>::generate_block() {
	uint64_t constants[4] = {0x6170786593810fab, 0x3320646ec7398aee, 0x79622d3217318274, 0x6b206574babadada};

	uint64_t input[16];
	for (uint32_t i = 0; i < 4; ++i) input[i] = constants[i];
	for (uint32_t i = 0; i < 4; ++i) input[4 + i] = keysetup[i];
	input[8] = 0x2ae36e593e46ad5f;
	input[9] = 0xb68f143029225fc9;
	input[10] = 0x8da1e08468303aa6;
	input[11] = 0xa48a209acd50a4a7;
	input[12] = 0x7fdc12f23f90778c;
	input[13] = block_idx + 1;
	input[14] = input[15] = 0; // Could be used for 192-bit counter.

	for (uint32_t i = 0; i < 16; ++i) block[i] = input[i];
	blabla_core();
	for (uint32_t i = 0; i < 16; ++i) block[i] += input[i];
}


template<uint32_t R>
inline void BlaBla<R>::mix_func(int a, int b, int c, int d) {
	block[a] += block[b]; block[d] ^= block[a]; block[d] = rotate_right(block[d], 32);
	block[c] += block[d]; block[b] ^= block[c]; block[b] = rotate_right(block[b], 24);
	block[a] += block[b]; block[d] ^= block[a]; block[d] = rotate_right(block[d], 16);
	block[c] += block[d]; block[b] ^= block[c]; block[b] = rotate_right(block[b], 63);
}


template<uint32_t R>
inline void BlaBla<R>::blabla_core() {
	for (uint32_t i = 0; i < R; ++i) {
	    	mix_func(0, 4,  8, 12);
		mix_func(1, 5,  9, 13);
		mix_func(2, 6, 10, 14);
		mix_func(3, 7, 11, 15);
		mix_func(0, 5, 10, 15);
		mix_func(1, 6, 11, 12);
		mix_func(2, 7,  8, 13);
		mix_func(3, 4,  9, 14);
	}
}


// Implement <random> interface.
template<uint32_t R>
inline bool operator==(const BlaBla<R>& lhs, const BlaBla<R>& rhs) {
	for (uint32_t i = 0; i < 4; ++i) {
		if (lhs.keysetup[i] != rhs.keysetup[i]) return false;
	}
	
	return lhs.ctr == rhs.ctr;
}

template<uint32_t R>
inline bool operator!=(const BlaBla<R>& lhs, const BlaBla<R>& rhs) { return !(lhs == rhs); }

template<uint32_t R, typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const BlaBla<R>& rng) {
	typedef typename std::basic_ostream<CharT, Traits>::ios_base ios_base;

	// Save old state.
	auto flags = os.flags();
	auto fill = os.fill();
    
	// Set flags and fill to space.
	auto space = os.widen(' ');
	os.flags(ios_base::dec | ios_base::fixed | ios_base::left);
	os.fill(space);

	// Serialize.
	for (uint32_t i = 0; i < 4; ++i) os << rng.keysetup[i] << space;
	os << rng.ctr;

	// Sestore old state.
	os.flags(flags);
	os.fill(fill);

	return os;
}

template<uint32_t R, typename CharT, typename Traits>
inline std::basic_istream<CharT, Traits>& operator>>(std::basic_istream<CharT, Traits>& is, BlaBla<R>& rng) {
	typedef typename std::basic_istream<CharT, Traits> ::ios_base ios_base;

	// Save old flags and set ours.
	auto flags = is.flags();
	is.flags(ios_base::dec);

	// Deserialize.
	for (uint32_t i = 0; i < 4; ++i) is >> rng.keysetup[i];
	is >> rng.ctr;
	rng.block_idx = uint64_t(-1); // Block is assumed to be uninitialized.

	// Restore old flags.
	is.flags(flags);

	return is;
}

}

#endif
