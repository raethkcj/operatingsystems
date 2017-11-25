#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

#define BUFF_LENGTH 4096

int main(int argc, char *argv[]) {
	// Read max prime as argument
	if(argc != 4) {
		printf("Usage: client <hostname> <port> <max_prime>\n");
		exit(1);
	}
	char *hostname = argv[1];
	int port = atoi(argv[2]);
	int max = atoi(argv[3]);

	// Resolve hostname
	struct hostent *host = gethostbyname(hostname);
	
	// Attempt to create socket
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		perror("Error creating socket");	
		exit(1);
	}

	// Setup sock addr
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind serv addr to socket
	int err = bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if(err < 0) {
		perror("Error on bind");
		exit(1);
	}

	// Listen
	err = listen(sock, 5);
	if(err < 0) {
		perror("Error on listen");
		exit(1);
	}

	// Write the odds initially
	if(!fork()) {
		// Attempt to create socket
		int odd_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(odd_sock < 0) {
			perror("Error creating socket");	
			exit(1);
		}

		// Setup sock addr
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr = *(struct in_addr *) host->h_addr;

		// Attempt to connect
		int err = connect(odd_sock, (struct sockaddr *)&addr, sizeof(addr));
		if(err < 0) {
			perror("Error on connect");
			exit(1);
		}

		// Send max
		write(odd_sock, (void *)&max, sizeof(max));

		// Write odds starting at 3
		int buffer[BUFF_LENGTH];
		int current = 3;
		int i;
		while(current < max) {
			// Fill the buffer
			for(i = 0; i < BUFF_LENGTH && current < max; i++) {
				buffer[i] = current;
				current += 2;	
			}
			// Terminate the list if we've reached the max
			if(current >= max) {
				buffer[i] = -1;
				printf("Sent: 3,5,7,...,%d,%d,%d\nTo: %s\n\n\n", buffer[i-3], buffer[i-2], buffer[i-1], hostname);
			}
			// Send the buffer
			int total = 0;
			int bytesleft = BUFF_LENGTH * sizeof(int);
			int n;
			while(total < BUFF_LENGTH) {
				n = write(odd_sock, (void *)(buffer + total), bytesleft);
				if(n == -1) perror("Error sending ");
				total += n;
				bytesleft -= n;
			}
		}

		// Close the socket
		close(odd_sock);
	}

	while(1) {
		// Accept connection
		struct sockaddr_in conn_addr;
		socklen_t len = sizeof(conn_addr);
		int in_socket = accept(sock, (struct sockaddr *)&conn_addr, &len);
		if(in_socket < 0) {
			perror("Error on accept");
			exit(1);
		}
		
		if(!fork()) {
			// Setup out_socket
			int out_socket = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr = *(struct in_addr *) host->h_addr;

			// Attempt to connect
			if(connect(out_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
				perror("Out sock error");
			}

			// Read from in_socket, sieve, and write to out_socket
			int in_buffer[BUFF_LENGTH];
			int out_buffer[BUFF_LENGTH];
			int first = -1;
			int notDone = 1;
			char *tmp_rcv_message;
			char *tmp_snd_message;
			char *rcv_message;
			char *snd_message;
			int wrote_sent = 0;
			while(notDone) {
				// Fill the in_buffer up to BUFF_LENGTH
				int total = 0;
				int bytesLeft = BUFF_LENGTH * sizeof(int);
				int n;
				while(total < BUFF_LENGTH) {
					n = read(in_socket, (void *)in_buffer + total, bytesLeft);
					if(n < 0) continue;
					total += n;
					bytesLeft -= n;
				}
				int i = 0;
				int j = 0;
				for(i = 0; i < BUFF_LENGTH; i++) {
					// Set number we'll be sieving
					if(first == -1) {
						first = in_buffer[0];
						asprintf(&tmp_rcv_message, "Recvd: %d,%d,%d,...,", in_buffer[0], in_buffer[1], in_buffer[2]);
						continue;
					}
					// End of this chunk
					if(in_buffer[i] == -2) {
						break;
					}
					// Stop if we've reached the end
					if(in_buffer[i] == -1) {
						asprintf(&rcv_message, "%s%d,%d,%d\n", tmp_rcv_message, in_buffer[i-3], in_buffer[i-2], in_buffer[i-1]);
						free(tmp_rcv_message);
						notDone = 0;
						break;
					}
					// Sieve
					if(in_buffer[i] % first != 0) {
						out_buffer[j] = in_buffer[i];
						j++;
					}
				}
				// Terminate the out_buffer
				out_buffer[j] = -2;
				if(wrote_sent == 0) {
					asprintf(&tmp_snd_message, "Sent: %d,%d,%d,...,", out_buffer[0], out_buffer[1], out_buffer[2]);
					wrote_sent = 1;
				}
				if(!notDone) {
					asprintf(&snd_message, "%s%d,%d,%d\n", tmp_snd_message, out_buffer[j-3], out_buffer[j-2], out_buffer[j-1]);
					free(tmp_snd_message);
					printf(rcv_message);
					free(rcv_message);
					if(first*first > max) {
						close(in_socket);
						close(out_socket);
						close(sock);
						free(snd_message);
						exit(0);
					}
					printf(snd_message);
					free(snd_message);
					printf("To: %s\n\n\n", hostname);
					out_buffer[j] = -1;
				}
				// Send the buffer
				total = 0;
				int bytesleft = BUFF_LENGTH * sizeof(int);
				while(total < BUFF_LENGTH) {
					n = write(out_socket, (void *)(out_buffer + total), bytesleft);
					if(n < 0) continue;
					total += n;
					bytesleft -= n;
				}
			}

			// Close the child's sockets
			close(in_socket);
			close(out_socket);
		}
	}

	// Close server socket
	close(sock);
}
