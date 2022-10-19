

typedef void* voidp;

#define CVECT_TEMPLATE(STRUCTNAME, TYPEDEFNAME, TYPE) \
 \
typedef struct STRUCTNAME { \
	 \
	type *d; \
	 \
	int  num; \
	int  maxnum; \
	int  element_size; \
	 \
	void (*write)(TYPEDEFNAME *thisp, int n, TYPE dt); \
	void (*push_back)(TYPEDEFNAME *thisp, TYPE dt); \
	void (*pop_back)(TYPEDEFNAME *thisp); \
	 \
	int (*size)(void);				/*要素数*/ \
	void (*resize)(int newsize);	/*要素数変更*/					\
	int (*max_size)(void);			/*reallocしないで使える最大サイズ*/ \
	 \
} TYPEDEFNAME;


CVECT_TEMPLATE(_CVECTi, CVECTi, int);
CVECT_TEMPLATE(_CVECTf, CVECTf, float);
CVECT_TEMPLATE(_CVECTd, CVECTd, double);
CVECT_TEMPLATE(_CVECTvp, CVECTvp, voidp);

CVECTi *CVECTi_alloc (int num, ,,,);
CVECTf *CVECTf_alloc (int num, ,,,);
CVECTd *CVECTd_alloc (int num, ,,,);
CVECTvp *CVECTvp_alloc (int num, ,,,);

void CVECTi_free (CVECTi *ptr);
void CVECTf_free (CVECTf *ptr);
void CVECTd_free (CVECTd *ptr);
void CVECTvp_free (CVECTvp *ptr);

CVECTi *__CVECTi_alloc0 (int num)
{
	CVECTi *thisp = malloc(sizeof(CVECTi));

	thisp->num = num;
	thisp->maxnum = num/5+5;
	thisp->element_size = sizeof(int);;

	thisp->d = (int *)malloc(thisp->maxnum * thisp->elementsize);

	//thisp>size = _cvi_size;				/*要素数*/
	//thisp->resize = _cvi_resize;				/*要素数変更*/
	//thisp->max_size = _max_resize;			/*reallocしないで使える最大サイズ*/
}
CVECTi *CVECTi_allocn (int num)
{
	CVECTi *thisp = __CVECTi_alloc0 (num);
	memset ((void*)thisp, 0, sizeof(CVECTi));	// ゼロ初期化
	return thisp;
}
CVECTi *CVECTi_alloc (int num, ,,,)
{
	CVECTi *thisp = __CVECTi_alloc0 (num);
	va_list ap;
    va_start(ap, num);							// valistから初期値代入
	for (int i=0; i<num; i++) {
		thisp->d[i] = va_arg(ap, int);
	}
    va_end(ap);
	return thisp;
}
CVECTi *CVECTi_setdata (CVECTi *thisp, int num, ,,,)
{
	va_list ap;
    va_start(ap, num);
	for (int i=0; i<num; i++) {
		thisp->d[i] = va_arg(ap, int);
	}
    va_end(ap);
	return thisp;
}
