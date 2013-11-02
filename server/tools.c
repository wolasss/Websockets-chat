#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <regex.h>
#include <fcntl.h>
#include "tools.h"

#define DEBUG 1
#define FRAME_BUFFER_SIZE 1024
#define MAX_ERROR_MSG 0x1000
#define MASK_SIZE 4


char* timestamp()
{
    time_t ltime; /* calendar time */
    ltime=time(NULL); /* get current cal time */
    return asctime( localtime(&ltime) );
}

char* base64_encode(const char*data, size_t data_len, char *out, size_t out_len)
{
    FILE *fp;
    int len;

    int fd_src = open("./.base64_src", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd_src < 0) {
        perror("open");
        return NULL;
    }

    if (write(fd_src, data, data_len) != data_len) {
        perror("write");
        return NULL;
    }

    close(fd_src);

    if ( (fp = popen("base64 ./.base64_src", "r")) == NULL) {
        perror("popen");
        return NULL;
    }

    len = 0;
    while (fgets(out + len, out_len - len, fp) != NULL) {
        len = strlen(out);
        out[--len] = '\0';
    }

    if (out[len - 1] == '\n')
        out[len - 1] = '\0';

    pclose(fp);
    system("rm ./.base64_src");
    return out;
}


int compile_regex (regex_t * r, const char * regex_text)
{
    int i_status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (i_status != 0) {
    char error_message[MAX_ERROR_MSG];
    regerror (i_status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

int match_regex (regex_t * r, const char * to_match)
{
    /* "P" is a pointer into the string which points to the end of the
       previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 10;
    /* "M" contains the matches found. */
    regmatch_t m[n_matches];

    while (1) {
        register unsigned int i = 0;
        int nomatch = regexec (r, p, n_matches, m, 0);
        if (nomatch) {
            printf ("No more matches.\n");
            return nomatch;
        }
        for (i = 0; i < n_matches; i++) {
            int start;
            int finish;
            if (m[i].rm_so == -1) {
                break;
            }
            start = m[i].rm_so + (p - to_match);
            finish = m[i].rm_eo + (p - to_match);
            if (i == 0) {
                printf ("$& is ");
            }
            else {
                printf ("$%d is ", i);
            }
            printf ("'%.*s' (bytes %d:%d)\n", (finish - start),
                    to_match + start, start, finish);
        }
        p += m[0].rm_eo;
    }
    return 0;
}