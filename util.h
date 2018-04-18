// based on cs3650 starter code

#ifndef UTIL_H
#define UTIL_H

int streq(const char* aa, const char* bb);
int min(int x, int y);
int max(int x, int y);
int bytes_to_pages(int bytes);
void join_to_path(char* buf, char* item);
void get_filename(char* buf, const char* path);
void get_parent_path(char* buf, const char* path);

#endif
