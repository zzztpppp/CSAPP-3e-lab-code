#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Helper functions */
int process_request(int connfd, char *request_for, char *servername, char *portname);
void contruct_request(rio_t *rp, char *request_for, char *method, char *uri, char *hostname);
int process_response(int connfd, char *response_for);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    char clientname[MAXLINE], clientport[MAXLINE], servername[MAXLINE], serverport[MAXLINE];
    struct sockaddr_storage client_addr;
    char response_for[MAXLINE] = {0};
    char request_for[MAXLINE] = {0};

    /* Check command line arguements */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    listenfd = Open_listenfd(argv[1]);

    /* Start sequentail proxy server main routine */
    while (1) {
        clientlen = sizeof(client_addr);
        connfd = Accept(listenfd, (SA *)&client_addr, &clientlen);
        Getnameinfo((SA *) &client_addr, clientlen, clientname, MAXLINE, 
                    clientport, MAXLINE, 0);
        printf("Conected from (%s %s)\n", clientname, clientport);

        /* Read and process request from the client*/
        if (process_request(connfd, request_for, servername, serverport) == -1) {
            /* Ignore malformed request */
            Close(connfd);
            continue;
        }

        /* If the request is valid, forward it to the server */
        connfd = Open_clientfd(servername, serverport);
        if (process_response(connfd, response_for) == -1){
            /* Ignore malformed response */
            Close(connfd);
            continue;
        }
        Rio_writen(connfd, response_for, strlen(response_for));
    }
}


/*
 * process_request - Process the comming request 
 * from the socket connection connfd.
 */
int process_request(int connfd, char *request_for, char *servername, char *portname){
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE], uri[MAXLINE];
    rio_t rp; 

    /* Read request lines and headers */
    Rio_readinitb(&rp, connfd);
    if (!Rio_readlineb(&rp, buf, MAXLINE)) {
        return -1;
    }
    printf("%s \n", buf);

    sscanf(buf, "%s %s %s", method, url, version);
    printf("%s %s %s\n", method, url, version);
    printf("%d\n", !strcasecmp(method, "GET"));
    if (strcasecmp(method, "GET")) {
        clienterror(connfd, method, "501", "Not implemented", 
                    "This method is not implemented");
        return -1;
    }
    
    char *ptr;
    /* Parse out host and port */
    if (strncasecmp(url, "http", 4)) {
        clienterror(connfd, url, "400", "Bad request", 
                    "Request cannot be understood");
        return -1;
    }

    /* Ignore "://" in the request body, and parse out host and port */
    ptr = url + 6;
    strcpy(servername, ptr);
    if (!(ptr = index(servername, ':'))) {
        strcpy(portname, "8080");
    }
    else {
        strcpy(portname, ptr);
        *ptr = '\0';    /* Terminate portname string */
    }

    /* Parse URI */
    ptr = index(portname, '/');
    strcpy(uri, ptr);
    *ptr = '\0';
    
    /* Re-construct the client request and add extra proxy headers */
    contruct_request(&rp, request_for, method, uri, servername);
    return 0;
}


/* 
 * construct_request - Decorate clinet request headers and body with 
 *     proxy headers
 */
void contruct_request(rio_t *rp, char *request_for, char *method, char *uri, char *hostname) {

    char buf[MAXLINE];
    int add_host = 1;
    
    /* Construct request lines */
    sprintf(request_for, "%s %s %s\r\n", method, uri, "HTTP/1.0");

    /* Append addional headers from client */
    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
        if (!strncmp("Host", buf, strlen("Host"))) {
            add_host = 0;  /* The Host header is all ready there, no need to add. */
        }
        sprintf(request_for + strlen(request_for), "%s", buf);
    }

    /* Consturct proxy-dependent request headers */
    if (add_host)
        sprintf(request_for + strlen(request_for), "Host: %s\r\n", hostname);
    sprintf(request_for + strlen(request_for), "User-Agent: %s\r\n", user_agent_hdr);
    sprintf(request_for + strlen(request_for), "Connection: close\r\n");
    sprintf(request_for + strlen(request_for), "Proxy-Connection: close\r\n");

    /* End the request */
    sprintf(request_for + strlen(request_for), "\r\n");
}


/*
 * construct_response - Process server response comming from connfd
 *     it simplely does nothing but buffer the respose.
 */
int process_response(int connfd, char *response_for) {
    rio_t rp;
    Rio_readinitb(&rp, connfd);
    char buf[MAXLINE];

    Rio_readlineb(&rp, buf, MAXLINE);
    sprintf(response_for, "%s", buf);
    while (Rio_readlineb(&rp, buf, MAXLINE) > 0)
    {
        sprintf(response_for + strlen(response_for),  "%s", buf);
    }

    return 0;
}



/*
 * clienterror - Emit error message to socket client 
 *     at fd.
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}