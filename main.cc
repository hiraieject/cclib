#include "cc_udpcomm.h"
#include "cc_md5.h"

#include "yostemplate.h"

int main(int argc, char *argv[])
{
	unsigned char *textp;
	int textsz;
	
	cc_udprecv *urecvp1 = new cc_udprecv ();
	//cc_udprecv *urecvp2 = new cc_udprecv (12345, INADDR_ANY, 1024/*buffer size*/);	//　同じポートを２個受信で開くことはできない
	cc_udpsend *usendp1 = new cc_udpsend ();
	cc_udpsend *usendp2 = new cc_udpsend ();
	cc_udpsend *usendp3 = new cc_udpsend (); // ポートが違うと受信できない

	urecvp1->connect (12345, INADDR_ANY);
	//udprecv *urecvap2 = new udprecv (12345, INADDR_ANY, 1024/*buffer size*/);	//　同じポートを２個受信で開くことはできない
	usendp1->connect (12345, INADDR_ANY);
	usendp2->connect (12345, INADDR_ANY);
	usendp3->connect (12340, INADDR_ANY); // ポートが違うと受信できない
	
	textp = (unsigned char*)"this is send test from module-1";
	textsz = strlen ((const char*)textp) +1/*NULLも送る*/;
	usendp1->send (textp, textsz);

	textp = (unsigned char*)"this is send test from module-2";
	textsz = strlen ((const char*)textp) +1/*NULLも送る*/;
	usendp2->send (textp, textsz);

	textp = (unsigned char*)"this is send test from module-3";
	textsz = strlen ((const char*)textp) +1/*NULLも送る*/;
	usendp3->send (textp, textsz);

	usleep (500/*msec*/*1000);
	
	delete usendp1;
	delete usendp2;
	delete usendp3;
	usleep (500/*msec*/*1000);

	delete urecvp1;
	//delete urecvp2;
	
	return 1;
}
