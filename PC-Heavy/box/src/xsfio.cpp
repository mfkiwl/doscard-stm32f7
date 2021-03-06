/*
 *  Copyright (C) 2013-2014  Dmitry Soloviov
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "dosbox.h"
#include "cross.h"
#include "ldb.h"

namespace dosbox {

#define FIOSTRINGMEMBUF 512

//LDB_CallbackFunc fio_call = NULL;
//#define CHECK_FIO_CALL if (!fio_call) fio_call = myldbi->GetCallback(DBCB_FileIOReq)

DBFILE* dbfopen(const char* p, const char* m)
{
	if ((!m) || (!p)) return NULL;
	DBFILE* r = new DBFILE;
	memset(r,0,sizeof(DBFILE));
	LOG_MSG("dbfopen(): Pretend to open file %s for %s",p,m);
	r->name = reinterpret_cast<char*>(malloc(strlen(p) + 1));
	if (!r->name) return NULL;
	strcpy(r->name,p);
	/* The following code is for embedded design, there's no reason
	 * to use it under real OS */
#ifdef LDB_EMBEDDED
	r->op = LDB_FOP_NEW | LDB_FOP_BEGIN;
	for (int i=0; i<strlen(m); i++) {
		switch (m[i]) {
		case 'w': r->op |= LDB_FOP_WRITE | LDB_FOP_TRUNC; break;
		case 'r': r->op |= LDB_FOP_READ; break;
		case 'a':
			r->op |= LDB_FOP_WRITE;
			r->op &= ~(LDB_FOP_BEGIN);
			break;
		case '+':
			r->op |= LDB_FOP_WRITE | LDB_FOP_READ;
			if (m[0] == 'a') r->op |= LDB_FOP_WRAPP;
			break;
		case 'b': break;
		default:
			LOG_MSG("dbfopen(): Unrecognized mode option %c",m[i]);
		}
	}
#else
	r->op = reinterpret_cast<char*>(malloc(strlen(m) + 1));
	strcpy(r->op,m);
	r->todo = 0;
#endif
	if (!myldbi->Callback(DBCB_FileIOReq,r,sizeof(DBFILE))) return r;
	delete r;
	return NULL;
}

void dbfclose(DBFILE* f)
{
	if (!f) return;
#ifdef LDB_EMBEDDED
	f->op = 0xff;
#else
	f->todo = 1;
	if (f->op) free(f->op);
#endif
	myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE));
	if (f->name) free(f->name);
	delete f;
}

uint32_t dbfread(void* p, uint32_t sz, uint32_t q, DBFILE* f)
{
	if ((!p) || (!f)) return 0;
#ifdef LDB_EMBEDDED
#else
	f->todo = 2;
#endif
	f->p_x = sz;
	f->p_y = q;
	f->buf = p;
	return (myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE)));
}

uint32_t dbfwrite(const void* p, uint32_t sz, uint32_t q, DBFILE* f)
{
	if ((!p) || (!f)) return 0;
#ifdef LDB_EMBEDDED
#else
	f->todo = 3;
#endif
	f->p_x = sz;
	f->p_y = q;
	f->buf = const_cast<void*>(p);
	return (myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE)));
}

int32_t dbfseek(DBFILE* f, uint64_t off, int32_t wh)
{
	if (!f) return 0;
#ifdef LDB_EMBEDDED
#else
	f->todo = 4;
#endif
	f->p_x = wh;
	f->p_y = 8;
	f->buf = &off;
	return (myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE)));
}

uint64_t dbftell(DBFILE* f)
{
	if (!f) return 0;
#ifdef LDB_EMBEDDED
#else
	f->todo = 5;
#endif
	uint64_t r = 0;
	f->p_y = 8;
	f->buf = &r;
	if (!myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE))) return r;
	return 0;
}

int32_t dbfeof(DBFILE* f)
{
	if (!f) return -1;
#ifdef LDB_EMBEDDED
#else
	f->todo = 6;
#endif
	return (myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE)));
}

