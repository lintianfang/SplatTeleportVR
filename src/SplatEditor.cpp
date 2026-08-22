#include "SplatEditor.h"

#include <algorithm>
#include <execution>

#include "GPUSorting/GPUSorting.h"
#include "GPUPrefixSums/GPUPrefixSums.h"
#include "GPUPrefixSums/GPUPrefixSumsCS.h"
#include "cudaGL.h"

#include "math.cuh"


//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980 + 1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

ImFont* getFont(int size){
	static unordered_map<int, ImFont*> cache;

	ImGuiIO& io = ImGui::GetIO();

	if(cache.find(size) == cache.end()){
		ImFontConfig font_cfg = ImFontConfig();
		font_cfg.OversampleH = font_cfg.OversampleV = 4;
		font_cfg.PixelSnapH = true;
		font_cfg.SizePixels = size;
		ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "ProggyClean.ttf, %dpx", (int)font_cfg.SizePixels);
		font_cfg.EllipsisChar = (ImWchar)0x0085;
		font_cfg.GlyphOffset.y = 1.0f * IM_FLOOR(font_cfg.SizePixels / 13.0f);  // Add +1 offset per 13 units

		const char* ttf_compressed_base85 = proggy_clean_ttf_compressed_data_base85;
		const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : io.Fonts->GetGlyphRangesDefault();
		ImFont* font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);

		cache[size] = font;
	}

	return cache[size];
}

ImFont* getFont(string path, int size){
	static unordered_map<string, ImFont*> cache;

	ImGuiIO& io = ImGui::GetIO();

	string id = format("{}_{}", path, size);
	string filename = fs::path(path).filename().string();

	if(cache.find(id) == cache.end()){
		ImFontConfig font_cfg = ImFontConfig();
		font_cfg.OversampleH = font_cfg.OversampleV = 4;
		font_cfg.PixelSnapH = true;
		font_cfg.SizePixels = size;
		ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %dpx", filename.c_str(), (int)font_cfg.SizePixels);
		font_cfg.EllipsisChar = (ImWchar)0x0085;
		font_cfg.GlyphOffset.y = 1.0f * IM_FLOOR(font_cfg.SizePixels / 13.0f);  // Add +1 offset per 13 units

		const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : io.Fonts->GetGlyphRangesDefault();
		
		ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), font_cfg.SizePixels, &font_cfg, glyph_ranges);

		cache[id] = font;
	}

	return cache[id];
}

void SplatEditor::setup(){

	SplatEditor::instance = new SplatEditor();
	SplatEditor* editor = SplatEditor::instance;

	editor->font_default      = getFont(13);
	editor->font_vr_title     = getFont("./resources/fonts/Carlito/Carlito-Bold.ttf", 50);
	editor->font_vr_text      = getFont("./resources/fonts/Carlito/Carlito-Bold.ttf", 33);
	editor->font_vr_smalltext = getFont("./resources/fonts/Carlito/Carlito-Bold.ttf", 24);

	editor->initCudaProgram();

	{ // init VR stuff
		editor->viewLeft.framebuffer  = GLRenderer::createFramebuffer(4096, 4096);
		editor->viewRight.framebuffer = GLRenderer::createFramebuffer(4096, 4096);

		editor->ovr = OpenVRHelper::instance();
	}

	editor->fbGuiVr = Framebuffer::create();
	editor->fbGuiVr_assets = Framebuffer::create();

	// fonts must be created before NewFrame and after EndFrame. Create them now.
	getFont(13);
	getFont(23);
	getFont(33);

	ImGuiIO& io = ImGui::GetIO();
	
	editor->imguicontext_desktop = ImGui::GetCurrentContext();
	editor->imguicontext_vr = ImGui::CreateContext(io.Fonts);

	editor->imn_assets = make_shared<ImguiNode>("Assets");
	editor->imn_assets->transform = mat4(
		 0.080,     -0.119,     -0.006,      0.000,
		 0.032,      0.015,      0.135,      0.000,
		-0.159,     -0.110,      0.050,      0.000,
		-0.270,     -0.240,      0.726,      1.000
	);

	editor->imn_brushes = make_shared<ImguiNode>("Brushes");
	editor->imn_brushes->transform = mat4(
		 0.001,      0.088,      0.002,      0.000,
		-0.030,     -0.001,      0.095,      0.000,
		 0.191,     -0.003,      0.059,      0.000,
		-1.251,     -0.582,      0.666,      1.000
	);

	editor->imn_layers = make_shared<ImguiNode>("Layers");
	editor->imn_layers->transform = mat4(
		 0.040,     -0.079,     -0.003,      0.000,
		 0.034,      0.012,      0.135,      0.000,
		-0.172,     -0.088,      0.051,      0.000,
		-0.138,     -0.456,      0.713,      1.000
	);

	editor->imn_painting = make_shared<ImguiNode>("Painting");
	editor->imn_painting->transform = mat4(
		 0.012,     -0.087,      0.004,      0.000,
		 0.028,      0.008,      0.096,      0.000,
		-0.190,     -0.024,      0.058,      0.000,
		-0.079,     -0.661,      0.673,      1.000
	);
	editor->imn_teleport = make_shared<ImguiNode>("Teleport");
	editor->imn_teleport->transform = editor->imn_painting->transform;

	editor->scene.root->children.push_back(editor->imn_assets);
	editor->scene.root->children.push_back(editor->imn_brushes);
	editor->scene.root->children.push_back(editor->imn_layers);
	editor->scene.root->children.push_back(editor->imn_painting);
	editor->scene.root->children.push_back(editor->imn_teleport);
	editor->setVRPanelMode(VRPanelMode::PAINTING);
	
	static shared_ptr<Mesh> mesh_wiresphere = nullptr;

	auto box = Mesh::createBox();
	editor->sn_box = make_shared<SNTriangles>("Box");
	editor->sn_box->set(box->position, box->uv, box->color);

	auto plane = Mesh::createPlane(512);
	editor->sn_vr_editing = make_shared<SNTriangles>("vr_gui_editing");
	editor->sn_vr_editing->set(plane->position, plane->uv);
	editor->sn_vr_editing->hidden = true;
	editor->sn_vr_editing->visible = false;
	editor->sn_vr_editing->transform = glm::translate(vec3{0.0f, 0.0f, 1.5f}) * glm::scale(vec3{1.0f, 1.0f, 1.0f} * 0.2f);

	float values[16] = {
		 0.054f,      0.069f,     -0.005f,      0.000f,
		-0.029f,      0.032f,      0.133f,      0.000f,
		 0.152f,     -0.115f,      0.060f,      0.000f,
		-1.303f,     -0.237f,      0.662f,      1.000f,
	};
	memcpy(&editor->sn_vr_editing->transform, values, 16 * 4);


	auto sphere = Mesh::createSphere();
	editor->sn_dbgsphere = make_shared<SNTriangles>("debugSphere");
	editor->sn_dbgsphere->set(sphere->position, sphere->uv);
	editor->sn_dbgsphere->transform = glm::scale(vec3{0.1f, 0.1f, 0.1f});
	editor->sn_dbgsphere->hidden = true;
	editor->sn_dbgsphere->visible = false;
	editor->scene.root->children.push_back(editor->sn_dbgsphere);

	auto brushsphere = Splats::createSphere();
	editor->sn_brushsphere = make_shared<SNSplats>("brush sphere", brushsphere);
	editor->sn_brushsphere->hidden = true;
	editor->sn_brushsphere->visible = false;
	editor->scene.vr->children.push_back(editor->sn_brushsphere);
}

void SplatEditor::imguiStyleVR(){
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	style->WindowBorderSize = 5.0f;

	colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.67f, 0.86f, 0.64f, 1.00f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.85f, 0.94f, 0.83f, 1.00f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.52f, 0.80f, 0.48f, 1.00f);
	colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
	colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
	colors[ImGuiCol_TableRowBg]             = ImVec4(0.67f, 0.86f, 0.64f, 1.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

Box3 SplatEditor::getSelectionAABB(){

	static CUdeviceptr cptr_box;
	static bool initialized = false;
	if(!initialized){
		cptr_box = CURuntime::alloc("AABB", sizeof(Box3));

		initialized = true;
	}

	float inf = Infinity;
	float ninf = -Infinity;
	uint32_t iinf = bit_cast<uint32_t>(inf);
	uint32_t niinf = bit_cast<uint32_t>(ninf);
	cuMemsetD32(cptr_box +  0, iinf, 3);
	cuMemsetD32(cptr_box + 12, niinf, 3);

	if(Runtime::numSelectedSplats > 0){
		// AABB of selected splats
		scene.process<SNSplats>([&](SNSplats* node) {

			if (!node->visible) return;
			if (node->locked) return;

			auto model = node->dmng.data;
			bool onlySelected = true;

			void* args[] = {&launchArgs, &model, &cptr_box, &onlySelected};
			prog_gaussians_editing->launch("kernel_compute_boundingbox2", args, {
				.gridsize = uint32_t(2 * CURuntime::getNumSMs()), 
				.blocksize = 256
			});
		});
	}else{
		// AABB of selected node
		scene.process<SNSplats>([&](SNSplats* node) {

			if (!node->visible) return;
			if (node->locked) return;
			if(!node->selected) return;

			auto model = node->dmng.data;
			bool onlySelected = false;

			void* args[] = {&launchArgs, &model, &cptr_box, &onlySelected};
			prog_gaussians_editing->launch("kernel_compute_boundingbox2", args, {
				.gridsize = uint32_t(2 * CURuntime::getNumSMs()), 
				.blocksize = 256
			});
		});
	}

	
	
	Box3 box;
	cuMemcpyDtoH(&box, cptr_box, sizeof(Box3));

	return box;
}

void SplatEditor::updateBoundingBox(SNSplats* node, bool onlySelected){

	auto model = node->dmng.data;

	static CUdeviceptr cptr_box;
	static bool initialized = false;
	if(!initialized){
		cptr_box = CURuntime::alloc("AABB", sizeof(Box3));

		initialized = true;
	}

	float inf = Infinity;
	float ninf = -Infinity;
	uint32_t iinf = bit_cast<uint32_t>(inf);
	uint32_t niinf = bit_cast<uint32_t>(ninf);
	cuMemsetD32(cptr_box +  0, iinf, 3);
	cuMemsetD32(cptr_box + 12, niinf, 3);

	bool _onlySelected = onlySelected;

	void* args[] = {&launchArgs, &model, &cptr_box, &_onlySelected};
	prog_gaussians_editing->launch("kernel_compute_boundingbox2", args, {
		.gridsize = uint32_t(2 * CURuntime::getNumSMs()), 
		.blocksize = 256
	});

	Box3 box;
	cuMemcpyDtoH(&box, cptr_box, sizeof(Box3));

	node->aabb.min = box.min;
	node->aabb.max = box.max;
	node->dmng.data.min = box.min;
	node->dmng.data.max = box.max;
}

void SplatEditor::updateBoundingBox(SNTriangles* node){

	auto& model = node->data;
	model.transform = node->transform;

	static CUdeviceptr cptr_min, cptr_max;
	static bool initialized = false;
	if(!initialized){
		cptr_min = CURuntime::alloc("min", sizeof(vec3));
		cptr_max = CURuntime::alloc("max", sizeof(vec3));

		initialized = true;
	}

	float inf = Infinity;
	float ninf = -Infinity;
	uint32_t iinf = bit_cast<uint32_t>(inf);
	uint32_t niinf = bit_cast<uint32_t>(ninf);
	cuMemsetD32(cptr_min, iinf, 3);
	cuMemsetD32(cptr_max, niinf, 3);

	void* args[] = {&launchArgs, &model, &cptr_min, &cptr_max};
	prog_triangles->launch("kernel_compute_boundingbox", args, model.count);


	cuMemcpyDtoH(&model.min, cptr_min, sizeof(vec3));
	cuMemcpyDtoH(&model.max, cptr_max, sizeof(vec3));

	node->aabb.min = model.min;
	node->aabb.max = model.max;

	int dbg = 10;
}

void SplatEditor::updateBoundingBox(PointData& model){

	static CUdeviceptr cptr_min, cptr_max;
	static bool initialized = false;
	if(!initialized){
		cptr_min = CURuntime::alloc("min", sizeof(vec3));
		cptr_max = CURuntime::alloc("max", sizeof(vec3));
	}

	float inf = Infinity;
	float ninf = -Infinity;
	uint32_t iinf = bit_cast<uint32_t>(inf);
	uint32_t niinf = bit_cast<uint32_t>(ninf);
	cuMemsetD32(cptr_min, iinf, 3);
	cuMemsetD32(cptr_max, niinf, 3);
	
	void* args[] = {&launchArgs, &model, &cptr_min, &cptr_max};
	prog_points->launch("kernel_compute_boundingbox", args, model.count);

	cuMemcpyDtoH(&model.min, cptr_min, sizeof(vec3));
	cuMemcpyDtoH(&model.max, cptr_max, sizeof(vec3));
}

// Inserts contents of one node into another node. 
// Node Types must match.
void SplatEditor::insertNodeToNode(shared_ptr<SceneNode> node, shared_ptr<SceneNode> layer, bool onlySelected){

	shared_ptr<SNSplats> source = dynamic_pointer_cast<SNSplats>(node);
	shared_ptr<SNSplats> target = dynamic_pointer_cast<SNSplats>(layer);

	if(source && target){
		GaussianData& gd_source = source->dmng.data;
		GaussianData& gd_target = target->dmng.data;

		int numRequiredSplats = target->dmng.data.count + source->dmng.data.count;
		target->dmng.commit(numRequiredSplats);

		cuMemcpy((CUdeviceptr)gd_target.position   + sizeof(*gd_target.position)   * gd_target.count, (CUdeviceptr)gd_source.position   , sizeof(*gd_target.position)   * gd_source.count);
		cuMemcpy((CUdeviceptr)gd_target.scale      + sizeof(*gd_target.scale)      * gd_target.count, (CUdeviceptr)gd_source.scale      , sizeof(*gd_target.scale)      * gd_source.count);
		cuMemcpy((CUdeviceptr)gd_target.quaternion + sizeof(*gd_target.quaternion) * gd_target.count, (CUdeviceptr)gd_source.quaternion , sizeof(*gd_target.quaternion) * gd_source.count);
		cuMemcpy((CUdeviceptr)gd_target.color      + sizeof(*gd_target.color)      * gd_target.count, (CUdeviceptr)gd_source.color      , sizeof(*gd_target.color)      * gd_source.count);
		cuMemcpy((CUdeviceptr)gd_target.flags      + sizeof(*gd_target.flags)      * gd_target.count, (CUdeviceptr)gd_source.flags      , sizeof(*gd_target.flags)      * gd_source.count);

		// We need to transform inserted splats from source's coordinate system into target's coordinate system
		mat4 transform = inverse(target->transform) * source->transform;

		bool _onlySelected = onlySelected;
		uint32_t first = gd_target.count;
		uint32_t count = gd_source.count;
		gd_target.count += gd_source.count;
		prog_gaussians_editing->launch(
			"kernel_apply_transformation", 
			{&launchArgs, &gd_target, &transform, &first, &count, &_onlySelected }, 
			count
		);
		
		
		// sortSplatsDevice(target);
	}

}

void SplatEditor::merge_undoable(shared_ptr<SceneNode> snsource, shared_ptr<SceneNode> sntarget){

	shared_ptr<SNSplats> source = dynamic_pointer_cast<SNSplats>(snsource);
	shared_ptr<SNSplats> target = dynamic_pointer_cast<SNSplats>(sntarget);

	if(source == nullptr || target == nullptr) return;


	struct UndoAction : public Action{

		shared_ptr<SNSplats> source;
		shared_ptr<SNSplats> target;
		uint32_t originalSourceCount = 0;
		uint32_t originalTargetCount = 0;
		int sourceIndex = 0;

		void undo(){

			cuCtxSynchronize();

			auto editor = SplatEditor::instance;

			source->dmng.commit(originalSourceCount);

			GaussianData& gd_source = source->dmng.data;
			GaussianData& gd_target = target->dmng.data;


			CURuntime::check(cuMemcpy((CUdeviceptr)gd_source.position   , (CUdeviceptr)gd_target.position   + sizeof(*gd_target.position)   * originalTargetCount , sizeof(*gd_target.position)   * originalSourceCount));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_source.scale      , (CUdeviceptr)gd_target.scale      + sizeof(*gd_target.scale)      * originalTargetCount , sizeof(*gd_target.scale)      * originalSourceCount));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_source.quaternion , (CUdeviceptr)gd_target.quaternion + sizeof(*gd_target.quaternion) * originalTargetCount , sizeof(*gd_target.quaternion) * originalSourceCount));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_source.color      , (CUdeviceptr)gd_target.color      + sizeof(*gd_target.color)      * originalTargetCount , sizeof(*gd_target.color)      * originalSourceCount));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_source.flags      , (CUdeviceptr)gd_target.flags      + sizeof(*gd_target.flags)      * originalTargetCount , sizeof(*gd_target.flags)      * originalSourceCount));

			// We need to transform splats back to the original source coordinate system
			mat4 transform = inverse(source->transform) * target->transform;

			bool _onlySelected = false;
			uint32_t first = 0;
			uint32_t count = originalSourceCount;
			gd_source.count = originalSourceCount;
			gd_target.count = originalTargetCount;
			
			editor->prog_gaussians_editing->launch(
				"kernel_apply_transformation", 
				{&editor->launchArgs, &gd_source, &transform, &first, &count, &_onlySelected }, 
				count
			);

			target->dmng.commit(originalTargetCount);

			// editor->scene.world->children.push_back(source);
			editor->scene.world->children.insert(editor->scene.world->children.begin() + sourceIndex, source);

			editor->setSelectedNode(source.get());
			cuCtxSynchronize();
		}

		void redo(){
			cuCtxSynchronize();
			auto editor = SplatEditor::instance;

			GaussianData& gd_source = source->dmng.data;
			GaussianData& gd_target = target->dmng.data;

			int numRequiredSplats = target->dmng.data.count + source->dmng.data.count;
			target->dmng.commit(numRequiredSplats);

			CURuntime::check(cuMemcpy((CUdeviceptr)gd_target.position   + sizeof(*gd_target.position)   * gd_target.count, (CUdeviceptr)gd_source.position   , sizeof(*gd_target.position)   * gd_source.count));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_target.scale      + sizeof(*gd_target.scale)      * gd_target.count, (CUdeviceptr)gd_source.scale      , sizeof(*gd_target.scale)      * gd_source.count));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_target.quaternion + sizeof(*gd_target.quaternion) * gd_target.count, (CUdeviceptr)gd_source.quaternion , sizeof(*gd_target.quaternion) * gd_source.count));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_target.color      + sizeof(*gd_target.color)      * gd_target.count, (CUdeviceptr)gd_source.color      , sizeof(*gd_target.color)      * gd_source.count));
			CURuntime::check(cuMemcpy((CUdeviceptr)gd_target.flags      + sizeof(*gd_target.flags)      * gd_target.count, (CUdeviceptr)gd_source.flags      , sizeof(*gd_target.flags)      * gd_source.count));

			// We need to transform inserted splats from source's coordinate system into target's coordinate system
			mat4 transform = inverse(target->transform) * source->transform;

			bool _onlySelected = false;
			uint32_t first = gd_target.count;
			uint32_t count = gd_source.count;
			gd_target.count += gd_source.count;
			editor->prog_gaussians_editing->launch(
				"kernel_apply_transformation", 
				{&editor->launchArgs, &gd_target, &transform, &first, &count, &_onlySelected }, 
				count
			);

			// TODO: Removing memory causes errors later on :(
			// source->dmng.commit(0);
			source->dmng.data.count = 0;
			
			editor->scene.world->remove(source.get());
			editor->setSelectedNode(target.get());
			cuCtxSynchronize();
		}

		int64_t byteSize(){
			return source->getGpuMemoryUsage();
		}

	};

	shared_ptr<UndoAction> action = make_shared<UndoAction>();
	action->source = source;
	action->target = target;
	action->originalSourceCount = source->dmng.data.count;
	action->originalTargetCount = target->dmng.data.count;
	action->sourceIndex = 0;
	for(int index = 0; index < scene.world->children.size(); index++){
		if(scene.world->children[index] == source){
			action->sourceIndex = index;
		}
	}
	

	action->redo();

	addAction(action);

}

