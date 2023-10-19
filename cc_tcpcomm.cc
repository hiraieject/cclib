/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#include "cc_tcpcomm.h"


// =====================================================================================
// ===================================================================================== TCP CLIENT
// =====================================================================================

cc_tcpconnect::cc_tcpconnect (key_t message_key, std::string nickname) :
	cc_thread (message_key, nickname),
    tcpcomm_dbg (nickname)
{
    // すべてのインスタンスの debugprint をまとめてオンにする
    //tcpcomm_dbg.enable();

	CC_TCPCOMM_DBGPR ("instance created\n");

	// initialize
	clientSocket = -1;
}
//cc_tcpconnect::cc_tcpconnect (void) :
//	cc_thread ("cc_tcpconnect")
//{
//	CC_TCPCOMM_DBGPR ("instance created\n");
//
//	// initialize
//	clientSocket = -1;
//}

cc_tcpconnect::~cc_tcpconnect ()
{
	exec_disconnect();

	CC_TCPCOMM_DBGPR ("instance deleted\n");
}

void
cc_tcpconnect::thread_main (void)
{
}

bool
cc_tcpconnect::exec_server_connect (unsigned int port, const char *addrstr)
{
    return exec_server_connect (port, inet_addr(addrstr));
}
bool
cc_tcpconnect::exec_server_connect (unsigned int port, in_addr_t ipaddr)
{
	if (clientSocket != -1) {
        CC_TCPCOMM_ERRPR ("already connected\n");
		return false;
    }

	// create socket
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror ("cc_tcpconnect::exec_server_connect: socket()\n");
		return false;
	}

	// address
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_port = htons(port);
	clientAddress.sin_addr.s_addr = ipaddr;
	CC_TCPCOMM_DBGPR ("PORT = %d\n", port);

    // サーバーに接続
    if (connect(clientSocket, (struct sockaddr*)&clientAddress, sizeof(clientAddress)) < 0) {
		perror ("cc_tcpconnect::exec_server_connect: connect()\n");
        CC_TCPCOMM_ERRPR ("connect error\n");
		close(clientSocket);
        return false;
    }
    // スレッドを起動する
    thread_up();

	// connect success
	CC_TCPCOMM_DBGPR ("connected\n");
	return true;
}
bool
cc_tcpconnect::exec_disconnect (void)
{
    // スレッドを終了する
    thread_down();

    // socket関連あとしまつ
	if (clientSocket != -1) {
		close(clientSocket);
		clientSocket = -1;
		CC_TCPCOMM_DBGPR ("disconnected\n");
	}
	return true;
}
ssize_t
cc_tcpconnect::exec_send (unsigned char *dptr, int dsize)
{
    ssize_t ret = send(clientSocket, (const void*)dptr, (size_t)dsize, 0);
	if (ret == -1) {
		perror("cc_tcpconnect::exec_send: send()");
	} else {
		CC_TCPCOMM_DBGPR ("send tcp packet retcode=%ldbyte\n", (unsigned long)ret);
	}
	return ret;
}
ssize_t
cc_tcpconnect::exec_recv (unsigned char *dptr, int dsize)
{
    ssize_t ret = recv(clientSocket, (void*)dptr, (size_t)dsize, 0);
	if (ret == -1) {
		perror("cc_tcpconnect::exec_recv: recv()");
	} else {
		CC_TCPCOMM_DBGPR ("recv tcp packet retcode=%ldbyte\n", (unsigned long)ret);
	}
	return ret;
}

int
cc_tcpconnect::get_fd (void) {
    return clientSocket;
}

bool
cc_tcpconnect::get_status (void) {
	return clientSocket > 0;
}

void
cc_tcpconnect::get_ip (struct in_addr &in) {
	struct ifreq ifr;
	get_ifinfo (ifr);
	in = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	//printf ("myip %s-%s\n" , "eth0" , inet_ntoa(in));
}

void
cc_tcpconnect::get_ifinfo (struct ifreq &ifr) {
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name , "eth0");
	ioctl(clientSocket, SIOCGIFADDR, &ifr);
}

// =====================================================================================
// ===================================================================================== TCP SERVER
// =====================================================================================

