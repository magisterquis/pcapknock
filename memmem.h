/*
 * memmem.h
 * Stolen memmem function
 * By J. Stuart McMurray
 * Created 20190324
 * Last Modified 20190324
 */

#ifndef HAVE_MEMMEM_H
#define HAVE_MEMMEM_H


void *bsd_memmem(const void *l, size_t l_len, const void *s, size_t s_len);

#endif /* #ifndef HAVE_MEMMEM_H */
