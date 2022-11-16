#include <stdlib.h>

#include "memory-stream.h"

#ifndef _WIN32

bool
memory_stream_open(struct memory_stream *m)
{
	*m = (struct memory_stream){ 0 };
	m->fp = open_memstream(&m->str, &m->str_len);

	return m->fp != NULL;
}

char *
memory_stream_close(struct memory_stream *m)
{
	char *str;
	int ret;

	ret = fclose(m->fp);
	str = m->str;
	*m = (struct memory_stream){ 0 };

	if (ret != 0) {
		free(str);
		str = NULL;
	}

	return str;
}

#else

bool
memory_stream_open(struct memory_stream *m)
{
	*m = (struct memory_stream){ 0 };
	m->fp = tmpfile();

	return m->fp != NULL;
}

char *
memory_stream_close(struct memory_stream *m)
{
	size_t size;
	char* str;
	FILE *fp;

	fp = m->fp;
	*m = (struct memory_stream){ 0 };

	if (fp == NULL) {
		return NULL;
	}

	_fseeki64(fp, 0ll, SEEK_END);
	size = (size_t)_ftelli64(fp);
	_fseeki64(fp, 0ll, SEEK_SET);

	str  = malloc(size + 1);
	if (str == NULL) {
		fclose(fp);
		return NULL;
	}

	fread(str, 1, size, fp);
	str[size] = '\0';
	fclose(fp);

	return str;
}

#endif
