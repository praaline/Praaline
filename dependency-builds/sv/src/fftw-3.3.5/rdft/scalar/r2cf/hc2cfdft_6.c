/*
 * Copyright (c) 2003, 2007-14 Matteo Frigo
 * Copyright (c) 2003, 2007-14 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sat Jul 30 16:48:41 EDT 2016 */

#include "codelet-rdft.h"

#ifdef HAVE_FMA

/* Generated by: ../../../genfft/gen_hc2cdft.native -fma -reorder-insns -schedule-for-pipeline -compact -variables 4 -pipeline-latency 4 -n 6 -dit -name hc2cfdft_6 -include hc2cf.h */

/*
 * This function contains 58 FP additions, 44 FP multiplications,
 * (or, 36 additions, 22 multiplications, 22 fused multiply/add),
 * 42 stack variables, 2 constants, and 24 memory accesses
 */
#include "hc2cf.h"

static void hc2cfdft_6(R *Rp, R *Ip, R *Rm, R *Im, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DK(KP866025403, +0.866025403784438646763723170752936183471402627);
     DK(KP500000000, +0.500000000000000000000000000000000000000000000);
     {
	  INT m;
	  for (m = mb, W = W + ((mb - 1) * 10); m < me; m = m + 1, Rp = Rp + ms, Ip = Ip + ms, Rm = Rm - ms, Im = Im - ms, W = W + 10, MAKE_VOLATILE_STRIDE(24, rs)) {
	       E TP, TT, TN, TM, TY, T13;
	       {
		    E T3, TQ, TJ, T12, Tu, TB, TX, T10, Tj, Tf, Ti, Td, Th, TU, TS;
		    {
			 E TC, TI, TF, TH, TA, Tw, TZ;
			 {
			      E T1, T2, TD, TE;
			      T1 = Ip[0];
			      T2 = Im[0];
			      TD = Rm[0];
			      TE = Rp[0];
			      TC = W[0];
			      T3 = T1 - T2;
			      TI = T1 + T2;
			      TQ = TE + TD;
			      TF = TD - TE;
			      TH = W[1];
			 }
			 {
			      E Tr, To, Ts, Tl, Tq;
			      {
				   E Tm, Tn, TG, T11;
				   Tm = Rm[WS(rs, 2)];
				   Tn = Rp[WS(rs, 2)];
				   TG = TC * TF;
				   T11 = TH * TF;
				   Tr = Ip[WS(rs, 2)];
				   TA = Tn + Tm;
				   To = Tm - Tn;
				   TJ = FNMS(TH, TI, TG);
				   T12 = FMA(TC, TI, T11);
				   Ts = Im[WS(rs, 2)];
			      }
			      Tl = W[8];
			      Tq = W[9];
			      {
				   E Tz, Ty, TW, Tx, Tt, Tp;
				   Tw = W[6];
				   Tx = Tr - Ts;
				   Tt = Tr + Ts;
				   Tp = Tl * To;
				   Tz = W[7];
				   Ty = Tw * Tx;
				   TW = Tl * Tt;
				   Tu = FNMS(Tq, Tt, Tp);
				   TZ = Tz * Tx;
				   TB = FNMS(Tz, TA, Ty);
				   TX = FMA(Tq, To, TW);
			      }
			 }
			 {
			      E T5, T6, Ta, Tb;
			      T5 = Ip[WS(rs, 1)];
			      T10 = FMA(Tw, TA, TZ);
			      T6 = Im[WS(rs, 1)];
			      Ta = Rp[WS(rs, 1)];
			      Tb = Rm[WS(rs, 1)];
			      {
				   E T4, Tg, T7, Tc, T9, T8, TR;
				   T4 = W[5];
				   Tg = T5 - T6;
				   T7 = T5 + T6;
				   Tj = Ta + Tb;
				   Tc = Ta - Tb;
				   T9 = W[4];
				   T8 = T4 * T7;
				   Tf = W[2];
				   Ti = W[3];
				   TR = T9 * T7;
				   Td = FMA(T9, Tc, T8);
				   Th = Tf * Tg;
				   TU = Ti * Tg;
				   TS = FNMS(T4, Tc, TR);
			      }
			 }
		    }
		    {
			 E Te, T1d, TK, Tv, T1a, T1b, Tk, TV;
			 TP = Td + T3;
			 Te = T3 - Td;
			 Tk = FNMS(Ti, Tj, Th);
			 TV = FMA(Tf, Tj, TU);
			 T1d = TQ + TS;
			 TT = TQ - TS;
			 TN = TJ - TB;
			 TK = TB + TJ;
			 Tv = Tk + Tu;
			 TM = Tu - Tk;
			 TY = TV - TX;
			 T1a = TV + TX;
			 T1b = T10 + T12;
			 T13 = T10 - T12;
			 {
			      E T1g, TL, T1e, T1c, T19, T1f;
			      T1g = Tv - TK;
			      TL = Tv + TK;
			      T1e = T1a + T1b;
			      T1c = T1a - T1b;
			      T19 = FNMS(KP500000000, TL, Te);
			      Ip[0] = KP500000000 * (Te + TL);
			      T1f = FNMS(KP500000000, T1e, T1d);
			      Rp[0] = KP500000000 * (T1d + T1e);
			      Im[WS(rs, 1)] = -(KP500000000 * (FNMS(KP866025403, T1c, T19)));
			      Ip[WS(rs, 2)] = KP500000000 * (FMA(KP866025403, T1c, T19));
			      Rm[WS(rs, 1)] = KP500000000 * (FMA(KP866025403, T1g, T1f));
			      Rp[WS(rs, 2)] = KP500000000 * (FNMS(KP866025403, T1g, T1f));
			 }
		    }
	       }
	       {
		    E TO, T16, T14, T18, T17, T15;
		    TO = TM + TN;
		    T16 = TN - TM;
		    T14 = TY + T13;
		    T18 = T13 - TY;
		    T17 = FMA(KP500000000, TO, TP);
		    Im[WS(rs, 2)] = KP500000000 * (TO - TP);
		    T15 = FNMS(KP500000000, T14, TT);
		    Rm[WS(rs, 2)] = KP500000000 * (TT + T14);
		    Im[0] = -(KP500000000 * (FNMS(KP866025403, T18, T17)));
		    Ip[WS(rs, 1)] = KP500000000 * (FMA(KP866025403, T18, T17));
		    Rm[0] = KP500000000 * (FNMS(KP866025403, T16, T15));
		    Rp[WS(rs, 1)] = KP500000000 * (FMA(KP866025403, T16, T15));
	       }
	  }
     }
}

