// Copyright 2011 INDILINX Co., Ltd.
//
// This file is part of Jasmine.
//
// Jasmine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Jasmine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Jasmine. See the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.


#ifndef	BANK_H
#define	BANK_H

// bank -> rbank translation table

#define A0	0x00
#define B0	0x01
#define C0	0x02
#define D0	0x03
#define A1	0x04
#define B1	0x05
#define C1	0x06
#define D1	0x07
#define A2	0x08
#define B2	0x09
#define C2	0x0A
#define D2	0x0B
#define A3	0x0C
#define B3	0x0D
#define C3	0x0E
#define D3	0x0F
#define A4	0x10
#define B4	0x11
#define C4	0x12
#define D4	0x13
#define A5	0x14
#define B5	0x15
#define C5	0x16
#define D5	0x17
#define A6	0x18
#define B6	0x19
#define C6	0x1A
#define D6	0x1B
#define A7	0x1C
#define B7	0x1D
#define C7	0x1E
#define D7	0x1F

// 	0xFFFFFFFF
#define BMAP_4CH_8WY													\
{ 																		\
	A0, B0, C0, D0, A1, B1, C1, D1, A2, B2, C2, D2, A3, B3, C3, D3, 	\
	A4, B4, C4, D4, A5, B5, C5, D5, A6, B6, C6, D6, A7, B7, C7, D7  	\
}

//	0x00FFFFFF
#define BMAP_4CH_6WY													\
{ 																		\
	A0, B0, C0, D0, A1, B1, C1, D1, A2, B2, C2, D2, A3, B3, C3, D3, 	\
	A4, B4, C4, D4, A5, B5, C5, D5										\
}

//	0x000FFFFF
#define BMAP_4CH_5WY													\
{ 																		\
	A0, B0, C0, D0, A1, B1, C1, D1, A2, B2, C2, D2, A3, B3, C3, D3, 	\
	A4, B4, C4, D4   													\
}

// 	0x0F0F0F0F
#define BMAP_4CH_4WY_B													\
{ 																		\
	A0, B0, C0, D0, 				A2, B2, C2, D2, 				 	\
	A4, B4, C4, D4, 				A6, B6, C6, D6	 				 	\
}

// 	0x00FF00FF
#define BMAP_4CH_4WY_C													\
{ 																		\
	A0, B0, C0, D0, A1, B1, C1, D1,				 				 		\
	A4, B4, C4, D4, A5, B5, C5, D5					 				 	\
}


// 	0x33333333
#define BMAP_2CH_8WY_A													\
{ 																		\
	A0, B0,  		A1, B1,			A2, B2, 		A3, B3,			 	\
	A4, B4, 		A5, B5, 		A6, B6, 		A7, B7	 		 	\
}

// 	0x00333333
#define BMAP_2CH_6WY_A													\
{ 																		\
	A0, B0,  		A1, B1,			A2, B2, 		A3, B3,			 	\
	A4, B4, 		A5, B5 					 							\
}

// 	0x00033333
#define BMAP_2CH_5WY_A													\
{ 																		\
	A0, B0,  		A1, B1,			A2, B2, 		A3, B3,			 	\
	A4, B4																\
}

// 	0x00330033
#define BMAP_2CH_4WY_C													\
{ 																		\
	A0, B0,  		A1, B1,						 						\
	A4, B4, 		A5, B5, 			 		 						\
}

// 	0x03030303
#define BMAP_2CH_4WY_F													\
{ 																		\
	A0, B0,  				A2, B2, 					 				\
	A4, B4, 		 		A6, B6, 			 		 				\
}

// 	0x00000333
#define BMAP_2CH_3WY													\
{ 																		\
	A0, B0,  		A1, B1,			A2, B2, 		 	\
	       																\
}
// 	0x00030003
#define BMAP_2CH_2WY_D													\
{ 																		\
	A0, B0,  							 								\
	A4, B4, 					 		 								\
}



