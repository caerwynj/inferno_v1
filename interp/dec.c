/* Machine generated by decgen.c */

#include "lib9.h"
#include "isa.h"
#include "interp.h"

#define DIND(reg, xxx) (uchar*)((*(ulong*)(R.reg+R.PC->xxx.i.f))+R.PC->xxx.i.s)
void
D00(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D01(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D02(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D03(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D04(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D05(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D06(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D07(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D08(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D09(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D0A(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D0B(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D0C(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D0D(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D0E(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D0F(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D10(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D11(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D12(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D13(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D14(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D15(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D16(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D17(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D18(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D19(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D1A(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D1B(void)
{
}
void
D1C(void)
{
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D1D(void)
{
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D1E(void)
{
}
void
D1F(void)
{
}
void
D20(void)
{
	R.s = DIND(MP, s);
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D21(void)
{
	R.s = DIND(MP, s);
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D22(void)
{
	R.s = DIND(MP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D23(void)
{
	R.s = DIND(MP, s);
}
void
D24(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D25(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D26(void)
{
	R.s = DIND(MP, s);
}
void
D27(void)
{
	R.s = DIND(MP, s);
}
void
D28(void)
{
	R.s = DIND(FP, s);
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D29(void)
{
	R.s = DIND(FP, s);
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D2A(void)
{
	R.s = DIND(FP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D2B(void)
{
	R.s = DIND(FP, s);
}
void
D2C(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D2D(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D2E(void)
{
	R.s = DIND(FP, s);
}
void
D2F(void)
{
	R.s = DIND(FP, s);
}
void
D30(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D31(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D32(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D33(void)
{
}
void
D34(void)
{
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D35(void)
{
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D36(void)
{
}
void
D37(void)
{
}
void
D38(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.d;
}
void
D39(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.d;
}
void
D3A(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.d;
}
void
D3B(void)
{
}
void
D3C(void)
{
	R.d = DIND(MP, d);
	R.m = R.d;
}
void
D3D(void)
{
	R.d = DIND(FP, d);
	R.m = R.d;
}
void
D3E(void)
{
}
void
D3F(void)
{
}
void
D40(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D41(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D42(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D43(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D44(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D45(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D46(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D47(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D48(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D49(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D4A(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D4B(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D4C(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D4D(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D4E(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D4F(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D50(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D51(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D52(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D53(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D54(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D55(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D56(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D57(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D58(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D59(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D5A(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D5B(void)
{
}
void
D5C(void)
{
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D5D(void)
{
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D5E(void)
{
}
void
D5F(void)
{
}
void
D60(void)
{
	R.s = DIND(MP, s);
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D61(void)
{
	R.s = DIND(MP, s);
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D62(void)
{
	R.s = DIND(MP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D63(void)
{
	R.s = DIND(MP, s);
}
void
D64(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D65(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D66(void)
{
	R.s = DIND(MP, s);
}
void
D67(void)
{
	R.s = DIND(MP, s);
}
void
D68(void)
{
	R.s = DIND(FP, s);
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D69(void)
{
	R.s = DIND(FP, s);
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D6A(void)
{
	R.s = DIND(FP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D6B(void)
{
	R.s = DIND(FP, s);
}
void
D6C(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D6D(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D6E(void)
{
	R.s = DIND(FP, s);
}
void
D6F(void)
{
	R.s = DIND(FP, s);
}
void
D70(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D71(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D72(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D73(void)
{
}
void
D74(void)
{
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D75(void)
{
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D76(void)
{
}
void
D77(void)
{
}
void
D78(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D79(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D7A(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D7B(void)
{
}
void
D7C(void)
{
	R.d = DIND(MP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D7D(void)
{
	R.d = DIND(FP, d);
	R.t = (short)R.PC->reg;
	R.m = &R.t;
}
void
D7E(void)
{
}
void
D7F(void)
{
}
void
D80(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D81(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D82(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
D83(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D84(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
D85(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
D86(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D87(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
D88(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D89(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D8A(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
D8B(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D8C(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
D8D(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
D8E(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D8F(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
D90(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D91(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D92(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
D93(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D94(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
D95(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
D96(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D97(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
D98(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D99(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
D9A(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
D9B(void)
{
}
void
D9C(void)
{
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
D9D(void)
{
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
D9E(void)
{
}
void
D9F(void)
{
}
void
DA0(void)
{
	R.s = DIND(MP, s);
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DA1(void)
{
	R.s = DIND(MP, s);
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DA2(void)
{
	R.s = DIND(MP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
DA3(void)
{
	R.s = DIND(MP, s);
}
void
DA4(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
DA5(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
DA6(void)
{
	R.s = DIND(MP, s);
}
void
DA7(void)
{
	R.s = DIND(MP, s);
}
void
DA8(void)
{
	R.s = DIND(FP, s);
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DA9(void)
{
	R.s = DIND(FP, s);
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DAA(void)
{
	R.s = DIND(FP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
DAB(void)
{
	R.s = DIND(FP, s);
}
void
DAC(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
DAD(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
DAE(void)
{
	R.s = DIND(FP, s);
}
void
DAF(void)
{
	R.s = DIND(FP, s);
}
void
DB0(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DB1(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DB2(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
DB3(void)
{
}
void
DB4(void)
{
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
DB5(void)
{
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
DB6(void)
{
}
void
DB7(void)
{
}
void
DB8(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DB9(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.FP+R.PC->reg;
}
void
DBA(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.FP+R.PC->reg;
}
void
DBB(void)
{
}
void
DBC(void)
{
	R.d = DIND(MP, d);
	R.m = R.FP+R.PC->reg;
}
void
DBD(void)
{
	R.d = DIND(FP, d);
	R.m = R.FP+R.PC->reg;
}
void
DBE(void)
{
}
void
DBF(void)
{
}
void
DC0(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DC1(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DC2(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DC3(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
DC4(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DC5(void)
{
	R.s = R.MP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DC6(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
DC7(void)
{
	R.s = R.MP+R.PC->s.ind;
}
void
DC8(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DC9(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DCA(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DCB(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
DCC(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DCD(void)
{
	R.s = R.FP+R.PC->s.ind;
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DCE(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
DCF(void)
{
	R.s = R.FP+R.PC->s.ind;
}
void
DD0(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DD1(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DD2(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DD3(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
DD4(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DD5(void)
{
	R.s = (uchar*)&R.PC->s.imm;
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DD6(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
DD7(void)
{
	R.s = (uchar*)&R.PC->s.imm;
}
void
DD8(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DD9(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DDA(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DDB(void)
{
}
void
DDC(void)
{
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DDD(void)
{
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DDE(void)
{
}
void
DDF(void)
{
}
void
DE0(void)
{
	R.s = DIND(MP, s);
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DE1(void)
{
	R.s = DIND(MP, s);
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DE2(void)
{
	R.s = DIND(MP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DE3(void)
{
	R.s = DIND(MP, s);
}
void
DE4(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DE5(void)
{
	R.s = DIND(MP, s);
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DE6(void)
{
	R.s = DIND(MP, s);
}
void
DE7(void)
{
	R.s = DIND(MP, s);
}
void
DE8(void)
{
	R.s = DIND(FP, s);
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DE9(void)
{
	R.s = DIND(FP, s);
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DEA(void)
{
	R.s = DIND(FP, s);
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DEB(void)
{
	R.s = DIND(FP, s);
}
void
DEC(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DED(void)
{
	R.s = DIND(FP, s);
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DEE(void)
{
	R.s = DIND(FP, s);
}
void
DEF(void)
{
	R.s = DIND(FP, s);
}
void
DF0(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DF1(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DF2(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DF3(void)
{
}
void
DF4(void)
{
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DF5(void)
{
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DF6(void)
{
}
void
DF7(void)
{
}
void
DF8(void)
{
	R.d = R.MP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DF9(void)
{
	R.d = R.FP+R.PC->d.ind;
	R.m = R.MP+R.PC->reg;
}
void
DFA(void)
{
	R.d = (uchar*)&R.PC->d.imm;
	R.m = R.MP+R.PC->reg;
}
void
DFB(void)
{
}
void
DFC(void)
{
	R.d = DIND(MP, d);
	R.m = R.MP+R.PC->reg;
}
void
DFD(void)
{
	R.d = DIND(FP, d);
	R.m = R.MP+R.PC->reg;
}
void
DFE(void)
{
}
void
DFF(void)
{
}

void	(*dec[])(void) =
{
	D00,
	D01,
	D02,
	D03,
	D04,
	D05,
	D06,
	D07,
	D08,
	D09,
	D0A,
	D0B,
	D0C,
	D0D,
	D0E,
	D0F,
	D10,
	D11,
	D12,
	D13,
	D14,
	D15,
	D16,
	D17,
	D18,
	D19,
	D1A,
	D1B,
	D1C,
	D1D,
	D1E,
	D1F,
	D20,
	D21,
	D22,
	D23,
	D24,
	D25,
	D26,
	D27,
	D28,
	D29,
	D2A,
	D2B,
	D2C,
	D2D,
	D2E,
	D2F,
	D30,
	D31,
	D32,
	D33,
	D34,
	D35,
	D36,
	D37,
	D38,
	D39,
	D3A,
	D3B,
	D3C,
	D3D,
	D3E,
	D3F,
	D40,
	D41,
	D42,
	D43,
	D44,
	D45,
	D46,
	D47,
	D48,
	D49,
	D4A,
	D4B,
	D4C,
	D4D,
	D4E,
	D4F,
	D50,
	D51,
	D52,
	D53,
	D54,
	D55,
	D56,
	D57,
	D58,
	D59,
	D5A,
	D5B,
	D5C,
	D5D,
	D5E,
	D5F,
	D60,
	D61,
	D62,
	D63,
	D64,
	D65,
	D66,
	D67,
	D68,
	D69,
	D6A,
	D6B,
	D6C,
	D6D,
	D6E,
	D6F,
	D70,
	D71,
	D72,
	D73,
	D74,
	D75,
	D76,
	D77,
	D78,
	D79,
	D7A,
	D7B,
	D7C,
	D7D,
	D7E,
	D7F,
	D80,
	D81,
	D82,
	D83,
	D84,
	D85,
	D86,
	D87,
	D88,
	D89,
	D8A,
	D8B,
	D8C,
	D8D,
	D8E,
	D8F,
	D90,
	D91,
	D92,
	D93,
	D94,
	D95,
	D96,
	D97,
	D98,
	D99,
	D9A,
	D9B,
	D9C,
	D9D,
	D9E,
	D9F,
	DA0,
	DA1,
	DA2,
	DA3,
	DA4,
	DA5,
	DA6,
	DA7,
	DA8,
	DA9,
	DAA,
	DAB,
	DAC,
	DAD,
	DAE,
	DAF,
	DB0,
	DB1,
	DB2,
	DB3,
	DB4,
	DB5,
	DB6,
	DB7,
	DB8,
	DB9,
	DBA,
	DBB,
	DBC,
	DBD,
	DBE,
	DBF,
	DC0,
	DC1,
	DC2,
	DC3,
	DC4,
	DC5,
	DC6,
	DC7,
	DC8,
	DC9,
	DCA,
	DCB,
	DCC,
	DCD,
	DCE,
	DCF,
	DD0,
	DD1,
	DD2,
	DD3,
	DD4,
	DD5,
	DD6,
	DD7,
	DD8,
	DD9,
	DDA,
	DDB,
	DDC,
	DDD,
	DDE,
	DDF,
	DE0,
	DE1,
	DE2,
	DE3,
	DE4,
	DE5,
	DE6,
	DE7,
	DE8,
	DE9,
	DEA,
	DEB,
	DEC,
	DED,
	DEE,
	DEF,
	DF0,
	DF1,
	DF2,
	DF3,
	DF4,
	DF5,
	DF6,
	DF7,
	DF8,
	DF9,
	DFA,
	DFB,
	DFC,
	DFD,
	DFE,
	DFF 
};