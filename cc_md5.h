/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

/*
 * MD5 class for C++  by hirai
 */

#ifndef __CC_MD5_H__
#define __CC_MD5_H__

/* F, G and H are basic MD5 functions: selection, majority, parity */
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z))) 

/* ROTATE_LEFT rotates x left n bits */
#define MD5_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define MD5_FF(a, b, c, d, x, s, ac) \
  {(a) += MD5_F ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
   (a) = MD5_ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define MD5_GG(a, b, c, d, x, s, ac) \
  {(a) += MD5_G ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
   (a) = MD5_ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define MD5_HH(a, b, c, d, x, s, ac) \
  {(a) += MD5_H ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
   (a) = MD5_ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define MD5_II(a, b, c, d, x, s, ac) \
  {(a) += MD5_I ((b), (c), (d)) + (x) + (unsigned long int)(ac); \
   (a) = MD5_ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

// ==============================================================================================
class cc_md5 {
private:
	unsigned long int i[2];                   /* number of _bits_ handled mod 2^64 */
	unsigned long int buf[4];                                    /* scratch buffer */
	unsigned char in[64];                              /* input buffer */

	unsigned char PADDING[64];
	
	/* Basic MD5 step. Transform buf based on in.
	 */
	void Transform (unsigned long int *buf, unsigned long int *in)
	{
		unsigned long int a = buf[0], b = buf[1], c = buf[2], d = buf[3];

		/* Round 1 */
#define MD5_S11 7
#define MD5_S12 12
#define MD5_S13 17
#define MD5_S14 22
		MD5_FF ( a, b, c, d, in[ 0], MD5_S11, 0xd76aa478); /* 1 */
		MD5_FF ( d, a, b, c, in[ 1], MD5_S12, 0xe8c7b756); /* 2 */
		MD5_FF ( c, d, a, b, in[ 2], MD5_S13, 0x242070db); /* 3 */
		MD5_FF ( b, c, d, a, in[ 3], MD5_S14, 0xc1bdceee); /* 4 */
		MD5_FF ( a, b, c, d, in[ 4], MD5_S11, 0xf57c0faf); /* 5 */
		MD5_FF ( d, a, b, c, in[ 5], MD5_S12, 0x4787c62a); /* 6 */
		MD5_FF ( c, d, a, b, in[ 6], MD5_S13, 0xa8304613); /* 7 */
		MD5_FF ( b, c, d, a, in[ 7], MD5_S14, 0xfd469501); /* 8 */
		MD5_FF ( a, b, c, d, in[ 8], MD5_S11, 0x698098d8); /* 9 */
		MD5_FF ( d, a, b, c, in[ 9], MD5_S12, 0x8b44f7af); /* 10 */
		MD5_FF ( c, d, a, b, in[10], MD5_S13, 0xffff5bb1); /* 11 */
		MD5_FF ( b, c, d, a, in[11], MD5_S14, 0x895cd7be); /* 12 */
		MD5_FF ( a, b, c, d, in[12], MD5_S11, 0x6b901122); /* 13 */
		MD5_FF ( d, a, b, c, in[13], MD5_S12, 0xfd987193); /* 14 */
		MD5_FF ( c, d, a, b, in[14], MD5_S13, 0xa679438e); /* 15 */
		MD5_FF ( b, c, d, a, in[15], MD5_S14, 0x49b40821); /* 16 */

		/* Round 2 */
#define MD5_S21 5
#define MD5_S22 9
#define MD5_S23 14
#define MD5_S24 20
		MD5_GG ( a, b, c, d, in[ 1], MD5_S21, 0xf61e2562); /* 17 */
		MD5_GG ( d, a, b, c, in[ 6], MD5_S22, 0xc040b340); /* 18 */
		MD5_GG ( c, d, a, b, in[11], MD5_S23, 0x265e5a51); /* 19 */
		MD5_GG ( b, c, d, a, in[ 0], MD5_S24, 0xe9b6c7aa); /* 20 */
		MD5_GG ( a, b, c, d, in[ 5], MD5_S21, 0xd62f105d); /* 21 */
		MD5_GG ( d, a, b, c, in[10], MD5_S22,  0x2441453); /* 22 */
		MD5_GG ( c, d, a, b, in[15], MD5_S23, 0xd8a1e681); /* 23 */
		MD5_GG ( b, c, d, a, in[ 4], MD5_S24, 0xe7d3fbc8); /* 24 */
		MD5_GG ( a, b, c, d, in[ 9], MD5_S21, 0x21e1cde6); /* 25 */
		MD5_GG ( d, a, b, c, in[14], MD5_S22, 0xc33707d6); /* 26 */
		MD5_GG ( c, d, a, b, in[ 3], MD5_S23, 0xf4d50d87); /* 27 */
		MD5_GG ( b, c, d, a, in[ 8], MD5_S24, 0x455a14ed); /* 28 */
		MD5_GG ( a, b, c, d, in[13], MD5_S21, 0xa9e3e905); /* 29 */
		MD5_GG ( d, a, b, c, in[ 2], MD5_S22, 0xfcefa3f8); /* 30 */
		MD5_GG ( c, d, a, b, in[ 7], MD5_S23, 0x676f02d9); /* 31 */
		MD5_GG ( b, c, d, a, in[12], MD5_S24, 0x8d2a4c8a); /* 32 */

		/* Round 3 */
#define MD5_S31 4
#define MD5_S32 11
#define MD5_S33 16
#define MD5_S34 23
		MD5_HH ( a, b, c, d, in[ 5], MD5_S31, 0xfffa3942); /* 33 */
		MD5_HH ( d, a, b, c, in[ 8], MD5_S32, 0x8771f681); /* 34 */
		MD5_HH ( c, d, a, b, in[11], MD5_S33, 0x6d9d6122); /* 35 */
		MD5_HH ( b, c, d, a, in[14], MD5_S34, 0xfde5380c); /* 36 */
		MD5_HH ( a, b, c, d, in[ 1], MD5_S31, 0xa4beea44); /* 37 */
		MD5_HH ( d, a, b, c, in[ 4], MD5_S32, 0x4bdecfa9); /* 38 */
		MD5_HH ( c, d, a, b, in[ 7], MD5_S33, 0xf6bb4b60); /* 39 */
		MD5_HH ( b, c, d, a, in[10], MD5_S34, 0xbebfbc70); /* 40 */
		MD5_HH ( a, b, c, d, in[13], MD5_S31, 0x289b7ec6); /* 41 */
		MD5_HH ( d, a, b, c, in[ 0], MD5_S32, 0xeaa127fa); /* 42 */
		MD5_HH ( c, d, a, b, in[ 3], MD5_S33, 0xd4ef3085); /* 43 */
		MD5_HH ( b, c, d, a, in[ 6], MD5_S34,  0x4881d05); /* 44 */
		MD5_HH ( a, b, c, d, in[ 9], MD5_S31, 0xd9d4d039); /* 45 */
		MD5_HH ( d, a, b, c, in[12], MD5_S32, 0xe6db99e5); /* 46 */
		MD5_HH ( c, d, a, b, in[15], MD5_S33, 0x1fa27cf8); /* 47 */
		MD5_HH ( b, c, d, a, in[ 2], MD5_S34, 0xc4ac5665); /* 48 */

		/* Round 4 */
#define MD5_S41 6
#define MD5_S42 10
#define MD5_S43 15
#define MD5_S44 21
		MD5_II ( a, b, c, d, in[ 0], MD5_S41, 0xf4292244); /* 49 */
		MD5_II ( d, a, b, c, in[ 7], MD5_S42, 0x432aff97); /* 50 */
		MD5_II ( c, d, a, b, in[14], MD5_S43, 0xab9423a7); /* 51 */
		MD5_II ( b, c, d, a, in[ 5], MD5_S44, 0xfc93a039); /* 52 */
		MD5_II ( a, b, c, d, in[12], MD5_S41, 0x655b59c3); /* 53 */
		MD5_II ( d, a, b, c, in[ 3], MD5_S42, 0x8f0ccc92); /* 54 */
		MD5_II ( c, d, a, b, in[10], MD5_S43, 0xffeff47d); /* 55 */
		MD5_II ( b, c, d, a, in[ 1], MD5_S44, 0x85845dd1); /* 56 */
		MD5_II ( a, b, c, d, in[ 8], MD5_S41, 0x6fa87e4f); /* 57 */
		MD5_II ( d, a, b, c, in[15], MD5_S42, 0xfe2ce6e0); /* 58 */
		MD5_II ( c, d, a, b, in[ 6], MD5_S43, 0xa3014314); /* 59 */
		MD5_II ( b, c, d, a, in[13], MD5_S44, 0x4e0811a1); /* 60 */
		MD5_II ( a, b, c, d, in[ 4], MD5_S41, 0xf7537e82); /* 61 */
		MD5_II ( d, a, b, c, in[11], MD5_S42, 0xbd3af235); /* 62 */
		MD5_II ( c, d, a, b, in[ 2], MD5_S43, 0x2ad7d2bb); /* 63 */
		MD5_II ( b, c, d, a, in[ 9], MD5_S44, 0xeb86d391); /* 64 */

		buf[0] += a;
		buf[1] += b;
		buf[2] += c;
		buf[3] += d;
	}
public:
	cc_md5 (void) {
		Init ();
	}
	virtual ~cc_md5 () {};

	// ---------------------------------
	// ワンショットで計算するAPI
	void Calc (unsigned char *inBuf, unsigned int inLen, unsigned char *outDigest)
	{
		Init ();
		Update (inBuf, inLen);
		Finish (outDigest);
	}

	// ---------------------------------
	// 複数呼び出し可能なAPI
	void Init (void)
	{
		const unsigned char PADDING_DATA[64] = {
			0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};

		i[0] = i[1] = (unsigned long int)0;
		
		/* Load magic initialization constants.
		 */
		buf[0] = (unsigned long int)0x67452301;
		buf[1] = (unsigned long int)0xefcdab89;
		buf[2] = (unsigned long int)0x98badcfe;
		buf[3] = (unsigned long int)0x10325476;

		memcpy (PADDING, PADDING_DATA, sizeof(PADDING));
	}
	// ---------------------------------
	void Update (unsigned char *inBuf, unsigned int inLen) 
	{
		unsigned long int in[16];
		int mdi;
		unsigned int lli, llii;

		/* compute number of bytes mod 64 */
		mdi = (int)((i[0] >> 3) & 0x3F);

		/* update number of bits */
		if ((i[0] + ((unsigned long int)inLen << 3)) < i[0])
			i[1]++;
		i[0] += ((unsigned long int)inLen << 3);
		i[1] += ((unsigned long int)inLen >> 29);

		while (inLen--) {
			/* add new character to buffer, increment mdi */
			in[mdi++] = *inBuf++;

			/* transform if necessary */
			if (mdi == 0x40) {
				for (lli = 0, llii = 0; lli < 16; lli++, llii += 4)
					in[lli] = (((unsigned long int)in[llii+3]) << 24) |
						(((unsigned long int)in[llii+2]) << 16) |
						(((unsigned long int)in[llii+1]) << 8) |
						((unsigned long int)in[llii]);
				Transform (buf, in);
				mdi = 0;
			}
		}
	}
	// ---------------------------------
	void Finish (unsigned char *digest/* actual digest after MD5Final call */)
	{
		unsigned long int in[16];
		int mdi;
		unsigned int lli, llii;
		unsigned int padLen;
		
		/* save number of bits */
		in[14] = i[0];
		in[15] = i[1];

		/* compute number of bytes mod 64 */
		mdi = (int)((i[0] >> 3) & 0x3F);

		/* pad out to 56 mod 64 */
		padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
		Update (PADDING, padLen);

		/* append length in bits and transform */
		for (lli = 0, llii = 0; lli < 14; lli++, llii += 4)
			in[lli] = (((unsigned long int)in[llii+3]) << 24) |
				(((unsigned long int)in[llii+2]) << 16) |
				(((unsigned long int)in[llii+1]) << 8) |
				((unsigned long int)in[llii]);
		Transform (buf, in);

		/* store buffer in digest */
		for (lli = 0, llii = 0; lli < 4; lli++, llii += 4) {
			digest[llii] = (unsigned char)(buf[lli] & 0xFF);
			digest[llii+1] = (unsigned char)((buf[lli] >> 8) & 0xFF);
			digest[llii+2] = (unsigned char)((buf[lli] >> 16) & 0xFF);
			digest[llii+3] = (unsigned char)((buf[lli] >> 24) & 0xFF);
		}
	}
	
};

#endif // __CC_MD5_H__
