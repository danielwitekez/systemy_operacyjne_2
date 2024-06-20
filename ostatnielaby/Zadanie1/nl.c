#include<stdio.h>
#include<string.h>
#include<netlink/netlink.h>

#define NETLINK_TEST 17
#define MSG_STR 0x11

int createSocket(struct n1_sock **ns) {
	*ns = nl_socket_alloc();
	if(!*ns) {
		fprintf(stderr,"Błąd tworzenia gniazda netlink: %s\t%d.\n",__FILE__,__LINE__-2);
		return -1;
	}

	return 0;
}

void printInformation(struct n1_sock *ns, int size) {
	printf("Mój port: %u\n",nl_socket_get_local_port(ns));
	printf("Port odbiorcy: %u\n",nl_socket_get_peer_port(ns));
	printf("Długość wiadomości: %u\n",size);
}

void disableSomething(struct n1_sock **ns) {
	nl_socket_disable_seq_check(*ns);
	nl_socket_disable_auto_ack(*ns);
	nl_socket_disable_msg_peek(*ns);
}

int connectToSocket(struct n1_sock **ns) {
	int result = nl_connect(*ns,NETLINK_TEST);
 	if(result<0) {
		fprintf(stderr,"Błąd łączenia gniazda netlink: %s\t%d\t%d.\n",__FILE__,__LINE__-2,result);
		nl_socket_free(*ns);
		return -2;
	}

	return 0;
}

int main(void)
{
	struct nl_sock *ns;
	char msg[] = "Hello netlink";
	int result;

	if(createSocket(&ns) == -1) {
		return -1;
	}
	printInformation(ns, sizeof(msg));
	disableSomething(&ns);
	
	if(connectToSocket(&ns) == -2) {
		return -2;
	}

 	result = nl_send_simple(ns,MSG_STR,NLM_F_REQUEST,(void *)msg,sizeof(msg));
	if(result<0) 
		fprintf(stderr,"Błąd wysyłania wiadomości: %s\t%d\t%d\n",__FILE__,__LINE__,result);
	else
		printf("Wysłano %d bajtów wiadomości.\n",result);

	nl_socket_free(ns);
	return 0;
}