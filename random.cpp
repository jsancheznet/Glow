/*
  This file implements xorshift as seen on wikipedia

  https://en.wikipedia.org/wiki/Xorshift
*/

#include <stdint.h>
#include "shared.h"

#include <stdint.h>

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
i32 RandomBetween(i32 Min, i32 Max)
{
    Assert(Min < Max);
    return Min + (i32)( RandomU32() / (i32) ( 0xffffffff/ (Max-Min)));
}

inline
f32 RandomBetween(f32 Min, f32 Max)
{
    Assert(Min < Max);

    return Min + (f32)( RandomU32() / (f32) ( 0xffffffff/ (Max-Min)));
}

inline
b32 RandomBool()
{
    return RandomU32() % 2;
}