void SplatEditor::applyTransformation(GaussianData& model, mat4 transformation, bool onlySelected){

	uint32_t start = 0;
	mat4 _transformation = transformation;
	bool _onlySelected = onlySelected;
	void* args[] = {&launchArgs, &model, &_transformation, &start, &model.count, &_onlySelected};

	prog_gaussians_editing->launch("kernel_apply_transformation", args, model.count);
}

void SplatEditor::apply(GaussianData& model, ColorCorrection value){
	void* args[] = {&launchArgs, &model, &value};

	prog_gaussians_editing->launch("kernel_apply_colorCorrection", args, model.count);
}

//void SplatEditor::setSelected(GaussianData& model){
//
//	// unselect all models
//	scene.process<SNSplats>([&](SNSplats* node){
//		if(node->dmng.data.locked) return;
//
//		void* changedmask = nullptr;
//		prog_gaussians_editing->launch("kernel_deselect", {&launchArgs, &node->dmng.data, &changedmask}, node->dmng.data.count);
//	});
//
//	// select specified
//	if(!model.locked){
//		void* changedmask = nullptr;
//		prog_gaussians_editing->launch("kernel_select", {&launchArgs, &model, changedmask}, model.count);
//	}
//}

void SplatEditor::transformAllSelected(mat4 transform){

	scene.process<SNSplats>([&](SNSplats* node){

		if(node->locked) return;
		if(!node->visible) return;

		void* args[] = {&launchArgs, &node->dmng.data};
		prog_gaussians_editing->launch("kernel_transform", args, node->dmng.data.count);
	});

}

void SplatEditor::selectAll(){
	
	scene.process<SNSplats>([&](SNSplats* node){

		if(node->dmng.data.locked) return;

		void* changedmask = nullptr;
		prog_gaussians_editing->launch("kernel_select", {&launchArgs, &node->dmng.data, &changedmask}, node->dmng.data.count);
	});
}

void SplatEditor::selectAllInNode_undoable(shared_ptr<SNSplats> node){

	if(!node->visible) return;
	if(node->locked) return;
	
	struct UndoData{
		shared_ptr<SNSplats> node;
		CUdeviceptr cptr_changedmask = 0;

		~UndoData(){
			println("Freeing SelectAllInNode Undodata");
			if(cptr_changedmask) CURuntime::free(cptr_changedmask);
		}
	};

	shared_ptr<UndoData> undodata = make_shared<UndoData>();

	int requiredBytes = node->dmng.data.count / 8 + 4;
	undodata->node = node;
	undodata->cptr_changedmask = CURuntime::alloc("selectAllInNode changedmask", requiredBytes);
	prog_gaussians_editing->launch("kernel_select", {&launchArgs, &node->dmng.data, &undodata->cptr_changedmask}, node->dmng.data.count);
	
	addAction({
		.undo = [=](){
			void* args[] = {&undodata->node->dmng.data, &undodata->cptr_changedmask};
			prog_gaussians_editing->launch("kernel_deselect_masked", args, undodata->node->dmng.data.count);
			
		},
		.redo = [=](){
			void* args[] = {&undodata->node->dmng.data, &undodata->cptr_changedmask};
			prog_gaussians_editing->launch("kernel_select_masked", args, undodata->node->dmng.data.count);
		},
	});
}

void SplatEditor::selectAll_undoable(){

	struct UndoData{
		vector<SNSplats*> nodes;
		vector<CUdeviceptr> changedmasks;

		~UndoData(){
			println("Freeing SelectAll Undodata");
			for(CUdeviceptr cptr : changedmasks){
				CURuntime::free(cptr);
			}
		}
	};

	shared_ptr<UndoData> undodata = make_shared<UndoData>();

	scene.process<SNSplats>([&](SNSplats* node){

		if(!node->visible) return;
		if(node->locked) return;

		int requiredBytes = node->dmng.data.count / 8 + 4;
		CUdeviceptr cptr_changedmask = CURuntime::alloc("selectAll changedmask", requiredBytes);
		prog_gaussians_editing->launch("kernel_select", {&launchArgs, &node->dmng.data, &cptr_changedmask}, node->dmng.data.count);

		undodata->nodes.push_back(node);
		undodata->changedmasks.push_back(cptr_changedmask);
	});

	addAction({
		.undo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				void* args[] = {&undodata->nodes[i]->dmng.data, &undodata->changedmasks[i]};
				prog_gaussians_editing->launch("kernel_deselect_masked", args, undodata->nodes[i]->dmng.data.count);
			}
		},
		.redo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				void* args[] = {&undodata->nodes[i]->dmng.data, &undodata->changedmasks[i]};
				prog_gaussians_editing->launch("kernel_select_masked", args, undodata->nodes[i]->dmng.data.count);
			}
		},
	});
}

void SplatEditor::deselectAll(){
	
	scene.process<SNSplats>([&](SNSplats* node){
		if(node->hidden) return;
		if(!node->visible) return;

		uint64_t size = CURuntime::getSize(CUdeviceptr(node->dmng.data.flags));
		// println("deselectAll");
		// println("    node: {}", uint64_t(node));
		// println("    node->name: {}", node->name);
		// println("    node->count: {}", node->dmng.data.count);
		// println("    flags size: {}", CURuntime::getSize(CUdeviceptr(node->dmng.data.flags)));
		// println("    position size: {}", CURuntime::getSize(CUdeviceptr(node->dmng.data.position)));
		// println("    position: {}", uint64_t(node->dmng.data.position));
		// println("    flags: {}", uint64_t(node->dmng.data.flags));


		void* changedmask = nullptr;
		prog_gaussians_editing->launch("kernel_deselect", {&launchArgs, &node->dmng.data, &changedmask}, node->dmng.data.count);
	});
}

void SplatEditor::deselectAll_undoable(){

	struct UndoData{
		vector<SNSplats*> nodes;
		vector<CUdeviceptr> changedmasks;

		~UndoData(){
			println("Freeing DeselectAll Undodata");
			for(CUdeviceptr cptr : changedmasks){
				CURuntime::free(cptr);
			}
		}
	};

	shared_ptr<UndoData> undodata = make_shared<UndoData>();

	scene.process<SNSplats>([&](SNSplats* node){

		if(!node->visible) return;
		if(node->locked) return;

		int requiredBytes = node->dmng.data.count / 8 + 4;
		CUdeviceptr cptr_changedmask = CURuntime::alloc("deselectAll changedmask", requiredBytes);
		prog_gaussians_editing->launch("kernel_deselect", {&launchArgs, &node->dmng.data, &cptr_changedmask}, node->dmng.data.count);

		undodata->nodes.push_back(node);
		undodata->changedmasks.push_back(cptr_changedmask);
	});

	addAction({
		.undo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				void* args[] = {&undodata->nodes[i]->dmng.data, &undodata->changedmasks[i]};
				prog_gaussians_editing->launch("kernel_select_masked", args, undodata->nodes[i]->dmng.data.count);
			}
		},
		.redo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				void* args[] = {&undodata->nodes[i]->dmng.data, &undodata->changedmasks[i]};
				prog_gaussians_editing->launch("kernel_deselect_masked", args, undodata->nodes[i]->dmng.data.count);
			}
		},
	});
}

void SplatEditor::invertSelection(){
	
	scene.process<SNSplats>([&](SNSplats* node){

		if(node->locked) return;
		if(!node->visible) return;

		prog_gaussians_editing->launch("kernel_invert_selection", {&launchArgs, &node->dmng.data}, node->dmng.data.count);
	});
}

void SplatEditor::invertSelection_undoable(){
	
	struct UndoData{
		vector<SNSplats*> nodes;
		vector<CUdeviceptr> selectionmasks;

		~UndoData(){
			println("Freeing invertSelection Undodata");
			for(CUdeviceptr cptr : selectionmasks){
				CURuntime::free(cptr);
			}
		}
	};

	shared_ptr<UndoData> undodata = make_shared<UndoData>();

	scene.process<SNSplats>([&](SNSplats* node){
		if(!node->visible) return;
		if(node->locked) return;

		int requiredBytes = node->dmng.data.count / 8 + 4;
		CUdeviceptr cptr_selectionmask = CURuntime::alloc("invert selectionmask", requiredBytes);
		
		prog_gaussians_editing->launch("kernel_get_selectionmask", {&node->dmng.data, &cptr_selectionmask}, node->dmng.data.count);

		bool inverted = true;
		prog_gaussians_editing->launch("kernel_set_selection", {&node->dmng.data, &cptr_selectionmask, &inverted}, node->dmng.data.count);

		undodata->nodes.push_back(node);
		undodata->selectionmasks.push_back(cptr_selectionmask);
	});

	addAction({
		.undo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				bool inverted = false;
				SNSplats* node = undodata->nodes[i];
				void* args[] = {&node->dmng.data, &undodata->selectionmasks[i], &inverted};
				prog_gaussians_editing->launch("kernel_set_selection", args, node->dmng.data.count);
			}
		},
		.redo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				bool inverted = true;
				SNSplats* node = undodata->nodes[i];
				void* args[] = {&node->dmng.data, &undodata->selectionmasks[i], &inverted};
				prog_gaussians_editing->launch("kernel_set_selection", args, node->dmng.data.count);
			}
		},
	});
}

void SplatEditor::deleteSelection(){

	scene.process<SNSplats>([&](SNSplats* node) {
		if(!node->visible) return;
		if(node->locked) return;

		void* deletionmask = nullptr;
		prog_gaussians_editing->launch("kernel_delete_selection", {&launchArgs, &node->dmng.data, &deletionmask}, node->dmng.data.count);
	});
}

void SplatEditor::deleteSelection_undoable(){
	
	struct UndoData{
		vector<SNSplats*> nodes;
		vector<CUdeviceptr> deletionmasks;

		~UndoData(){
			println("Freeing deleteSelection Undodata");
			for(CUdeviceptr cptr : deletionmasks){
				CURuntime::free(cptr);
			}
		}
	};

	shared_ptr<UndoData> undodata = make_shared<UndoData>();
	
	scene.process<SNSplats>([&](SNSplats* node) {
		if (node->locked) return;
		if (!node->visible) return;

		int requiredBytes = node->dmng.data.count / 8 + 4;
		CUdeviceptr cptr_deletionmask = CURuntime::alloc("deletionmask", requiredBytes);
		cuMemsetD8(cptr_deletionmask, 0, requiredBytes);

		prog_gaussians_editing->launch("kernel_delete_selection", {&node->dmng.data, &cptr_deletionmask}, node->dmng.data.count);

		undodata->nodes.push_back(node);
		undodata->deletionmasks.push_back(cptr_deletionmask);
	});

	addAction({
		.undo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				SNSplats* node = undodata->nodes[i];
				void* args[] = {&node->dmng.data, &undodata->deletionmasks[i]};
				prog_gaussians_editing->launch("kernel_undelete_selection", args, node->dmng.data.count);
			}
		},
		.redo = [=](){
			for(int i = 0; i < undodata->nodes.size(); i++){
				SNSplats* node = undodata->nodes[i];
				void* deletionmask = nullptr;
				prog_gaussians_editing->launch("kernel_delete_selection", {&node->dmng.data, &deletionmask}, node->dmng.data.count);
			}
		},
	});
}

void SplatEditor::deleteNode_undoable(shared_ptr<SceneNode> node)
{
	if (!node) {
		return;
	}

	int originalIndex = -1;

	for (int i = 0; i < static_cast<int>(scene.world->children.size()); ++i) {
		if (scene.world->children[i] == node) {
			originalIndex = i;
			break;
		}
	}

	if (originalIndex < 0) {
		return;
	}

	scene.erase(node);

	addAction({
		.undo = [=]() {
			auto& children = SplatEditor::instance->scene.world->children;

			size_t insertIndex = static_cast<size_t>(originalIndex);

			if (insertIndex > children.size()) {
				insertIndex = children.size();
			}

			children.insert(children.begin() + insertIndex, node);
			SplatEditor::instance->deselectAll();
			SplatEditor::instance->setSelectedNode(node.get());
		},
		.redo = [=]() {
			SplatEditor::instance->scene.erase(node);
		},
		});
}

void SplatEditor::createOrUpdateThumbnail(SceneNode* node){
	
	{ // precheck to see if updating is necessary
		static unordered_map<SceneNode*, Box3> previousBoxes;
		if(previousBoxes.find(node) == previousBoxes.end()){
			previousBoxes[node] = Box3();
		}
		Box3 previous = previousBoxes[node];
		if(node->aabb.isDefault()) return;
		if(previous.isEqual(node->aabb, 0.001f)) return;

		previousBoxes[node] = node->aabb;
	}


	constexpr int thumbnailSize = 256;

	static CUdeviceptr cptr_framebuffer = 0;
	static CUdeviceptr cptr_tmplinecounters = 0;
	if(cptr_framebuffer == 0){
		cptr_framebuffer = CURuntime::alloc("framebuffer", 8 * thumbnailSize * thumbnailSize);
		cptr_tmplinecounters = CURuntime::alloc("temporary linecounters", 8);
	}

	// tmpscene wants a shared instead of regular pointer, so create a special shared ptr that does not delete node when it runs out of scope.
	auto shnode = shared_ptr<SceneNode>(shared_ptr<SceneNode>{}, node);

	Scene tmpscene;
	tmpscene.root->children.push_back(shnode);

	OrbitControls tmpcontrols;
	tmpcontrols.yaw = 1.5f;
	tmpcontrols.pitch = -0.5f;

	Box3 box = node->getBoundingBox();
	tmpcontrols.focus(box.min, box.max, 0.7f);
	tmpcontrols.update();

	RenderTarget target;
	target.framebuffer = (uint64_t*)cptr_framebuffer;
	target.width = thumbnailSize;
	target.height = thumbnailSize;
	target.view = inverse(tmpcontrols.world);
	target.proj = Camera::createProjectionMatrix(0.1f, 3.1415f * 60.0f / 180.0f, 1.0f);
	target.VP = Camera::createVP(0.2f, 1000.0f, 3.1415f * 60.0f / 180.0f, 1.0f, target.width, target.height);

	View view;
	view.view = target.view;
	view.proj = target.proj;
	view.VP = target.VP;

	// Temporarily remove lines that were queued for rendering
	cuMemcpy(cptr_tmplinecounters + 0, cptr_numLines, 4);
	cuMemcpy(cptr_tmplinecounters + 4, cptr_numLines_host, 4);
	cuMemsetD32(cptr_numLines, 0, 1);
	cuMemsetD32(cptr_numLines_host, 0, 1);

	/*vector<RenderTarget> targets = { target };
	draw(&tmpscene, targets);*/
	vector<RenderTarget> targets = { target };

	const bool previousSuppressVRGui = suppress_vr_gui_rendering;
	suppress_vr_gui_rendering = true;

	draw(&tmpscene, targets);

	suppress_vr_gui_rendering = previousSuppressVRGui;

	// bring lines back
	cuMemcpy(cptr_numLines, cptr_tmplinecounters + 0, 4);
	cuMemcpy(cptr_numLines_host, cptr_tmplinecounters + 4, 4);


	shared_ptr<GLTexture> texture = node->thumbnail;
	if(!texture){
		texture = GLTexture::create(target.width, target.height, GL_RGBA8);
	}
	texture->setSize(thumbnailSize, thumbnailSize);

	Rectangle targetViewport;
	targetViewport.x = 0;
	targetViewport.y = 0;
	targetViewport.width = thumbnailSize;
	targetViewport.height = thumbnailSize;

	auto glMapping = mapCudaGl(texture);
	prog_gaussians_rendering->launch("kernel_blit_opengl", 
		{&launchArgs, &target, &glMapping.surface, &targetViewport}, 
		targetViewport.width * targetViewport.height);
	glMapping.unmap();


	node->thumbnail = texture;
}

