#include <stdio.h>
#include <string.h>

const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 
int base64_encode(const unsigned char *data, size_t input_length, char *encoded_data) {
    int output_length = 4 * ((input_length + 2) / 3);
    
    if (encoded_data == NULL)
		return 0;
	int i;
	int j;
	
    for (i = 0, j = 0; i < input_length;) {
        unsigned int octet_a = i < input_length ? data[i++] : 0;
        unsigned int octet_b = i < input_length ? data[i++] : 0;
        unsigned int octet_c = i < input_length ? data[i++] : 0;
 
        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
 
        encoded_data[j++] = base64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = base64_table[(triple >> 0 * 6) & 0x3F];
    }
 
    for (i = 0; i < (3 - input_length % 3) % 3; i++) {
        encoded_data[output_length - 1 - i] = '=';
    }
 
    encoded_data[output_length] = '\0';
    return output_length;
}

int base64_decode_char(char c) 
{
	// strchr(base64_table, c) - base64_table;
	
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return 0;
    
    return -1; // Invalid character
}
 
int base64_decode(const char *base64_string, unsigned char *decoded_data) 
{
	int input_length = strlen(base64_string);
	int output_length;
	
    if (input_length % 4 != 0)
		return 0; // Invalid input length
 
    output_length = input_length / 4 * 3;
    if (base64_string[input_length - 1] == '=') (output_length)--;
    if (base64_string[input_length - 2] == '=') (output_length)--;
	int i;
	int j;
	
    for (i = 0, j = 0; i < input_length;) {
        unsigned int sextet_a = base64_decode_char(base64_string[i++]);
        unsigned int sextet_b = base64_decode_char(base64_string[i++]);
        unsigned int sextet_c = base64_decode_char(base64_string[i++]);
        unsigned int sextet_d = base64_decode_char(base64_string[i++]);
 
        if (sextet_a == -1 || sextet_b == -1 || sextet_c == -1 || sextet_d == -1) {
            return 0; // Invalid character
        }
 
        unsigned int triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);
 
        if (j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
 
    return output_length;
}

