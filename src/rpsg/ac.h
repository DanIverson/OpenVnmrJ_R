

char *getAcode(int code)
{

if (code == 0)		// 0x000
return(" INTER_REV_CHK");
if (code == 1)		// 0x001
return(" FIFOSTART");
if (code == 2)		// 0x002
return(" FIFOHALT");
if (code == 3)		// 0x003
return(" WAIT4XSYNC");
if (code == 4)		// 0x004
return(" WAIT4ISYNC");
if (code == 5)		// 0x005
return(" WAIT4STOP");

if (code == 6)		// 0x006
return(" DLOAD");
if (code == 7)		// 0x007
return(" TLOAD");
if (code == 8)		// 0x008
return(" EXECUTEPATTERN");
if (code == 9)		// 0x009
return(" MULTIPATTERN");
if (code == 10)		// 0x00d
return(" PATTERNDEF");

if (code == 11)		// 0x00b
return(" TABLEDEF");
if (code == 12)		// 0x00c
return(" SAFESTATE");
if (code == 13)		// 0x00d
return(" SOFTDELAY");
if (code == 14)		// 0x00e
return(" BIGDELAY");

if (code == 15)           // 0x00f
return(" EXECRTPATTERN");

if (code == 16)		// 0x010
return(" FLUSHFFBUF");

/* real time variable group with sub code definitions */
if (code == 100)		// 0x064
return(" RTOP");
if (code == 101)		// 0x065
return(" RT2OP");
if (code == 102)		// 0x066
return(" RT3OP");
if (code == 51)
return(" SYNC_XGATE_COUNT");

if (code == 103)	// 0x067
return(" RTINIT");
if (code == 104)	// 0x068
return(" FRTINIT");

if (code == 105)	// 0x069
return(" RTASSERT");
if (code == 106)	// 0x06a
return(" RTASSERTH");
if (code == 107)	// 0x06b
return(" RTASSERTL		");
if (code == 108)	// 0x06c
return(" LRTASSERT		");

if (code == 109)	// 0x06d
return(" RTCLIP");
if (code == 110)	// 0x06e
return(" RTCLIPH		        ");
if (code == 111)	// 0x06f
return(" RTCLIPL		        ");
if (code == 112)	// 0x070
return(" LRTCLIP");

if (code == 115)		// 0x073
return(" JUMP");
if (code == 116)		// 0x074
return(" JMP_LT		");
if (code == 117)		// 0x075
return(" JMP_NE		");
if (code == 119)          // 0x077
return(" VDELAY_LIST");
if (code == 120)		// 0x078
return(" VDELAY");
if (code == 121)		// 0x079
return(" BRANCH");
if (code == 122)		// 0x07a
return(" BRA_EQ");
if (code == 123)		// 0x07b
return(" BRA_GT");
if (code == 124)		// 0x07c
return(" BRA_LT");
if (code == 125)		// 0x07d
return(" BRA_NE");
/* */
if (code == 126)		// 0x07e
return(" PAD_DELAY");
if (code == 127)		// 0x07f
return(" TASSIGN");
if (code == 128)		// 0x080
return(" TPUT            ");
if (code == 129)		// 0x081
return(" TABLE");

if (code == 130)		// 0x082
return(" NVLOOP");
if (code == 131)		// 0x083
return(" ENDNVLOOP");

if (code == 132)          // 0x084
return(" VLOOPTICKS");

if (code == 135)		// 0x087
return(" IFZERO");
if (code == 136)		// 0x088
return(" ELSENZ");
if (code == 137)		// 0x089
return(" ENDIFZERO");
if (code == 138)		// 0x089
return(" IFMOD2ZERO");
if (code == 139)		// 0x089
return(" TCOUNT          ");

/* control and DDR group */
if (code == 200)		// 0x0c8
return(" INITSCAN");
if (code == 201)		// 0x0c9
return(" NEXTSCAN");
if (code == 202)		// 0x0ca
return(" ENDOFSCAN");
if (code == 203)		// 0x0cb
return(" NEXTCODESET");
if (code == 204)		// 0x0cc
return(" END_PARSE");
if (code == 205)		// 0x0cc
return(" SEND_ZERO_FID");
if (code == 206)		// 0x0ce
return(" NOISE");
if (code == 207)		// 0x0cf
return(" IL_MODE");
if (code == 208)		// 0x0d0
return(" ILCJMP");


/* DDR Related           */
if (code == 210)		// 0x0d2
return(" INITDDR");
if (code == 211)		// 0x0d3
return(" SETACQ_DDR");
if (code == 212)		// 0x0d4
return(" NEXTSCANDDR");
if (code == 213)		// 0x0d5
return(" PINSYNC_DDR");
if (code == 214)		// 0x0d6
return(" RG_MOD_DDR");
if (code == 215)		// 0x0d6
return(" SETAVAR_DDR");
if (code == 216)		// 0x0d6
return(" SETPVAR_DDR");

/* RF group */
if (code == 300)      // 0x12c
return(" VRFAMP");
if (code == 301)      // 0x12d
return(" VRFAMPS");
if (code == 302)      // 0x12e
return(" VPHASE");
if (code == 303)      // 0x12f
return(" VPHASEC");
if (code == 304)      // 0x130
return(" VPHASEQ");
if (code == 305)      // 0x131
return(" VPHASECQ");
if (code == 306)      // 0x132
return(" DECPROGON");
if (code == 307)      // 0x133
return(" DECPROGOFF");
if (code == 308)      // 0x134
return(" TINFO");
if (code == 309)      // 0x135
return(" PNEUTEST");
if (code == 310)      // 0x136
return(" MASTER_CHECK");
if (code == 311)      // 0x137
return(" RFSPARELINE");
if (code == 312)      // 0x138
return(" RFOPMODES");
if (code == 313)      // 0x139
return(" RTFREQ");
if (code == 314)      // 0x13a
return(" AMPTBLS");
if (code == 315)      // 0x13b
return(" TEMPCOMP");
if (code == 316)      // 0x13c
return(" ADVISE_FREQ");
if (code == 317)      // 0x13d
return(" FORTH");
if (code == 318)      // 0x13e
return(" SELECTRFAMP");


/* PFG GROUP */
if (code == 340)		// 0x154
return(" PFGVGRADIENT    ");
if (code == 341)		// 0x155
return(" PFGENABLE");
if (code == 342)		// 0x156
return(" PFGSETZLVL      ");
if (code == 343)		// 0x157
return(" PFGSETZCORR     ");

if (code == 351)		// 0x15f
return(" VGRADIENT");
if (code == 360)		// 0x168
return(" ECC_TIMES	");
if (code == 361)		// 0x169
return(" ECC_AMPS	");
if (code == 362)		// 0x16a
return(" SDAC_VALUES	");
if (code == 363)		// 0x16b
return(" DUTYCYCLE_VALUES ");
if (code == 364)		// 0x16c
return(" GRAD_DELAYS ");

if (code == 365)          // 0x16d
return(" OBLGRD          ");
if (code == 366)          // 0x16e
return(" OBLPEGRD        ");
if (code == 367)          // 0x16f
return(" OBLSHAPEDGRD    ");
if (code == 368)          // 0x170
return(" OBLPESHAPEDGRD  ");
if (code == 369)          // 0x171
return(" SHAPEDGRD       ");

if (code == 390)          // 0x186
return(" SETGRDROTATION  ");
if (code == 391)          // 0x187
return(" SETVGRDROTATION ");
if (code == 392)          // 0x188
return(" SETVGRDANGLIST  ");
if (code == 393)          // 0x189
return(" EXEVGRDROTATION ");
if (code == 395)          // 0x18A
return(" SETPEVALUE      ");


if (code == 400)		// 0x190
return(" LOCKINFO");
if (code == 401)		// 0x191
return(" SETNSR");
if (code == 402)		// 0x192
return(" SMPL_HOLD");
if (code == 403)		// 0x193
return(" NSRVSET");
if (code == 404)		// 0x194
return(" TNLK");

if (code == 500)		// 0x1f4
return(" ROTORSYNC_TRIG	");
if (code == 501)		// 0x1f5
return(" READHSROTOR	");
if (code == 502)		// 0x1f6
return(" MASTERLOCKSETTC ");
if (code == 503)		// 0x1f7
return(" MASTERLOCKHOLD  ");
if (code == 504)		// 0x1f8
return(" LOCKAUTO	");
if (code == 505)		// 0x1f9
return(" SHIMAUTO	");

if (code == 506)		// 0x1fa
return(" SETVT");
if (code == 507)		// 0x1fb
return(" WAIT4VT");
if (code == 508)		// 0x1fc
return(" SETSPIN");
if (code == 509)		// 0x1fd
return(" CHECKSPIN");
if (code == 510)		// 0x1fe
return(" LOCKCHECK");
if (code == 511)		// 0x1ff
return(" SETSHIMS");
if (code == 512)		// 0x200
return(" SYNCSETSHIMS");
if (code == 513)		// 0x201
return(" SSHA");
if (code == 514)		// 0x202
return(" RECEIVERGAIN");
if (code == 515)		// 0x203
return(" AUTOGAIN");
if (code == 516)		// 0x204
return(" GETSAMP");
if (code == 517)		// 0x205
return(" HOMOSPOIL");
if (code == 518)		// 0x206
return(" TUNEOPS         ");
if (code == 519)		// 0x207
return(" SENDSYNC");
if (code == 520)		// 0x208
return(" ROLLCALL");
if (code == 521)		// 0x209
return(" LOADSAMP");
if (code == 522)		// 0x20a
return(" SETACQSTATE");

if (code == 525)          // 0x20d
return(" MRIUSERBYTE	");
if (code == 526)          // 0x20e
return(" MRIUSERGATES	");


if (code == 599)		// 0x257
return(" OLDCODE         ");
return("UNKNOWN");

}