void SplatEditor::uploadSplats(SNSplats* node){

	shared_ptr<Splats> splats = node->splats;

	if(!splats) return;

	// very important: numLoaded changes concurrently by a loader thread, so splats->numSplatsLoaded could be different
	// at different points in this function. Store it in a variable so it stays the same for commit and upload.
	uint64_t numLoaded = splats->numSplatsLoaded;

	node->dmng.commit(numLoaded);

	GaussianData& gd = node->dmng.data;

	int64_t numUploaded = gd.numUploaded;
	int64_t numToUpload = numLoaded - numUploaded;

	if(numToUpload > 0){
		CURuntime::check(cuMemcpyHtoD((CUdeviceptr)gd.position           + sizeof(*gd.position)          * numUploaded, splats->position->ptr + sizeof(*gd.position)   * numUploaded , sizeof(*gd.position)   * numToUpload));
		CURuntime::check(cuMemcpyHtoD((CUdeviceptr)gd.scale              + sizeof(*gd.scale)             * numUploaded, splats->scale->ptr    + sizeof(*gd.scale)      * numUploaded , sizeof(*gd.scale)      * numToUpload));
		CURuntime::check(cuMemcpyHtoD((CUdeviceptr)gd.quaternion         + sizeof(*gd.quaternion)        * numUploaded, splats->rotation->ptr + sizeof(*gd.quaternion) * numUploaded , sizeof(*gd.quaternion) * numToUpload));
		CURuntime::check(cuMemcpyHtoD((CUdeviceptr)gd.color              + sizeof(*gd.color)             * numUploaded, splats->color->ptr    + sizeof(*gd.color)      * numUploaded , sizeof(*gd.color)      * numToUpload));

		if(gd.shDegree > 0){
			int64_t shBytesPerSplat = gd.numSHCoefficients * sizeof(float);
			CURuntime::check(cuMemcpyHtoD(
				(CUdeviceptr)gd.sphericalHarmonics + shBytesPerSplat * numUploaded, 
				splats->SHs->ptr + shBytesPerSplat * numUploaded,
				shBytesPerSplat * numToUpload));
		}


		gd.count += numToUpload;
		gd.numUploaded += numToUpload;

		updateBoundingBox(node);
		createOrUpdateThumbnail(node);
	}
	
	bool lastSplatsUploaded = numToUpload > 0 && splats->numSplatsLoaded == splats->numSplats && gd.count == splats->numSplats;
	if (lastSplatsUploaded) {
		sortSplatsDevice(node);

		// remove splats from node once loading is finished
		node->splats = nullptr;
	}
}

void SplatEditor::resetEditor(){
	scene.world->children.clear();
	AssetLibrary::assets.clear();
}

void SplatEditor::unloadTempSplats(){

	if(tempSplats){
		scene.remove(tempSplats);
		tempSplats = nullptr;
	}
}

void SplatEditor::loadTempSplats(string path){

	if(tempSplats){
		unloadTempSplats();
	}

	auto splats = GSPlyLoader::load(path);

	shared_ptr<SNSplats> node = make_shared<SNSplats>(splats->name, splats);
	node->dmng.data.writeDepth = false;
	node->hidden = true;

	tempSplats = node;

	scene.world->children.push_back(tempSplats);
}

void SplatEditor::setDesktopMode(){
	viewmode = VIEWMODE_DESKTOP;
	ovr->stop();

	// reset scene world matrix
	scene.world->transform = mat4(1.0f);
}

void SplatEditor::setDesktopVrMode(){
	viewmode = VIEWMODE_DESKTOP_VR;
	ovr->start();
}

void SplatEditor::setImmersiveVrMode(){
	viewmode = VIEWMODE_IMMERSIVE_VR;
	ovr->start();
}

shared_ptr<SNSplats> SplatEditor::clone(SNSplats* source){

	shared_ptr<Splats> bogusSplats = make_shared<Splats>();
	//bogusSplats->numSplatsLoaded = source->splats->numSplatsLoaded;
	auto oldNode = source;
	auto newNode = make_shared<SNSplats>(oldNode->name);

	auto count = oldNode->dmng.data.count;

	vector<uint32_t> flags(count, 0);
	cuMemcpyDtoH(flags.data(), (CUdeviceptr)oldNode->dmng.data.flags, count * 4);

	int numNondeletedSplats = 0;
	for(int i = 0; i < count; i++){
		if(flags[i] & FLAGS_DELETED) continue;

		numNondeletedSplats++;
	}

	int cloneSplatCount = numNondeletedSplats;
	
	newNode->aabb = oldNode->aabb;
	newNode->dmng.commit(cloneSplatCount);
	newNode->dmng.data.count        = cloneSplatCount;
	newNode->dmng.data.numUploaded  = cloneSplatCount;
	newNode->dmng.data.visible      = oldNode->dmng.data.visible;
	newNode->dmng.data.locked       = oldNode->dmng.data.locked;
	newNode->dmng.data.writeDepth   = oldNode->dmng.data.writeDepth;
	newNode->dmng.data.transform    = oldNode->dmng.data.transform;
	newNode->dmng.data.min          = oldNode->dmng.data.min;
	newNode->dmng.data.max          = oldNode->dmng.data.max;

	bool ignoreDeleted = true;
	void* args[] = {&launchArgs, &oldNode->dmng.data, &newNode->dmng.data, &ignoreDeleted};
	prog_gaussians_editing->launchCooperative("kernel_copy_model", args);

	return newNode;
}

void SplatEditor::sortSplatsDevice(SNSplats* node, bool putDeletedLast){

	// return;

	println("{}", format(getSaneLocale(), "sortSplatsDevice, node: {}, splats: {:L}", node->name, node->dmng.data.count));

	auto t_start = now();

	GaussianData data = node->dmng.data;
	Box3 aabb = node->aabb;

	// int64_t maxBytesPerSplatAttribute = max(16llu, data.numSHCoefficients * sizeof(float));
	int64_t maxBytesPerSplatAttribute = max<unsigned long long>(16llu, data.numSHCoefficients * sizeof(float));

	#define USE_32BIT

	#if defined(USE_32BIT)
		CUdeviceptr cptr_mortoncodes;
		CUdeviceptr cptr_ordering;
		CUdeviceptr cptr_tmpstorage;
		cuMemAlloc(&cptr_mortoncodes, 4 * data.count);
		cuMemAlloc(&cptr_ordering, 4 * data.count);
		cuMemAlloc(&cptr_tmpstorage, maxBytesPerSplatAttribute * data.count);

		prog_gaussians_editing->launch("kernel_compute_mortoncodes_32bit", {&data, &aabb, &cptr_mortoncodes, &cptr_ordering}, data.count);

		if(putDeletedLast){
			// set mortoncode to maxval to put them last.
			prog_gaussians_editing->launch("kernel_compute_deletionlist", {&data, &cptr_mortoncodes}, data.count);
		}

		GPUSorting::sort_32bit_keyvalue(data.count, cptr_mortoncodes, cptr_ordering);
	#else
	
		// CUdeviceptr cptr_mortoncodes_lower;
		// CUdeviceptr cptr_mortoncodes_higher;
		// CUdeviceptr cptr_ordering;
		// CUdeviceptr cptr_tmpstorage;
		// cuMemAlloc(&cptr_mortoncodes_lower, 4 * data.count);
		// cuMemAlloc(&cptr_mortoncodes_higher, 4 * data.count);
		// cuMemAlloc(&cptr_ordering, 4 * data.count);
		// cuMemAlloc(&cptr_tmpstorage, 16 * data.count);

		// prog_gaussians_editing->launch("kernel_compute_mortoncodes_2x32bit", {&data, &aabb, &cptr_mortoncodes_lower, &cptr_mortoncodes_higher, &cptr_ordering}, data.count);

		// if(putDeletedLast){
		// 	// set mortoncode to maxval to put them last.
		// 	prog_gaussians_editing->launch("kernel_compute_deletionlist", {&data, &cptr_mortoncodes_lower}, data.count);
		// 	prog_gaussians_editing->launch("kernel_compute_deletionlist", {&data, &cptr_mortoncodes_higher}, data.count);
		// }

		// // sort by 32 least-significant bits of the morton code
		// GPUSorting::sort_32bit_keyvalue(data.count, cptr_mortoncodes_lower, cptr_ordering);

		// // Next, we need to apply that ordering to the higher-significant bits of the morton code, 
		// // before sorting by the higher-significant bits.
		// {
		// 	// We don't need cptr_mortoncodes_lower lower anymore, 
		// 	// but we need a source and target buffer for unsorted and sorted cptr_mortoncodes_higher.
		// 	// Reusing cptr_mortoncodes_lower as that target buffer.
		// 	CUdeviceptr cptr_higher_unsorted = cptr_mortoncodes_higher;
		// 	CUdeviceptr cptr_higher_sorted = cptr_mortoncodes_lower;

		// 	void* argsApply[] = {&cptr_higher_unsorted, &cptr_higher_sorted, &cptr_ordering, &data.count};
		// 	prog_gaussians_rendering->launch("kernel_applyOrdering_u32", argsApply, data.count);

		// 	// Now sort ordering by the 32 most-significant bits of the 64 bit morton code
		// 	GPUSorting::sort_32bit_keyvalue(data.count, cptr_higher_sorted, cptr_ordering);
		// }
	#endif



	uint64_t stride;

	// POSITION
	stride = sizeof(*data.position);
	cuMemcpy(cptr_tmpstorage, (CUdeviceptr)data.position, stride * data.count);
	prog_gaussians_rendering->launch("kernel_applyOrdering_xxx", {&cptr_tmpstorage, &data.position, &cptr_ordering, &stride, &data.count}, data.count);

	// SCALE
	stride = sizeof(*data.scale);
	cuMemcpy(cptr_tmpstorage, (CUdeviceptr)data.scale, stride * data.count);
	prog_gaussians_rendering->launch("kernel_applyOrdering_xxx", {&cptr_tmpstorage, &data.scale, &cptr_ordering, &stride, &data.count}, data.count);

	// QUATERNION
	stride = sizeof(*data.quaternion);
	cuMemcpy(cptr_tmpstorage, (CUdeviceptr)data.quaternion, stride * data.count);
	prog_gaussians_rendering->launch("kernel_applyOrdering_xxx", {&cptr_tmpstorage, &data.quaternion, &cptr_ordering, &stride, &data.count}, data.count);

	// COLOR
	stride = sizeof(*data.color);
	cuMemcpy(cptr_tmpstorage, (CUdeviceptr)data.color, stride * data.count);
	prog_gaussians_rendering->launch("kernel_applyOrdering_xxx", {&cptr_tmpstorage, &data.color, &cptr_ordering, &stride, &data.count}, data.count);

	// FLAGS
	stride = sizeof(*data.flags);
	cuMemcpy(cptr_tmpstorage, (CUdeviceptr)data.flags, stride * data.count);
	prog_gaussians_rendering->launch("kernel_applyOrdering_xxx", {&cptr_tmpstorage, &data.flags, &cptr_ordering, &stride, &data.count}, data.count);

	// SHs
	if(data.shDegree > 0){
		stride = data.numSHCoefficients * sizeof(float);
		cuMemcpy(cptr_tmpstorage, (CUdeviceptr)data.sphericalHarmonics, stride * data.count);
		prog_gaussians_rendering->launch("kernel_applyOrdering_xxx", {&cptr_tmpstorage, &data.sphericalHarmonics, &cptr_ordering, &stride, &data.count}, data.count);
	}
	

	#if defined(USE_32BIT)
		cuMemFree(cptr_mortoncodes);
	#else
		cuMemFree(cptr_mortoncodes_lower);
		cuMemFree(cptr_mortoncodes_higher);
	#endif
	
	cuMemFree(cptr_ordering);
	cuMemFree(cptr_tmpstorage);

	node->dirty = true;

	printElapsedTime("duration sort splats", t_start);
}


void SplatEditor::drawSphere(_DrawSphereArgs args){

	vec3 position = args.pos;
	vec3 scale = args.scale;

	static shared_ptr<Mesh> sphere = nullptr;
	static CUdeviceptr cptr_positions = 0;
	static CUdeviceptr cptr_uvs       = 0;
	static CUdeviceptr cptr_colors    = 0;
	static uint32_t numTriangles      = 0;

	if(sphere == nullptr){
		sphere = Mesh::createSphere();

		uint64_t size_triangles = sizeof(vec3) * sphere->position.size();
		uint64_t size_uvs       = sizeof(vec2) * sphere->uv.size();
		uint64_t size_colors    = sizeof(uint32_t) * sphere->color.size();

		cptr_positions = CURuntime::alloc("sphere positions", size_triangles + 4 * sphere->position.size());
		cptr_uvs       = CURuntime::alloc("sphere uvs", size_uvs);
		cptr_colors    = CURuntime::alloc("sphere colors", size_colors);

		cuMemcpyHtoD(cptr_positions, &sphere->position[0], size_triangles);
		cuMemcpyHtoD(cptr_uvs, &sphere->uv[0], size_uvs);
		cuMemcpyHtoD(cptr_colors, &sphere->color[0], size_colors);

		numTriangles = sphere->position.size() / 3;
	}

	TriangleData data;
	data.count     = numTriangles;
	data.position  = (vec3*)cptr_positions;
	data.uv        = (vec2*)cptr_uvs;
	data.colors    = (uint32_t*)cptr_colors;
	data.transform = translate(position) * glm::scale(scale);

	TriangleMaterial material;
	material.color = args.color;
	material.mode = MATERIAL_MODE_COLOR;
	// material.mode = MATERIAL_MODE_UVS;

	TriangleQueueItem item;
	item.geometry = data;
	item.material = material;

	triangleQueue.push_back(item);
}