#define BMAP_4CH_4WY	  { A0, B0, C0, D0, A1, B1, C1, D1, A2, B2, C2, D2, A3, B3, C3, D3 }	// 	0xFFFF
#define BMAP_4CH_3WY	  { A0, B0, C0, D0, A1, B1, C1, D1, A2, B2, C2, D2                 }	//  0x0FFF
#define BMAP_4CH_2WY_A	{ A0, B0, C0, D0, A1, B1, C1, D1                                 }	// 	0x00FF
#define BMAP_4CH_2WY_B	{ A0, B0, C0, D0,                 A2, B2, C2, D2                 }	// 	0x0F0F
#define BMAP_4CH_1WY	  { A0, B0, C0, D0                                                 }	// 	0x000F
#define BMAP_2CH_4WY_A	{ A0, B0,         A1, B1,         A2, B2,         A3, B3         }	// 	0x3333
#define BMAP_2CH_4WY_B	{         C0, D0, 		    C1, D1, 		    C2, D2,         C3, D3 }	// 	0xCCCC
#define BMAP_2CH_4WY_D	{ A0,     C0,     A1,     C1,     A2,     C2,     A3,     C3     }  // 	0x5555
#define BMAP_2CH_4WY_E	{ A0,         D0, A1,         D1, A2,         D2, A3,         D3 }  // 	0x9999
#define BMAP_2CH_2WY_A	{ A0, B0,         A1, B1                                         }	// 	0x0033
#define BMAP_2CH_2WY_B	{ A0, B0,                         A2, B2                         }	// 	0x0303
#define BMAP_2CH_2WY_C	{         C0, D0,                         C2, D2                 }	// 	0x0C0C
#define BMAP_2CH_1WY	  { A0, B0                                                         }	// 	0x0003
#define BMAP_1CH_4WY	  { A0,             A1,             A2,             A3             }	// 	0x1111
#define BMAP_1CH_2WY_A	{ A0,             A1                                             }	// 	0x0011
#define BMAP_1CH_2WY_B	{ A0,                             A2                             }	// 	0x0101
#define BMAP_1CH_1WY	  { A0                                                      		   }	// 	0x0001
#define BMAP_3CH_4WY	  { A0, B0, C0,     A1, B1, C1,     A2, B2, C2,     A3, B3, C3     }	// 	0x7777
#define BMAP_3CH_2WY	  { A0, B0, C0,     A1, B1, C1                                     }	// 	0x0077
#define BMAP_3CH_2WY_B  { A0, B0, C0,                     A2, B2, C2                     }	// 	0x0707

// rbank -> bank translation table

#define _X_ 0xFF

// 	0xFFFFFFFF
#define BRMAP_4CH_8WY																					\
{ 																										\
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 	\
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F 		\
}

