/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <arch/stdio.h>

#ifndef FOPEN_MAX
#define FOPEN_MAX 12
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define EOF            -1
#define SEEK_CUR        0
#define SEEK_END        1
#define SEEK_SET        2


#define _IOWRITE        (1 << 0)
#define _IOREAD         (1 << 1)
#define _IORW           (1 << 2)
#define _IOEOF          (1 << 3)
#define _IOERR          (1 << 4)
#define _IOSTRG         (1 << 5)
#define _IOTXT          (1 << 6)
#define _IOFBF          (1 << 7)
#define _IOLBF          (1 << 8)
#define _IONBF          (1 << 9)
#define _IOALLOC        (1 <<10)

typedef struct {
	int fd;        	        /* file descriptor */
	unsigned char *buf;     /* pointer to i/o buffer */
	unsigned char *rp;      /* read pointer */
	unsigned char *wp;      /* write pointer */
	unsigned char *lp;      /* write pointer used when line-buffering */
	size_t len;             /* actual length of buffer */
	unsigned short flags;
	unsigned char unbuf[1];  /* tiny buffer for unbuffered io */
} FILE;

extern FILE __iob[FOPEN_MAX];

#define	stdin	(&__iob[0])
#define	stdout	(&__iob[1])
#define	stderr	(&__iob[2])

extern int remove(const char *filename);
extern int rename(const char *old, const char *new);
extern FILE *tmpfile(void);
extern char *tmpnam(char *s);
extern int fclose(FILE *fp);
extern int fflush(FILE *fp);
extern FILE *fopen(const char * restrict fname, const char * restrict mode);
extern FILE *freopen(const char * restrict fname, const char * restrict mode,
                     FILE * restrict fp);
extern void setbuf(FILE * restrict fp, char * restrict buf);
extern int setvbuf(FILE * restrict fp,
                   char * restrict buf, int mode, size_t size);
extern int fprintf(FILE * restrict fp, const char * restrict fmt, ...);
extern int fscanf(FILE * restrict fp, const char * restrict fmt, ...);
extern int printf(const char * restrict fmt, ...);
extern int scanf(const char * restrict fmt, ...);
extern int snprintf(char * restrict s,
                    size_t n, const char * restrict fmt, ...);
extern int sprintf(char * restrict s, const char * restrict fmt, ...);
extern int sscanf(const char * restrict s, const char * restrict fmt, ...);

#ifdef _STDARG_H
extern int vfprintf(FILE * restrict fp,
                    const char * restrict fmt, va_list arg);
extern int vfscanf(FILE * restrict fp,
                   const char * restrict fmt, va_list arg);
extern int vprintf(const char * restrict fmt, va_list arg);
extern int vscanf(const char * restrict fmt, va_list arg);
extern int vsnprintf(char * restrict s, size_t n, const char * restrict fmt,
                     va_list arg);
extern int vsprintf(char * restrict s,
                    const char * restrict fmt, va_list arg);
extern int vsscanf(const char * restrict s,
                   const char * restrict fmt, va_list arg);
#endif

extern int fgetc(FILE *fp);
extern char *fgets(char * restrict s, int n, FILE * restrict fp);
extern int fputc(int c, FILE *fp);
extern int fputs(const char * restrict s, FILE * restrict fp);
extern int getc(FILE *fp);
extern int getchar(void);
extern char *gets(char *s);
extern int putc(int c, FILE *fp);
extern int putchar(int c);
extern int puts(const char *s);
extern int ungetc(int c, FILE *fp);
extern size_t fread(void * restrict ptr, size_t size, size_t nmemb,
             FILE * restrict fp);
extern size_t fwrite(const void * restrict ptr, size_t size, size_t nmemb,
              FILE * restrict fp);
extern int fgetpos(FILE * restrict fp, fpos_t * restrict pos);
extern int fseek(FILE *fp, long int offset, int whence);
extern int fsetpos(FILE *fp, const fpos_t *pos);
extern long int ftell(FILE *fp);
extern void rewind(FILE *fp);
extern void clearerr(FILE *fp);
extern int feof(FILE *fp);
extern int ferror(FILE *fp);
extern void perror(const char *s);

extern int __getc(FILE *fp);
extern int __putc(int, FILE *fp);

#ifdef __USE_MACROS
#ifdef __UNIX_FILES
#define getc(fp)     ((fp)->rp >= (fp)->wp ?  __getc(fp) : *(fp)->rp++)
#define putc(c, fp)  ((fp)->wp >= (fp)->rp ? __putc(c,fp) : (*(fp)->wp++ = c))
#endif

#define ferror(fp)          ((fp)->flags & _IOERR)
#define feof(fp)            ((fp)->flags & _IOEOF)
#define clearerr(fp)        (void) ((fp)->flags &= ~(_IOERR|_IOEOF))
#define getchar()           getc(stdin)
#define putchar(c)          putc((c), stdout)
#define setbuf(fp, b)       (void) setvbuf(fp, b, b ? _IOFBF:_IONBF, BUFSIZ)
#endif

#endif
