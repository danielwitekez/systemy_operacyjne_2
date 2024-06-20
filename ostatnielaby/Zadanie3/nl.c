#include <linux/netlink.h>
#include <netlink/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define NETLINK_TEST 17
#define MSG_STR 0x11
#define MAX_PAYLOAD 64

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg1;

int main()
{
	struct nl_sock *ns;
	char msg[] = "Hello netlink";
	ns = nl_socket_alloc();
	if(!ns) {
		fprintf(stderr,"Błąd tworzenia gniazda netlink: %s\t%d.\n",__FILE__,__LINE__-2);
		return -1;
	}
	printf("Mój port: %u\n",nl_socket_get_local_port(ns));
	printf("Port odbiorcy: %u\n",nl_socket_get_peer_port(ns));
	printf("Długość wiadomości: %u\n",sizeof(msg));

	nl_socket_disable_seq_check(ns);
	nl_socket_disable_auto_ack(ns);
	nl_socket_disable_msg_peek(ns);
	
	int result = nl_connect(ns,NETLINK_TEST);
 	if(result<0) {
		fprintf(stderr,"Błąd łączenia gniazda netlink: %s\t%d\t%d.\n",__FILE__,__LINE__-2,result);
		nl_socket_free(ns);
		return -2;
	}

 	result = nl_send_simple(ns,MSG_STR,NLM_F_REQUEST,(void *)msg,sizeof(msg));
	if(result<0) 
		fprintf(stderr,"Błąd wysyłania wiadomości: %s\t%d\t%d\n",__FILE__,__LINE__,result);
	else
		printf("Wysłano %d bajtów wiadomości.\n",result);

	nl_socket_free(ns);

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg1.msg_name = (void *)&dest_addr;
    msg1.msg_namelen = sizeof(dest_addr);
    msg1.msg_iov = &iov;
    msg1.msg_iovlen = 1;
    int result1 = sendmsg(sock_fd, &msg1, 0);

    printf("Oczekiwanie na wiadomość od modułu jądra:\n");
    recvmsg(sock_fd, &msg1, 0);
    printf("\nOtrzymano wiadomość: %s\n", NLMSG_DATA(nlh));
    close(sock_fd);
}