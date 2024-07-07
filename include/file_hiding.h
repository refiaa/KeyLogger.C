#ifndef FILE_HIDING_H
#define FILE_HIDING_H

int hide_file_attribute(const char* filepath);
int unhide_file_attribute(const char* filepath);
int hide_file_ads(const char* filepath, const char* content);
int read_hidden_file_ads(const char* filepath, char** content);

#endif // FILE_HIDING_H