#ifndef __UNICHAR_H__
#define __UNICHAR_H__

typedef wchar_t unichar;

/* for declare unicode static string 
 * in unicode supported compiler, use wchar_t and L"string" may save a lot of works
 * any way, you should declare a static unicode string like this:
 * 
 * static UNISTR(5) hello = { 5, { 'h', 'e', 'l', 'l', 'o' } };
 * 
 * comment: alway declare one more byte for objkey
 */
#define UNISTR(_len) struct{int len; unichar unistr[(_len)+1];}

#define unistrlen(str) (*((int*)(((int)(str)) - sizeof(int))))
#define ushort2unistr(p)   (unsigned short*)(((char*)p)+sizeof(int))

unichar* unistrdup(void*, const unichar* str);
unichar* unistrdup_str(void*, const char* str);
int unistrcmp(const unichar* str1, const unichar* str2);
unichar* unistrcat(void*, const unichar* str1, const unichar* str2);
void strcpyuni(unichar* to, const char* from, size_t len);
void unistrcpy(unichar* to, const unichar* from);
void unifree(void*, unichar* d);
int unistrchr(const unichar* str, int c);
char* c_strdup(void*, const char* buf);
void c_strfree(void*, char* buf);
int unistrpos(unichar* str, int start, unichar* nid);
unichar* unisubstrdup(void*, const unichar* a, int start, int len);

/* those two are very dangerous, keep your eyes on */
const unichar* tounichars(void*, const char* str);
const char* tochars(void*, const unichar* str);
void _uniprint(unichar* s);

#endif