// 	0x00FFFFFF
#define BRMAP_4CH_6WY																					\
{ 																										\
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 	\
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x000FFFFF
#define BRMAP_4CH_5WY																					\
{ 																										\
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 	\
	0x10, 0x11, 0x12, 0x13,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x0F0F0F0F
#define BRMAP_4CH_4WY_B																					\
{ 																										\
	0x00, 0x01, 0x02, 0x03,  _X_,  _X_,  _X_,  _X_, 0x04, 0x05, 0x06, 0x07,  _X_,  _X_,  _X_,  _X_, 	\
	0x08, 0x09, 0x0A, 0x0B,  _X_,  _X_,  _X_,  _X_, 0x0C, 0x0D, 0x0E, 0x0F,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x00FF00FF
#define BRMAP_4CH_4WY_C																					\
{ 																										\
	0x00, 0x01, 0x02, 0x03,  0x04, 0x05, 0x06, 0x07, _X_,  _X_,  _X_,  _X_,   _X_,  _X_,  _X_,  _X_, 	\
	0x08, 0x09, 0x0A, 0x0B,  0x0C, 0x0D, 0x0E, 0x0F, _X_,  _X_,  _X_,  _X_,   _X_,  _X_,  _X_,  _X_ 	\
}


// 	0x33333333
#define BRMAP_2CH_8WY_A																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,  0x02, 0x03,  _X_,  _X_, 0x04, 0x05,  _X_,  _X_, 0x06, 0x07,  _X_,  _X_, 	\
	0x08, 0x09, _X_,  _X_,  0x0A, 0x0B,  _X_,  _X_, 0x0C, 0x0D,  _X_,  _X_, 0x0E, 0x0F,  _X_,  _X_ 		\
}

// 	0x00333333
#define BRMAP_2CH_6WY_A																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,  0x02, 0x03,  _X_,  _X_, 0x04, 0x05,  _X_,  _X_, 0x06, 0x07,  _X_,  _X_, 	\
	0x08, 0x09, _X_,  _X_,  0x0A, 0x0B,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x00033333
#define BRMAP_2CH_5WY_A																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,  0x02, 0x03,  _X_,  _X_, 0x04, 0x05,  _X_,  _X_, 0x06, 0x07,  _X_,  _X_, 	\
	0x08, 0x09, _X_,  _X_,   _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x00330033
#define BRMAP_2CH_4WY_C																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,  0x02, 0x03,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_, 	\
	0x04, 0x05, _X_,  _X_,  0x06, 0x07,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x03030303
#define BRMAP_2CH_4WY_F																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,   _X_,  _X_,  _X_,  _X_, 0x02, 0x03,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_, 	\
	0x04, 0x05, _X_,  _X_,   _X_,  _X_,  _X_,  _X_, 0x06, 0x07,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x00330033
#define BRMAP_2CH_2WY_D																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,   _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_, 	\
	0x02, 0x03, _X_,  _X_,   _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

// 	0x00000333
#define BRMAP_2CH_3WY																					\
{ 																										\
	0x00, 0x01, _X_,  _X_,  0x02, 0x03,  _X_,  _X_, 0x04, 0x05,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_, 	\
	_X_,  _X_,  _X_,  _X_,   _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_,  _X_ 		\
}

#define BRMAP_4CH_4WY	{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF }	// 	0xFFFF
#define BRMAP_4CH_3WY	{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, _X_, _X_, _X_, _X_ }	//  0x0FFF
#define BRMAP_4CH_2WY_A	{ 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x00FF
#define BRMAP_4CH_2WY_B	{ 0x0, 0x1, 0x2, 0x3, _X_, _X_, _X_, _X_, 0x4, 0x5, 0x6, 0x7, _X_, _X_, _X_, _X_ }	// 	0x0F0F
#define BRMAP_4CH_1WY	{ 0x0, 0x1, 0x2, 0x3, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x000F
#define BRMAP_2CH_4WY_A	{ 0x0, 0x1, _X_, _X_, 0x2, 0x3, _X_, _X_, 0x4, 0x5, _X_, _X_, 0x6, 0x7, _X_, _X_ }	// 	0x3333
#define BRMAP_2CH_4WY_B	{ _X_, _X_, 0x0, 0x1, _X_, _X_, 0x2, 0x3, _X_, _X_, 0x4, 0x5, _X_, _X_, 0x6, 0x7 }	// 	0xCCCC
#define BRMAP_2CH_4WY_D	{ 0x0, _X_, 0x1, _X_, 0x2, _X_, 0x3, _X_, 0x4, _X_, 0x5, _X_, 0x6, _X_, 0x7, _X_ }  //  0x5555
#define BRMAP_2CH_4WY_E	{ 0x0, _X_, _X_, 0x1, 0x2, _X_, _X_, 0x3, 0x4, _X_, _X_, 0x5, 0x6, _X_, _X_, 0x7 }  //  0x9999
#define BRMAP_2CH_2WY_A	{ 0x0, 0x1, _X_, _X_, 0x2, 0x3, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0033
#define BRMAP_2CH_2WY_B	{ 0x0, 0x1, _X_, _X_, _X_, _X_, _X_, _X_, 0x2, 0x3, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0303
#define BRMAP_2CH_2WY_C	{ _X_, _X_, 0x0, 0x1, _X_, _X_, _X_, _X_, _X_, _X_, 0x2, 0x3, _X_, _X_, _X_, _X_ }	// 	0x0C0C
#define BRMAP_2CH_1WY	{ 0x0, 0x1, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0003
#define BRMAP_1CH_4WY	{ 0x0, _X_, _X_, _X_, 0x1, _X_, _X_, _X_, 0x2, _X_, _X_, _X_, 0x3, _X_, _X_, _X_ }	// 	0x1111
#define BRMAP_1CH_2WY_A	{ 0x0, _X_, _X_, _X_, 0x1, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0011
#define BRMAP_1CH_2WY_B	{ 0x0, _X_, _X_, _X_, _X_, _X_, _X_, _X_, 0x1, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0101
#define BRMAP_1CH_1WY	{ 0x0, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0001
#define BRMAP_3CH_4WY	{ 0x0, 0x1, 0x2, _X_, 0x3, 0x4, 0x5, _X_, 0x6, 0x7, 0x8, _X_, 0x9, 0xA, 0xB, _X_ }	// 	0x7777
#define BRMAP_3CH_2WY	{ 0x0, 0x1, 0x2, _X_, 0x3, 0x4, 0x5, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_ }	// 	0x0077
#define BRMAP_3CH_2WY_B { 0x0, 0x1, 0x2, _X_, _X_, _X_, _X_, _X_, 0x3, 0x4, 0x5, _X_, _X_, _X_, _X_, _X_ }	// 	0x0707

#if BANK_BMP == 0xFFFFFFFF
	#define	BANK_MAP		BMAP_4CH_8WY
	#define	BANK_RMAP		BRMAP_4CH_8WY
	#define NUM_BANKS		32

#elif BANK_BMP == 0x00FFFFFF
	#define	BANK_MAP		BMAP_4CH_6WY
	#define	BANK_RMAP		BRMAP_4CH_6WY
	#define NUM_BANKS		24

#elif BANK_BMP == 0x000FFFFF
	#define	BANK_MAP		BMAP_4CH_5WY
	#define	BANK_RMAP		BRMAP_4CH_5WY
	#define NUM_BANKS		20

#elif BANK_BMP == 0x0F0F0F0F
	#define	BANK_MAP		BMAP_4CH_4WY_B
	#define	BANK_RMAP		BRMAP_4CH_4WY_B
	#define NUM_BANKS		16

#elif BANK_BMP == 0x00FF00FF
	#define	BANK_MAP		BMAP_4CH_4WY_C
	#define	BANK_RMAP		BRMAP_4CH_4WY_C
	#define NUM_BANKS		16

#elif BANK_BMP == 0x33333333
	#define	BANK_MAP		BMAP_2CH_8WY_A
	#define	BANK_RMAP		BRMAP_2CH_8WY_A
	#define NUM_BANKS		16

#elif BANK_BMP == 0x00333333
	#define	BANK_MAP		BMAP_2CH_6WY_A
	#define	BANK_RMAP		BRMAP_2CH_6WY_A
	#define NUM_BANKS		12

#elif BANK_BMP == 0x00033333
	#define	BANK_MAP		BMAP_2CH_5WY_A
	#define	BANK_RMAP		BRMAP_2CH_5WY_A
	#define NUM_BANKS		10

#elif BANK_BMP == 0x00330033
	#define	BANK_MAP		BMAP_2CH_4WY_C
	#define	BANK_RMAP		BRMAP_2CH_4WY_C
	#define NUM_BANKS		8

#elif BANK_BMP == 0x03030303
	#define	BANK_MAP		BMAP_2CH_4WY_F
	#define	BANK_RMAP		BRMAP_2CH_4WY_F
	#define NUM_BANKS		8

#elif BANK_BMP == 0xFFFF
	#define	BANK_MAP		BMAP_4CH_4WY
	#define	BANK_RMAP		BRMAP_4CH_4WY
	#define NUM_BANKS		16

#elif BANK_BMP == 0x7777
	#define	BANK_MAP		BMAP_3CH_4WY
	#define	BANK_RMAP		BRMAP_3CH_4WY
	#define NUM_BANKS		12

#elif BANK_BMP == 0x0077
	#define	BANK_MAP		BMAP_3CH_2WY
	#define	BANK_RMAP		BRMAP_3CH_2WY
	#define NUM_BANKS		6

#elif BANK_BMP == 0x0707
	#define	BANK_MAP		BMAP_3CH_2WY_B
	#define	BANK_RMAP		BRMAP_3CH_2WY_B
	#define NUM_BANKS		6

#elif BANK_BMP == 0x0FFF
	#define	BANK_MAP		BMAP_4CH_3WY
	#define	BANK_RMAP		BRMAP_4CH_3WY
	#define NUM_BANKS		12

#elif BANK_BMP == 0x00FF
	#define	BANK_MAP		BMAP_4CH_2WY_A
	#define	BANK_RMAP		BRMAP_4CH_2WY_A
	#define NUM_BANKS		8

#elif BANK_BMP == 0x0F0F
	#define	BANK_MAP		BMAP_4CH_2WY_B
	#define	BANK_RMAP		BRMAP_4CH_2WY_B
	#define NUM_BANKS		8

#elif BANK_BMP == 0x000F
	#define	BANK_MAP		BMAP_4CH_1WY
	#define	BANK_RMAP		BRMAP_4CH_1WY
	#define NUM_BANKS		4

#elif BANK_BMP == 0x3333
	#define	BANK_MAP		BMAP_2CH_4WY_A
	#define	BANK_RMAP		BRMAP_2CH_4WY_A
	#define NUM_BANKS		8

#elif BANK_BMP == 0xCCCC
	#define BANK_MAP		BMAP_2CH_4WY_B
	#define BANK_RMAP		BRMAP_2CH_4WY_B
	#define NUM_BANKS		8

#elif BANK_BMP == 0x5555
	#define BANK_MAP		BMAP_2CH_4WY_D
	#define BANK_RMAP		BRMAP_2CH_4WY_D
	#define NUM_BANKS		8

#elif BANK_BMP == 0x9999
	#define BANK_MAP		BMAP_2CH_4WY_E
	#define BANK_RMAP		BRMAP_2CH_4WY_E
	#define NUM_BANKS		8

#elif BANK_BMP == 0x0333
	#define	BANK_MAP		BMAP_2CH_3WY
	#define	BANK_RMAP		BRMAP_2CH_3WY
	#define NUM_BANKS		6

#elif BANK_BMP == 0x0033
	#define	BANK_MAP		BMAP_2CH_2WY_A
	#define	BANK_RMAP		BRMAP_2CH_2WY_A
	#define NUM_BANKS		4

#elif BANK_BMP == 0x0303
	#define	BANK_MAP		BMAP_2CH_2WY_B
	#define	BANK_RMAP		BRMAP_2CH_2WY_B
	#define NUM_BANKS		4

#elif BANK_BMP == 0x0C0C
	#define BANK_MAP		BMAP_2CH_2WY_C
	#define BANK_RMAP		BRMAP_2CH_2WY_C
	#define NUM_BANKS		4

#elif BANK_BMP == 0x0003
	#define	BANK_MAP		BMAP_2CH_1WY
	#define	BANK_RMAP		BRMAP_2CH_1WY
	#define NUM_BANKS		2

#elif BANK_BMP == 0x1111
	#define	BANK_MAP		BMAP_1CH_4WY
	#define	BANK_RMAP		BRMAP_1CH_4WY
	#define NUM_BANKS		4

#elif BANK_BMP == 0x0011
	#define	BANK_MAP		BMAP_1CH_2WY_A
	#define	BANK_RMAP		BRMAP_1CH_2WY_A
	#define NUM_BANKS		2

#elif BANK_BMP == 0x0101
	#define	BANK_MAP		BMAP_1CH_2WY_B
	#define	BANK_RMAP		BRMAP_1CH_2WY_B
	#define NUM_BANKS		2

#elif BANK_BMP == 0x0001
	#define BANK_MAP		BMAP_1CH_1WY
	#define BANK_RMAP		BRMAP_1CH_1WY
	#define NUM_BANKS		1

#elif BANK_BMP == 0x00030003
	#define BANK_MAP		BMAP_2CH_2WY_D
	#define BANK_RMAP		BRMAP_2CH_2WY_D
	#define NUM_BANKS		4

#else
	#error("unsupported flash configuration");
#endif

extern const UINT8 c_bank_map[NUM_BANKS];

#endif	// BANK_H