static const tw_instr twinstr[] = {
     {TW_FULL, 1, 6},
     {TW_NEXT, 1, 0}
};

static const hc2c_desc desc = { 6, "hc2cfdft_6", twinstr, &GENUS, {36, 22, 22, 0} };

void X(codelet_hc2cfdft_6) (planner *p) {
     X(khc2c_register) (p, hc2cfdft_6, &desc, HC2C_VIA_DFT);
}
#else				/* HAVE_FMA */

/* Generated by: ../../../genfft/gen_hc2cdft.native -compact -variables 4 -pipeline-latency 4 -n 6 -dit -name hc2cfdft_6 -include hc2cf.h */

/*
 * This function contains 58 FP additions, 36 FP multiplications,
 * (or, 44 additions, 22 multiplications, 14 fused multiply/add),
 * 40 stack variables, 3 constants, and 24 memory accesses
 */
#include "hc2cf.h"

static void hc2cfdft_6(R *Rp, R *Ip, R *Rm, R *Im, const R *W, stride rs, INT mb, INT me, INT ms)
{
     DK(KP250000000, +0.250000000000000000000000000000000000000000000);
     DK(KP500000000, +0.500000000000000000000000000000000000000000000);
     DK(KP433012701, +0.433012701892219323381861585376468091735701313);
     {
	  INT m;
	  for (m = mb, W = W + ((mb - 1) * 10); m < me; m = m + 1, Rp = Rp + ms, Ip = Ip + ms, Rm = Rm - ms, Im = Im - ms, W = W + 10, MAKE_VOLATILE_STRIDE(24, rs)) {
	       E T3, TM, Tc, TN, Ts, T10, TI, TR, TF, T11, TH, TU;
	       {
		    E T1, T2, TD, Tz, TA, TB, T7, Tf, Tb, Th, Tq, Tw, Tm, Tu, T4;
		    E T8;
		    {
			 E T5, T6, T9, Ta;
			 T1 = Ip[0];
			 T2 = Im[0];
			 TD = T1 + T2;
			 Tz = Rm[0];
			 TA = Rp[0];
			 TB = Tz - TA;
			 T5 = Ip[WS(rs, 1)];
			 T6 = Im[WS(rs, 1)];
			 T7 = T5 + T6;
			 Tf = T5 - T6;
			 T9 = Rp[WS(rs, 1)];
			 Ta = Rm[WS(rs, 1)];
			 Tb = T9 - Ta;
			 Th = T9 + Ta;
			 {
			      E To, Tp, Tk, Tl;
			      To = Rp[WS(rs, 2)];
			      Tp = Rm[WS(rs, 2)];
			      Tq = To - Tp;
			      Tw = To + Tp;
			      Tk = Ip[WS(rs, 2)];
			      Tl = Im[WS(rs, 2)];
			      Tm = Tk + Tl;
			      Tu = Tk - Tl;
			 }
		    }
		    T3 = T1 - T2;
		    TM = TA + Tz;
		    T4 = W[5];
		    T8 = W[4];
		    Tc = FMA(T4, T7, T8 * Tb);
		    TN = FNMS(T4, Tb, T8 * T7);
		    {
			 E Ti, TP, Tr, TQ;
			 {
			      E Te, Tg, Tj, Tn;
			      Te = W[2];
			      Tg = W[3];
			      Ti = FNMS(Tg, Th, Te * Tf);
			      TP = FMA(Tg, Tf, Te * Th);
			      Tj = W[9];
			      Tn = W[8];
			      Tr = FMA(Tj, Tm, Tn * Tq);
			      TQ = FNMS(Tj, Tq, Tn * Tm);
			 }
			 Ts = Ti - Tr;
			 T10 = TP + TQ;
			 TI = Ti + Tr;
			 TR = TP - TQ;
		    }
		    {
			 E Tx, TS, TE, TT;
			 {
			      E Tt, Tv, Ty, TC;
			      Tt = W[6];
			      Tv = W[7];
			      Tx = FNMS(Tv, Tw, Tt * Tu);
			      TS = FMA(Tv, Tu, Tt * Tw);
			      Ty = W[0];
			      TC = W[1];
			      TE = FNMS(TC, TD, Ty * TB);
			      TT = FMA(TC, TB, Ty * TD);
			 }
			 TF = Tx + TE;
			 T11 = TS + TT;
			 TH = TE - Tx;
			 TU = TS - TT;
		    }
	       }
	       {
		    E T12, Td, TG, TZ;
		    T12 = KP433012701 * (T10 - T11);
		    Td = T3 - Tc;
		    TG = Ts + TF;
		    TZ = FNMS(KP250000000, TG, KP500000000 * Td);
		    Ip[0] = KP500000000 * (Td + TG);
		    Im[WS(rs, 1)] = T12 - TZ;
		    Ip[WS(rs, 2)] = TZ + T12;
	       }
	       {
		    E T16, T13, T14, T15;
		    T16 = KP433012701 * (Ts - TF);
		    T13 = TM + TN;
		    T14 = T10 + T11;
		    T15 = FNMS(KP250000000, T14, KP500000000 * T13);
		    Rp[WS(rs, 2)] = T15 - T16;
		    Rp[0] = KP500000000 * (T13 + T14);
		    Rm[WS(rs, 1)] = T16 + T15;
	       }
	       {
		    E TY, TJ, TK, TX;
		    TY = KP433012701 * (TU - TR);
		    TJ = TH - TI;
		    TK = Tc + T3;
		    TX = FMA(KP500000000, TK, KP250000000 * TJ);
		    Im[WS(rs, 2)] = KP500000000 * (TJ - TK);
		    Im[0] = TY - TX;
		    Ip[WS(rs, 1)] = TX + TY;
	       }
	       {
		    E TL, TO, TV, TW;
		    TL = KP433012701 * (TI + TH);
		    TO = TM - TN;
		    TV = TR + TU;
		    TW = FNMS(KP250000000, TV, KP500000000 * TO);
		    Rp[WS(rs, 1)] = TL + TW;
		    Rm[WS(rs, 2)] = KP500000000 * (TO + TV);
		    Rm[0] = TW - TL;
	       }
	  }
     }
}

static const tw_instr twinstr[] = {
     {TW_FULL, 1, 6},
     {TW_NEXT, 1, 0}
};

static const hc2c_desc desc = { 6, "hc2cfdft_6", twinstr, &GENUS, {44, 22, 14, 0} };

void X(codelet_hc2cfdft_6) (planner *p) {
     X(khc2c_register) (p, hc2cfdft_6, &desc, HC2C_VIA_DFT);
}
#endif				/* HAVE_FMA */
