
#ifndef C_TYPE_ID
#define C_TYPE_ID

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// ============================================================================================ TYPE ID
typedef enum {
	CTYPEID_NONE,		//!< タイプ指定なし
	CTYPEID_CHAR,		//!< タイプ指定　char型
	CTYPEID_UCHAR,		//!< タイプ指定　unsigned char型
	CTYPEID_INT,		//!< タイプ指定　int型
	CTYPEID_UINT,		//!< タイプ指定　unsigned int型
	CTYPEID_FLOAT,		//!< タイプ指定　float型
	CTYPEID_DOUBLE,		//!< タイプ指定　double型
	CTYPEID_VOIDP,		//!< タイプ指定　voidp型
	CTYPEID_MSPACE,		//!< 仮想タイプ指定　メモリ空間
} CTYPEID;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
