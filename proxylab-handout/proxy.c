#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* Debug control */
// #define DEBUG

#ifdef DEBUG
    #define debug_print(msg) printf("At %d:%s\n", __LINE__, __FILE__);print_msg(msg);
#else
    #define debug_print(msg) ((void)0)
#endif

#define MIN(x, y) ((x) < (y)? (x) : (y))


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

/* Helper functions */
int process_request(int connfd, char *request_for, char *servername, char *portname);
void contruct_request(rio_t *rp, char *request_for, char *method, char *uri, char *hostname);
int process_response(int connfd, char *response_for, int clientfd);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void print_msg(char *msg);
void parse_url(char *url, char *servername, char *portname, char *uri);
void *thread_doit(void *vargp);

static sbuf_t sbuf;

int main(int argc, char **argv)
{
    int listenfd, clientfd;
    struct sockaddr_storage client_addr;
    int n_threads = 4;
    pthread_t tid;
    char clientname[MAXLINE], clientport[MAXLINE];
    socklen_t clientlen;


    /* Check command line arguements */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sbuf_init(&sbuf, MAXLINE);
    for (int i = 0; i < n_threads; i++) {
        Pthread_create(&tid, NULL, thread_doit, NULL);
    }
    
    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(client_addr);
        clientfd = Accept(listenfd, (SA *)&client_addr, &clientlen);
        Getnameinfo((SA *) &client_addr, clientlen, clientname, MAXLINE, 
                    clientport, MAXLINE, 0);
        printf("Conected from (%s %s)\n", clientname, clientport);
        sbuf_insert(&sbuf, clientfd);
    }
}


/*
 * thread_doit - Thread function that handles request and redirect response 
 *     independently
 */
void *thread_doit(void *vargp) {

    Pthread_detach(Pthread_self());

    int serverfd;
    char  servername[MAXLINE], serverport[MAXLINE];
    char response_for[MAXLINE] = {0};
    char request_for[MAXLINE] = {0};


    while (1) {

        int clientfd = sbuf_remove(&sbuf);

        /* Read and process request from the client*/
        if (process_request(clientfd, request_for, servername, serverport) == -1) {
            /* Ignore malformed request */
            Close(clientfd);
            continue;
        }

        debug_print(servername);
        debug_print(serverport);
        /* If the request is valid, forward it to the server */
        serverfd = Open_clientfd(servername, serverport);
        Rio_writen(serverfd, request_for, strlen(request_for));

        printf("Redirect request to %s:%s", servername, serverport);
        if ((process_response(serverfd, response_for, clientfd)) == -1){
            /* Ignore malformed response */
            Close(serverfd);
            continue;
        }
        Close(clientfd);
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

    sscanf(buf, "%s %s %s", method, url, version);
    if (strcasecmp(method, "GET")) {
        clienterror(connfd, method, "501", "Not implemented", 
                    "This method is not implemented");
        return -1;
    }
    
    /* Parse out host and port */
    if (strncasecmp(url, "http", 4)) {
        clienterror(connfd, url, "400", "Bad request", 
                    "Request cannot be understood");
        return -1;
    }

    /* Parse url into elements */
    parse_url(url, servername, portname, uri);
     
    debug_print(servername);
    debug_print(portname);
    debug_print(uri);
    
    /* Re-construct the client request and add extra proxy headers */
    contruct_request(&rp, request_for, method, uri, servername);

    debug_print(request_for);
    return 0;
}

/*
 * parse_url - Parse url into servername, portname and uri
 */
void parse_url(char *url, char *servername, char *portname, char *uri) {
    char *dest, *ptr;
    ptr = url + 7;
    int port_suplied = 0;
    dest = servername;    /* We are reading server name at the begining */
    int j = 0;
    for (int i = 0; i <= strlen(ptr); i++) {
        if (ptr[i] == ':') {
            port_suplied = 1;
            dest[j] = '\0';
            j = 0;
            dest = portname;   /* Begin to read portname */
            continue;
        }
        else if (ptr[i] == '/')
        {
            dest[j] = '\0';
            j = 0;
            dest = uri;    /* Begin to read uri */
        }
        dest[j] = ptr[i];
        j++;
    }
    if (!port_suplied) strcpy(portname, "80");
    return;
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
        if (!strncmp("Host", buf, strlen("Host"))) {
            add_host = 0;  /* The Host header is all ready there, no need to add. */
        }

        /* Ignore proxy dependent headers. */
        if (!strncmp(buf, "User-Agent:", 11)){
            Rio_readlineb(rp, buf, MAXLINE);
            continue;
        }
        if (!strncmp(buf, "Connection:", 11)) {
            Rio_readlineb(rp, buf, MAXLINE);
            continue;
        }

        if (!strncmp(buf, "Proxy-Connection:", 17)) {
            Rio_readlineb(rp, buf, MAXLINE);
            continue;
        }

        sprintf(request_for + strlen(request_for), "%s", buf);
        Rio_readlineb(rp, buf, MAXLINE);
    }

    /* Consturct proxy-dependent request headers */
    if (add_host)
        sprintf(request_for + strlen(request_for), "Host: %s\r\n", hostname);
    sprintf(request_for + strlen(request_for), "User-Agent: %s", user_agent_hdr);
    sprintf(request_for + strlen(request_for), "Connection: close\r\n");
    sprintf(request_for + strlen(request_for), "Proxy-Connection: close\r\n");

    /* End the request */
    sprintf(request_for + strlen(request_for), "\r\n");
}


/*
 * construct_response - Process server response comming from connfd
 *     it simplely does nothing but buffer the respose.
 */
int process_response(int connfd, char *response_for, int clientfd) {
    rio_t rp;
    Rio_readinitb(&rp, connfd);
    char buf[MAXLINE];
    size_t content_length, hdr_length;

    Rio_readlineb(&rp, buf, MAXLINE);

    /* Read response headers */
    sprintf(response_for, "%s", buf);
    while (strcmp(buf, "\r\n"))
    {
        sprintf(response_for + strlen(response_for),  "%s", buf);
        if (!strncmp(buf, "Content-length:", 15)) {
            strcpy(buf, index(buf, ' ') + 1);
            content_length = atoi(buf);
        }
        Rio_readlineb(&rp, buf, MAXLINE);
    }
    sprintf(response_for + strlen(response_for), "%s", "\r\n");
    hdr_length = strlen(response_for);
    printf("Redirecting content with length %ld\n", content_length);
    Rio_writen(clientfd, response_for, hdr_length);

    /* Write contents */
    int nread;
    while (content_length > 0) {
        nread = Rio_readnb(&rp, buf, MIN(content_length, MAXLINE));
        Rio_writen(clientfd, buf, nread);
        content_length -= nread;
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


/***************************** 
 * Helper-functions for debug
 *****************************/
void print_msg(char *msg) {
    printf("%s\n", msg);
    return;
}