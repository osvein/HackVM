/* MIT License
 *
 * Copyright (c) 2017 Oskar Sveinsen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "HackCPU.h"

#define MASK_I (1 << 15) // instruction code: 1 for C, 0 for A

#define MASK_A (1 << 12) // y input code: 1 for M, 0 for A

// comp field
#define MASK_C1 (1 << 11) // zx	- zero the x input
#define MASK_C2 (1 << 10) // nx	- negate the x input
#define MASK_C3 (1 <<  9) // zy	- zero the y input
#define MASK_C4 (1 <<  8) // ny	- negate the y input
#define MASK_C5 (1 <<  7) // f	- function code: 1 for Add, 0 for And
#define MASK_C6 (1 <<  6) // no	- negate the out output

// dest field
#define MASK_D1 (1 << 5) // A register
#define MASK_D2 (1 << 4) // D register
#define MASK_D3 (1 << 3) // M register

// jump field
#define MASK_J1 (1 << 2) // out < 0
#define MASK_J2 (1 << 1) // out = 0
#define MASK_J3 (1 << 0) // out > 0

extern struct hackcpu_output hackcpu_execute(struct hackcpu_state *state, int16_t inM, int16_t instruction, bool reset) {
	state->pc++;

	struct hackcpu_output output = {
		.writeM = false;
		.addressM = state->a;
	};

	if (instruction & MASK_I) {
		// C-instruction
		int16_t x = state->d;
		int16_t y = (instruction & MASK_A) ? inM : state->a;
		if (instruction & MASK_C1) x = 0;
		if (instruction & MASK_C2) x = ~x;
		if (instruction & MASK_C1) y = 0;
		if (instruction & MASK_C2) y = ~y;

		int16_t out = (instruction & MASK_C5) ? (x + y) : (x & y);
		if (instruction & MASK_C6) out = ~out;
		output.outM = out;

		if (instruction & MASK_D1) state->a = out;
		if (instruction & MASK_D2) state->d = out;
		if (instruction & MASK_D3) output.writeM = true;

		bool jump = false;
		jump = jump || ((instruction & MASK_J1) && (out < 0));
		jump = jump || ((instruction & MASK_J2) && (out == 0));
		jump = jump || ((instruction & MASK_J3) && (out > 0));
		if (jump) state->pc = state->a;
	}
	else {
		// A-instruction
		state->a = instruction;
	}

	if (reset) state->pc = 0;
	output.pc = state->pc;
	return output;
}
