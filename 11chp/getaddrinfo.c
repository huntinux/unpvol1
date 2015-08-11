typedef int SOCKET;

static SOCKET create_and_connec(const char *address, const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;
	SOCKET sfd;

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo(address, port, &hints, &result);
	if (s != 0)
	{
		CLog::Error("getaddrinfo: %s", gai_strerror(s));
		return INVALID_SOCKET;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == INVALID_SOCKET)
		  continue;

		s = connect(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			printf_address(sfd, rp->ai_addr, rp->ai_addrlen, "Connect to");
			int flags = 1;
			if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flags, sizeof(int)) < 0)
			{
				CLog::Error("setsockopt TCP_NODELAY error, errno %d", errno);
			}
			break;
		}
		closesocket(sfd);
	}

	if (rp == NULL)
	{
		CLog::Error("Could not bind\n");
		return INVALID_SOCKET;
	}

	freeaddrinfo(result);

	return sfd;
}
