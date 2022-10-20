#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//include <tchar.h>
//#include <ctype.h> 

char rfc3986[256] = {0};

unsigned char mint_pass[11];
unsigned short mint_a;
unsigned short mint_b;
unsigned short mint_c;
unsigned short mint_d;
unsigned short mint_e;
unsigned short mint_f;
unsigned short mint_h[5];
unsigned short mint_i;
unsigned short mint_j;
unsigned short mint_k;
unsigned short mint_l;
unsigned short mint_m;

void eCod() 
{
    unsigned short m_tmp;
    mint_d = mint_f + mint_j;
    mint_a = mint_h[mint_j];
    mint_c = 0x015a;
    mint_b = 0x4e35;
    m_tmp = mint_a;
    mint_a = mint_e;
    mint_e = m_tmp;
    m_tmp = mint_a;
    mint_a = mint_d;
    mint_d = m_tmp;
    if (mint_a != 0) {
        mint_a = mint_a * mint_b;
    }
    m_tmp = mint_a;
    mint_a = mint_c;
    mint_c = m_tmp;
    if (mint_a != 0) {
        mint_a = mint_a * mint_e;
        mint_c = mint_a + mint_c;
    }
    m_tmp = mint_a;
    mint_a = mint_e;
    mint_e = m_tmp;
    mint_a = mint_a * mint_b;
    mint_d = mint_c + mint_d;
    mint_a = mint_a + 1;
    mint_f = mint_d;
    mint_h[mint_j] = mint_a;
    mint_i = mint_a ^ mint_d;
    mint_j = mint_j + 1;
}

void calcInt() 
{
    mint_h[0] = (mint_pass[0]*256) + mint_pass[1];
    eCod();
    mint_k = mint_i;
    mint_h[1] = mint_h[0] ^ ((mint_pass[2]*256) + mint_pass[3]);
    eCod();
    mint_k = mint_k ^ mint_i;
    mint_h[2] = mint_h[1] ^ ((mint_pass[4]*256) + mint_pass[5]);
    eCod();
    mint_k = mint_k ^ mint_i;
    mint_h[3] = mint_h[2] ^ ((mint_pass[6]*256) + mint_pass[7]);
    eCod();
    mint_k = mint_k ^ mint_i;
    mint_h[4] = mint_h[3] ^ ((mint_pass[8]*256) + mint_pass[9]);
    ;
    eCod();
    mint_k = mint_k ^ mint_i;
    mint_j = 0;
}

void strt() 
{
    int iCnt;
    for (iCnt = 0; iCnt <= 9; iCnt++) {
        mint_pass[iCnt] = 0;
    }
    strcpy((char*) mint_pass, "inbrascytkey");
    mint_a = 0;
    mint_b = 0;
    mint_c = 0;
    mint_d = 0;
    mint_e = 0;
    mint_f = 0;
    for (iCnt = 0; iCnt < 5; iCnt++) {
        mint_h[iCnt];
    }
    mint_i = 0;
    mint_j = 0;
    mint_k = 0;
    mint_l = 0;
    mint_m = 0;
}

void cryptBuffer(unsigned char strCryptBuffer[1024], unsigned int dataLength, unsigned char strBuffer[1024]) 
{
    unsigned char ch;
    mint_e = 0;
    mint_f = 0;
    mint_j = 0;
    int iCnt = 0;
    int iCnt1 = 0;
    strt();
    calcInt();
    mint_l = mint_k >> 8;
    mint_m = mint_k & 255; /* cfc^cfd = random byte */
    for (iCnt = 0; iCnt < dataLength; iCnt++) {
        ch = strCryptBuffer[iCnt] ^ (mint_l ^ mint_m);
        for (iCnt1 = 0; iCnt1 <= 9; iCnt1++) {
            /* we mix the plaintext byte with the key */
            mint_pass[iCnt1] = mint_pass[iCnt1]^strCryptBuffer[iCnt];
        }
        strBuffer[iCnt] = ch;
    }
}

void encode(unsigned char *s, char *enc, char *tb) 
{
    for (; *s; s++) {
        if (tb[*s])
            sprintf(enc, "%c", tb[*s]);
        else
            sprintf(enc, "%%%02x", *s);

        while (*++enc);
    }
}

int mac2mac (char *s, char *str)
{
	int i;
	for (i=0; i<5; i++) {
		*(s+i*3+2)='-';
	}

	strcpy(str, s);
	return 0;
}

const char b64all[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789" "+/";

int intelbrasCrypt(char *mac, char *encodekey) 
{
    unsigned char coded[1024];
    unsigned char dmn[1024] = {0}; // WAN MAC!!!!
	//sprintf(dmn, "%s", "70-71-BC-19-D0-57");
		
    int lendmn;
	char enmac[18];	
	mac2mac(mac, enmac);
	strcpy(dmn, enmac);
	
    lendmn = strlen(dmn);
    cryptBuffer(dmn, lendmn, dmn);
    int n, nn, count;
    unsigned char c[4];
    count = 0;
    nn = 0;
    int urln = 0;
    unsigned char url[((lendmn / 3) + 1)*4];

    while (nn <= lendmn) {
        c[0] = b64all[((dmn[nn] & 0xFC) >> 2)];
        c[1] = b64all[(dmn[nn] & 0x03) << 4 | (dmn[nn + 1] & 0xF0) >> 4];
        c[2] = b64all[(dmn[nn + 1] & 0x0F) << 2 | (dmn[nn + 2] & 0xC0) >> 6];
        c[3] = b64all[dmn[nn + 2] & 0x3F];
        n = lendmn - nn;
        nn = nn + 3;

        url[urln] = c[0];
        url[urln + 1] = c[1];

        if (n == 1) {
            url[urln + 2] = '=';
            url[urln + 3] = '=';

        } else if (n == 2) {
            url[urln + 2] = c[2];
            url[urln + 3] = '=';

        } else {
            url[urln + 2] = c[2];
            url[urln + 3] = c[3];
        }

        if (count == 18) {
            count = 0;
        } else {
            count++;
        }
        urln = urln + 4;
    }

    url[((lendmn / 3) + 1)*4] = '\0';

    char enc[sizeof (url) * 3];
    int i;
    for (i = 0; i < 256; i++) {
        rfc3986[i] = isalnum(i) || i == '~' || i == '-' || i == '.' || i == '_' ? i : 0;
    }
	
    encode(url, enc, rfc3986);	
	//printf("ENCODE                = %s\n\n\n\n", enc);
	strcpy(encodekey,enc);   
    
    return 0;
}
