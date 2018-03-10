/*
 * main.c
 * Stub code to turn pcapknock into a program
 * By J. Stuart McMurray
 * Created 20180308
 * Last Modified 20180310
 */

#include <pthread.h>

#include "pcapknock.h"

int
main(int argc, char **argv) {
        int ret;
        /* Fire off the listener */
        if (0 != (ret = pcapknock()))
                return ret;
        /* Main's done, keep listener going */
        pthread_exit(NULL);
}