int32_t dbftruncate(DBFILE* f, int64_t len)
{
	/* ftruncate() uses off_t, which is signed type, so that's weird */
	if ((!f) || (len < 0)) return -1;
#ifdef LDB_EMBEDDED
#else
	f->todo = 7;
#endif
	f->p_y = 8;
	f->buf = &len;
	return (myldbi->Callback(DBCB_FileIOReq,f,sizeof(DBFILE)));
}

int32_t dbfprintf(DBFILE *f, const char *fmt, ...)
{
	if (!f) return -1;
	char buf[FIOSTRINGMEMBUF];
	va_list msg;
	va_start(msg,fmt);
	vsnprintf(buf,FIOSTRINGMEMBUF-1,fmt,msg);
	va_end(msg);
	dbfwrite(buf,strlen(buf),1,f);
	return 0;
}

int32_t dbfngetl(char* buf, int32_t n, DBFILE* f)
{
	if ((!f) || (!buf) || (n < 1)) return -1;
	int cnt = 0;
	char cc;
	while (!dbfeof(f)) {
		if (dbfread(&cc,1,1,f) != 1) return -1;
		//printable chars + TAB
		if ((!cc) || (cc == '\n')) break;
		buf[cnt++] = cc;
		if (cnt >= n) break;
	}
	return cnt;
}

static int32_t dbgetinfo(const char* name, int kind)
{
	DBFILE f;
	if (!name) return -1;
	memset(&f,0,sizeof(f));
	f.name = const_cast<char*> (name);
	f.todo = kind + 10;
	return (myldbi->Callback(DBCB_FileIOReq,&f,sizeof(f)));
}

bool dbisfilex(const char* name)
{
	return (dbgetinfo(name,0) == 0);
}

bool dbisdirex(const char* path)
{
	return (dbgetinfo(path,1) == 0);
}

bool dbisitexist(const char* path)
{
	return (dbisfilex(path) || dbisdirex(path));
}

int32_t dbgetfilesize(const char* path)
{
	return (dbgetinfo(path,2));
}

int32_t dbrename(const char* oldn, const char* newn)
{
#ifndef LDB_EMBEDDED
	//FIXME: impl
#endif
	return -1;
}

DBFILE* dbdiropen(const char* p)
{
#ifdef LDB_SUPPORT_DIRS
	if (!p) return NULL;
	DBFILE* nd = new DBFILE;
	memset(nd,0,sizeof(DBFILE));
	LOG_MSG("dbdiropen(): Pretend to open directory %s",p);
	nd->name = reinterpret_cast<char*>(malloc(strlen(p) + 1));
	if (!nd->name) return NULL;
	strcpy(nd->name,p);
	nd->todo = 20;
	if (!myldbi->Callback(DBCB_FileIOReq,nd,sizeof(DBFILE))) return nd;
	delete nd;
#endif
	return NULL;
}

bool dbdirread(DBFILE* d, char* entry, bool& is_dir)
{
	if ((!d) || (!entry)) return false;
#ifdef LDB_SUPPORT_DIRS
	d->todo = 21;
	d->buf = entry;
	d->p_x = 0;
	d->p_y = CROSS_LEN;
	if (!myldbi->Callback(DBCB_FileIOReq,d,sizeof(DBFILE))) {
		is_dir = (d->p_x > 0);
		return true;
	}
#endif
	return false;
}

void dbdirclose(DBFILE* d)
{
#ifdef LDB_SUPPORT_DIRS
	if (d) return;
	d->todo = 22;
	myldbi->Callback(DBCB_FileIOReq,d,sizeof(DBFILE));
	if (d->name) free(d->name);
#endif
}

int32_t dbdirmake(const char* n)
{
	//return valid value if dir is already exists
#ifdef LDB_SUPPORT_DIRS
	//FIXME: impl
	LOG_MSG("dbdirmake(): STUB");
#endif
	return -1;
}

int32_t dbdirdel(const char* n)
{
#ifdef LDB_SUPPORT_DIRS
	//FIXME: impl
	LOG_MSG("dbdirdel(): STUB");
#endif
	return -1;
}

} //namespace dosbox
