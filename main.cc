#include "cc_udpcomm.h"
#include "cc_md5.h"
#include "cc_mutex.h"

#include "yostemplate.h"

#include "c_list.h"
#include "c_vector.h"

#define TESTPORT 11000

int main(int argc, char *argv[])
{
	unsigned char *textp;
	int textsz;
	struct in_addr in;
	struct ifreq ifr;

	printf ("--------\n");
	
	cc_udprecv *urecvp1 = new cc_udprecv ();
	cc_udpsend *usendp1 = new cc_udpsend ();
	cc_udpsend *usendp2 = new cc_udpsend ();
	cc_udpsend *usendp3 = new cc_udpsend (); // ポートが違うと受信できない

	urecvp1->connect (TESTPORT, INADDR_ANY);
	urecvp1->get_ifinfo (ifr);
	printf ("## recv1 %s:%s:%s",
			inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
			inet_ntoa(((struct sockaddr_in *)&ifr.ifr_broadaddr)->sin_addr),
			inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr));
	
	usendp1->connect (TESTPORT, INADDR_ANY);
	usendp1->get_ip (in);
	printf ("## origin ip %s-%s\n" , "eth0" , inet_ntoa(in));

	usendp2->connect (TESTPORT+1, INADDR_ANY);
	usendp2->get_ip (in);
	printf ("## origin ip %s-%s\n" , "eth0" , inet_ntoa(in));

	usendp3->connect (TESTPORT+2, INADDR_ANY); // ポートが違うと受信できない
	usendp3->get_ip (in);
	printf ("## origin ip %s-%s\n" , "eth0" , inet_ntoa(in));

	printf ("--------\n");
	
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

	printf ("--------\n");
	{
		unsigned char buf[100];
		urecvp1->send_to_thread (buf, sizeof (buf));
		usleep (500/*msec*/*1000);
		urecvp1->send_to_thread (buf, sizeof (buf)/2);
		usleep (500/*msec*/*1000);
		urecvp1->send_to_thread (buf, sizeof (buf)/4);
		usleep (500/*msec*/*1000);
	}

	printf ("--------\n");
	
	delete usendp1;
	delete usendp2;
	delete usendp3;
	delete urecvp1;

	printf ("--------\n");

	clist_test_main();
	c_vector_test_main();

	printf ("--------\n");
	
	return 0;
}
