#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>


//struct hostent {
//    char  *h_name;            /* official name of host */
//    char **h_aliases;         /* alias list */
//    int    h_addrtype;        /* host address type */
//    int    h_length;          /* length of address */
//    char **h_addr_list;       /* list of addresses */
//}
//#define h_addr h_addr_list[0] /* for backward compatibility */

int main(int argc, char * argv[])
{
	struct hostent *he;
	char buff[INET_ADDRSTRLEN];
	char **p;

	if(argc != 2) {
		fprintf(stderr, "Usage: %s hostname\n", argv[0]);
		exit(1);
	}

	he = gethostbyname(argv[1]);
	if(he != NULL) {
		printf("offical name: %s\n", he->h_name);
		for (p = he->h_aliases; *p; p++)
			printf("alias name: %s\n", *p);
		if(he->h_addrtype == AF_INET) {
			for (p = he->h_addr_list; *p; p++)
				printf("address: %s\n", inet_ntop(AF_INET, *p, buff,INET_ADDRSTRLEN));
		}
	}

	return 0;
}
