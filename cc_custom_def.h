/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file cc_custom_def.h
 * @brief project custom defines
 */

#ifndef __CC_CUSTOM_DEF_H__
#define __CC_CUSTOM_DEF_H__

#ifdef __cplusplus
extern "C" {
#if 0
} // dummy
#endif
#endif /* __cplusplus */


///  メッセージキュー生成用キーID
enum MSG_KEY_ID {
	MSG_KEY_MAIN = 2001,
	MSG_KEY_PSCCOMM,
	MSG_KEY_ELCOMM,
	MSG_KEY_GSCOMM,
	MSG_KEY_TESTCORE,
	MSG_KEY_WSSERVER,
	MSG_KEY_ELSIM,
	MSG_KEY_GSSIM,
	MSG_KEY_LOCALGUI,
};

// MODULE's NICKNAME
#define NN_MAIN     "main"
#define NN_PSCCOMM  "psccomm"
#define NN_ELCOMM   "elcomm"
#define NN_GSCOMM   "gscomm"
#define NN_ELSIM    "elsim"
#define NN_GSSIM    "gssim"
#define NN_TESTCORE "testcore"
#define NN_WSSERVER "wsserver"

#define NN_LOCALGUI "localgui"
#define NN_PYSCRIPT "pyscript"
#define NN_SIGNAL   "signal"

#define NN_WEBAPP   "webapp"
#define NN_BACKEND  "backend"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __CC_CUSTOM_DEF_H__