void
cc_tcpserver::thread_main (void)
{
	struct timeval timeout;
	fd_set	readSet;

	CC_TCPCOMM_DBGPR ("conn waiting thread: thread up\n");
    
    while (loop_continue()) {

        // タイムアウト設定
        timeout.tv_sec	= 0;
		timeout.tv_usec	= 1000*1000; // 1sec loop

        // 待受対象
		FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        // select でイベント待ち
		select(FD_SETSIZE, &readSet, 0, 0, &timeout);		// wait

        // イベント処理
        if( FD_ISSET(serverSocket, &readSet) ) {
            // 新しいクライアント接続の受け入れ
            {
                cc_tcpconnect *conn = create_conn(); // connectionのインスタンス生成
                socklen_t addr_size = sizeof(struct sockaddr_in);
                conn->clientSocket = accept(serverSocket, (struct sockaddr*)&(conn->clientAddress), &addr_size);
                if ((int)connections.size() < max_connection
                    && conn->clientSocket > 0) {
                    CC_TCPCOMM_DBGPR ("conn waiting thread: new client accepted\n");
                    // クライアント接続のスレッドを起動する
                    conn->thread_up();
                    // 接続が確立したので、connectionsに保存
                    connections.push_back(conn);
                    CC_TCPCOMM_DBGPR ("connection count %d\n", (int)connections.size());
                } else {
                    // 接続失敗
                    CC_TCPCOMM_ERRPR ("conn waiting thread: new client ignored\n");
                    delete conn;
                }
            }
		}
        // ガベージコレクション、クローズしているコネクションインスタンスをサーチしてdeleteする
        for (auto it = connections.begin(); it != connections.end(); ) {
            if ((*it)->clientSocket == -1) {
                CC_TCPCOMM_DBGPR ("conn waiting thread: found closed conn\n");
                delete (*it);               // インスタンスをdelete
                it = connections.erase(it); // vectorからポインタを削除
                CC_TCPCOMM_DBGPR ("connection count %d\n", (int)connections.size());
            } else {
                it++;
            }
        }
	}
    // コネクションインスタンスをdeleteする
    for (auto it = connections.begin(); it != connections.end(); ) {
        delete (*it);               // インスタンスをdelete
        it = connections.erase(it); // vectorからポインタを削除
    }
	CC_TCPCOMM_DBGPR ("conn waiting thread: thread down\n");
}

cc_tcpserver::cc_tcpserver (key_t message_key, std::string nickname) :
	cc_thread (message_key, nickname)
{
    // すべてのインスタンスの debugprint をまとめてオンにする
    //tcpcomm_dbg.enable();

	CC_TCPCOMM_DBGPR ("instance created\n");

	// initialize
	serverSocket = -1;
}

//cc_tcpserver::cc_tcpserver (void) :
//    cc_thread ("cc_tcpserver")
//{
//	CC_TCPCOMM_DBGPR ("instance created\n");
//
//	// initialize
//	serverSocket = -1;
//}

cc_tcpserver::~cc_tcpserver ()
{
	// detach resource
	stop_server ();

	CC_TCPCOMM_DBGPR ("instance deleted\n");
}

cc_tcpconnect *
cc_tcpserver::create_conn (void)
{
    return new cc_tcpconnect(-1,"cc_tcpconnect");
}

bool
cc_tcpserver::start_server (unsigned int port, int cmax)
{
    if (get_server_status()) {
        CC_TCPCOMM_ERRPR ("server already opened\n");
        return false;
    }
    // 変数の初期化
    max_connection = cmax;

	// create socket
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cc_tcpserver::start_server: socket()");
		return false;
	}

    // SO_REUSEADDRオプションを設定(解放後、すぐ再利用可能とする)
    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		perror("cc_tcpserver::start_server: setsockopt()");
        CC_TCPCOMM_ERRPR ("server start failed\n");
		close (serverSocket);
        return false;
    }
	
	// address
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	CC_TCPCOMM_DBGPR ("PORT = %d\n", port);

	// bind
	if (bind(serverSocket, (const struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
		perror("cc_tcpserver::start_server: bind()");
        CC_TCPCOMM_ERRPR ("server start failed\n");
		close (serverSocket);
		return false;
	}

    // listen
    if (listen(serverSocket, 10) == 0) {
        CC_TCPCOMM_DBGPR ("server start success\n");
    } else {
		perror("cc_tcpserver::start_server: listen()");
        CC_TCPCOMM_ERRPR ("server start failed\n");
		close (serverSocket);
        return false;
    }
	
	// threadを上げる
	CC_TCPCOMM_DBGPR ("now start server thread\n");
	thread_up ();
    
	return true;
}

bool
cc_tcpserver::stop_server (void)
{
    // server close
    if (get_server_status()) {
        // threadを落とす
        thread_down();
        // socket類の後始末
		close (serverSocket);
		serverSocket = -1;
		CC_TCPCOMM_DBGPR ("stoped server thread\n");
	}
	return true;
}

bool
cc_tcpserver::get_server_status (void)
{
	return serverSocket >= 0;           // 接続していればtrueを返す
}
void
cc_tcpserver::get_ip (struct in_addr &in) {
	struct ifreq ifr;
	get_ifinfo (ifr);
	in = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	//printf ("myip %s-%s\n" , "eth0" , inet_ntoa(in));
}
void cc_tcpserver::get_ifinfo (struct ifreq &ifr) {
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name , "eth3");
	ioctl(serverSocket, SIOCGIFADDR, &ifr);
}


