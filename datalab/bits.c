/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int tmp1 = ~x & y;
  int tmp2 = x & ~y;
  return ~(~tmp1 & ~tmp2);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  //Ah ha! Easy peasy lemon sequeezy!
  //
  return 1 << 31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
    int tmp1 = (x + 1)^(x);
    tmp1 = tmp1 + 1; 
    return (!tmp1) & (!!(~x));
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  // Create a mask with odd numbered bits as 1
  int mask = 0xA;
  int face;
  mask = (mask << 4) + mask;
  mask = (mask << 8) + mask;
  mask = (mask << 16) + mask;
  face = ~(mask & x);
  return !(~(face ^ mask));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x) + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  // I think this is all about binary comparison.
  // I'd like to decomposite solution into 2
  // parts. 
  // Firt: Comparing the 4th to 7th bit.
  // Second: Comparing the 0th to 3rd bit.
  // Hope this will do.(This is not gonna do...)
  int mask1 = 0x0000000f;
  int mask2 = ~mask1;
  int face_up = mask2 & x;
  int face_low = mask1 & x;
  // Ah how dum I am to come up with such
  // an approach.
  int tmp1 = face_low ^ 0x8;
  int tmp2 = face_low ^ 0x9;
  int tmp3 = face_low & 0x8;
  int tmp4 = face_up ^ 0x30;
  return (!tmp4) & ((!tmp1) | (!tmp2) |(!tmp3));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  // If x is not zero, we need somehow convert
  // it to 0xffffffff otherwise 0x0 and return a number
  // masked by the two.
  int mask = (!x);
  mask = (mask << 1) + mask;
  mask = (mask << 2) + mask;
  mask = (mask << 4) + mask;
  mask = (mask << 8) + mask;
  mask = (mask << 16) + mask;
  return (mask & z) | (~mask & y);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  // Thoughts: We first determine wether x equals y
  // and then detertimine if x is less than y by substructing 
  // x from y and comparing the reuslt to 0. Need to be careful 
  // with the tmin, since it will overflow if it is negated.
  int tmin = 1 << 31;
  int sign_x = !!(x & tmin);
  int sign_y = !!(y & tmin);
  int neg_x = ~x + 1;
  int diff = y + neg_x;
  int sign_diff = diff & tmin;
  
  // If y is neg. while x is pos. we can just simply 
  // return False and vice versa.I'd like to call this
  // "dummy case"
  int dummy_1 = sign_x & (!sign_y);
  int dummy_2 = (!sign_x) & sign_y;
  int part_res = dummy_1 | (!dummy_2);
  int indicator = dummy_1 | dummy_2;
  // It's bassically returns a conditional expression:
  // if x and y is of same sign we return not sign_diff
  // other wise we return part res.
  return (part_res & indicator) | ((!sign_diff) & (part_res));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  // Thoughts: Maybe we need to convert any non-zero value
  // to 1 first. And then consider how to negate them logically.
  // If x is 0, ~x + 1 is 0 itself. What's been said before is bullshit,
  // I'll do it in this way: firt determine if it is negative, if yes, 
  // we return 0 otherwise we negate x and substruct it from 0, if the
  // restult is negative, we return 0, otherwise we return 1. Nope! I've
  // figured out a simpler way, see the code.
  
  int tmin = 1 << 31;
  int sign_x = tmin & x;
  int neg_x = ~x + 1;
  int sign_neg = tmin & neg_x;
  return ((~(sign_x | sign_neg)) >> 31) & 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  // 90 ops looks formidable...
  // Gut feeling tells me this problesm invovlves
  // a lot of masking...
  // Thouhts: we left shift the x bit by bit and store 
  // the number of shifts into a counter. We keep track of 
  // the sign bit, once we ecounter a 1, we somehow stop 
  // and return. Since loops are not allowed, we need to 
  // hard code it out. Damn! The best I can do so far is 
  // with 98 ops. After couple of days thinking, I have 
  // no clue at all about how to reduce the ops under 90...i
  // Maybe I should turn to the text book. Well, a better idea
  // will be shifting 1 as masks to mask the given number and
  // keep replacing the result if the face is not zero. What a 
  // dummy I was, why did I just dwell on the algorithm that is 
  // of O(n), this problem is essentially about calculating the 
  // log2(x), and t2here is algorithms of O(lg(N)). Yes! Still I 
  // have to take negative number as a special case. Since here
  // we are required to "compress" the representation of negative
  // 2s complement.
  
  int r;
  int shift;
  int sign_x;
  int tmin = 1 << 31;
  int is_tmin, is_n2, is_zero;
  int c_0xFFFF = (0xFF << 8) + 0xFF;
  int abs_x;
  sign_x = (x & tmin) >> 31;
  
  // If x is tmin then return 32
  is_tmin = !(tmin ^ x);
  is_tmin = is_tmin << 31 >> 31; 
  
  // If x is 0 return 1
  is_zero = (!(x ^ 0)) << 31 >> 31;  
 

  // Negate x if x if negative.
  x = ((~x + 1) & sign_x) | ((~sign_x)  & x);

  abs_x = x;

  // Calculate the logrithm of abs(x)
  r = (!!((c_0xFFFF + (~x + 1)) & tmin)) << 4; x = x >>r;
  shift = (!!((0xFF + (~x + 1)) & tmin)) << 3; x = x >> shift;
  r = shift | r;
  shift = (!!((0xF + (~x + 1)) & tmin)) << 2; x = x >> shift;
  r = shift | r;
  shift = (!!((0x3 + (~x + 1)) & tmin)) << 1; x = x >> shift;
  r = r | shift;
  r = r | (x >> 1); 

  // If x is negative and also a
  // power of 2. The bits it needs
   // is one bit less than its positive 
  // counter part. We set a mark
  // for x being this case here.
  is_n2 = (!((1 << r) ^ abs_x)) << 31 >> 31;
  is_n2 = sign_x & is_n2;

  return (((~is_zero) & (((r + 2) & (~is_tmin)) | (is_tmin & 32))) | (is_zero & 1)) + (is_n2 & (~1 + 1));
 }
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and
 *   result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  // If it is normalized value, we simply increment the 
  // exponent bits. If it is subnormalized value, we double
  // the significant bits. The tricky points of this 
  // problem is about those coner cases where we need to 
  // swich the normalized values into subnormalized ones.

  int frac_mask = (((0xFF << 16) +(0xFF << 8) + 0xFF) - (1 << 23)); 
  int exponent = (0xFF << 23) & uf;
  int sign_bit = (1 << 31) & uf;
  int frac = frac_mask & uf;
  int result_frac, result_exponent;
  int is_exp_zero = exponent ^ 0;
  
  result_frac = frac;
  result_exponent = exponent;
  // +-inf or NaN case
  if (!(exponent ^ (0xFF << 23))) {
      return uf;
  }

  // Denormalized case 
  if (!is_exp_zero) {

     // 0 value case
     if (!(frac ^ 0))
         return uf;

     result_frac = (frac + frac) & frac_mask;

     // If there is a overflow in
     // 2 * frac. Convert the number
     // to normalized case.
     if (result_frac < frac) {
	 result_exponent = 1 << 23;
     }
  }

  // Normalized case
  if (is_exp_zero) {
     result_exponent  = result_exponent + (1 << 23);
  }
  
  return sign_bit | result_exponent | result_frac;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  
  // Thoughts: floating points are 
  // of 1.xxxx * 2^(e), converting them 
  // to ints is about shifting the point 
  // to the left as much as we can and trancate
  // the what are at the right of the shifted
  // point.
  // I'm not sure whether - is allowed in float functions,
  // but when I use -, the dlc checker does'nt complain 
  // unlike that in the int functions where dlc checker
  // will complain of the use of -. I just simply assume
  // that - is a valid operator in float functions.

  int frac_mask = (((0xFF << 16) +(0xFF << 8) + 0xFF) - (1 << 23)); 
  int exponent = ((0xFF << 23) & uf) >> 23;
  int sign_bit = (1 << 31) & uf;
  int frac = frac_mask & uf;
  int shift_right;
  int is_exp_zero = exponent ^ 0;


  if (!(exponent ^ (0xFF << 23))) {
      return 0x80000000u;
  }
  
  // Normalized case
  if (is_exp_zero) {
      frac = frac + (1 << 23);

      if (exponent < 127) { 
          return 0; 
      }
      if (exponent > 157) {
          return 0x80000000u;
      }

      exponent = exponent - 127;
      shift_right = exponent < 23;
      if (shift_right) {
          exponent = 23 - exponent;
	  frac = frac >> exponent;
      }

      if (!shift_right) {
         exponent = exponent - 23;
	 frac = frac << exponent;
      }
      
      if (sign_bit) {
          frac  = -frac;
      }
  }
  
  // All numbers in the denormailzed case are less
  // than 1, just return 0.
  if (!is_exp_zero) {
      frac = 0;
  }
   
  return frac;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    int result_sign = 0;
    int result_exponent = 0;
    int result_frac = 0;
    int is_pos_inf = x > 127;
    int is_neg_inf = x < (-149);
    int inf_exponent = 0xFF << 23;
    int norm2de_boarder = -127;
    int shift;

    // +-inf case
    if (is_pos_inf) {
        result_exponent = inf_exponent;
    }
    if (is_neg_inf) {
        result_sign = 0;
	result_exponent = 0;
    }

    // Normalized and denormalized case
    if ((!is_pos_inf) && (!is_neg_inf)) {
        result_exponent = x + 127;
        
	// Denormalized case
	if (result_exponent < norm2de_boarder) {
            shift = norm2de_boarder - result_exponent;
	    result_frac = 1 << 22;
            result_frac = result_frac >> shift;
	}
	result_exponent = result_exponent << 23;
    }
     
    return result_sign | result_exponent | result_frac;
}
