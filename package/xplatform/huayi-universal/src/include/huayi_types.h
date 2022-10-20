#ifndef __HUAYI_TYPES_H__
#define __HUAYI_TYPES_H__

#if defined(__linux__) || defined(__linux)
#include <linux/types.h>
#else
#error "TODO: porting for new OS!"
#endif

#ifndef __KERNEL__
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif


/* MAC Address definitions */
#define A_MACADDR_SIZE      6
typedef union {
    unsigned char  octets[A_MACADDR_SIZE];
    unsigned short words[A_MACADDR_SIZE/2];
} A_MACADDR;

/*
 * Words (16b) should be aligned correctly on all platforms, so it
 * can be used even on platforms (mips) with strict alignment
 * requirements.
 */
#define A_MACADDR_COPY(from, to)              \
    do {                                      \
        (to)->words[0] = (from)->words[0];    \
        (to)->words[1] = (from)->words[1];    \
        (to)->words[2] = (from)->words[2];    \
    } while (0)

#define A_MACADDR_COMP(m1, m2)                \
    ((((m1)->octets[5] == (m2)->octets[5]) &&   \
      ((m1)->octets[4] == (m2)->octets[4]) &&   \
      ((m1)->octets[3] == (m2)->octets[3]) &&   \
      ((m1)->octets[2] == (m2)->octets[2]) &&   \
      ((m1)->octets[1] == (m2)->octets[1]) &&   \
      ((m1)->octets[0] == (m2)->octets[0])) == 0)

#define A_MACADDR_COMP_ALIGN(m1, m2)                \
    ((((m1)->words[2] == (m2)->words[2]) &&   \
      ((m1)->words[1] == (m2)->words[1]) &&   \
      ((m1)->words[0] == (m2)->words[0])) == 0)

static inline unsigned int A_MACADDR_HASH(A_MACADDR *m, unsigned int max) {
    return (int32_t)(m->octets[3]^m->octets[4]^m->octets[5])%max;
}

#define A_MACADDR_DELI ':'
static inline char* A_MACADDR_TOSTR_18(A_MACADDR *m,
                                       char *s, size_t size) {
    static char h[] = {
        '0','1','2','3','4','5','6','7','8','9',
        'a','b','c','d','e','f'
    };
    if (unlikely(size < 18)) {
        return NULL;
    }
    s[0] = h[m->octets[0] >> 4];
    s[1] = h[m->octets[0] & 0xf];
    s[2] = A_MACADDR_DELI;
    s[3] = h[m->octets[1] >> 4];
    s[4] = h[m->octets[1] & 0xf];
    s[5] = A_MACADDR_DELI;
    s[6] = h[m->octets[2] >> 4];
    s[7] = h[m->octets[2] & 0xf];
    s[8] = A_MACADDR_DELI;
    s[9]  = h[m->octets[3] >> 4];
    s[10] = h[m->octets[3] & 0xf];
    s[11] = A_MACADDR_DELI;
    s[12] = h[m->octets[4] >> 4];
    s[13] = h[m->octets[4] & 0xf];
    s[14] = A_MACADDR_DELI;
    s[15] = h[m->octets[5] >> 4];
    s[16] = h[m->octets[5] & 0xf];
    s[17] = '\0';
    return s;
}

static inline char* A_MACADDR_TOSTR_13(A_MACADDR *m,
                                       char *s, size_t size) {
    static char h[] = {
        '0','1','2','3','4','5','6','7','8','9',
        'a','b','c','d','e','f'
    };
    if (unlikely(size < 13)) {
        return NULL;
    }
    s[0] = h[m->octets[0] >> 4];
    s[1] = h[m->octets[0] & 0xf];
    s[2] = h[m->octets[1] >> 4];
    s[3] = h[m->octets[1] & 0xf];
    s[4] = h[m->octets[2] >> 4];
    s[5] = h[m->octets[2] & 0xf];
    s[6]  = h[m->octets[3] >> 4];
    s[7] = h[m->octets[3] & 0xf];
    s[8] = h[m->octets[4] >> 4];
    s[9] = h[m->octets[4] & 0xf];
    s[10] = h[m->octets[5] >> 4];
    s[11] = h[m->octets[5] & 0xf];
    s[12] = '\0';
    return s;
}

static inline void A_MACADDR_FROMSTR(A_MACADDR *m, const char *str)
{
    unsigned char a,b,c,d,e,f;
    sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &a,&b,&c,&d,&e,&f);
    m->octets[0] = a;
    m->octets[1] = b;
    m->octets[2] = c;
    m->octets[3] = d;
    m->octets[4] = e;
    m->octets[5] = f;
    return;
}

#endif /* __HUAYI_TYPES_H__ */

