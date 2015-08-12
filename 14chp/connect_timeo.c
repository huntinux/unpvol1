/* include connect_timeo */

static void	connect_alarm(int);

typedef	void	Sigfunc(int);	/* for signal handlers */

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}

int
connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	Sigfunc	*sigfunc;
	int		n;

	sigfunc = Signal(SIGALRM, connect_alarm);
	if (alarm(nsec) != 0)
		err_msg("connect_timeo: alarm was already set");

	if ( (n = connect(sockfd, saptr, salen)) < 0) { // ��alarm���õ�ʱ�䳬ʱ,connect���ж�,errnoΪEINTR
		close(sockfd);
		if (errno == EINTR) // ��⵽errnoΪEINTRʱ,����ȷ����ʱ�ǳ�ʱ������.
			errno = ETIMEDOUT;
	}
	alarm(0);					/* turn off the alarm */
	Signal(SIGALRM, sigfunc);	/* restore previous signal handler */

	return(n);
}

static void
connect_alarm(int signo)
{
	return;		/* just interrupt the connect() */
}
/* end connect_timeo */

void
Connect_timeo(int fd, const SA *sa, socklen_t salen, int sec)
{
	if (connect_timeo(fd, sa, salen, sec) < 0)
		err_sys("connect_timeo error");
}
