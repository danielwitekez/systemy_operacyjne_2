#include<stdio.h>
#include<string.h>
#include<netlink/netlink.h>
#include<netlink/msg.h>
#include<netlink/attr.h>
#include<netlink/genl/genl.h>
#include<netlink/genl/ctrl.h>

#define TEST_GENETLINK_CMD 1
#define TEST_GENETLINK_A_MSG 1

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

int main(void)
{
	char msg[] = "Hello genetlink";
	struct nl_sock *ns = NULL;

	if(createSocket(&ns) == -1) {
		return -1;
	}

	printInformation(ns, sizeof(msg));
	disableSomething(&ns);

	int result = genl_connect(ns);
 	if(result<0) {
		fprintf(stderr,"Błąd łączenia gniazda netlink: %s\t%d\t%d.\n",__FILE__,__LINE__-2,result);
		nl_socket_free(ns);
		return -2;
	}

	int family = genl_ctrl_resolve(ns,"GENETLINK_TEST");
	if(family<0) {
		fprintf(stderr,"Błąd uzyskiwania numeru rodziny: %s\t%d\t%d\n",__FILE__,__LINE__-2,family);
		return -3;
	}
	printf("Numer rodziny: %d\n",family);
	
	struct nl_msg *message = nlmsg_alloc();
	if(message==NULL) {
		fprintf(stderr,"Błąd przydziału pamięci na komunikat %s\t%d\n",__FILE__,__LINE__-2);
		nl_socket_free(ns);
		return -3;
	}
	
	void *gen_hdr = genlmsg_put(message,0,NL_AUTO_SEQ,family,0,0,TEST_GENETLINK_CMD,1);
	if(gen_hdr==NULL) {
		fprintf(stderr,"Błąd tworzenia nagłówka komunikatu %s\t%d\n",__FILE__,__LINE__-2);
		nlmsg_free(message);
		nl_socket_free(ns);
		return -4;
	}
	
	result = nla_put_string(message,TEST_GENETLINK_A_MSG,msg);
	if(result<0) {
		fprintf(stderr,"Błąd dodawania wiadomości do komunikatu %s\t%d\t%d\n",__FILE__,__LINE__-2,result);
		nlmsg_free(message);
		nl_socket_free(ns);
		return -5;
	}
	
	result = nl_send_auto(ns,message);
	if(result<0) {
		fprintf(stderr,"Błąd wysyłania wiadomości do jądra %s\t%d\t%d\n",__FILE__,__LINE__-2,result);
		nlmsg_free(message);
		nl_socket_free(ns);
		return -6;
	}
	printf("Wysłano %d bajtów do jądra\n",result);
	
	nlmsg_free(message);
	nl_socket_free(ns);
	return 0;
}
