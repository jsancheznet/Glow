/*
  This file implements xorshift as seen on wikipedia

  https://en.wikipedia.org/wiki/Xorshift
*/

// TODO(Jorge): Implement Xorshift+ instead of regular Xorshift, it's much better according to wikipedia

#include <stdint.h>
#include "shared.h"

#include <stdint.h>

// TODO(Jorge): We need to seed the RNG by changing this value
global u32 _State = 1;

inline
void RandomSeed(u32 Input)
{
    Assert(Input != 0);

    _State = Input;
}

inline
u32 RandomU32()
{
	u32 X = _State;
	X ^= X << 13;
	X ^= X >> 17;
	X ^= X << 5;
	return _State= X;
}

inline
i32 RandomI32()
{
	i32 X = _State;
	X ^= X << 13;
	X ^= X >> 17;
	X ^= X << 5;
	return _State= X;
}

inline
f32 RandomBetween(f32 Min, f32 Max)
{
    Assert(Min < Max);

    return Min + (f32)( RandomU32() / (f32) ( 0xffffffff/ (Max-Min)));
}
