/* copyright (c) 2014 Matilda Helou <sadieperkins@riseup.net> */

/* the functions in this file handle the whole "sending data out" thing */

#include "net.h"




int main(int argc, char* argv[])
{
    int data_listener, controlfd, control_listener, listening, datafd = -1;
    ssize_t rval, numbytes, rval_send;
    uint8_t *buf;
    size_t bufsize;
    uint8_t cont = CONT;
    uint8_t done = DONE;

    if ((argc != 4) || (argc != 5)) {
        fprintf(stderr, "usage: %s chunksize dataport controlport [host]\nchunksize is in bytes\n", argv[0]);
        exit(10);
    }

    if (argc == 4) {
        listening = 1;
    } else {
        listening = 0;
    }


    bufsize = (size_t) atol(argv[1]);
    buf = malloc(bufsize);
    assert(buf != NULL);

    control_listener = make_listener(argv[3]);
    controlfd = accept(control_listener, NULL, NULL);
    if (controlfd == -1) {
        perror("accept on control port");
        exit(1);
    }

    data_listener = make_listener(argv[2]);
    while (1) {
        numbytes = read_all(0, buf, bufsize);
        if (numbytes == -1) {
            perror("read on stdin");
            exit(1);
        }
        if (numbytes == 0) { /* eof on stdin */
            rval = write(controlfd, &done, 1);
            if (rval != 1) {
                perror("write on controlfd (while exiting, which makes this even more humiliating)");
                exit(1);
            }

            break;
        }
        rval = write(controlfd, &cont, 1);

        if (rval != 1) {
            perror("write on controlfd");
            exit(1);
        }

        datafd = accept(data_listener, NULL, NULL);
        if (datafd == -1) {
            perror("accept on data port");
            exit(1);
        }
        rval_send = write_all(datafd, buf, (size_t) numbytes);

        if (rval_send != numbytes) {
            perror("incomplete write on datafd");
            exit(1);
        }

        rval = close(datafd);
        if (rval != 0) {
            perror("close");
            exit(1);
        }

    }


    free(buf);
    if (datafd != -1) {
        close(datafd);
    }

    close(controlfd);

    return 0;
}

