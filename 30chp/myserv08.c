/*
	main thread: create several thread
				 accept connection from client
				 store accept_fd in an array
				 signal thread to get a accept_fd 

				 using mutex and conditional variable
   */


#define MAX_CLIENT	32
int accept_fd[MAX_CLIENT];
int iput = 0;
int iget = -1;
pthread_mutex_t accept_fd_mutex;

int main()
{
	// socket, bind, listen ---> listen_fd

	// create threads

	// accept ---> accept_fd
	accept_fd = accept();	
	pthread_mutex_lock(accept_fd_mutex);
	accept_fd[iput] = accept_fd;
	if ( ++iput == MAX_CLIENT )
	  iput = 0;
	if (iput == iget) {
		array full;
		exit
	}
	pthread_cond_signal(accept_fd_cond);
	pthread_mutex_unlock(accept_fd_mutex);
}


void thread_main()
{
	
	pthread_mutex_lock(accept_fd_mutex);

	while(iput == iget)
		pthread_cond_wait(accept_fd_cond);

	acceptfd = accept_fd[iget];
	if ( iget++ == MAX_CLIENT )
		iget = 0;
	pthread_mutex_unlock(accept_fd_mutex);

	do_it(acceptfd);
}
