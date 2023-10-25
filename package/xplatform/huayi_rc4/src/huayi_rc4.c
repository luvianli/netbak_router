#include <stdio.h>
#include "huayi_atcmd.h"
#include "huayi_uci_fun.h"

void rc4_init(unsigned char* s, unsigned char* key, unsigned long Len_k) {
    int i, j = 0;
    unsigned char tmp;
    unsigned char k[256] = {0};

    for (i = 0; i < 256; i++) {
        s[i] = (unsigned char)i;
        k[i] = key[i % Len_k];
    }

    for (i = 0; i < 256; i++) {
        j = (j + s[i] + k[i]) % 256;
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

void rc4_crypt(unsigned char* Data, unsigned long Len_D, unsigned char* key, unsigned long Len_k) {
    unsigned char s[256];
    rc4_init(s, key, Len_k);
    int i = 0, j = 0, t = 0;
    unsigned long k = 0;
    unsigned char tmp;

    for (k = 0; k < Len_D; k++) {
        i = (i + 1) % 256;
        j = (j + s[i]) % 256;
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        t = (s[i] + s[j]) % 256;
        Data[k] = Data[k] ^ s[t];
    }
}

void rc4_decrypt(unsigned char* Data, unsigned long Len_D, unsigned char* key, unsigned long Len_k) {
    rc4_crypt(Data, Len_D, key, Len_k);
}


static const char base64char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_decode(const char *base64, unsigned char *bindata) {
    int i, j;
    unsigned char k;
    unsigned char temp[4];

    for (i = 0, j = 0; base64[i] != '\0'; i += 4) {
        memset(temp, 0xFF, sizeof(temp));
        for (k = 0; k < 64; k++) {
            if (base64char[k] == base64[i])
                temp[0] = k;
            if (base64char[k] == base64[i + 1])
                temp[1] = k;
            if (base64char[k] == base64[i + 2])
                temp[2] = k;
            if (base64char[k] == base64[i + 3])
                temp[3] = k;
        }

        bindata[j++] = (temp[0] << 2) | (temp[1] >> 4);
        if (base64[i + 2] == '=')
            break;

        bindata[j++] = (temp[1] << 4) | (temp[2] >> 2);
        if (base64[i + 3] == '=')
            break;

        bindata[j++] = (temp[2] << 6) | temp[3];
    }
    return j;
}

int base64_encode(const unsigned char *bindata, char *base64, int binlength) {
    if (bindata == NULL || base64 == NULL || binlength <= 0) {
        return 0; // 返回0表示失败
    }
    int i, j;
    unsigned char current;

    for (i = 0, j = 0; i < binlength; i += 3) {
        current = (bindata[i] >> 2);
        current &= 0x3F;
        base64[j++] = base64char[(int)current];

        current = ((unsigned char)(bindata[i] << 4)) & 0x30;
        if (i + 1 >= binlength) {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
        } else {
            current |= (bindata[i + 1] >> 4) & 0x0F;
            base64[j++] = base64char[(int)current];

            current = ((unsigned char)(bindata[i + 1] << 2)) & 0x3C;
            if (i + 2 >= binlength) {
                base64[j++] = base64char[(int)current];
                base64[j++] = '=';
            } else {
                current |= (bindata[i + 2] >> 6) & 0x03;
                base64[j++] = base64char[(int)current];

                current = bindata[i + 2] & 0x3F;
                base64[j++] = base64char[(int)current];
            }
        }
    }
    base64[j] = '\0';
    return 1;
}


int main() {
    // 字符串密钥
    int i;
    char encoded_data[100],encoded_key[100];
    unsigned char key[] = "zstuctf";
    unsigned long key_len = sizeof(key) - 1;
    char lte_value[TEMP_STR_LEN] = {0},cmd[CMD_STR_LEN] = {0};

    /*

    // 加解密数据
    unsigned char data[] = {0x7E, 0x6D, 0x55, 0xC0, 0x0C, 0xF0, 0xB4, 0xC7, 0xDC, 0x45,
                            0xCE, 0x15, 0xD1, 0xB5, 0x1E, 0x11, 0x14, 0xDF, 0x6E, 0x95,
                            0x77, 0x9A, 0x12, 0x99};
    unsigned long data_len = sizeof(data);

*/
againt:
	while ( LTE_TRUE != atcmd_check() )
	{
		sleep(1);
	}
    atcmd_get_EC20_imei(lte_value);
	if(strlen(lte_value) < 5)
	{
		goto againt;
	}
    //char lte_value[] = "YourLTEValueHere";  // Replace with your actual LTE value
    long lte_len = sizeof(lte_value) - 1;
    rc4_crypt((unsigned char*)lte_value, lte_len, (unsigned char*)key, key_len);
    if (base64_encode((unsigned char*)lte_value, encoded_data, lte_len)) {
        CON_LOG("Encrypted data: %s\n", encoded_data);
    }
    if (base64_encode(key, encoded_key, key_len)) {
        CON_LOG("Encryption key: %s；Encrypted data: %s\n", encoded_key,encoded_data);
    }
    snprintf(cmd,strlen(cmd),"AT+RZY=\"%s\",\"%s\"",encoded_key,encoded_data);
    while(LTE_TRUE !=atcmd_get_rzy()){
       AtcmdSendDelay(cmd,strlen(cmd),0);
       sleep(40);
    }
    
/*
    // 加密
    rc4_crypt(data, data_len, key, key_len);

    // 编码
    if (base64_encode(data, encoded, data_len)) {
        printf("Encoded String: %s\n", encoded);
        CON_LOG("Encoded String: %s\n", encoded);
    }

    // 解码
    unsigned char decoded_data[100]; // 创建一个用于存储解码后数据的缓冲区
    int decoded_len = base64_decode(encoded, decoded_data);

    // 解密
    rc4_decrypt(decoded_data, decoded_len, key, key_len);

    // 打印解密后的数据
    for (i = 0; i < decoded_len; i++) {
        printf("%02X ", decoded_data[i]);
    }
    printf("\n");
    */

    return 0;
}