void SplatEditor::drawTeleportBeam(vec3 start, vec3 end, float startRadius, float endRadius, vec4 color) {
	auto toByte = [](float v) -> uint32_t { return static_cast<uint32_t>(glm::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };

	uint32_t r = toByte(color.r);
	uint32_t g = toByte(color.g);
	uint32_t b = toByte(color.b);
	uint32_t a = toByte(color.a);

	teleport_beam.start = start;
	teleport_beam.end = end;
	teleport_beam.startRadius = startRadius;
	teleport_beam.endRadius = endRadius;
	teleport_beam.color = r | (g << 8) | (b << 16) | (a << 24);
	teleport_beam.visible = true;
}
//void SplatEditor::drawTeleportBeam(vec3 start, vec3 end, float startRadius, float endRadius, vec4 color) {
//	auto toByte = [](float v) -> uint32_t { return static_cast<uint32_t>(glm::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };
//
//	uint32_t r = toByte(color.r);
//	uint32_t g = toByte(color.g);
//	uint32_t b = toByte(color.b);
//	uint32_t a = toByte(color.a);
//
//	teleport_beam.start = start;
//	teleport_beam.end = end;
//	teleport_beam.startRadius = startRadius;
//	teleport_beam.endRadius = endRadius;
//	teleport_beam.color = r | (g << 8) | (b << 16) | (a << 24);
//	teleport_beam.visible = true;
//
//	static int dbgCounter = 0;
//	if ((dbgCounter++ % 60) == 0) println("[TeleportBeam DRAW] visible={} start=({}, {}, {}) end=({}, {}, {}) r0={} r1={}", teleport_beam.visible, start.x, start.y, start.z, end.x, end.y, end.z, startRadius, endRadius);
//}

//void SplatEditor::renderTeleportBeam(RenderTarget& target, CUstream stream) {
//	if (!teleport_beam.visible || !prog_teleport_beam || suppress_vr_gui_rendering) return;
//
//	mat4 viewProj = target.proj * target.view;
//	mat4 invViewProj = glm::inverse(viewProj);
//	mat4 invView = glm::inverse(target.view);
//	vec3 cameraOrigin = vec3(invView * vec4(0.0f, 0.0f, 0.0f, 1.0f));
//
//	const float maxRadius = glm::max(teleport_beam.startRadius, teleport_beam.endRadius);
//	const vec3 lo = glm::min(teleport_beam.start, teleport_beam.end) - vec3(maxRadius);
//	const vec3 hi = glm::max(teleport_beam.start, teleport_beam.end) + vec3(maxRadius);
//
//	float minScreenX = float(target.width);
//	float minScreenY = float(target.height);
//	float maxScreenX = 0.0f;
//	float maxScreenY = 0.0f;
//	bool fullScreen = false;
//
//	for (int i = 0; i < 8; i++) {
//		vec3 p((i & 1) ? hi.x : lo.x, (i & 2) ? hi.y : lo.y, (i & 4) ? hi.z : lo.z);
//		vec4 clip = viewProj * vec4(p, 1.0f);
//
//		if (clip.w <= 1e-5f) {
//			fullScreen = true;
//			break;
//		}
//
//		float sx = (clip.x / clip.w * 0.5f + 0.5f) * float(target.width);
//		float sy = (clip.y / clip.w * 0.5f + 0.5f) * float(target.height);
//
//		minScreenX = glm::min(minScreenX, sx);
//		minScreenY = glm::min(minScreenY, sy);
//		maxScreenX = glm::max(maxScreenX, sx);
//		maxScreenY = glm::max(maxScreenY, sy);
//	}
//
//	if (!fullScreen && (maxScreenX < 0.0f || maxScreenY < 0.0f || minScreenX >= float(target.width) || minScreenY >= float(target.height))) return;
//
//	int minX = 0;
//	int minY = 0;
//	int maxX = target.width - 1;
//	int maxY = target.height - 1;
//
//	if (!fullScreen) {
//		const int pad = 4;
//
//		minX = glm::clamp(int(std::floor(minScreenX)) - pad, 0, target.width - 1);
//		minY = glm::clamp(int(std::floor(minScreenY)) - pad, 0, target.height - 1);
//		maxX = glm::clamp(int(std::ceil(maxScreenX)) + pad, 0, target.width - 1);
//		maxY = glm::clamp(int(std::ceil(maxScreenY)) + pad, 0, target.height - 1);
//	}
//
//	int rectWidth = maxX - minX + 1;
//	int rectHeight = maxY - minY + 1;
//
//	if (rectWidth <= 0 || rectHeight <= 0) return;
//
//	int numPixels = rectWidth * rectHeight;
//
//	void* args[] = { &launchArgs, &target, &teleport_beam.start, &teleport_beam.end, &teleport_beam.startRadius, &teleport_beam.endRadius, &teleport_beam.color, &invViewProj, &cameraOrigin, &minX, &minY, &rectWidth, &rectHeight };
//	prog_teleport_beam->launch("kernel_drawTeleportBeam", args, numPixels, stream);
//}

void SplatEditor::renderTeleportBeam(RenderTarget& target, CUstream stream) {
	if (!teleport_beam.visible || !prog_teleport_beam || suppress_vr_gui_rendering) return;

	mat4 viewProj = target.proj * target.view;
	mat4 invViewProj = glm::inverse(viewProj);
	mat4 invView = glm::inverse(target.view);
	vec3 cameraOrigin = vec3(invView * vec4(0.0f, 0.0f, 0.0f, 1.0f));

	vec3 cameraRight = glm::normalize(vec3(invView[0]));
	vec3 cameraUp = glm::normalize(vec3(invView[1]));

	float minScreenX = float(target.width);
	float minScreenY = float(target.height);
	float maxScreenX = 0.0f;
	float maxScreenY = 0.0f;
	bool hasValidProjection = false;

	constexpr int lengthSamples = 16;
	constexpr int radialSamples = 8;
	constexpr float PI = 3.14159265358979323846f;

	for (int j = 0; j <= lengthSamples; j++) {
		float t = float(j) / float(lengthSamples);
		vec3 center = glm::mix(teleport_beam.start, teleport_beam.end, t);
		float radius = glm::mix(teleport_beam.startRadius, teleport_beam.endRadius, t);

		for (int i = 0; i < radialSamples; i++) {
			float angle = 2.0f * PI * float(i) / float(radialSamples);
			vec3 offset = radius * (cos(angle) * cameraRight + sin(angle) * cameraUp);
			vec3 p = center + offset;
			vec4 clip = viewProj * vec4(p, 1.0f);

			if (clip.w <= 1e-4f) continue;

			float sx = (clip.x / clip.w * 0.5f + 0.5f) * float(target.width);
			float sy = (clip.y / clip.w * 0.5f + 0.5f) * float(target.height);

			minScreenX = glm::min(minScreenX, sx);
			minScreenY = glm::min(minScreenY, sy);
			maxScreenX = glm::max(maxScreenX, sx);
			maxScreenY = glm::max(maxScreenY, sy);

			hasValidProjection = true;
		}
	}

	if (!hasValidProjection) return;
	if (maxScreenX < 0.0f || maxScreenY < 0.0f || minScreenX >= float(target.width) || minScreenY >= float(target.height)) return;

	const int pad = 8;

	int minX = glm::clamp(int(std::floor(minScreenX)) - pad, 0, target.width - 1);
	int minY = glm::clamp(int(std::floor(minScreenY)) - pad, 0, target.height - 1);
	int maxX = glm::clamp(int(std::ceil(maxScreenX)) + pad, 0, target.width - 1);
	int maxY = glm::clamp(int(std::ceil(maxScreenY)) + pad, 0, target.height - 1);

	int rectWidth = maxX - minX + 1;
	int rectHeight = maxY - minY + 1;

	if (rectWidth <= 0 || rectHeight <= 0) return;

	int numPixels = rectWidth * rectHeight;

	void* args[] = { &launchArgs, &target, &teleport_beam.start, &teleport_beam.end, &teleport_beam.startRadius, &teleport_beam.endRadius, &teleport_beam.color, &invViewProj, &cameraOrigin, &minX, &minY, &rectWidth, &rectHeight };
	prog_teleport_beam->launch("kernel_drawTeleportBeam", args, numPixels, stream);
}
//void SplatEditor::renderTeleportBeam(RenderTarget& target, CUstream stream) {
//	static int dbgCounter = 0;
//	bool dbg = (dbgCounter++ % 60) == 0;
//
//	if (dbg) println("[TeleportBeam RENDER] entered visible={} prog={} suppress={} target={}x{}", teleport_beam.visible, prog_teleport_beam != nullptr, suppress_vr_gui_rendering, target.width, target.height);
//
//	if (!teleport_beam.visible) {
//		if (dbg) println("[TeleportBeam RENDER] RETURN: visible=false");
//		return;
//	}
//
//	if (!prog_teleport_beam) {
//		if (dbg) println("[TeleportBeam RENDER] RETURN: prog_teleport_beam=null");
//		return;
//	}
//
//	if (suppress_vr_gui_rendering) {
//		if (dbg) println("[TeleportBeam RENDER] RETURN: suppress_vr_gui_rendering=true");
//		return;
//	}
//
//	mat4 viewProj = target.proj * target.view;
//	mat4 invViewProj = glm::inverse(viewProj);
//	mat4 invView = glm::inverse(target.view);
//	vec3 cameraOrigin = vec3(invView * vec4(0.0f, 0.0f, 0.0f, 1.0f));
//
//	float maxRadius = glm::max(teleport_beam.startRadius, teleport_beam.endRadius);
//	vec3 lo = glm::min(teleport_beam.start, teleport_beam.end) - vec3(maxRadius);
//	vec3 hi = glm::max(teleport_beam.start, teleport_beam.end) + vec3(maxRadius);
//
//	float minScreenX = float(target.width);
//	float minScreenY = float(target.height);
//	float maxScreenX = 0.0f;
//	float maxScreenY = 0.0f;
//	bool fullScreen = false;
//
//	for (int i = 0; i < 8; i++) {
//		vec3 p((i & 1) ? hi.x : lo.x, (i & 2) ? hi.y : lo.y, (i & 4) ? hi.z : lo.z);
//		vec4 clip = viewProj * vec4(p, 1.0f);
//
//		if (clip.w <= 1e-5f) {
//			fullScreen = true;
//			break;
//		}
//
//		float sx = (clip.x / clip.w * 0.5f + 0.5f) * float(target.width);
//		float sy = (clip.y / clip.w * 0.5f + 0.5f) * float(target.height);
//
//		minScreenX = glm::min(minScreenX, sx);
//		minScreenY = glm::min(minScreenY, sy);
//		maxScreenX = glm::max(maxScreenX, sx);
//		maxScreenY = glm::max(maxScreenY, sy);
//	}
//
//	if (dbg) println("[TeleportBeam BBOX] fullScreen={} screen=({}, {}) -> ({}, {})", fullScreen, minScreenX, minScreenY, maxScreenX, maxScreenY);
//
//	if (!fullScreen && (maxScreenX < 0.0f || maxScreenY < 0.0f || minScreenX >= float(target.width) || minScreenY >= float(target.height))) {
//		if (dbg) println("[TeleportBeam RENDER] RETURN: bbox outside screen");
//		return;
//	}
//
//	int minX = 0;
//	int minY = 0;
//	int maxX = target.width - 1;
//	int maxY = target.height - 1;
//
//	if (!fullScreen) {
//		const int pad = 4;
//		minX = glm::clamp(int(std::floor(minScreenX)) - pad, 0, target.width - 1);
//		minY = glm::clamp(int(std::floor(minScreenY)) - pad, 0, target.height - 1);
//		maxX = glm::clamp(int(std::ceil(maxScreenX)) + pad, 0, target.width - 1);
//		maxY = glm::clamp(int(std::ceil(maxScreenY)) + pad, 0, target.height - 1);
//	}
//
//	int rectWidth = maxX - minX + 1;
//	int rectHeight = maxY - minY + 1;
//
//	if (dbg) println("[TeleportBeam RECT] x={} y={} w={} h={}", minX, minY, rectWidth, rectHeight);
//
//	if (rectWidth <= 0 || rectHeight <= 0) {
//		if (dbg) println("[TeleportBeam RENDER] RETURN: invalid rect");
//		return;
//	}
//
//	int numPixels = rectWidth * rectHeight;
//
//	void* args[] = { &launchArgs, &target, &teleport_beam.start, &teleport_beam.end, &teleport_beam.startRadius, &teleport_beam.endRadius, &teleport_beam.color, &invViewProj, &cameraOrigin, &minX, &minY, &rectWidth, &rectHeight };
//
//	if (dbg) println("[TeleportBeam LAUNCH] numPixels={} camera=({}, {}, {})", numPixels, cameraOrigin.x, cameraOrigin.y, cameraOrigin.z);
//
//	prog_teleport_beam->launch("kernel_drawTeleportBeam", args, numPixels, stream);
//
//	if (dbg) println("[TeleportBeam LAUNCH] done");
//}
void SplatEditor::drawTeleportRefinementDisk(vec3 center, vec3 axisDepth, vec3 axisHeight, float radius, vec4 color) {
	if (glm::length(axisDepth) < 0.0001f || glm::length(axisHeight) < 0.0001f || radius <= 0.0f) return;

	vec3 depth = glm::normalize(axisDepth);
	vec3 height = axisHeight - glm::dot(axisHeight, depth) * depth;
	if (glm::length(height) < 0.0001f) return;

	teleport_refinement_disk.center = center;
	teleport_refinement_disk.axisDepth = depth;
	teleport_refinement_disk.axisHeight = glm::normalize(height);
	teleport_refinement_disk.radius = radius;
	teleport_refinement_disk.color = color;
	teleport_refinement_disk.visible = true;
}

void SplatEditor::renderTeleportRefinementDisk(RenderTarget& target, CUstream stream) {
	if (!teleport_refinement_disk.visible || !prog_teleport_beam || suppress_vr_gui_rendering) return;

	vec3 axisDepth = glm::normalize(teleport_refinement_disk.axisDepth);
	vec3 axisHeight = teleport_refinement_disk.axisHeight - glm::dot(teleport_refinement_disk.axisHeight, axisDepth) * axisDepth;
	if (glm::length(axisHeight) < 0.0001f) return;
	axisHeight = glm::normalize(axisHeight);

	mat4 viewProj = target.proj * target.view;
	mat4 invViewProj = glm::inverse(viewProj);
	mat4 invView = glm::inverse(target.view);
	vec3 cameraOrigin = vec3(invView * vec4(0.0f, 0.0f, 0.0f, 1.0f));

	float minScreenX = float(target.width);
	float minScreenY = float(target.height);
	float maxScreenX = 0.0f;
	float maxScreenY = 0.0f;
	bool hasValidProjection = false;

	constexpr int samples = 48;
	constexpr float PI = 3.14159265358979323846f;

	for (int i = 0; i < samples; i++) {
		float angle = 2.0f * PI * float(i) / float(samples);

		vec3 p =
			teleport_refinement_disk.center +
			teleport_refinement_disk.radius *
			(cos(angle) * axisDepth + sin(angle) * axisHeight);

		vec4 clip = viewProj * vec4(p, 1.0f);
		if (clip.w <= 1e-4f) continue;

		float sx = (clip.x / clip.w * 0.5f + 0.5f) * float(target.width);
		float sy = (clip.y / clip.w * 0.5f + 0.5f) * float(target.height);

		minScreenX = glm::min(minScreenX, sx);
		minScreenY = glm::min(minScreenY, sy);
		maxScreenX = glm::max(maxScreenX, sx);
		maxScreenY = glm::max(maxScreenY, sy);

		hasValidProjection = true;
	}

	if (!hasValidProjection) return;
	if (maxScreenX < 0.0f || maxScreenY < 0.0f || minScreenX >= float(target.width) || minScreenY >= float(target.height)) return;

	const int pad = 4;

	int minX = glm::clamp(int(std::floor(minScreenX)) - pad, 0, target.width - 1);
	int minY = glm::clamp(int(std::floor(minScreenY)) - pad, 0, target.height - 1);
	int maxX = glm::clamp(int(std::ceil(maxScreenX)) + pad, 0, target.width - 1);
	int maxY = glm::clamp(int(std::ceil(maxScreenY)) + pad, 0, target.height - 1);

	int rectWidth = maxX - minX + 1;
	int rectHeight = maxY - minY + 1;

	if (rectWidth <= 0 || rectHeight <= 0) return;

	int numPixels = rectWidth * rectHeight;

	void* args[] = {
		&launchArgs,
		&target,
		&teleport_refinement_disk.center,
		&axisDepth,
		&axisHeight,
		&teleport_refinement_disk.radius,
		&teleport_refinement_disk.color,
		&invViewProj,
		&cameraOrigin,
		&minX,
		&minY,
		&rectWidth,
		&rectHeight
	};

	prog_teleport_beam->launch("kernel_drawTeleportRefinementDisk", args, numPixels, stream);
}

void SplatEditor::drawLine(vec3 start, vec3 end, uint32_t color){

	uint32_t index = *h_numLines;

	// BOTTOM
	h_lines[index] = Line{.start = start, .end = end, .color = color,};

	*h_numLines += 1;
}

void SplatEditor::drawBox(Box3 box, uint32_t color){

	auto l = box.min;
	auto h = box.max;

	uint32_t first = *h_numLines;

	// BOTTOM
	h_lines[first +  0] = Line{.start = {l.x, l.y, l.z}, .end = {h.x, l.y, l.z}, .color = color,};
	h_lines[first +  1] = Line{.start = {h.x, l.y, l.z}, .end = {h.x, h.y, l.z}, .color = color,};
	h_lines[first +  2] = Line{.start = {h.x, h.y, l.z}, .end = {l.x, h.y, l.z}, .color = color,};
	h_lines[first +  3] = Line{.start = {l.x, h.y, l.z}, .end = {l.x, l.y, l.z}, .color = color,};

	// TOP
	h_lines[first +  4] = Line{.start = {l.x, l.y, h.z}, .end = {h.x, l.y, h.z}, .color = color,};
	h_lines[first +  5] = Line{.start = {h.x, l.y, h.z}, .end = {h.x, h.y, h.z}, .color = color,};
	h_lines[first +  6] = Line{.start = {h.x, h.y, h.z}, .end = {l.x, h.y, h.z}, .color = color,};
	h_lines[first +  7] = Line{.start = {l.x, h.y, h.z}, .end = {l.x, l.y, h.z}, .color = color,};

	// BOTTOM TO TOP
	h_lines[first +  8] = Line{.start = {l.x, l.y, l.z}, .end = {l.x, l.y, h.z}, .color = color,};
	h_lines[first +  9] = Line{.start = {h.x, l.y, l.z}, .end = {h.x, l.y, h.z}, .color = color,};
	h_lines[first + 10] = Line{.start = {h.x, h.y, l.z}, .end = {h.x, h.y, h.z}, .color = color,};
	h_lines[first + 11] = Line{.start = {l.x, h.y, l.z}, .end = {l.x, h.y, h.z}, .color = color,};

	*h_numLines += 12;
}

Uniforms SplatEditor::getUniforms(){
	Uniforms uniforms;
	uniforms.time                     = now();
	uniforms.frameCount               = GLRenderer::frameCount;
	uniforms.viewmode                 = viewmode;
	uniforms.measure                  = Runtime::measureTimings;
	uniforms.fragmentCounter          = 0;
	uniforms.showSolid                = settings.showSolid;
	uniforms.showTiles                = settings.showTiles;
	uniforms.showRing                 = settings.showRing;
	uniforms.makePoints               = settings.makePoints;
	uniforms.rendermode               = settings.rendermode;
	uniforms.frontToBack              = settings.frontToBack;
	uniforms.sortEnabled              = settings.sort;
	uniforms.splatSize                = settings.splatSize;
	uniforms.cullSmallSplats          = settings.cullSmallSplats;
	uniforms.brushColorMode           = settings.brushColorMode;
	uniforms.intersectionMode         = settings.intersectionMode;

	uniforms.inset.show               = settings.showInset;
	uniforms.inset.start              = {16 * 60, 16 * 50};
	uniforms.inset.size               = {16, 16};

	glm::mat4 world(1.0f);
	glm::mat4 view                     = GLRenderer::camera->view;
	glm::mat4 camWorld                 = GLRenderer::camera->world;
	glm::mat4 proj                     = GLRenderer::camera->proj;

	if(viewmode == VIEWMODE_DESKTOP){
		// world = splats.world;
	}else if(viewmode == VIEWMODE_DESKTOP_VR){
		// world = splats.world;
	}else if(viewmode == VIEWMODE_IMMERSIVE_VR){
		world = viewmodeImmersiveVR.world_vr;
	}

	uniforms.world        = world;
	uniforms.camWorld     = camWorld;
	uniforms.vrEnabled    = ovr->isActive();

	return uniforms;
}

CommonLaunchArgs SplatEditor::getCommonLaunchArgs(){

	KeyEvents keyEvents;
	keyEvents.numEvents = 0;
	for(int64_t i = 0; i < Runtime::frame_keys.size(); i++){
		KeyEvents::KeyEvent event;
		event.key = Runtime::frame_keys[i];
		event.action = Runtime::frame_actions[i];
		event.mods = Runtime::frame_mods[i];

		keyEvents.events[i] = event;

		keyEvents.numEvents++;
	}

	CommonLaunchArgs launchArgs;
	launchArgs.uniforms              = getUniforms();
	launchArgs.state                 = (DeviceState*)cptr_state;
	launchArgs.mouseEvents           = Runtime::mouseEvents;
	launchArgs.mouseEvents_prev      = mouse_prev;
	launchArgs.keys                  = (Keys*)cptr_keys;
	launchArgs.keyEvents             = keyEvents;
	launchArgs.brush                 = settings.brush;
	launchArgs.rectselect            = settings.rectselect;

	// for sorting
	// launchArgs.tiles                 = (Tile*)cptr_tiles;                    // Stores range of first and last splat in each tile
	
	return launchArgs;
};

void SplatEditor::initCudaProgram(){

	cuStreamCreate(&stream_upload, CU_STREAM_NON_BLOCKING);
	cuStreamCreate(&mainstream, CU_STREAM_NON_BLOCKING);
	cuStreamCreate(&sidestream, CU_STREAM_NON_BLOCKING);

	// Teleport query resources
	cuMemAlloc(&this->cptr_teleport_best, sizeof(unsigned long long));
	cuMemHostAlloc(reinterpret_cast<void**>(&this->h_teleport_best), sizeof(unsigned long long), CU_MEMHOSTALLOC_PORTABLE);
	cuEventCreate(&this->event_teleport_query_done, CU_EVENT_DISABLE_TIMING);

	cuEventCreate(&event_mainstream, CU_EVENT_DEFAULT);
	cuEventCreate(&event_edl_applied, CU_EVENT_DEFAULT);
	cuEventCreate(&ev_reset_stagecounters, CU_EVENT_DEFAULT);

	cptr_uniforms               = CURuntime::alloc("uniforms", sizeof(Uniforms));
	cptr_keys                   = CURuntime::alloc("keys", sizeof(Keys));
	cptr_lines                  = CURuntime::alloc("lines", sizeof(Line) * MAX_LINES);
	cptr_numLines               = CURuntime::alloc("num lines", 8);
	cptr_numLines_host          = CURuntime::alloc("num lines (host)", 8);
	
	cuMemAllocHost((void**)&h_lines, sizeof(Line) * MAX_LINES);
	cuMemAllocHost((void**)&h_numLines, 4);

	cuMemAllocHost((void**)&h_state_pinned , sizeof(DeviceState));
	cuMemAllocHost((void**)&h_tilecounter  , sizeof(uint32_t));
	cptr_state = CURuntime::alloc("device state", sizeof(DeviceState));
	cuMemsetD8(cptr_state, 0, sizeof(DeviceState));

	// int tileSize = 16;
	// int MAX_WIDTH = 4096;
	// int MAX_HEIGHT = 4096;
	// int MAX_TILES_X = MAX_WIDTH / tileSize;
	// int MAX_TILES_Y = MAX_HEIGHT / tileSize;

	// cptr_tiles = CURuntime::alloc("tiles", sizeof(Tile) * MAX_TILES_X * MAX_TILES_Y);

	cuMemsetD8(cptr_keys, 0, sizeof(Keys));

	double t_start = now();
	
	CUcontext context;
	cuCtxGetCurrent(&context);

	struct Program{
		CudaModularProgram** ptr;
		string path;
	};

	CudaModularProgram* prog_dbg = nullptr;

	//vector<Program> programs = {
	//	{&prog_gaussians_rendering,  "./src/gaussians_rendering.cu"},
	//	{&prog_gaussians_editing,    "./src/gaussians_editing.cu"},
	//	{&prog_points,               "./src/render/points.cu"},
	//	{&prog_triangles,            "./src/render/triangles.cu"},
	//	{&prog_lines,                "./src/render/lines.cu"},
	//	{&prog_helpers,              "./src/render/helpers.cu"},
	//	// {&prog_dbg,                  "./src/GPUSorting/RadixSort.cu"},
	//};
	vector<Program> programs = {
	{&prog_gaussians_rendering, "./src/gaussians_rendering.cu"},
	{&prog_gaussians_editing, "./src/gaussians_editing.cu"},
	{&prog_points, "./src/render/points.cu"},
	{&prog_triangles, "./src/render/triangles.cu"},
	{&prog_lines, "./src/render/lines.cu"},
	{&prog_helpers, "./src/render/helpers.cu"},
	{&prog_teleport_beam, "./src/render/teleport_beam.cu"},
	};

	// single-threaded compilation
	for(Program program : programs){
		string filename = fs::path(program.path).filename().string();
		string cubinPath = format("./cubins/{}.cubin", filename);

		if(fs::exists(cubinPath)){
			auto buffer = readBinaryFile(cubinPath);
			*program.ptr = CudaModularProgram::fromCubin(buffer->data, buffer->size);
		}else{
			*program.ptr = new CudaModularProgram({program.path});
		}
	}

	// multi-threaded compilation
	// println("NOTE: compiling cuda programs in parallel, output will be garbled up");
	// for_each(std::execution::par, programs.begin(), programs.end(), [&](Program program){
	// 	cuCtxSetCurrent(context);

	// 	string filename = fs::path(program.path).filename().string();
	// 	string cubinPath = format("./cubins/{}.cubin", filename);

	// 	if(fs::exists(cubinPath)){
	// 		auto buffer = readBinaryFile(cubinPath);
	// 		*program.ptr = CudaModularProgram::fromCubin(buffer->data, buffer->size);
	// 	}else{
	// 		*program.ptr = new CudaModularProgram({program.path});
	// 	}
	// });

	double seconds = now() - t_start;
	println("finished compiling and linking CUDA programs in {:.1f} seconds.", seconds);
}

shared_ptr<SNTriangles> SplatEditor::ovrToNode(string name, RenderModel_t* model, RenderModel_TextureMap_t* texture){

	auto node = make_shared<SNTriangles>(name);

	vector<vec3> positions;
	vector<vec2> uvs;

	struct Triangle{
		vec3 p0, p1, p2;
		vec2 uv0, uv1, uv2;
	};

	auto split = [&](Triangle t){

		vec3 center = (t.p0 +t. p1 + t.p2) / 3.0f;
		vec2 center_uv = (t.uv0 + t.uv1 + t.uv2) / 3.0f;

		Triangle t0 = {t.p0, t.p1, center, t.uv0, t.uv1, center_uv};
		Triangle t1 = {t.p1, t.p2, center, t.uv1, t.uv2, center_uv};
		Triangle t2 = {t.p2, t.p0, center, t.uv2, t.uv0, center_uv};
		
		return std::tuple{t0, t1, t2};
	};

	auto insert = [&](Triangle t){
		positions.push_back(t.p0);
		positions.push_back(t.p1);
		positions.push_back(t.p2);

		uvs.push_back(t.uv0);
		uvs.push_back(t.uv1);
		uvs.push_back(t.uv2);
	};

	vector<Triangle> triangles;
	
	for(int i = 0; i < model->unTriangleCount; i++){
		uint16_t i0 = model->rIndexData[3 * i + 0];
		uint16_t i1 = model->rIndexData[3 * i + 1];
		uint16_t i2 = model->rIndexData[3 * i + 2];

		RenderModel_Vertex_t v0 = model->rVertexData[i0];
		RenderModel_Vertex_t v1 = model->rVertexData[i1];
		RenderModel_Vertex_t v2 = model->rVertexData[i2];

		vec3 p0 = *((vec3*)&v0.vPosition);
		vec3 p1 = *((vec3*)&v1.vPosition);
		vec3 p2 = *((vec3*)&v2.vPosition);

		vec2 uv0 = *((vec2*)&v0.rfTextureCoord);
		vec2 uv1 = *((vec2*)&v1.rfTextureCoord);
		vec2 uv2 = *((vec2*)&v2.rfTextureCoord);

		triangles.push_back({p0, p1, p2, uv0, uv1, uv2});
	}

	vector<Triangle> tmp;

	for(int i : {0, 1}){
		for(Triangle triangle : triangles){
			auto [t0, t1, t2] = split(triangle);
			tmp.push_back(t0);
			tmp.push_back(t1);
			tmp.push_back(t2);
		}

		triangles.clear();

		for(Triangle triangle : tmp){
			auto [t0, t1, t2] = split(triangle);
			triangles.push_back(t0);
			triangles.push_back(t1);
			triangles.push_back(t2);
		}

		tmp.clear();
	}

	for(Triangle t : triangles){
		insert(t);
	}

	node->set(positions, uvs);

	vec2 size = {texture->unWidth, texture->unHeight};
	node->setTexture(size, (void*)texture->rubTextureMapData);

	return node;
}

void SplatEditor::inputHandling(){

	if(ovr->isActive()){
		ImGui::SetCurrentContext(imguicontext_vr);
		inputHandlingVR();
	}else{
		ImGui::SetCurrentContext(imguicontext_desktop);
		inputHandlingDesktop();
	}
}

void makeContextMenu(){

	auto editor = SplatEditor::instance;

	if(editor->settings.openContextMenu){
		ImGui::OpenPopup("MainContextMenu");
		editor->settings.openContextMenu = false;
	}

	if (ImGui::BeginPopup("MainContextMenu"))
	{
		// ImGui::Text("(Context Menu)");
		ImGui::MenuItem("(Context Menu)", NULL, false, false);
		ImGui::Separator();

		bool enableDuplication = Runtime::numSelectedSplats > 0;
		if(ImGui::MenuItem("Duplicate Selection to new Layer (ctrl + d)", nullptr, nullptr, enableDuplication)){

			FilterRules rules;
			rules.selection = FILTER_SELECTION_SELECTED;
			
			editor->filterToNewLayer_undoable(rules);
		}

		ImGui::EndPopup();
	}
}

void SplatEditor::drawGUI() {

	if(!settings.hideGUI){
		makeMenubar();
		makeToolbar();
		makeDevGUI();
		makeEditingGUI();
		makeColorCorrectionGui();
		makeStats();
		// makeTODOs();
		makeContextMenu();
		makeSaveFileGUI();
		makeGettingStarted();

		// { // PROTOTYPING / DEBUG: Toggle between 3DGS and perspective correct scenes and rendering

		// 	ImGui::SetNextWindowPos(ImVec2(100, 100));
		// 	ImGui::SetNextWindowSize(ImVec2(500, 120));
		// 	ImGui::SetNextWindowBgAlpha(1.0f);

		// 	if(ImGui::Begin("Toggle Splatrenderer")){

		// 		ImGui::Text("Toggle between 3DGS and Perspective Correct models and renderer");

		// 		ImGui::RadioButton("3DGS", &settings.splatRenderer, SPLATRENDERER_3DGS); ImGui::SameLine();
		// 		ImGui::RadioButton("Persp.Correct", &settings.splatRenderer, SPLATRENDERER_PERSPECTIVE_CORRECT); 

		// 		ImGui::Text("Alternatively press space to toggle. ");
		// 	}

		// 	ImGui::End();

			
		// 	auto node_3dgs  = scene.find("garden_3dgs");
		// 	auto node_persp = scene.find("garden_perspcorrect");

		// 	if(node_3dgs)  node_3dgs->visible  = settings.splatRenderer == SPLATRENDERER_3DGS;
		// 	if(node_persp) node_persp->visible = settings.splatRenderer == SPLATRENDERER_PERSPECTIVE_CORRECT;
		// }
	}else{
		ImVec2 kernelWindowSize = {70, 25};
		ImGui::SetNextWindowPos({GLRenderer::width - kernelWindowSize.x, -8});
		ImGui::SetNextWindowSize(kernelWindowSize);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoCollapse
			// | ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoSavedSettings
			| ImGuiWindowFlags_NoDecoration;
		static bool open;

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

		if(ImGui::Begin("ShowGuiWindow", &open, flags)){
			if(ImGui::Button("Show GUI")){
				settings.hideGUI = !settings.hideGUI;
			}
		}
		ImGui::End();
		
		ImGui::PopStyleColor(2);

	}

	
}

CudaGlMappings SplatEditor::mapCudaGl(shared_ptr<GLTexture> source){

	CudaGlMappings mappings;

	struct RegisteredTexture{
		GLTexture texture;
		CUgraphicsResource resource;
	};

	int64_t version = source->version;
	
	static unordered_map<int64_t, RegisteredTexture> registeredResources;
	if(registeredResources.find(source->ID) == registeredResources.end()){
		// does not exist - register
		println("register new texture");
		
		CUgraphicsResource resource;
		CURuntime::check(cuGraphicsGLRegisterImage(
			&resource, 
			source->handle, 
			GL_TEXTURE_2D, 
			CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));

		RegisteredTexture registration;
		registration.texture = *source;
		registration.resource = resource;

		println("    registered source->handle: {}, source->ID: {}, resource: {}", source->handle, source->ID, uint64_t(resource));

		registeredResources[source->ID] = registration;
	}else if(registeredResources[source->ID].texture.version != version){

		println("texture size changed - re-register cuda graphics resource");

		// changed - register new
		CUgraphicsResource resource = registeredResources[source->ID].resource;

		CURuntime::check(cuGraphicsUnregisterResource(resource));

		CURuntime::check(cuGraphicsGLRegisterImage(
			&resource, 
			source->handle, 
			GL_TEXTURE_2D, 
			CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));

		RegisteredTexture registration;
		registration.texture = *source;
		registration.resource = resource;

		// println("    registered source->handle: {}, source->ID: {}, resource: {}", source->handle, source->ID, uint64_t(resource));

		registeredResources[source->ID] = registration;
	}

	CUgraphicsResource resource = registeredResources[source->ID].resource;

	std::vector< CUgraphicsResource> resources = {resource};
	CURuntime::check(cuGraphicsMapResources(resources.size(), resources.data(), ((CUstream)CU_STREAM_DEFAULT)));

	{
		CUDA_RESOURCE_DESC res_desc = {};
		res_desc.resType = CUresourcetype::CU_RESOURCE_TYPE_ARRAY;
		CURuntime::check(cuGraphicsSubResourceGetMappedArray(&res_desc.res.array.hArray, resource, 0, 0));

		CUsurfObject surface;
		CURuntime::check(cuSurfObjectCreate(&surface, &res_desc));


		CUDA_TEXTURE_DESC tex_desc = {};
		tex_desc.addressMode[0] = CU_TR_ADDRESS_MODE_CLAMP;
		tex_desc.addressMode[1] = CU_TR_ADDRESS_MODE_CLAMP;
		tex_desc.filterMode = CU_TR_FILTER_MODE_LINEAR;

		CUDA_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.format = CU_RES_VIEW_FORMAT_UINT_4X8;
		view_desc.width = source->width;
		view_desc.height = source->height;

		CUtexObject texture;
		CURuntime::check(cuTexObjectCreate(&texture, &res_desc, &tex_desc, nullptr));

		mappings.resource = resource;
		mappings.surface = surface;
		mappings.texture = texture;
	}

	mappings.resources = resources;

	return mappings;
};

shared_ptr<SceneNode> SplatEditor::getSelectedNode(){

	shared_ptr<SceneNode> selected = nullptr;

	scene.process<SceneNode>([&selected](shared_ptr<SceneNode> node){
		if(node->selected){
			selected = node;
		}
	});

	return selected;
}

void SplatEditor::temporarilyDisableShortcuts(int numFrames){
	settings.shortcutsDisabledForXFrames = max(settings.shortcutsDisabledForXFrames, numFrames);
}

bool SplatEditor::areShortcutsDisabled(){
	return settings.shortcutsDisabledForXFrames > 0;
}


void SplatEditor::filter(SNSplats* source, SNSplats* target, FilterRules _rules){

	FilterRules rules = _rules;

	CUdeviceptr cptr_counter;
	cuMemAlloc(&cptr_counter, 8);
	cuMemsetD32(cptr_counter, 0, 1);
	
	uint32_t numAccepted = 0;

	{ // We first count the amount of accepted splats
		GaussianData dummyTarget;
		dummyTarget.position = nullptr;

		void* args[] = {&source->dmng.data, &dummyTarget, &rules, &cptr_counter};
		prog_gaussians_editing->launchCooperative("kernel_filter", args);

		cuMemcpyDtoH(&numAccepted, cptr_counter, sizeof(numAccepted));
	}

	println("numAccepted: {}", numAccepted);

	if(numAccepted > 0)
	{ // Then we allocate the needed memory and copy the accepted splats

		uint32_t numSplatsNew = target->dmng.data.count + numAccepted;

		target->dmng.commit(numSplatsNew);

		cuMemsetD32(cptr_counter, target->dmng.data.count, 1);

		void* args[] = {&source->dmng.data, &target->dmng.data, &rules, &cptr_counter };
		prog_gaussians_editing->launchCooperative("kernel_filter", args);

		// apply soure node's transformation to copied splats
		mat4 transform = inverse(target->transform_global) * source->transform;
		bool onlySelected = false;
		uint32_t first = target->dmng.data.count;
		target->dmng.data.count = numSplatsNew;

		// println("kernel_apply_transformation");
		// println("    target        {}", target->name);
		// println("    target->count {}", target->dmng.data.count);
		// println("    first         {}", first);
		// println("    numAccepted   {}", numAccepted);
		// println("    numSplatsNew  {}", numSplatsNew);

		prog_gaussians_editing->launch(
			"kernel_apply_transformation", 
			{&launchArgs, &target->dmng.data, &transform, &first, &numAccepted, &onlySelected }, 
			numSplatsNew
		);

		void* changedmask = nullptr;
		prog_gaussians_editing->launch(
			"kernel_deselect", 
			{&launchArgs, &target->dmng.data, &changedmask}, 
			target->dmng.data.count
		);
	}

	cuMemFree(cptr_counter);
}

void SplatEditor::setSelectedNode(SceneNode* target){
	scene.process<SceneNode>([&](SceneNode* node){
		node->selected = (node == target);
	});
}

shared_ptr<SNSplats> SplatEditor::filterToNewLayer(FilterRules rules){
	string name = "Duplicated Splats";
	shared_ptr<SNSplats> newNode = make_shared<SNSplats>(name);

	scene.process<SNSplats>([&](SNSplats* node){
		if (!node->visible) return;
		if (node->locked) return;
		filter(node, newNode.get(), rules);
	});

	scene.world->children.insert(scene.world->children.begin(), newNode);

	deselectAll();
	setSelectedNode(newNode.get());

	return newNode;
}

shared_ptr<SNSplats> SplatEditor::duplicateLayer_undoable(shared_ptr<SNSplats> node){

	if(!node) return nullptr;

	shared_ptr<SNSplats> duplicate = clone(node.get());
	duplicate->name = format("{}_duplicate", node->name);

	scene.world->children.push_back(duplicate);

	// this action/lambda keeps the duplicate shared_ptr alive, 
	// meaning the node can only be removed from memory
	// once the history is cleared, or at least this particular action. 
	addAction({
		.undo = [=](){
			this->scene.world->remove(duplicate.get());
		},
		.redo = [=](){
			this->scene.world->children.push_back(duplicate);
		}
	});

	// TODO: Clear node's splat GPU memory upon undo, and recreate upon redo. 
	// - As long as the splats have the same order every time, that should be fine.
	//   (other undoable actions such as painting depend on the exact order)

	return duplicate;
}

// // First duplicate to new layer, then delete selection from selected layer.
// shared_ptr<SNSplats> SplatEditor::extractLayer_undoable(shared_ptr<SNSplats> node){

// 	if(!node) return nullptr;

// 	shared_ptr<SNSplats> duplicate = clone(node.get());
// 	duplicate->name = format("{}_duplicate", node->name);

// 	scene.world->children.push_back(duplicate);

// 	// this action/lambda keeps the duplicate shared_ptr alive, 
// 	// meaning the node can only be removed from memory
// 	// once the history is cleared, or at least this particular action. 
// 	addAction({
// 		.undo = [=](){
// 			this->scene.world->remove(duplicate.get());
// 		},
// 		.redo = [=](){
// 			this->scene.world->children.push_back(duplicate);
// 		}
// 	});

// 	// TODO: Clear node's splat GPU memory upon undo, and recreate upon redo. 
// 	// - As long as the splats have the same order every time, that should be fine.
// 	//   (other undoable actions such as painting depend on the exact order)

// 	return duplicate;
// }

// undo: remove newly added node and restore selection to what it was
//       - store node to delete
//       - store selection mask
// redo: Simply filter again based on selection - nothing needed to store.
shared_ptr<SNSplats> SplatEditor::filterToNewLayer_undoable(FilterRules rules){

	struct DuplicateUndoAction : public Action{
		unordered_map<shared_ptr<SNSplats>, CUdeviceptr> undodata;
		shared_ptr<SNSplats> filteredNode;
		FilterRules rules;

		void undo(){
			auto editor = SplatEditor::instance;

			for(auto [node, selectionmask] : undodata){
				bool inverted = false;
				void* args[] = {&node->dmng.data, &selectionmask, &inverted};
				editor->prog_gaussians_editing->launch("kernel_set_selection", args, node->dmng.data.count);
			}

			SplatEditor::instance->scene.world->remove(filteredNode.get());
			
			// Idealy we would like to fully remove the node data from memory on undo, and simply re-create it on redo.
			// Unfortunately, as of now, we can not destroy and recreate the filtered model.
			// - The order of splats in the filtered model is not stable/deterministic. 
			//   The atomic index computation via atomicAdd may produce different orders each time.
			// - Painting, for example, compacts the undo/redo diff by storing indices of the modified splats.
			//   These indices will not be valid upon re-creating the filtered node.

			// So for now, we keep the entire filtered model in the undo history.
		}

		void redo(){
			auto editor = SplatEditor::instance;

			editor->scene.world->children.insert(editor->scene.world->children.begin(), filteredNode);

			editor->deselectAll();
			editor->setSelectedNode(filteredNode.get());
		}
	};

	// compute selection mask 
	shared_ptr<DuplicateUndoAction> undoable = make_shared<DuplicateUndoAction>();
	undoable->rules = rules;
	undoable->filteredNode = make_shared<SNSplats>("Duplicated Splats");

	scene.process<SNSplats>([&](shared_ptr<SNSplats> node){
		if(!node->visible) return;
		if(node->locked) return;

		int requiredBytes = node->dmng.data.count / 8 + 4;
		CUdeviceptr cptr_changedmask = CURuntime::alloc("filterToNewLayer_undoable selectionmask", requiredBytes);
		prog_gaussians_editing->launch("kernel_get_selectionmask", {&node->dmng.data, &cptr_changedmask}, node->dmng.data.count);

		undoable->undodata[node] = cptr_changedmask;
	});

	scene.process<SNSplats>([&](SNSplats* node){
		if (!node->visible) return;
		if (node->locked) return;
		filter(node, undoable->filteredNode.get(), rules);
	});

	undoable->redo();

	addAction(undoable);

	return undoable->filteredNode;
}

vector<shared_ptr<SceneNode>> SplatEditor::getLayers(){
	vector<shared_ptr<SceneNode>> layers;

	scene.process<SceneNode>([&](shared_ptr<SceneNode> node){
		if(node->hidden) return;

		layers.push_back(node);
	});

	return layers;
}

bool SplatEditor::merge(shared_ptr<SceneNode> snsource, shared_ptr<SceneNode> sntarget){

	shared_ptr<SNSplats> source = dynamic_pointer_cast<SNSplats>(snsource);
	shared_ptr<SNSplats> target = dynamic_pointer_cast<SNSplats>(sntarget);

	if(source != nullptr && target != nullptr){
		insertNodeToNode(source, target);

		// scheduleRemoval(snsource);
		scene.erase(snsource);
	}

	return false;
}

void SplatEditor::scheduleRemoval(SceneNode* node){
	// scheduledForRemoval.push_back(node);
}

void SplatEditor::postRenderStuff(){
	// for(int i = 0; i < scheduledForRemoval.size(); i++){
	// 	SceneNode* toRemove = scheduledForRemoval[i];

	// 	scene.erase([&](SceneNode* node){
	// 		return node == toRemove;
	// 	});
	// }

	// scheduledForRemoval.clear();
}

void SplatEditor::applyDeletion(){

	scene.process<SNSplats>([&](SNSplats* node){
		sortSplatsDevice(node, true);

		CUdeviceptr cptr_counter;
		cuMemAlloc(&cptr_counter, 8);
		cuMemsetD32(cptr_counter, 0, 1);
		
		uint32_t numNondeleted = 0;

		{ // We first count the amount of accepted splats
			GaussianData dummyTarget;
			dummyTarget.position = nullptr;

			FilterRules rules;
			rules.deleted = FILTER_DELETED_NONDELETED;

			void* args[] = {&node->dmng.data, &dummyTarget, &rules, &cptr_counter};
			prog_gaussians_editing->launchCooperative("kernel_filter", args);

			cuMemcpyDtoH(&numNondeleted, cptr_counter, sizeof(numNondeleted));
		}

		uint32_t numDeleted = node->dmng.data.count - numNondeleted;
		// println("numAccepted: {}", numAccepted);

		node->dmng.data.count = numNondeleted;
		node->dmng.commit(numNondeleted);

	});

	//state.hasPendingDeletionTask = false;

}

void SplatEditor::revertDeletion(){

	scene.process<SNSplats>([&](SNSplats* node){

		if(node->hidden) return;
		if(node->locked) return;

		GaussianData model = node->dmng.data;
		uint32_t newFlags = 0;

		prog_gaussians_editing->launchCooperative("kernel_setFlags", {&model, &newFlags});
	});

	//state.hasPendingDeletionTask = false;
}

void SplatEditor::addAction(shared_ptr<Action> action){
	history.resize(history.size() + history_offset);
	history.push_back(action);
	history_offset = 0;
}

void SplatEditor::addAction(LambdaAction action){
		
	struct ActualAction : public Action{
		LambdaAction action;

		void undo(){
			action.undo();
		}

		void redo(){
			action.redo();
		}
	};

	shared_ptr<ActualAction> actualAction = make_shared<ActualAction>();
	actualAction->action = action;

	addAction(actualAction);
}

void SplatEditor::undo(){
	if(history.size() == 0) return;

	int actionIndex = int(history.size()) - 1 + history_offset;

	if(actionIndex < 0) return;

	shared_ptr<Action> action = history[actionIndex];

	action->undo();
	history_offset--;
}

void SplatEditor::redo(){
	if(history_offset == 0) return;

	history_offset++;
	shared_ptr<Action> action = history[history.size() - 1 + history_offset];

	action->redo();
}

void SplatEditor::clearHistory(){
	history_offset = 0;
	history.clear();

	
}

int32_t SplatEditor::getNumSelectedSplats(){

	static CUdeviceptr cptr_counter = CURuntime::alloc("selected splat counter", 8);

	cuMemsetD32(cptr_counter, 0, 1);

	scene.process<SNSplats>([&](SNSplats* node){

		GaussianData source = node->dmng.data;
		GaussianData targetDummy;
		FilterRules rules;
		rules.selection = FILTER_SELECTION_SELECTED;

		void* args[] = { &node->dmng.data, &targetDummy, &rules, &cptr_counter};
		prog_gaussians_editing->launch("kernel_filter", args, node->dmng.data.count);
	});

	uint32_t numSelected = 0;
	cuMemcpyDtoH(&numSelected, cptr_counter, sizeof(uint32_t));

	// might as well update runtime info
	Runtime::numSelectedSplats = numSelected;

	return numSelected;
}

int32_t SplatEditor::getNumDeletedSplats(){

	static CUdeviceptr cptr_counter = CURuntime::alloc("deleted splat counter", 8);

	cuMemsetD32(cptr_counter, 0, 1);

	scene.process<SNSplats>([&](SNSplats* node){

		GaussianData source = node->dmng.data;
		GaussianData targetDummy;
		FilterRules rules;
		rules.deleted = FILTER_DELETED_DELETED;

		void* args[] = { &node->dmng.data, &targetDummy, &rules, &cptr_counter};
		prog_gaussians_editing->launch("kernel_filter", args, node->dmng.data.count);
	});

	uint32_t numDeleted = 0;
	cuMemcpyDtoH(&numDeleted, cptr_counter, sizeof(uint32_t));

	return numDeleted;
}

void SplatEditor::setVRPanelMode(VRPanelMode mode)
{
	active_vr_panel = mode;

	if (imn_painting) {
		imn_painting->visible = mode == VRPanelMode::PAINTING;
	}

	if (imn_brushes) {
		imn_brushes->visible = mode == VRPanelMode::BRUSHES;
	}

	if (imn_layers) {
		imn_layers->visible = mode == VRPanelMode::LAYERS;
	}

	if (imn_assets) {
		imn_assets->visible = mode == VRPanelMode::ASSETS;
	}

	if (imn_teleport) {
		imn_teleport->visible = mode == VRPanelMode::TELEPORT;
	}
}

void SplatEditor::setTeleportMode(TeleportMode mode)
{
	if (teleport_mode == mode) {
		return;
	}

	teleport_mode = mode;
	teleport_target_valid = false;
	hovered_teleport_indicator = -1;

	teleport_refinement_active = false;
	teleport_refinement_depth = 0.0f;
	teleport_refinement_height = 0.0f;
	teleport_content_anchor_position = vec3(0.0f);

	if (sn_dbgsphere) {
		sn_dbgsphere->visible = false;
	}
}

void SplatEditor::requestVRPanelMode(VRPanelMode mode)
{
	
	if (!vr_panel_switch_pending &&
		mode == active_vr_panel) {
		return;
	}

	
	pending_vr_panel = mode;
	vr_panel_switch_pending = true;
}


void SplatEditor::applyPendingVRPanelMode()
{
	if (!vr_panel_switch_pending) {
		return;
	}

	VRPanelMode mode = pending_vr_panel;

	vr_panel_switch_pending = false;

	setVRPanelMode(mode);
}

//void SplatEditor::addTeleportIndicator(const vec3& worldPosition) 
//{
//	const mat4 worldToScene = glm::inverse(scene.world->transform);
//	const vec3 localPosition = vec3(worldToScene * vec4(worldPosition, 1.0f));
//
//	TeleportIndicator indicator;
//	indicator.id = next_teleport_indicator_id++;
//	indicator.localPosition = localPosition;
//
//	teleport_indicators.push_back(indicator);
//}
void SplatEditor::addTeleportIndicator(const vec3& worldPosition)
{
	SNSplats* environment = getTeleportEnvironmentLayer();

	if (!environment) {
		return;
	}

	const mat4 environmentToWorld = scene.world->transform * environment->transform;
	const mat4 worldToEnvironment = glm::inverse(environmentToWorld);

	const vec3 localPosition = vec3(worldToEnvironment * vec4(worldPosition, 1.0f));

	TeleportIndicator indicator;
	indicator.id = next_teleport_indicator_id++;
	indicator.localPosition = localPosition;
	indicator.basisAtCreation = mat3(environmentToWorld);

	teleport_indicators.push_back(indicator);
}

void SplatEditor::initializeTeleportEnvironmentLayer()
{
	if (teleport_environment_layer_id >= 0) {
		return;
	}

	shared_ptr<SNSplats> candidate = nullptr;

	for (auto& child : scene.world->children) {
		auto splatNode = dynamic_pointer_cast<SNSplats>(child);

		if (!splatNode) {
			continue;
		}

		if (splatNode->hidden) {
			continue;
		}

		if (splatNode->dmng.data.count == 0) {
			continue;
		}

		if (!candidate || splatNode->dmng.data.count > candidate->dmng.data.count) {
			candidate = splatNode;
		}
	}
	if (!candidate) {
		return;
	}
	teleport_environment_layer_id = candidate->ID;
	println("[Teleport] Environment layer: {} (ID {})", candidate->name, candidate->ID);
}

SNSplats* SplatEditor::getTeleportEnvironmentLayer()
{
	if (teleport_environment_layer_id < 0) {
		return nullptr;
	}
	for (auto& child : scene.world->children) {
		if (!child) {
			continue;
		}
		if (child->ID != teleport_environment_layer_id) {
			continue;
		}
		return dynamic_cast<SNSplats*>(child.get());
	}

	return nullptr;
}

void SplatEditor::resetTeleportEnvironmentLayer()
{
	teleport_environment_layer_id = -1;
	teleport_target_valid = false;
	teleport_target_index = 0;

	if (sn_dbgsphere) {
		sn_dbgsphere->visible = false;
	}
}

void SplatEditor::createAssetFromLayer(shared_ptr<SNSplats> node)
{
	if (!node) {
		return;
	}

	println("[Asset] 1 start");

	auto newNode = clone(node.get());
	println("[Asset] 2 cloned");

	applyTransformation(newNode->dmng.data, newNode->dmng.data.transform);
	println("[Asset] 3 transformed");

	newNode->dmng.data.transform = mat4(1.0f);
	newNode->transform = mat4(1.0f);
	newNode->dmng.data.transform = mat4(1.0f);
	newNode->dmng.data.writeDepth = false;

	println("[Asset] 4 reset transform");

	updateBoundingBox(newNode.get());
	println("[Asset] 5 bbox");

	newNode->aabb.min = newNode->dmng.data.min;
	newNode->aabb.max = newNode->dmng.data.max;

	println("[Asset] 6 before thumbnail");

	createOrUpdateThumbnail(newNode.get());

	println("[Asset] 7 thumbnail done");

	AssetLibrary::assets.push_back(newNode);

	println("[Asset] 8 asset added");
}

//void SplatEditor::setTeleportEvalTargetFromSelection(string name) {
//	int32_t numSelected = getNumSelectedSplats();
//	if (numSelected <= 0) {
//		println("[Teleport Eval] No splats selected. Target was not created.");
//		return;
//	}
//
//	Box3 bounds = getSelectionAABB();
//
//	teleport_eval_target.name = name;
//	teleport_eval_target.bounds = bounds;
//	teleport_eval_target.aimPoint = 0.5f * (bounds.min + bounds.max);
//	teleport_eval_target.viewOrigins.clear();
//	teleport_eval_target.hasViewOrigin = false;
//	teleport_eval_target.hasPlaneViewA = false;
//	teleport_eval_target.hasPlaneViewB = false;
//	teleport_eval_target.hasPlaneViewC = false;
//
//	println("[Teleport Eval] Target '{}' created from {} selected splats.", name, numSelected);
//	println("[Teleport Eval] Bounds min = ({}, {}, {}), max = ({}, {}, {})", bounds.min.x, bounds.min.y, bounds.min.z, bounds.max.x, bounds.max.y, bounds.max.z);
//	println("[Teleport Eval] Aim point = ({}, {}, {})", teleport_eval_target.aimPoint.x, teleport_eval_target.aimPoint.y, teleport_eval_target.aimPoint.z);
//}
void SplatEditor::setTeleportEvalTargetFromSelection(string name) {
	int32_t numSelected = getNumSelectedSplats();

	if (numSelected <= 0) {
		println("[Teleport Eval] No splats selected. Target was not created.");
		return;
	}

	initializeTeleportEnvironmentLayer();

	SNSplats* environment = getTeleportEnvironmentLayer();

	if (!environment || environment->dmng.data.count == 0) {
		println("[Teleport Eval] No valid teleport environment.");
		return;
	}

	static CUdeviceptr cptr_box = 0;

	if (cptr_box == 0) {
		cptr_box = CURuntime::alloc(
			"Teleport Eval Target AABB",
			sizeof(Box3)
		);
	}

	float inf = Infinity;
	float ninf = -Infinity;

	uint32_t iinf = bit_cast<uint32_t>(inf);
	uint32_t niinf = bit_cast<uint32_t>(ninf);

	cuMemsetD32(cptr_box + 0, iinf, 3);
	cuMemsetD32(cptr_box + 12, niinf, 3);

	GaussianData model = environment->dmng.data;

	// IMPORTANT:
	// compute AABB in environment-local coordinates,
	// independent of scene.world grab/rotation/scale.
	model.transform = mat4(1.0f);

	bool onlySelected = true;

	void* args[] = {
		&launchArgs,
		&model,
		&cptr_box,
		&onlySelected
	};

	prog_gaussians_editing->launch(
		"kernel_compute_boundingbox2",
		args,
		{
			.gridsize = uint32_t(2 * CURuntime::getNumSMs()),
			.blocksize = 256
		}
	);

	Box3 bounds;
	cuMemcpyDtoH(&bounds, cptr_box, sizeof(Box3));

	teleport_eval_target.name = name;
	teleport_eval_target.bounds = bounds;
	teleport_eval_target.aimPoint =
		0.5f * (bounds.min + bounds.max);

	teleport_eval_target.viewOrigins.clear();
	teleport_eval_target.hasViewOrigin = false;
	teleport_eval_target.hasPlaneViewA = false;
	teleport_eval_target.hasPlaneViewB = false;
	teleport_eval_target.hasPlaneViewC = false;

	println(
		"[Teleport Eval] Target '{}' created from {} selected splats in ENVIRONMENT-LOCAL coordinates.",
		name,
		numSelected
	);

	println(
		"[Teleport Eval] Local bounds min = ({}, {}, {}), max = ({}, {}, {})",
		bounds.min.x,
		bounds.min.y,
		bounds.min.z,
		bounds.max.x,
		bounds.max.y,
		bounds.max.z
	);

	println(
		"[Teleport Eval] Local aim point = ({}, {}, {})",
		teleport_eval_target.aimPoint.x,
		teleport_eval_target.aimPoint.y,
		teleport_eval_target.aimPoint.z
	);
}

void SplatEditor::setTeleportEvalViewFromHMD() {
	if (!ovr || !ovr->isActive()) {
		println("[Teleport Eval] VR is not active.");
		return;
	}

	teleport_eval_target.viewOrigin = vec3(ovr->flip * ovr->getHmdPose() * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	teleport_eval_target.hasViewOrigin = true;

	vec3 direction = normalize(teleport_eval_target.aimPoint - teleport_eval_target.viewOrigin);

	println("[Teleport Eval] View origin = ({}, {}, {})", teleport_eval_target.viewOrigin.x, teleport_eval_target.viewOrigin.y, teleport_eval_target.viewOrigin.z);
	println("[Teleport Eval] Ideal direction = ({}, {}, {})", direction.x, direction.y, direction.z);
}

void SplatEditor::runTeleportEvalRay(float radius) {
	if (!teleport_eval_target.hasViewOrigin) {
		println("[Teleport Eval] No evaluation viewpoint defined.");
		return;
	}

	vec3 rayOrigin = teleport_eval_target.viewOrigin;
	vec3 rayDirection = normalize(teleport_eval_target.aimPoint - rayOrigin);

	TeleportEvalResult result = runTeleportEvalRay(rayOrigin, rayDirection, radius);

	if (result.type == TeleportEvalResult::Type::NO_HIT) {
		println("[Teleport Eval] target='{}' radius={} result=NO_HIT", teleport_eval_target.name, radius);
		return;
	}

	const char* type = result.type == TeleportEvalResult::Type::CORRECT ? "CORRECT" : "OUTSIDE_TARGET";
	println("[Teleport Eval] target='{}' radius={} result={} index={} t={}", teleport_eval_target.name, radius, type, result.selectedIndex, result.rayT);
}

//SplatEditor::TeleportEvalResult SplatEditor::runTeleportEvalRay(vec3 rayOrigin, vec3 rayDirection, float radius) {
//	TeleportEvalResult result;
//	result.rayOrigin = rayOrigin;
//	result.rayDirection = normalize(rayDirection);
//
//	initializeTeleportEnvironmentLayer();
//
//	SNSplats* node = getTeleportEnvironmentLayer();
//	if (!node || !node->visible || node->dmng.data.count == 0) return result;
//
//	float minDistance = 0.20f;
//	float maxDistance = 30.0f;
//
//	GaussianData model = node->dmng.data;
//	model.transform = scene.world->transform * node->transform;
//
//	cuMemsetD8Async(cptr_teleport_best, 0xFF, sizeof(unsigned long long), sidestream);
//
//	void* args[] = { &model, &rayOrigin, &result.rayDirection, &radius, &minDistance, &maxDistance, &cptr_teleport_best };
//	prog_gaussians_editing->launch("kernel_findTeleportPoint", args, model.count, sidestream);
//
//	cuMemcpyDtoHAsync(h_teleport_best, cptr_teleport_best, sizeof(unsigned long long), sidestream);
//	cuStreamSynchronize(sidestream);
//
//	const unsigned long long packed = *h_teleport_best;
//	const unsigned long long noResult = std::numeric_limits<unsigned long long>::max();
//
//	if (packed == noResult) return result;
//
//	result.selectedIndex = static_cast<uint32_t>(packed & 0xffffffffull);
//
//	uint32_t tBits = static_cast<uint32_t>(packed >> 32);
//	std::memcpy(&result.rayT, &tBits, sizeof(float));
//
//	result.projectedTarget = rayOrigin + result.rayT * result.rayDirection;
//
//	vec3 gaussianLocalPosition;
//	CURuntime::check(cuMemcpyDtoH(&gaussianLocalPosition, (CUdeviceptr)model.position + sizeof(vec3) * result.selectedIndex, sizeof(vec3)));
//
//	result.gaussianWorldPosition = vec3(model.transform * vec4(gaussianLocalPosition, 1.0f));
//
//	bool insideTarget = result.gaussianWorldPosition.x >= teleport_eval_target.bounds.min.x && result.gaussianWorldPosition.x <= teleport_eval_target.bounds.max.x
//		&& result.gaussianWorldPosition.y >= teleport_eval_target.bounds.min.y && result.gaussianWorldPosition.y <= teleport_eval_target.bounds.max.y
//		&& result.gaussianWorldPosition.z >= teleport_eval_target.bounds.min.z && result.gaussianWorldPosition.z <= teleport_eval_target.bounds.max.z;
//
//	result.type = insideTarget ? TeleportEvalResult::Type::CORRECT : TeleportEvalResult::Type::OUTSIDE_TARGET;
//
//	return result;
//}
SplatEditor::TeleportEvalResult SplatEditor::runTeleportEvalRay(vec3 rayOrigin, vec3 rayDirection, float radius) {
	TeleportEvalResult result;

	result.rayOrigin = rayOrigin;

	if (glm::length(rayDirection) < 0.0001f) {
		println("[Teleport Eval] Invalid ray direction.");
		return result;
	}

	result.rayDirection = glm::normalize(rayDirection);

	// ------------------------------------------------------------
	// Get teleport environment
	// ------------------------------------------------------------

	initializeTeleportEnvironmentLayer();

	SNSplats* node = getTeleportEnvironmentLayer();

	if (!node || !node->visible || node->dmng.data.count == 0) {
		println("[Teleport Eval] No valid teleport environment.");
		return result;
	}

	float minDistance = 0.20f;
	float maxDistance = 30.0f;

	// ------------------------------------------------------------
	// Gaussian model
	//
	// Gaussian positions are stored in environment-local space.
	// The CUDA teleport kernel receives model.transform and therefore
	// evaluates them in the current VR world coordinate system.
	// ------------------------------------------------------------

	GaussianData model = node->dmng.data;

	model.transform =
		scene.world->transform *
		node->transform;

	// ------------------------------------------------------------
	// Reset query result
	// ------------------------------------------------------------

	cuMemsetD8Async(
		cptr_teleport_best,
		0xFF,
		sizeof(unsigned long long),
		sidestream
	);

	// ------------------------------------------------------------
	// Run exactly the same CUDA query used by teleportation
	//
	// rayOrigin / rayDirection are WORLD SPACE here.
	// ------------------------------------------------------------

	void* args[] = {
		&model,
		&rayOrigin,
		&result.rayDirection,
		&radius,
		&minDistance,
		&maxDistance,
		&cptr_teleport_best
	};

	prog_gaussians_editing->launch(
		"kernel_findTeleportPoint",
		args,
		model.count,
		sidestream
	);

	// ------------------------------------------------------------
	// Copy query result back to CPU
	// ------------------------------------------------------------

	cuMemcpyDtoHAsync(
		h_teleport_best,
		cptr_teleport_best,
		sizeof(unsigned long long),
		sidestream
	);

	cuStreamSynchronize(sidestream);

	const unsigned long long packed = *h_teleport_best;

	const unsigned long long noResult =
		std::numeric_limits<unsigned long long>::max();

	if (packed == noResult) {
		result.type = TeleportEvalResult::Type::NO_HIT;
		return result;
	}

	// ------------------------------------------------------------
	// Unpack selected Gaussian index
	// ------------------------------------------------------------

	result.selectedIndex =
		static_cast<uint32_t>(
			packed & 0xffffffffull
			);

	// ------------------------------------------------------------
	// Unpack ray parameter t
	// ------------------------------------------------------------

	const uint32_t tBits =
		static_cast<uint32_t>(
			packed >> 32
			);

	std::memcpy(
		&result.rayT,
		&tBits,
		sizeof(float)
	);

	// ------------------------------------------------------------
	// Actual teleport destination projected onto WORLD SPACE ray
	// ------------------------------------------------------------

	result.projectedTarget =
		rayOrigin +
		result.rayT *
		result.rayDirection;

	// ------------------------------------------------------------
	// Read selected Gaussian center
	//
	// IMPORTANT:
	// model.position is stored in ENVIRONMENT-LOCAL coordinates.
	// ------------------------------------------------------------

	vec3 gaussianLocalPosition;

	CURuntime::check(
		cuMemcpyDtoH(
			&gaussianLocalPosition,
			(CUdeviceptr)model.position +
			sizeof(vec3) * result.selectedIndex,
			sizeof(vec3)
		)
	);

	// ------------------------------------------------------------
	// Store world-space Gaussian position for logging/debugging
	// ------------------------------------------------------------

	result.gaussianWorldPosition =
		vec3(
			model.transform *
			vec4(
				gaussianLocalPosition,
				1.0f
			)
		);

	// ------------------------------------------------------------
	// Classification
	//
	// teleport_eval_target.bounds is now stored in
	// ENVIRONMENT-LOCAL coordinates.
	//
	// Therefore compare LOCAL Gaussian center against LOCAL AABB.
	// Do NOT compare gaussianWorldPosition against local bounds.
	// ------------------------------------------------------------

	const Box3& bounds =
		teleport_eval_target.bounds;

	const bool insideTarget =
		gaussianLocalPosition.x >= bounds.min.x &&
		gaussianLocalPosition.x <= bounds.max.x &&

		gaussianLocalPosition.y >= bounds.min.y &&
		gaussianLocalPosition.y <= bounds.max.y &&

		gaussianLocalPosition.z >= bounds.min.z &&
		gaussianLocalPosition.z <= bounds.max.z;

	result.type =
		insideTarget
		? TeleportEvalResult::Type::CORRECT
		: TeleportEvalResult::Type::OUTSIDE_TARGET;

	return result;
}

void SplatEditor::setTeleportEvalViewFromDesktopCamera() {
	if (!GLRenderer::camera) {
		println("[Teleport Eval] Desktop camera is not available.");
		return;
	}

	teleport_eval_target.viewOrigin = vec3(GLRenderer::camera->world * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	teleport_eval_target.hasViewOrigin = true;

	vec3 direction = normalize(teleport_eval_target.aimPoint - teleport_eval_target.viewOrigin);

	println("[Teleport Eval] View origin = ({}, {}, {})", teleport_eval_target.viewOrigin.x, teleport_eval_target.viewOrigin.y, teleport_eval_target.viewOrigin.z);
	println("[Teleport Eval] Aim point = ({}, {}, {})", teleport_eval_target.aimPoint.x, teleport_eval_target.aimPoint.y, teleport_eval_target.aimPoint.z);
	println("[Teleport Eval] Ideal direction = ({}, {}, {})", direction.x, direction.y, direction.z);
}

void SplatEditor::generateTeleportEvalViewsFromDesktopCamera() {
	if (!GLRenderer::camera) {
		println("[Teleport Eval] Desktop camera is not available.");
		return;
	}

	int numViews = teleport_eval_config.numViews;
	if (numViews <= 0) {
		println("[Teleport Eval] Invalid number of views.");
		return;
	}

	vec3 aimPoint = teleport_eval_target.aimPoint;
	vec3 seedOrigin = vec3(GLRenderer::camera->world * vec4(0.0f, 0.0f, 0.0f, 1.0f));

	vec3 offset = seedOrigin - aimPoint;

	float horizontalRadius = sqrt(offset.x * offset.x + offset.y * offset.y);
	float heightOffset = offset.z;

	if (horizontalRadius < 0.0001f) {
		println("[Teleport Eval] Seed viewpoint is vertically aligned with target.");
		return;
	}

	float seedAngle = atan2(offset.y, offset.x);

	teleport_eval_target.viewOrigins.clear();
	teleport_eval_target.viewOrigins.reserve(numViews);

	for (int i = 0; i < numViews; i++) {
		float angleOffset = glm::radians(360.0f) * float(i) / float(numViews);
		float angle = seedAngle + angleOffset;

		vec3 origin;
		origin.x = aimPoint.x + horizontalRadius * cos(angle);
		origin.y = aimPoint.y + horizontalRadius * sin(angle);
		origin.z = aimPoint.z + heightOffset;

		teleport_eval_target.viewOrigins.push_back(origin);
	}

	teleport_eval_target.viewOrigin = seedOrigin;
	teleport_eval_target.hasViewOrigin = true;

	float distance = glm::length(seedOrigin - aimPoint);
	float angleStep = 360.0f / float(numViews);

	println("[Teleport Eval] Generated {} viewpoints over full 360-degree circle.", numViews);
	println("[Teleport Eval] Angular interval = {} deg", angleStep);
	println("[Teleport Eval] Distance to target = {} m", distance);
	println("[Teleport Eval] Horizontal radius = {} m, height offset = {} m", horizontalRadius, heightOffset);
	println("[Teleport Eval] Seed origin = ({}, {}, {})", seedOrigin.x, seedOrigin.y, seedOrigin.z);
}

//void SplatEditor::runTeleportEvalBatch(float radius) {
//	if (teleport_eval_target.viewOrigins.empty()) {
//		println("[Teleport Eval] No generated viewpoints. Generate evaluation views first.");
//		return;
//	}
//
//	int numViews = static_cast<int>(teleport_eval_target.viewOrigins.size());
//
//	int radiusCm = int(std::round(radius * 100.0f));
//	string filename = format("teleport_eval_{}_R{}cm.txt", teleport_eval_target.name, radiusCm);
//	std::ofstream file(filename);
//
//	file << "viewIndex\tangleDeg\tresult\tradius\trayT\tselectedIndex\toriginX\toriginY\toriginZ\tdirX\tdirY\tdirZ\tgaussianX\tgaussianY\tgaussianZ\tprojectedX\tprojectedY\tprojectedZ\n";
//
//	int correct = 0;
//	int outside = 0;
//	int noHit = 0;
//
//	for (int viewIndex = 0; viewIndex < numViews; viewIndex++) {
//		vec3 origin = teleport_eval_target.viewOrigins[viewIndex];
//		vec3 toTarget = teleport_eval_target.aimPoint - origin;
//
//		if (glm::length(toTarget) < 0.0001f) {
//			println("[Teleport Eval] Skipping invalid view {}.", viewIndex);
//			continue;
//		}
//
//		vec3 direction = normalize(toTarget);
//
//		TeleportEvalResult result = runTeleportEvalRay(origin, direction, radius);
//
//		string type;
//
//		if (result.type == TeleportEvalResult::Type::CORRECT) {
//			type = "CORRECT";
//			correct++;
//		}
//		else if (result.type == TeleportEvalResult::Type::OUTSIDE_TARGET) {
//			type = "OUTSIDE_TARGET";
//			outside++;
//		}
//		else {
//			type = "NO_HIT";
//			noHit++;
//		}
//
//		float angleDeg = 360.0f * float(viewIndex) / float(numViews);
//
//		file << viewIndex << "\t" << angleDeg << "\t" << type << "\t" << radius << "\t" << result.rayT << "\t" << result.selectedIndex << "\t";
//		file << origin.x << "\t" << origin.y << "\t" << origin.z << "\t";
//		file << direction.x << "\t" << direction.y << "\t" << direction.z << "\t";
//		file << result.gaussianWorldPosition.x << "\t" << result.gaussianWorldPosition.y << "\t" << result.gaussianWorldPosition.z << "\t";
//		file << result.projectedTarget.x << "\t" << result.projectedTarget.y << "\t" << result.projectedTarget.z << "\n";
//	}
//
//	file.close();
//
//	int total = correct + outside + noHit;
//	float correctRate = total > 0 ? 100.0f * float(correct) / float(total) : 0.0f;
//
//	println("[Teleport Eval] Batch finished: {} views, radius={} cm", total, radiusCm);
//	println("[Teleport Eval] Correct={} Outside={} NoHit={}", correct, outside, noHit);
//	println("[Teleport Eval] Correct rate={}%", correctRate);
//	println("[Teleport Eval] Saved to '{}'", filename);
//}
void SplatEditor::runTeleportEvalBatch(float radius) {
	if (teleport_eval_target.viewOrigins.empty()) {
		println("[Teleport Eval] No generated viewpoints. Generate evaluation views first.");
		return;
	}

	initializeTeleportEnvironmentLayer();

	SNSplats* environment = getTeleportEnvironmentLayer();
	if (!environment || !environment->visible || environment->dmng.data.count == 0) {
		println("[Teleport Eval] No valid teleport environment.");
		return;
	}

	// Evaluation geometry is stored in environment-local coordinates.
	// Convert it to the current VR world coordinate system before querying.
	const mat4 localToWorld =
		scene.world->transform *
		environment->transform;

	const vec3 aimWorld = vec3(
		localToWorld *
		vec4(teleport_eval_target.aimPoint, 1.0f)
	);

	int numViews = static_cast<int>(
		teleport_eval_target.viewOrigins.size()
		);

	int radiusCm = int(std::round(radius * 100.0f));

	string filename = format(
		"teleport_eval_{}_R{}cm.txt",
		teleport_eval_target.name,
		radiusCm
	);

	std::ofstream file(filename);

	file << "viewIndex\tangleDeg\tresult\tradius\trayT\tselectedIndex\t";
	file << "originX\toriginY\toriginZ\t";
	file << "dirX\tdirY\tdirZ\t";
	file << "gaussianX\tgaussianY\tgaussianZ\t";
	file << "projectedX\tprojectedY\tprojectedZ\n";

	int correct = 0;
	int outside = 0;
	int noHit = 0;

	for (int viewIndex = 0; viewIndex < numViews; viewIndex++) {

		// --------------------------------------------------------
		// Evaluation viewpoint is stored in environment-local space
		// --------------------------------------------------------

		const vec3 originLocal =
			teleport_eval_target.viewOrigins[viewIndex];

		// --------------------------------------------------------
		// Convert synthetic viewpoint to current VR world space
		// --------------------------------------------------------

		const vec3 originWorld = vec3(
			localToWorld *
			vec4(originLocal, 1.0f)
		);

		const vec3 toTargetWorld =
			aimWorld - originWorld;

		if (glm::length(toTargetWorld) < 0.0001f) {
			println(
				"[Teleport Eval] Skipping invalid view {}.",
				viewIndex
			);
			continue;
		}

		const vec3 directionWorld =
			glm::normalize(toTargetWorld);

		// --------------------------------------------------------
		// Use exactly the same world-space query as real VR teleport
		// --------------------------------------------------------

		TeleportEvalResult result =
			runTeleportEvalRay(
				originWorld,
				directionWorld,
				radius
			);

		string type;

		if (result.type == TeleportEvalResult::Type::CORRECT) {
			type = "CORRECT";
			correct++;
		}
		else if (result.type == TeleportEvalResult::Type::OUTSIDE_TARGET) {
			type = "OUTSIDE_TARGET";
			outside++;
		}
		else {
			type = "NO_HIT";
			noHit++;
		}

		const float angleDeg =
			360.0f *
			float(viewIndex) /
			float(numViews);

		// --------------------------------------------------------
		// Save the actual world-space ray used by the CUDA query
		// --------------------------------------------------------

		file
			<< viewIndex << "\t"
			<< angleDeg << "\t"
			<< type << "\t"
			<< radius << "\t"
			<< result.rayT << "\t"
			<< result.selectedIndex << "\t";

		file
			<< originWorld.x << "\t"
			<< originWorld.y << "\t"
			<< originWorld.z << "\t";

		file
			<< directionWorld.x << "\t"
			<< directionWorld.y << "\t"
			<< directionWorld.z << "\t";

		file
			<< result.gaussianWorldPosition.x << "\t"
			<< result.gaussianWorldPosition.y << "\t"
			<< result.gaussianWorldPosition.z << "\t";

		file
			<< result.projectedTarget.x << "\t"
			<< result.projectedTarget.y << "\t"
			<< result.projectedTarget.z << "\n";
	}

	file.close();

	const int total =
		correct +
		outside +
		noHit;

	const float correctRate =
		total > 0
		? 100.0f * float(correct) / float(total)
		: 0.0f;

	println(
		"[Teleport Eval] Batch finished: {} views, radius={} cm",
		total,
		radiusCm
	);

	println(
		"[Teleport Eval] Correct={} Outside={} NoHit={}",
		correct,
		outside,
		noHit
	);

	println(
		"[Teleport Eval] Correct rate={}%",
		correctRate
	);

	println(
		"[Teleport Eval] Saved to '{}'",
		filename
	);
}

void SplatEditor::addTeleportEvalViewFromDesktopCamera() {
	if (!GLRenderer::camera) {
		println("[Teleport Eval] Desktop camera is not available.");
		return;
	}

	vec3 origin = vec3(GLRenderer::camera->world * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	teleport_eval_target.viewOrigins.push_back(origin);

	int index = static_cast<int>(teleport_eval_target.viewOrigins.size());
	vec3 direction = normalize(teleport_eval_target.aimPoint - origin);

	println("[Teleport Eval] Added view {} origin=({}, {}, {}) direction=({}, {}, {})", index, origin.x, origin.y, origin.z, direction.x, direction.y, direction.z);
}

void SplatEditor::clearTeleportEvalViews() {
	teleport_eval_target.viewOrigins.clear();
	println("[Teleport Eval] All evaluation views cleared.");
}

void SplatEditor::drawCross(vec3 position, float size, uint32_t color) {
	drawLine(position + vec3(-size, 0.0f, 0.0f), position + vec3(size, 0.0f, 0.0f), color);
	drawLine(position + vec3(0.0f, -size, 0.0f), position + vec3(0.0f, size, 0.0f), color);
	drawLine(position + vec3(0.0f, 0.0f, -size), position + vec3(0.0f, 0.0f, size), color);
}

//void SplatEditor::drawTeleportEvalDebug() {
//	if (!showTeleportEvalDebug) return;
//
//	const float crossSize = teleport_eval_debugCrossSize;
//
//	// 1. draw target AABB
//	if (showTeleportEvalAABB) {
//		drawBox(teleport_eval_target.bounds, 0xff00ffff);
//		drawCross(teleport_eval_target.aimPoint, crossSize * 1.5f, 0xffffff00);
//	}
//
//	// 2. draw single debug viewOrigin if available
//	if (teleport_eval_target.hasViewOrigin) {
//		drawCross(teleport_eval_target.viewOrigin, crossSize * 1.2f, 0xffff0000);
//		drawLine(teleport_eval_target.viewOrigin, teleport_eval_target.aimPoint, 0xffff0000);
//	}
//
//	if (teleport_eval_target.viewOrigins.size() >= 2) {
//		for (int i = 0; i < teleport_eval_target.viewOrigins.size(); i++) {
//			int next = (i + 1) % teleport_eval_target.viewOrigins.size();
//
//			drawLine(
//				teleport_eval_target.viewOrigins[i],
//				teleport_eval_target.viewOrigins[next],
//				0xff00ffff
//			);
//		}
//	}
//
//	// 3. draw generated evaluation viewpoints and rays
//	for (int i = 0; i < teleport_eval_target.viewOrigins.size(); i++) {
//		vec3 origin = teleport_eval_target.viewOrigins[i];
//
//		if (showTeleportEvalViews) {
//			uint32_t color = (i == 0) ? 0xff00ff00 : 0xff88ff88;
//			drawCross(origin, crossSize, color);
//		}
//
//		if (showTeleportEvalRays) {
//			uint32_t color = (i == 0) ? 0xff00ff00 : 0xff888888;
//			drawLine(origin, teleport_eval_target.aimPoint, color);
//		}
//	}
//}
void SplatEditor::drawTeleportEvalDebug() {
	if (!showTeleportEvalDebug) return;

	initializeTeleportEnvironmentLayer();

	SNSplats* environment = getTeleportEnvironmentLayer();
	if (!environment) return;

	const mat4 localToWorld =
		scene.world->transform *
		environment->transform;

	auto toWorld = [&](vec3 p) {
		return vec3(localToWorld * vec4(p, 1.0f));
		};

	const float crossSize = teleport_eval_debugCrossSize;

	// ============================================================
	// TARGET AIM POINT
	// ============================================================

	vec3 aimWorld = toWorld(teleport_eval_target.aimPoint);

	drawCross(
		aimWorld,
		crossSize * 1.5f,
		0xffffff00
	);

	// ============================================================
	// TARGET AABB
	// Transform all 8 corners instead of only min/max.
	// ============================================================

	if (showTeleportEvalAABB) {
		vec3 l = teleport_eval_target.bounds.min;
		vec3 h = teleport_eval_target.bounds.max;

		vec3 p000 = toWorld(vec3(l.x, l.y, l.z));
		vec3 p100 = toWorld(vec3(h.x, l.y, l.z));
		vec3 p110 = toWorld(vec3(h.x, h.y, l.z));
		vec3 p010 = toWorld(vec3(l.x, h.y, l.z));

		vec3 p001 = toWorld(vec3(l.x, l.y, h.z));
		vec3 p101 = toWorld(vec3(h.x, l.y, h.z));
		vec3 p111 = toWorld(vec3(h.x, h.y, h.z));
		vec3 p011 = toWorld(vec3(l.x, h.y, h.z));

		uint32_t color = 0xff00ffff;

		// bottom
		drawLine(p000, p100, color);
		drawLine(p100, p110, color);
		drawLine(p110, p010, color);
		drawLine(p010, p000, color);

		// top
		drawLine(p001, p101, color);
		drawLine(p101, p111, color);
		drawLine(p111, p011, color);
		drawLine(p011, p001, color);

		// vertical
		drawLine(p000, p001, color);
		drawLine(p100, p101, color);
		drawLine(p110, p111, color);
		drawLine(p010, p011, color);
	}

	// ============================================================
	// GENERATED VIEWPOINTS + RAYS
	// ============================================================

	for (int i = 0; i < static_cast<int>(teleport_eval_target.viewOrigins.size()); i++) {
		vec3 originWorld = toWorld(
			teleport_eval_target.viewOrigins[i]
		);

		if (showTeleportEvalViews) {
			uint32_t color =
				i == 0
				? 0xff00ff00
				: 0xff88ff88;

			drawCross(
				originWorld,
				crossSize,
				color
			);
		}

		if (showTeleportEvalRays) {
			uint32_t color =
				i == 0
				? 0xff00ff00
				: 0xff888888;

			drawLine(
				originWorld,
				aimWorld,
				color
			);
		}
	}

	// ============================================================
	// CIRCLE OUTLINE
	// ============================================================

	if (teleport_eval_target.viewOrigins.size() >= 2) {
		for (int i = 0; i < static_cast<int>(teleport_eval_target.viewOrigins.size()); i++) {
			int next =
				(i + 1) %
				static_cast<int>(teleport_eval_target.viewOrigins.size());

			vec3 a = toWorld(
				teleport_eval_target.viewOrigins[i]
			);

			vec3 b = toWorld(
				teleport_eval_target.viewOrigins[next]
			);

			drawLine(
				a,
				b,
				0xff00ffff
			);
		}
	}
}

void SplatEditor::setTeleportEvalPlaneViewAFromDesktopCamera() {
	if (!GLRenderer::camera) {
		println("[Teleport Eval] Desktop camera is not available.");
		return;
	}

	teleport_eval_target.planeViewA = vec3(GLRenderer::camera->world * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	teleport_eval_target.hasPlaneViewA = true;
	teleport_eval_target.viewOrigins.clear();

	println("[Teleport Eval] Plane View A = ({}, {}, {})",
		teleport_eval_target.planeViewA.x,
		teleport_eval_target.planeViewA.y,
		teleport_eval_target.planeViewA.z);
}

void SplatEditor::setTeleportEvalPlaneViewBFromDesktopCamera() {
	if (!GLRenderer::camera) {
		println("[Teleport Eval] Desktop camera is not available.");
		return;
	}

	teleport_eval_target.planeViewB = vec3(GLRenderer::camera->world * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	teleport_eval_target.hasPlaneViewB = true;
	teleport_eval_target.viewOrigins.clear();

	println("[Teleport Eval] Plane View B = ({}, {}, {})",
		teleport_eval_target.planeViewB.x,
		teleport_eval_target.planeViewB.y,
		teleport_eval_target.planeViewB.z);
}

void SplatEditor::setTeleportEvalPlaneViewCFromDesktopCamera() {
	if (!GLRenderer::camera) {
		println("[Teleport Eval] Desktop camera is not available.");
		return;
	}

	teleport_eval_target.planeViewC = vec3(GLRenderer::camera->world * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	teleport_eval_target.hasPlaneViewC = true;
	teleport_eval_target.viewOrigins.clear();

	println("[Teleport Eval] Plane View C = ({}, {}, {})",
		teleport_eval_target.planeViewC.x,
		teleport_eval_target.planeViewC.y,
		teleport_eval_target.planeViewC.z);
}

void SplatEditor::generateTeleportEvalViewsFromPlane() {
	if (!teleport_eval_target.hasPlaneViewA ||
		!teleport_eval_target.hasPlaneViewB ||
		!teleport_eval_target.hasPlaneViewC) {

		println("[Teleport Eval] Plane View A, B and C must be defined first.");
		return;
	}

	int numViews = teleport_eval_config.numViews;
	if (numViews <= 0) {
		println("[Teleport Eval] Invalid number of views.");
		return;
	}

	vec3 center = teleport_eval_target.aimPoint;

	vec3 A = teleport_eval_target.planeViewA;
	vec3 B = teleport_eval_target.planeViewB;
	vec3 C = teleport_eval_target.planeViewC;

	// Three observation points define the orientation of the plane.
	vec3 normalRaw = glm::cross(B - A, C - A);
	float normalLength = glm::length(normalRaw);

	if (normalLength < 0.0001f) {
		println("[Teleport Eval] Invalid plane: View A, B and C are nearly collinear.");
		return;
	}

	vec3 normal = normalRaw / normalLength;

	// Use View A to define the starting direction,
	// but project that direction onto the target-centered plane.
	vec3 seedVector = A - center;
	float radius = glm::length(seedVector);

	if (radius < 0.0001f) {
		println("[Teleport Eval] Plane View A is too close to target.");
		return;
	}

	vec3 projectedSeed = seedVector - glm::dot(seedVector, normal) * normal;

	if (glm::length(projectedSeed) < 0.0001f) {
		println("[Teleport Eval] View A direction is nearly parallel to plane normal.");
		return;
	}

	vec3 u = glm::normalize(projectedSeed);
	vec3 v = glm::normalize(glm::cross(normal, u));

	teleport_eval_target.viewOrigins.clear();
	teleport_eval_target.viewOrigins.reserve(numViews);

	for (int i = 0; i < numViews; i++) {
		float angle = glm::two_pi<float>() * float(i) / float(numViews);

		vec3 origin = center + radius * (cos(angle) * u + sin(angle) * v);
		teleport_eval_target.viewOrigins.push_back(origin);
	}

	println("[Teleport Eval] Generated {} viewpoints over full 360-degree scene-aligned circle.", numViews);
	println("[Teleport Eval] Radius = {} m", radius);
	println("[Teleport Eval] Angular interval = {} deg", 360.0f / float(numViews));
	println("[Teleport Eval] Plane normal = ({}, {}, {})", normal.x, normal.y, normal.z);

	for (int i : {0, 45, 90, 135}) {
		if (i >= numViews) continue;

		float d = glm::length(teleport_eval_target.viewOrigins[i] - center);
		float planeError = abs(glm::dot(teleport_eval_target.viewOrigins[i] - center, normal));

		println("[Teleport Eval] View {} distance = {} m, plane error = {} m", i, d, planeError);
	}
}

void SplatEditor::runTeleportEvalRadiusSweep(int minRadiusCm, int maxRadiusCm, int stepCm) {
	if (teleport_eval_target.viewOrigins.empty()) {
		println("[Teleport Eval] No generated viewpoints.");
		return;
	}

	if (minRadiusCm <= 0 || maxRadiusCm < minRadiusCm || stepCm <= 0) {
		println("[Teleport Eval] Invalid radius sweep parameters.");
		return;
	}

	println("[Teleport Eval] ========================================");
	println("[Teleport Eval] Radius sweep started.");
	println("[Teleport Eval] Radius: {} cm to {} cm, step {} cm", minRadiusCm, maxRadiusCm, stepCm);
	println("[Teleport Eval] Views per radius: {}", teleport_eval_target.viewOrigins.size());
	println("[Teleport Eval] ========================================");

	for (int radiusCm = minRadiusCm; radiusCm <= maxRadiusCm; radiusCm += stepCm) {
		float radius = float(radiusCm) / 100.0f;

		println("[Teleport Eval] ----- Radius {} cm -----", radiusCm);

		runTeleportEvalBatch(radius);
	}

	println("[Teleport Eval] ========================================");
	println("[Teleport Eval] Radius sweep finished.");
	println("[Teleport Eval] ========================================");
}

bool SplatEditor::getTeleportEvalRightControllerPositionInEnvironment(vec3& position) {
	if (!ovr || !ovr->isActive()) {
		println("[Teleport Eval] VR is not active.");
		return false;
	}

	Controller right = ovr->getRightController();

	if (!right.valid) {
		println("[Teleport Eval] Right controller is not available.");
		return false;
	}

	initializeTeleportEnvironmentLayer();

	SNSplats* environment = getTeleportEnvironmentLayer();

	if (!environment) {
		println("[Teleport Eval] No teleport environment.");
		return false;
	}

	const mat4 controllerPose = mat4(ovr->flip * right.transform);

	const vec3 controllerWorld = vec3(
		controllerPose *
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);

	const mat4 environmentToWorld =
		scene.world->transform *
		environment->transform;

	const mat4 worldToEnvironment =
		glm::inverse(environmentToWorld);

	position = vec3(
		worldToEnvironment *
		vec4(controllerWorld, 1.0f)
	);

	return true;
}

void SplatEditor::setTeleportEvalPlaneViewAFromRightController() {
	vec3 position;
	if (!getTeleportEvalRightControllerPositionInEnvironment(position)) return;

	teleport_eval_target.planeViewA = position;
	teleport_eval_target.hasPlaneViewA = true;
	teleport_eval_target.viewOrigins.clear();

	println("[Teleport Eval] VR Plane A = ({}, {}, {})", position.x, position.y, position.z);
}

void SplatEditor::setTeleportEvalPlaneViewBFromRightController() {
	vec3 position;
	if (!getTeleportEvalRightControllerPositionInEnvironment(position)) return;

	teleport_eval_target.planeViewB = position;
	teleport_eval_target.hasPlaneViewB = true;
	teleport_eval_target.viewOrigins.clear();

	println("[Teleport Eval] VR Plane B = ({}, {}, {})", position.x, position.y, position.z);
}

void SplatEditor::setTeleportEvalPlaneViewCFromRightController() {
	vec3 position;
	if (!getTeleportEvalRightControllerPositionInEnvironment(position)) return;

	teleport_eval_target.planeViewC = position;
	teleport_eval_target.hasPlaneViewC = true;
	teleport_eval_target.viewOrigins.clear();

	println("[Teleport Eval] VR Plane C = ({}, {}, {})", position.x, position.y, position.z);
}



#include "update/inputHandlingDesktop.h"
#include "update/inputHandlingVR.h"

#include "SplatEditor_draw.h"
#include "SplatEditor_render.h"
#include "SplatEditor_update.h"

#include "gui/settings.h"
#include "gui/perf.h"
#include "gui/layers.h"
#include "gui/toolbar.h"
#include "gui/guivr.h"
#include "gui/editing.h"
#include "gui/dev.h"
#include "gui/debugInfo.h"
#include "gui/assets.h"
#include "gui/colorCorrection.h"
#include "gui/menubar.h"
#include "gui/stats.h"
#include "gui/todos.inc"
#include "gui/saveFile.h"
#include "gui/gettingStarted.h"