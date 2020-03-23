/*
 * netlink_userapp.c
 *
 ***********************************************************
 * Brief Description
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_MY_UNIT_PROTO    31 // kernel netlink protocol # (regd by our kernel module)
#define USER_NL_ID		  1
#define NLINK_MSG_LEN		128

static const char *thedata = "sample user data to send to kernel via netlink";

int main(int argc, char **argv)
{
	int sd;
	struct sockaddr_nl src_nl, dest_nl;
	struct nlmsghdr *nlhdr; // 'carries' the payload
	struct iovec iov;
	struct msghdr msg;
	ssize_t nsent, nrecv;

	sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_MY_UNIT_PROTO);
	if (sd < 0) {
		perror("netlink_u: netlink socket creation failed");
		exit(EXIT_FAILURE);
	}
	printf("%s:%d: netlink socket created\n", argv[0], getpid());

	/* Setup the netlink source addr structure and bind it */
	memset(&src_nl, 0, sizeof(src_nl));
	src_nl.nl_family = AF_NETLINK;
	/* Note carefully: nl_pid is NOT necessarily the PID of the sender
	 * process; it's actually 'port id' and can be any unique number
	 */
	src_nl.nl_pid = getpid();
	src_nl.nl_groups = 0x0;   // no multicast
	if (bind(sd, (struct sockaddr *)&src_nl, sizeof(src_nl)) < 0) {
		perror("netlink_u: bind failed");
		exit(EXIT_FAILURE);
	}
	printf("%s: bind done\n", argv[0]);
		
	/* Setup the netlink destination addr structure */
	memset(&dest_nl, 0, sizeof(dest_nl));
	dest_nl.nl_family = AF_NETLINK;
	dest_nl.nl_groups = 0x0; // no multicast
	dest_nl.nl_pid = 0; // destined for the kernel

	/* Setup the netlink header structure */
	nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(1024));
	if (!nlhdr) {
		fprintf(stderr, "netlink_u: malloc nlhdr failed");
		exit(EXIT_FAILURE);
	}
	memset(nlhdr, 0, NLMSG_SPACE(1024));
	nlhdr->nlmsg_len = NLMSG_SPACE(1024);
	nlhdr->nlmsg_pid = getpid(); //USER_NL_ID;

	/* Setup the payload to transmit */
	strncpy(NLMSG_DATA(nlhdr), thedata, strlen(thedata));
	printf("%s: destination struct, netlink hdr, payload setup\n", argv[0]);

	/* Setup the iovec */
	memset(&iov, 0, sizeof(struct iovec));
	iov.iov_base = (void *)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;
	printf("%s: folded nl header into iov structure\n", argv[0]);

	/* Setup the message header structure */
	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&dest_nl;   // dest addr
	msg.msg_namelen = sizeof(dest_nl); // size of dest addr
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1; // # elements in msg_iov
	printf("%s: initialized msghdr structure\n", argv[0]);
	
	/* Actually (finally!) send the message via sendmsg(2) */
	nsent = sendmsg(sd, &msg, 0);
	if (nsent < 0) {
		perror("netlink_u: sendmsg(2) failed");
		free(nlhdr);
		exit(EXIT_FAILURE);
	} else if (nsent == 0) {
		printf(" 0 bytes sent\n");
		free(nlhdr);
		exit(EXIT_FAILURE);
	}
	printf("%s:sendmsg(): *** success (sent %ld bytes)\n",
		argv[0], nsent);
	fflush(stdout);

	/* Block on incoming msg from the kernelspace component */
	printf("%s: now blocking on kernel nl msg w/ recvmsg() ...\n", argv[0]);
	nrecv = recvmsg(sd, &msg, 0);
	if (nrecv < 0) {
		perror("netlink_u: recvmsg(2) failed");
		free(nlhdr);
		exit(EXIT_FAILURE);
	}
	printf("%s:recvmsg(): *** success (got %ld bytes):"
		"\nmsg from kernel netlink: \"%s\"\n",
		argv[0], nrecv, (char *)NLMSG_DATA(nlhdr));

	free(nlhdr);
	close(sd);

	exit(EXIT_SUCCESS);
}