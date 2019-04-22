/*
 * spawn.h
 * Spawn callbacks and commands
 * By J. Stuart McMurray
 * Created 20190322
 * Last Modified 20190322
 */

#ifndef HAVE_SPAWN_H
#define HAVE_SPAWN_H

/* handle_cb double-forks and calls back addr with a shell */
void handle_cb(char *addr);

/* handle_cmd double-forks and executes cmd with /bin/sh -c */
void handle_cmd(const char *cmd);

#endif /* #ifndef HAVE_SPAWN_H */
